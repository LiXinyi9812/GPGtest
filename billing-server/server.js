require('dotenv').config();

const express = require('express');
const { google } = require('googleapis');
const helmet = require('helmet');
const rateLimit = require('express-rate-limit');
const path = require('path');

const app = express();
app.use(express.json());
app.use(helmet());

// 速率限制: 每个 IP 每分钟最多 30 次请求
const limiter = rateLimit({
  windowMs: 60 * 1000,
  max: 30,
  message: { error: 'Too many requests, please try again later.' },
});
app.use('/api/', limiter);

// ============================================================
// Google Play Developer API 初始化
// ============================================================

const PACKAGE_NAME = process.env.PACKAGE_NAME || 'com.mycompany.mygame';
const API_SECRET_KEY = process.env.API_SECRET_KEY || 'your-secret-key-here';
const ALLOWED_PRODUCT_IDS = (process.env.ALLOWED_PRODUCT_IDS || '100_coins')
  .split(',')
  .map((id) => id.trim());

let playDeveloperApi = null;

async function initGoogleApi() {
  const keyFilePath = process.env.GOOGLE_SERVICE_ACCOUNT_KEY
    || './service-account-key.json';

  const auth = new google.auth.GoogleAuth({
    keyFile: path.resolve(__dirname, keyFilePath),
    scopes: ['https://www.googleapis.com/auth/androidpublisher'],
  });

  playDeveloperApi = google.androidpublisher({
    version: 'v3',
    auth: auth,
  });

  console.log('[Billing Server] Google Play API initialized.');
}

// ============================================================
// 简单鉴权中间件
// ============================================================

function authMiddleware(req, res, next) {
  const apiKey = req.headers['x-api-key'];
  if (!apiKey || apiKey !== API_SECRET_KEY) {
    return res.status(401).json({ error: 'Unauthorized: invalid API key.' });
  }
  next();
}

// ============================================================
// 已处理的 purchase token 缓存 (防重复发货)
// 生产环境应使用数据库
// ============================================================

const processedTokens = new Set();

// ============================================================
// API 路由
// ============================================================

// 健康检查
app.get('/api/health', (req, res) => {
  res.json({ status: 'ok', timestamp: new Date().toISOString() });
});

/**
 * POST /api/verify-and-consume
 * Body: { product_id, purchase_token }
 * Headers: x-api-key
 *
 * 验证购买、确认 (acknowledge) 并消耗 (consume)
 * 这是完整的后端购买处理流程
 */
app.post('/api/verify-and-consume', authMiddleware, async (req, res) => {
  const { product_id, purchase_token } = req.body;

  // 参数校验
  if (!product_id || !purchase_token) {
    return res.status(400).json({
      error: 'Missing required fields: product_id, purchase_token',
    });
  }

  // 记录前端传来的购买ID
  console.log(`[REQUEST] 收到购买请求 - 产品ID: ${product_id}, Token: ${purchase_token.substring(0, 30)}...`);
  console.log(`[REQUEST] 完整请求体:`, JSON.stringify(req.body, null, 2));

  // 商品 ID 白名单校验
  if (!ALLOWED_PRODUCT_IDS.includes(product_id)) {
    return res.status(400).json({ error: `Invalid product_id: ${product_id}` });
  }

  // 幂等性校验: 防止重复处理
  if (processedTokens.has(purchase_token)) {
    console.log(`[WARN] Duplicate token received: ${purchase_token.substring(0, 20)}...`);
    return res.json({
      success: true,
      order_id: 'duplicate',
      message: 'Already processed (idempotent).',
    });
  }

  try {
    // 第 1 步: 验证购买
    console.log(`[VERIFY] Checking purchase: product=${product_id}, token=${purchase_token.substring(0, 20)}...`);
    const response = await playDeveloperApi.purchases.products.get({
      packageName: PACKAGE_NAME,
      productId: product_id,
      token: purchase_token,
    });

    const purchaseInfo = response.data;

    // purchaseState: 0=已购买, 1=已取消, 2=处理中
    if (purchaseInfo.purchaseState !== 0) {
      console.log(`[REJECT] Invalid purchaseState: ${purchaseInfo.purchaseState}`);
      return res.status(400).json({
        success: false,
        error: `Purchase state invalid: ${purchaseInfo.purchaseState}`,
      });
    }

    // 第 2 步: 确认购买 (Acknowledge) - 防止自动退款
    if (purchaseInfo.acknowledgementState === 0) {
      console.log(`[ACK] Acknowledging purchase: ${purchaseInfo.orderId}`);
      await playDeveloperApi.purchases.products.acknowledge({
        packageName: PACKAGE_NAME,
        productId: product_id,
        token: purchase_token,
      });
      console.log(`[ACK] ✓ Purchase acknowledged`);
    }

    // 第 3 步: 消耗购买 (Consume)
    console.log(`[CONSUME] Consuming purchase: ${purchaseInfo.orderId}`);
    await playDeveloperApi.purchases.products.consume({
      packageName: PACKAGE_NAME,
      productId: product_id,
      token: purchase_token,
    });
    console.log(`[CONSUME] ✓ Purchase consumed`);

    // 记录已处理的 token
    processedTokens.add(purchase_token);

    console.log(`[SUCCESS] Purchase fully processed: orderId=${purchaseInfo.orderId}, product=${product_id}`);

    // TODO: 在此处执行业务逻辑
    // 1. 将购买记录写入数据库
    // 2. 为用户发放权益 (如加金币)
    // 3. 记录到审计日志

    return res.json({
      success: true,
      order_id: purchaseInfo.orderId,
      purchase_time: purchaseInfo.purchaseTimeMillis,
      message: 'Purchase verified, acknowledged and consumed successfully.',
    });
  } catch (error) {
    console.error(`[ERROR] Purchase processing failed:`, error.message);

    if (error.code === 404) {
      return res.status(404).json({
        success: false,
        error: 'Purchase token not found (invalid or expired).',
      });
    }

    if (error.message && error.message.includes('insufficient permissions')) {
      return res.status(403).json({
        success: false,
        error: 'Server has insufficient permissions to consume purchase. Check Google service account permissions.',
      });
    }

    return res.status(500).json({
      success: false,
      error: 'Internal server error during purchase processing.',
    });
  }
});

// ============================================================
// 启动服务器
// ============================================================

const PORT = parseInt(process.env.PORT, 10) || 3000;

async function start() {
  try {
    await initGoogleApi();
  } catch (err) {
    console.error('[FATAL] Failed to init Google API:', err.message);
    console.error('Make sure service-account-key.json is in place.');
    process.exit(1);
  }

  app.listen(PORT, () => {
    console.log(`[Billing Server] Running on port ${PORT}`);
    console.log(`[Billing Server] Package: ${PACKAGE_NAME}`);
    console.log(`[Billing Server] Allowed products: ${ALLOWED_PRODUCT_IDS.join(', ')}`);
  });
}

start();

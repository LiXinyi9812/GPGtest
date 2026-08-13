require('dotenv').config();

const express = require('express');
const { google } = require('googleapis');
const helmet = require('helmet');
const rateLimit = require('express-rate-limit');
const path = require('path');
const Database = require('better-sqlite3');

const crypto = require('crypto');

const app = express();
// 保存原始 body 以便后续计算 hash (与客户端一致)
app.use(express.json({
  verify: (req, _res, buf) => {
    req.rawBody = buf;
  },
}));
app.use(helmet());

// 速率限制: 每个 IP 每分钟最多 30 次请求
const limiter = rateLimit({
  windowMs: 60 * 1000,
  max: 30,
  message: { error: 'Too many requests, please try again later.' },
});
app.use('/api/', limiter);

// ============================================================
// SQLite 数据库初始化 - 记录金币余额
// ============================================================

const db = new Database(path.join(__dirname, 'coins.db'));
db.pragma('journal_mode = DELETE');

// 创建金币表 (单用户简化版，仅一行记录)
db.exec(`
  CREATE TABLE IF NOT EXISTS wallet (
    id INTEGER PRIMARY KEY CHECK (id = 1),
    coins INTEGER NOT NULL DEFAULT 0,
    updated_at TEXT NOT NULL DEFAULT (datetime('now'))
  )
`);

// 创建分数表 (单用户简化版，仅一行记录)
db.exec(`
  CREATE TABLE IF NOT EXISTS score (
    id INTEGER PRIMARY KEY CHECK (id = 1),
    score INTEGER NOT NULL DEFAULT 0,
    updated_at TEXT NOT NULL DEFAULT (datetime('now'))
  )
`);

// 确保有一行初始数据
const initRow = db.prepare('INSERT OR IGNORE INTO wallet (id, coins) VALUES (1, 0)');
initRow.run();
const initScore = db.prepare('INSERT OR IGNORE INTO score (id, score) VALUES (1, 0)');
initScore.run();

// 预编译常用语句
const getCoins = db.prepare('SELECT coins FROM wallet WHERE id = 1');
const addCoins = db.prepare(`
  UPDATE wallet SET coins = coins + ?, updated_at = datetime('now') WHERE id = 1
`);
const getScore = db.prepare('SELECT score FROM score WHERE id = 1');
const addScore = db.prepare(`
  UPDATE score SET score = score + ?, updated_at = datetime('now') WHERE id = 1
`);

// ============================================================
// Google Play Developer API 初始化
// ============================================================

const PACKAGE_NAME = process.env.PACKAGE_NAME || 'com.mycompany.mygame';
const API_SECRET_KEY = process.env.API_SECRET_KEY || 'your-secret-key-here';
const CLOUD_PROJECT_NUMBER = process.env.CLOUD_PROJECT_NUMBER || '';
const ALLOWED_PRODUCT_IDS = (process.env.ALLOWED_PRODUCT_IDS || '100_coins')
  .split(',')
  .map((id) => id.trim());

let playDeveloperApi = null;
let integrityAuthClient = null;

async function initGoogleApi() {
  const keyFilePath = process.env.GOOGLE_SERVICE_ACCOUNT_KEY
    || './service-account-key.json';

  const auth = new google.auth.GoogleAuth({
    keyFile: path.resolve(__dirname, keyFilePath),
    scopes: [
      'https://www.googleapis.com/auth/androidpublisher',
      'https://www.googleapis.com/auth/playintegrity',
    ],
  });

  playDeveloperApi = google.androidpublisher({
    version: 'v3',
    auth: auth,
  });

  // 保存 auth client，用于直接调用 Play Integrity REST API
  integrityAuthClient = await auth.getClient();

  console.log('[Billing Server] Google Play API initialized.');
  console.log('[Billing Server] Play Integrity API initialized (REST mode for PC).');
}

// ============================================================
// Play Integrity 验证 (PC)
// ============================================================

/**
 * 解密并验证 Play Integrity 令牌
 * @param {string} integrityToken - 客户端获取的完整性令牌
 * @param {string} expectedRequestHash - 期望的 request_hash (客户端请求体的 SHA256)
 * @returns {{ valid: boolean, error?: string, verdict?: object }}
 */
async function verifyIntegrityToken(integrityToken, expectedRequestHash) {
  if (!integrityToken) {
    return { valid: false, error: 'Missing integrity token' };
  }

  try {
    // 直接调用 REST API: playintegrity.googleapis.com/v1/{packageName}:decodePcIntegrityToken
    const url = `https://playintegrity.googleapis.com/v1/${PACKAGE_NAME}:decodePcIntegrityToken`;
    const response = await integrityAuthClient.request({
      url,
      method: 'POST',
      data: { integrity_token: integrityToken },
    });

    const rawResponse = response.data;
    console.log('[INTEGRITY] Decoded verdict:', JSON.stringify(rawResponse, null, 2));

    // PC 端返回结构: { tokenPayloadExternal: { requestDetails, deviceIntegrity, accountDetails } }
    const verdict = rawResponse.tokenPayloadExternal || rawResponse;

    // 1) 验证 requestPackageName
    const requestDetails = verdict.requestDetails || {};
    if (requestDetails.requestPackageName !== PACKAGE_NAME) {
      return {
        valid: false,
        error: `Package name mismatch: ${requestDetails.requestPackageName}`,
        verdict,
      };
    }

    // 2) 验证 requestHash (防止请求被篡改)
    if (expectedRequestHash && requestDetails.requestHash !== expectedRequestHash) {
      return {
        valid: false,
        error: 'Request hash mismatch - potential tampering detected',
        verdict,
      };
    }

    // 3) 验证设备完整性 (PC 环境)
    const deviceIntegrity = verdict.deviceIntegrity || {};
    const deviceVerdict = deviceIntegrity.deviceRecognitionVerdict || [];
    if (!deviceVerdict.includes('MEETS_PC_INTEGRITY')) {
      return {
        valid: false,
        error: `Device integrity check failed: [${deviceVerdict.join(', ')}]`,
        verdict,
      };
    }

    // 4) 验证应用许可证 (暂时跳过，测试阶段可能返回 UNLICENSED)
    // const accountDetails = verdict.accountDetails || {};
    // if (accountDetails.appLicensingVerdict === 'UNLICENSED') {
    //   console.log('[INTEGRITY] WARNING: App is UNLICENSED (sideloaded).');
    //   return {
    //     valid: false,
    //     error: 'App is not licensed through Google Play',
    //     verdict,
    //   };
    // }

    return { valid: true, verdict };
  } catch (error) {
    console.error('[INTEGRITY] Token verification failed:', error.message);
    return { valid: false, error: `Integrity API error: ${error.message}` };
  }
}

/**
 * Express 中间件: 对敏感接口强制执行 Play Integrity 校验
 * 客户端需在 Header 中传入:
 *   x-integrity-token: <token>
 *   x-request-hash: <sha256 of request body>
 */
function integrityMiddleware(req, res, next) {
  // 如果未配置 Play Integrity (开发环境)，跳过
  if (!integrityAuthClient || process.env.SKIP_INTEGRITY === 'true') {
    return next();
  }

  const integrityToken = req.headers['x-integrity-token'];
  if (!integrityToken) {
    return res.status(403).json({
      error: 'Integrity token required. Request rejected.',
    });
  }

  const expectedHash = req.headers['x-request-hash'] || '';

  verifyIntegrityToken(integrityToken, expectedHash)
    .then((result) => {
      if (!result.valid) {
        console.log(`[INTEGRITY] REJECTED: ${result.error}`);
        return res.status(403).json({
          error: `Integrity verification failed: ${result.error}`,
        });
      }
      // 通过，继续处理请求
      req.integrityVerdict = result.verdict;
      next();
    })
    .catch((err) => {
      console.error('[INTEGRITY] Middleware error:', err);
      return res.status(500).json({
        error: 'Integrity verification internal error.',
      });
    });
}

/**
 * Express 中间件: 对 add-score 等接口执行 Play Integrity 校验 (服务端计算 hash)
 *
 * 与 integrityMiddleware 不同的是:
 *   - 客户端不传 x-request-hash，只传 x-integrity-token 和请求体
 *   - 服务端根据与客户端相同的规则 (SHA256 of raw body) 自行计算 hash
 *   - 将计算的 hash 与 integrity token 解密后的 requestHash 进行比对
 */
function integrityWithServerHashMiddleware(req, res, next) {
  // 如果未配置 Play Integrity (开发环境)，跳过
  if (!integrityAuthClient || process.env.SKIP_INTEGRITY === 'true') {
    return next();
  }

  const integrityToken = req.headers['x-integrity-token'];
  if (!integrityToken) {
    return res.status(403).json({
      error: 'Integrity token required. Request rejected.',
    });
  }

  // 服务端根据 raw body 计算 SHA256，与客户端 ComputeSHA256(body) 规则一致
  if (!req.rawBody) {
    return res.status(400).json({
      error: 'Missing request body for hash computation.',
    });
  }
  const computedHash = crypto.createHash('sha256').update(req.rawBody).digest('hex');
  console.log(`[INTEGRITY] Server computed request hash: ${computedHash}`);

  verifyIntegrityToken(integrityToken, computedHash)
    .then((result) => {
      if (!result.valid) {
        console.log(`[INTEGRITY] REJECTED: ${result.error}`);
        return res.status(403).json({
          error: `Integrity verification failed: ${result.error}`,
        });
      }
      // 通过，继续处理请求
      req.integrityVerdict = result.verdict;
      next();
    })
    .catch((err) => {
      console.error('[INTEGRITY] Middleware error:', err);
      return res.status(500).json({
        error: 'Integrity verification internal error.',
      });
    });
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
 * GET /api/coins
 * Headers: x-api-key
 *
 * 查询当前金币余额
 */
app.get('/api/coins', authMiddleware, (req, res) => {
  const row = getCoins.get();
  res.json({ coins: row ? row.coins : 0 });
});

/**
 * GET /api/score
 * Headers: x-api-key
 *
 * 查询当前分数
 */
app.get('/api/score', authMiddleware, (req, res) => {
  const row = getScore.get();
  res.json({ score: row ? row.score : 0 });
});

/**
 * POST /api/add-score
 * Body: { addscore: <int> }
 * Headers: x-api-key, x-integrity-token
 *
 * 给数据库中的 score 增加 addscore 分
 * 受 Play Integrity 中间件保护 (服务端自行计算 request hash 并与 token 中的 hash 比对)
 */
app.post('/api/add-score', authMiddleware, integrityWithServerHashMiddleware, (req, res) => {
  const { addscore, timestamp } = req.body;

  console.log(`[SCORE] Received add-score request body:`, JSON.stringify(req.body));

  if (addscore == null || !Number.isInteger(addscore) || addscore <= 0) {
    return res.status(400).json({ error: 'Invalid addscore: must be a positive integer.' });
  }

  // 校验时间戳: 拒绝超过 60 秒的请求 (防重放)
  if (!timestamp || !Number.isInteger(timestamp)) {
    return res.status(400).json({ error: 'Missing or invalid timestamp.' });
  }
  const now = Date.now();
  const diff = Math.abs(now - timestamp);
  if (diff > 60 * 1000) {
    console.log(`[SCORE] REJECTED: timestamp too old/future, diff=${diff}ms`);
    return res.status(403).json({ error: 'Request expired or timestamp invalid.' });
  }

  addScore.run(addscore);
  const newScore = getScore.get().score;

  console.log(`[SCORE] Added ${addscore} points, new score: ${newScore}`);
  return res.json({ success: true, score_added: addscore, total_score: newScore });
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
    const currentCoins = getCoins.get().coins;
    return res.json({
      success: true,
      order_id: 'duplicate',
      coins_added: 0,
      total_coins: currentCoins,
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

    // 如果是非消耗型物品: 确认购买 (Acknowledge) - 防止自动退款
    // if (purchaseInfo.acknowledgementState === 0) {
    //   console.log(`[ACK] Acknowledging purchase: ${purchaseInfo.orderId}`);
    //   await playDeveloperApi.purchases.products.acknowledge({
    //     packageName: PACKAGE_NAME,
    //     productId: product_id,
    //     token: purchase_token,
    //   });
    //   console.log(`[ACK] ✓ Purchase acknowledged`);
    // }

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

    // 发放金币: 每次购买 100_coins 增加 100 金币
    const COINS_PER_PURCHASE = { '100_coins': 100 };
    const coinsToAdd = COINS_PER_PURCHASE[product_id] || 100;
    addCoins.run(coinsToAdd);
    const newBalance = getCoins.get().coins;

    console.log(`[SUCCESS] Purchase fully processed: orderId=${purchaseInfo.orderId}, product=${product_id}, +${coinsToAdd} coins, balance=${newBalance}`);

    return res.json({
      success: true,
      order_id: purchaseInfo.orderId,
      purchase_time: purchaseInfo.purchaseTimeMillis,
      coins_added: coinsToAdd,
      total_coins: newBalance,
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

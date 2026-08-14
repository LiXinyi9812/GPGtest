const express = require('express');
const authMiddleware = require('../middleware/auth');
const deviceIdMiddleware = require('../middleware/deviceId');
const { getCoins, addCoins } = require('../db');

const router = express.Router();

const PACKAGE_NAME = process.env.PACKAGE_NAME || 'com.mycompany.mygame';
const ALLOWED_PRODUCT_IDS = (process.env.ALLOWED_PRODUCT_IDS || '100_coins')
  .split(',')
  .map((id) => id.trim());

// 已处理的 purchase token 缓存 (防重复发货)
// 生产环境应使用数据库
const processedTokens = new Set();

// playDeveloperApi 由 server.js 初始化后注入
let playDeveloperApi = null;

function setPlayDeveloperApi(api) {
  playDeveloperApi = api;
}

/**
 * GET /api/coins
 * 查询当前金币余额 (按设备ID)
 */
router.get('/coins', authMiddleware, deviceIdMiddleware, (req, res) => {
  const row = getCoins.get(req.deviceId);
  res.json({ coins: row ? row.coins : 0 });
});

/**
 * POST /api/verify-and-consume
 * 验证购买、确认 (acknowledge) 并消耗 (consume)
 * 金币入账到对应设备ID
 */
router.post('/verify-and-consume', authMiddleware, deviceIdMiddleware, async (req, res) => {
  const { product_id, purchase_token } = req.body;
  const deviceId = req.deviceId;

  // 参数校验
  if (!product_id || !purchase_token) {
    return res.status(400).json({
      error: 'Missing required fields: product_id, purchase_token',
    });
  }

  console.log(`[REQUEST] 收到购买请求 - 设备: ${deviceId.substring(0, 16)}..., 产品ID: ${product_id}, Token: ${purchase_token.substring(0, 30)}...`);

  // 商品 ID 白名单校验
  if (!ALLOWED_PRODUCT_IDS.includes(product_id)) {
    return res.status(400).json({ error: `Invalid product_id: ${product_id}` });
  }

  // 幂等性校验: 防止重复处理
  if (processedTokens.has(purchase_token)) {
    console.log(`[WARN] Duplicate token received: ${purchase_token.substring(0, 20)}...`);
    const row = getCoins.get(deviceId);
    const currentCoins = row ? row.coins : 0;
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

    // 发放金币 (入账到对应设备ID)
    const COINS_PER_PURCHASE = { '100_coins': 100 };
    const coinsToAdd = COINS_PER_PURCHASE[product_id] || 100;
    addCoins.run(deviceId, coinsToAdd);
    const newBalance = getCoins.get(deviceId).coins;

    console.log(`[SUCCESS] Purchase processed: device=${deviceId.substring(0, 16)}..., orderId=${purchaseInfo.orderId}, +${coinsToAdd} coins, balance=${newBalance}`);

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

module.exports = { router, setPlayDeveloperApi };
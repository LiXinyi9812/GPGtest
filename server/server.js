require('dotenv').config();

const express = require('express');
const { google } = require('googleapis');
const helmet = require('helmet');
const rateLimit = require('express-rate-limit');
const path = require('path');

// 模块引入
const { setIntegrityAuthClient } = require('./middleware/integrity');
const { router: purchaseRouter, setPlayDeveloperApi } = require('./routes/purchase');
const scoreRouter = require('./routes/score');
const { router: recallRouter, setGamesAuthClient } = require('./routes/recall');

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
// Google Play Developer API 初始化
// ============================================================

const PACKAGE_NAME = process.env.PACKAGE_NAME || 'com.mycompany.mygame';

async function initGoogleApi() {
  const keyFilePath = process.env.GOOGLE_SERVICE_ACCOUNT_KEY
    || './service-account-key.json';

  const auth = new google.auth.GoogleAuth({
    keyFile: path.resolve(__dirname, keyFilePath),
    scopes: [
      'https://www.googleapis.com/auth/androidpublisher',
      'https://www.googleapis.com/auth/playintegrity',
      'https://www.googleapis.com/auth/games',
    ],
  });

  const playDeveloperApi = google.androidpublisher({
    version: 'v3',
    auth: auth,
  });

  // 注入到各模块
  setPlayDeveloperApi(playDeveloperApi);

  const integrityAuthClient = await auth.getClient();
  setIntegrityAuthClient(integrityAuthClient);
  setGamesAuthClient(integrityAuthClient);

  console.log('[Billing Server] Google Play API initialized.');
  console.log('[Billing Server] Play Integrity API initialized (REST mode for PC).');
  console.log('[Billing Server] Recall API initialized.');
}

// ============================================================
// API 路由
// ============================================================

// 健康检查
app.get('/api/health', (req, res) => {
  res.json({ status: 'ok', timestamp: new Date().toISOString() });
});

// 购买相关路由
app.use('/api', purchaseRouter);

// 分数相关路由
app.use('/api', scoreRouter);

// Recall 相关路由
app.use('/api', recallRouter);

// ============================================================
// 启动服务器
// ============================================================

const PORT = parseInt(process.env.PORT, 10) || 3000;
const ALLOWED_PRODUCT_IDS = (process.env.ALLOWED_PRODUCT_IDS || '100_coins')
  .split(',')
  .map((id) => id.trim());

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

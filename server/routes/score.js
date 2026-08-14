const express = require('express');
const authMiddleware = require('../middleware/auth');
const accountIdMiddleware = require('../middleware/accountId');
const { integrityWithServerHashMiddleware } = require('../middleware/integrity');
const { getScore, addScore } = require('../db');

const router = express.Router();

/**
 * GET /api/score
 * 查询当前分数 (按账户ID)
 */
router.get('/score', authMiddleware, accountIdMiddleware, (req, res) => {
  const row = getScore.get(req.accountId);
  res.json({ score: row ? row.score : 0 });
});

/**
 * POST /api/add-score
 * Body: { addscore: <int>, timestamp: <int> }
 * Headers: x-api-key, x-account-id, x-integrity-token
 *
 * 给该账户ID的 score 增加 addscore 分
 * 受 Play Integrity 中间件保护 (服务端自行计算 request hash)
 */
router.post('/add-score', authMiddleware, accountIdMiddleware, integrityWithServerHashMiddleware, (req, res) => {
  const { addscore, timestamp } = req.body;
  const accountId = req.accountId;

  console.log(`[SCORE] Received add-score request: account=${accountId.substring(0, 16)}..., body:`, JSON.stringify(req.body));

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

  addScore.run(accountId, addscore);
  const row = getScore.get(accountId);
  const newScore = row ? row.score : 0;

  console.log(`[SCORE] Account ${accountId.substring(0, 16)}... added ${addscore} points, new score: ${newScore}`);
  return res.json({ success: true, score_added: addscore, total_score: newScore });
});

module.exports = router;

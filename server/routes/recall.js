const express = require('express');
const authMiddleware = require('../middleware/auth');

const router = express.Router();

// Google Games API client (注入)
let gamesAuthClient = null;

function setGamesAuthClient(client) {
  gamesAuthClient = client;
}

const GAMES_API_BASE = 'https://games.googleapis.com/games/v1/recall';

/**
 * POST /api/recall/retrieve
 * Body: { recall_session_id: string }
 * 用 recall_session_id 查询是否已有关联的 account_id
 */
router.post('/recall/retrieve', authMiddleware, async (req, res) => {
  const { recall_session_id } = req.body;

  if (!recall_session_id || typeof recall_session_id !== 'string') {
    return res.status(400).json({ error: 'Missing recall_session_id.' });
  }

  try {
    const token = await gamesAuthClient.getAccessToken();
    const url = `${GAMES_API_BASE}/tokens/${encodeURIComponent(recall_session_id)}`;

    const response = await fetch(url, {
      method: 'GET',
      headers: {
        'Authorization': `Bearer ${token.token}`,
        'Content-Type': 'application/json',
      },
    });

    if (!response.ok) {
      const errText = await response.text();
      console.error(`[RECALL] retrieveTokens failed (${response.status}):`, errText);
      return res.status(response.status).json({
        error: 'Failed to retrieve recall tokens.',
        detail: errText,
      });
    }

    const data = await response.json();
    // data.tokens 是 RecallToken 数组，每个有 .token 字段 (即我们存入的 account_id)
    const tokens = data.tokens || [];

    if (tokens.length > 0) {
      // 返回第一个关联的 account_id
      const accountId = tokens[0].token;
      console.log(`[RECALL] Found linked account: ${accountId.substring(0, 16)}...`);
      return res.json({ found: true, account_id: accountId });
    }

    console.log('[RECALL] No linked account found.');
    return res.json({ found: false, account_id: null });
  } catch (error) {
    console.error('[RECALL] retrieveTokens error:', error.message);
    return res.status(500).json({ error: 'Internal error during recall retrieve.' });
  }
});

/**
 * POST /api/recall/link
 * Body: { recall_session_id: string, account_id: string }
 * 将 account_id 关联到当前 PGS 用户
 */
router.post('/recall/link', authMiddleware, async (req, res) => {
  const { recall_session_id, account_id } = req.body;

  if (!recall_session_id || typeof recall_session_id !== 'string') {
    return res.status(400).json({ error: 'Missing recall_session_id.' });
  }
  if (!account_id || typeof account_id !== 'string') {
    return res.status(400).json({ error: 'Missing account_id.' });
  }
  // 校验 account_id 格式 (十六进制)
  if (!/^[0-9a-fA-F]+$/.test(account_id)) {
    return res.status(400).json({ error: 'Invalid account_id format.' });
  }

  try {
    const token = await gamesAuthClient.getAccessToken();
    const url = `${GAMES_API_BASE}:linkPersona`;

    const body = {
      sessionId: recall_session_id,
      persona: account_id,
      token: account_id,
      cardinalityConstraint: 'ONE_PERSONA_TO_ONE_PLAYER',
      conflictingLinksResolutionPolicy: 'CREATE_NEW_LINK',
    };

    const response = await fetch(url, {
      method: 'POST',
      headers: {
        'Authorization': `Bearer ${token.token}`,
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(body),
    });

    if (!response.ok) {
      const errText = await response.text();
      console.error(`[RECALL] linkPersona failed (${response.status}):`, errText);
      return res.status(response.status).json({
        error: 'Failed to link persona.',
        detail: errText,
      });
    }

    const data = await response.json();
    console.log(`[RECALL] linkPersona result: state=${data.state}`);

    return res.json({
      success: true,
      state: data.state,
      account_id: account_id,
    });
  } catch (error) {
    console.error('[RECALL] linkPersona error:', error.message);
    return res.status(500).json({ error: 'Internal error during recall link.' });
  }
});

module.exports = { router, setGamesAuthClient };

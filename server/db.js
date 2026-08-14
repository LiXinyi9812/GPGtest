const path = require('path');
const Database = require('better-sqlite3');

// ============================================================
// SQLite 数据库初始化 - 记录金币余额 & 分数 (按账户ID绑定)
// ============================================================

const db = new Database(path.join(__dirname, 'coins.db'));
db.pragma('journal_mode = DELETE');

// 创建金币表 (按账户ID存储)
db.exec(`
  CREATE TABLE IF NOT EXISTS wallet (
    account_id TEXT PRIMARY KEY,
    coins INTEGER NOT NULL DEFAULT 0,
    updated_at TEXT NOT NULL DEFAULT (datetime('now'))
  )
`);

// 创建分数表 (按账户ID存储)
db.exec(`
  CREATE TABLE IF NOT EXISTS score (
    account_id TEXT PRIMARY KEY,
    score INTEGER NOT NULL DEFAULT 0,
    updated_at TEXT NOT NULL DEFAULT (datetime('now'))
  )
`);

// 创建 recall 关联表 (存储 recall_session_id 和 account_id 的关联)
db.exec(`
  CREATE TABLE IF NOT EXISTS recall_links (
    recall_session_id TEXT PRIMARY KEY,
    account_id TEXT NOT NULL,
    created_at TEXT NOT NULL DEFAULT (datetime('now'))
  )
`);

// 预编译常用语句 (按 account_id 查询)
const getCoins = db.prepare('SELECT coins FROM wallet WHERE account_id = ?');
const addCoins = db.prepare(`
  INSERT INTO wallet (account_id, coins, updated_at) VALUES (?, ?, datetime('now'))
  ON CONFLICT(account_id) DO UPDATE SET coins = coins + excluded.coins, updated_at = datetime('now')
`);
const getScore = db.prepare('SELECT score FROM score WHERE account_id = ?');
const addScore = db.prepare(`
  INSERT INTO score (account_id, score, updated_at) VALUES (?, ?, datetime('now'))
  ON CONFLICT(account_id) DO UPDATE SET score = score + excluded.score, updated_at = datetime('now')
`);
const getRecallLink = db.prepare('SELECT account_id FROM recall_links WHERE recall_session_id = ?');
const setRecallLink = db.prepare(`
  INSERT INTO recall_links (recall_session_id, account_id, created_at) VALUES (?, ?, datetime('now'))
  ON CONFLICT(recall_session_id) DO UPDATE SET account_id = excluded.account_id, created_at = datetime('now')
`);

module.exports = { db, getCoins, addCoins, getScore, addScore, getRecallLink, setRecallLink };

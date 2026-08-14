const path = require('path');
const Database = require('better-sqlite3');

// ============================================================
// SQLite 数据库初始化 - 记录金币余额 & 分数
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
db.prepare('INSERT OR IGNORE INTO wallet (id, coins) VALUES (1, 0)').run();
db.prepare('INSERT OR IGNORE INTO score (id, score) VALUES (1, 0)').run();

// 预编译常用语句
const getCoins = db.prepare('SELECT coins FROM wallet WHERE id = 1');
const addCoins = db.prepare(`
  UPDATE wallet SET coins = coins + ?, updated_at = datetime('now') WHERE id = 1
`);
const getScore = db.prepare('SELECT score FROM score WHERE id = 1');
const addScore = db.prepare(`
  UPDATE score SET score = score + ?, updated_at = datetime('now') WHERE id = 1
`);

module.exports = { db, getCoins, addCoins, getScore, addScore };

const path = require('path');
const Database = require('better-sqlite3');

// ============================================================
// SQLite 数据库初始化 - 记录金币余额 & 分数 (按设备ID绑定)
// ============================================================

const db = new Database(path.join(__dirname, 'coins.db'));
db.pragma('journal_mode = DELETE');

// 创建金币表 (按设备ID存储)
db.exec(`
  CREATE TABLE IF NOT EXISTS wallet (
    device_id TEXT PRIMARY KEY,
    coins INTEGER NOT NULL DEFAULT 0,
    updated_at TEXT NOT NULL DEFAULT (datetime('now'))
  )
`);

// 创建分数表 (按设备ID存储)
db.exec(`
  CREATE TABLE IF NOT EXISTS score (
    device_id TEXT PRIMARY KEY,
    score INTEGER NOT NULL DEFAULT 0,
    updated_at TEXT NOT NULL DEFAULT (datetime('now'))
  )
`);

// 迁移: 如果旧表中有 id=1 的数据但没有 device_id 列，需要处理
// (新表结构会在首次请求时自动为该设备初始化一行)

// 预编译常用语句 (按 device_id 查询)
const getCoins = db.prepare('SELECT coins FROM wallet WHERE device_id = ?');
const addCoins = db.prepare(`
  INSERT INTO wallet (device_id, coins, updated_at) VALUES (?, ?, datetime('now'))
  ON CONFLICT(device_id) DO UPDATE SET coins = coins + excluded.coins, updated_at = datetime('now')
`);
const getScore = db.prepare('SELECT score FROM score WHERE device_id = ?');
const addScore = db.prepare(`
  INSERT INTO score (device_id, score, updated_at) VALUES (?, ?, datetime('now'))
  ON CONFLICT(device_id) DO UPDATE SET score = score + excluded.score, updated_at = datetime('now')
`);

module.exports = { db, getCoins, addCoins, getScore, addScore };

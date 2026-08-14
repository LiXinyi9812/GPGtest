/**
 * 账户ID中间件: 校验 x-account-id 头
 * account_id 必须是非空的十六进制字符串
 */
function accountIdMiddleware(req, res, next) {
  const accountId = req.headers['x-account-id'];
  if (!accountId || typeof accountId !== 'string') {
    return res.status(400).json({ error: 'Missing x-account-id header.' });
  }
  // 校验格式: 非空十六进制字符串
  if (!/^[0-9a-fA-F]+$/.test(accountId)) {
    return res.status(400).json({ error: 'Invalid account ID format.' });
  }
  req.accountId = accountId;
  next();
}

module.exports = accountIdMiddleware;

const API_SECRET_KEY = process.env.API_SECRET_KEY || 'your-secret-key-here';

/**
 * 简单鉴权中间件: 校验 x-api-key
 */
function authMiddleware(req, res, next) {
  const apiKey = req.headers['x-api-key'];
  if (!apiKey || apiKey !== API_SECRET_KEY) {
    return res.status(401).json({ error: 'Unauthorized: invalid API key.' });
  }
  next();
}

module.exports = authMiddleware;

/**
 * 设备ID中间件: 校验 x-device-id 头
 * device_id 必须是 64 字符的十六进制字符串 (SHA256 输出)
 */
function deviceIdMiddleware(req, res, next) {
  const deviceId = req.headers['x-device-id'];
  if (!deviceId || typeof deviceId !== 'string') {
    return res.status(400).json({ error: 'Missing x-device-id header.' });
  }
  // 校验格式: 64位十六进制
  if (!/^[0-9a-f]{64}$/.test(deviceId)) {
    return res.status(400).json({ error: 'Invalid device ID format.' });
  }
  req.deviceId = deviceId;
  next();
}

module.exports = deviceIdMiddleware;

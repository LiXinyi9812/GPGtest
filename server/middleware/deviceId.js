/**
 * 设备ID中间件: 校验 x-device-id 头
 * device_id 必须是非空的十六进制字符串
 */
function deviceIdMiddleware(req, res, next) {
  const deviceId = req.headers['x-device-id'];
  if (!deviceId || typeof deviceId !== 'string') {
    return res.status(400).json({ error: 'Missing x-device-id header.' });
  }
  // 校验格式: 非空十六进制字符串
  if (!/^[0-9a-fA-F]+$/.test(deviceId)) {
    return res.status(400).json({ error: 'Invalid device ID format.' });
  }
  req.deviceId = deviceId;
  next();
}

module.exports = deviceIdMiddleware;

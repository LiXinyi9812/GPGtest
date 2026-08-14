const crypto = require('crypto');

const PACKAGE_NAME = process.env.PACKAGE_NAME || 'com.mycompany.mygame';

// Play Integrity auth client (由 server.js 初始化后注入)
let integrityAuthClient = null;

function setIntegrityAuthClient(client) {
  integrityAuthClient = client;
}

// ============================================================
// Play Integrity 验证 (PC)
// ============================================================

/**
 * 解密并验证 Play Integrity 令牌
 * @param {string} integrityToken - 客户端获取的完整性令牌
 * @param {string} expectedRequestHash - 期望的 request_hash (客户端请求体的 SHA256)
 * @returns {{ valid: boolean, error?: string, verdict?: object }}
 */
async function verifyIntegrityToken(integrityToken, expectedRequestHash) {
  if (!integrityToken) {
    return { valid: false, error: 'Missing integrity token' };
  }

  try {
    const url = `https://playintegrity.googleapis.com/v1/${PACKAGE_NAME}:decodePcIntegrityToken`;
    const response = await integrityAuthClient.request({
      url,
      method: 'POST',
      data: { integrity_token: integrityToken },
    });

    const rawResponse = response.data;
    console.log('[INTEGRITY] Decoded verdict:', JSON.stringify(rawResponse, null, 2));

    const verdict = rawResponse.tokenPayloadExternal || rawResponse;

    // 1) 验证 requestPackageName
    const requestDetails = verdict.requestDetails || {};
    if (requestDetails.requestPackageName !== PACKAGE_NAME) {
      return {
        valid: false,
        error: `Package name mismatch: ${requestDetails.requestPackageName}`,
        verdict,
      };
    }

    // 2) 验证 requestHash (防止请求被篡改)
    if (expectedRequestHash && requestDetails.requestHash !== expectedRequestHash) {
      return {
        valid: false,
        error: 'Request hash mismatch - potential tampering detected',
        verdict,
      };
    }

    // 3) 验证设备完整性 (PC 环境)
    const deviceIntegrity = verdict.deviceIntegrity || {};
    const deviceVerdict = deviceIntegrity.deviceRecognitionVerdict || [];
    if (!deviceVerdict.includes('MEETS_PC_INTEGRITY')) {
      return {
        valid: false,
        error: `Device integrity check failed: [${deviceVerdict.join(', ')}]`,
        verdict,
      };
    }

    // 4) 验证应用许可证 (暂时跳过，测试阶段可能返回 UNLICENSED)
    // const accountDetails = verdict.accountDetails || {};
    // if (accountDetails.appLicensingVerdict === 'UNLICENSED') { ... }

    return { valid: true, verdict };
  } catch (error) {
    console.error('[INTEGRITY] Token verification failed:', error.message);
    return { valid: false, error: `Integrity API error: ${error.message}` };
  }
}

// ============================================================
// Express 中间件
// ============================================================

/**
 * 对敏感接口强制执行 Play Integrity 校验
 * 客户端需在 Header 中传入:
 *   x-integrity-token: <token>
 *   x-request-hash: <sha256 of request body>
 */
function integrityMiddleware(req, res, next) {
  if (!integrityAuthClient || process.env.SKIP_INTEGRITY === 'true') {
    return next();
  }

  const integrityToken = req.headers['x-integrity-token'];
  if (!integrityToken) {
    return res.status(403).json({
      error: 'Integrity token required. Request rejected.',
    });
  }

  const expectedHash = req.headers['x-request-hash'] || '';

  verifyIntegrityToken(integrityToken, expectedHash)
    .then((result) => {
      if (!result.valid) {
        console.log(`[INTEGRITY] REJECTED: ${result.error}`);
        return res.status(403).json({
          error: `Integrity verification failed: ${result.error}`,
        });
      }
      req.integrityVerdict = result.verdict;
      next();
    })
    .catch((err) => {
      console.error('[INTEGRITY] Middleware error:', err);
      return res.status(500).json({
        error: 'Integrity verification internal error.',
      });
    });
}

/**
 * 服务端自行计算 request hash 并与 integrity token 中的 hash 比对
 * 客户端只传 x-integrity-token 和请求体，不传 x-request-hash
 */
function integrityWithServerHashMiddleware(req, res, next) {
  if (!integrityAuthClient || process.env.SKIP_INTEGRITY === 'true') {
    return next();
  }

  const integrityToken = req.headers['x-integrity-token'];
  if (!integrityToken) {
    return res.status(403).json({
      error: 'Integrity token required. Request rejected.',
    });
  }

  if (!req.rawBody) {
    return res.status(400).json({
      error: 'Missing request body for hash computation.',
    });
  }
  const computedHash = crypto.createHash('sha256').update(req.rawBody).digest('hex');
  console.log(`[INTEGRITY] Server computed request hash: ${computedHash}`);

  verifyIntegrityToken(integrityToken, computedHash)
    .then((result) => {
      if (!result.valid) {
        console.log(`[INTEGRITY] REJECTED: ${result.error}`);
        return res.status(403).json({
          error: `Integrity verification failed: ${result.error}`,
        });
      }
      req.integrityVerdict = result.verdict;
      next();
    })
    .catch((err) => {
      console.error('[INTEGRITY] Middleware error:', err);
      return res.status(500).json({
        error: 'Integrity verification internal error.',
      });
    });
}

module.exports = {
  setIntegrityAuthClient,
  integrityMiddleware,
  integrityWithServerHashMiddleware,
};

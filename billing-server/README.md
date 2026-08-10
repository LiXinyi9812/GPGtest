# GPG Billing 后端验证服务器

Google Play 购买验证后端，接收客户端的 `purchase_token`，通过 Google Play Developer API 校验购买合法性并确认 (acknowledge)。

## 快速开始

### 1. 安装依赖

```bash
cd billing-server
npm install
```

### 2. 配置

复制 `.env.example` 为 `.env` 并填写：

```bash
cp .env.example .env
```

需要配置：
- `PACKAGE_NAME` — 你的应用包名
- `GOOGLE_SERVICE_ACCOUNT_KEY` — Google 服务账号密钥文件路径
- `API_SECRET_KEY` — 客户端鉴权密钥 (需与 game.cpp 中一致)
- `ALLOWED_PRODUCT_IDS` — 允许的商品 ID 白名单

### 3. 获取服务账号密钥

1. 前往 [Google Cloud Console](https://console.cloud.google.com/)
2. 创建服务账号或使用已有的
3. 下载 JSON 密钥文件，放到本项目目录并命名为 `service-account-key.json`
4. 在 Google Play Console 中，将该服务账号添加为具有"查看财务数据"权限的用户

### 4. 启动

```bash
npm start
```

服务器默认运行在 `http://localhost:3000`

## API 接口

### `POST /api/verify-and-consume`

完整的购买处理流程：验证 → 确认 → 消耗

后端负责全部处理，包括：
1. 验证购买的合法性
2. Acknowledge 购买（防止自动退款）
3. Consume 购买（允许后续购买）

**Headers:**
- `Content-Type: application/json`
- `x-api-key: <your-api-secret-key>`

**Body:**
```json
{
  "product_id": "100_coins",
  "purchase_token": "xxx..."
}
```

**成功响应 (200):**
```json
{
  "success": true,
  "order_id": "GPA.1234-5678-9012",
  "purchase_time": "1234567890000",
  "message": "Purchase verified, acknowledged and consumed successfully."
}
```

**失败响应 (400/404/500):**
```json
{
  "success": false,
  "error": "Error message"
}
```

### `GET /api/health`

健康检查接口。

## 安全说明

- 生产环境请使用 HTTPS
- `API_SECRET_KEY` 要保密，不要提交到代码库
- 生产环境中 `processedTokens` 应替换为数据库存储
- 建议部署时添加防火墙，仅允许游戏客户端 IP 访问

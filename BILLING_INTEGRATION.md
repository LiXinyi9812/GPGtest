# Google Play Billing Integration

## 概述 (Overview)

已在 `hello_world.cpp` 中集成 Google Play Billing SDK，实现了虚拟商品购买流程。

The Google Play Billing SDK has been integrated into `hello_world.cpp` to implement in-app purchase flow for virtual items.

## 功能 (Features)

1. **SDK 初始化** - 在应用启动时初始化 Google Play SDK
2. **计费客户端初始化** - 创建并配置 BillingClient，启用待处理购买
3. **自动购买流程** - 在输出 "Hello, World!" 后自动触发购买流程
4. **交互式购买** - 用户输入 'buy' 命令可再次进入购买流程
5. **商品查询** - 查询商品详情（ID: 100_coins）
6. **购买确认** - 提示用户输入 y/n 确认是否购买
7. **购买执行** - 启动 Google Play 购买流程
8. **消耗处理** - 自动消耗购买（因为虚拟货币是消耗品）

## 代码结构 (Code Structure)

### 主要函数 (Main Functions)

- `InitializeBillingClient()` - 初始化计费客户端
- `StartPurchaseFlow()` - 启动购买流程，包含以下步骤：
  1. 查询商品详情 (QueryProductDetails)
  2. 显示商品信息和价格
  3. 请求用户确认
  4. 启动购买 (LaunchPurchaseFlow)
  5. 消耗购买 (ConsumePurchase)

### 工作流程 (Workflow)

```
SDK初始化
    ↓
输出 "Hello, World!"
    ↓
初始化计费客户端
    ↓
自动触发购买流程
    ↓
查询商品 "100_coins"
    ↓
显示商品信息和价格
    ↓
询问用户: "Do you want to purchase 100 coins? (y/n):"
    ↓
[y] → 启动购买 → 消耗购买 → "You now have 100 coins"
[n] → "Purchase canceled"
    ↓
进入游戏循环
    ↓
等待用户输入 "buy" 或 "exit"
```

## 使用方法 (Usage)

### 编译项目 (Build)

使用 Visual Studio 打开 `GPG.sln` 并编译，或使用命令行:

```bash
msbuild GPG.vcxproj /p:Configuration=Debug /p:Platform=x64
```

### 运行程序 (Run)

```bash
.\bin\Debug\GPG.exe
```

### 交互命令 (Commands)

- 启动后自动进入购买流程
- 输入 `y` 或 `Y` 确认购买
- 输入 `n` 或 `N` 取消购买
- 购买完成后，输入 `buy` 再次购买
- 输入 `exit` 退出程序

## 配置要求 (Requirements)

1. **Google Play Console 配置**
   - 已在 Play Developer Console 中配置虚拟商品
   - 商品 ID: `100_coins`
   - 商品类型: In-App Product (应用内商品)

2. **SDK 配置**
   - Google Play PC SDK 已正确安装
   - `play_pc_sdk.dll` 和 `play_pc_sdk.lib` 在正确的路径

3. **项目配置**
   - 包含目录: `$(SolutionDir)includes`
   - 库目录: `$(SolutionDir)imports\x64`
   - 链接库: `play_pc_sdk.lib`

## 注意事项 (Notes)

1. **消耗型商品**: 代码中使用 `ConsumePurchase()` 因为虚拟货币是消耗品，可重复购买
2. **异步处理**: 所有 Billing API 调用都是异步的，使用 promise/future 模式等待结果
3. **错误处理**: 包含对各种错误情况的处理（用户取消、网络错误等）
4. **待处理购买**: 已启用 `enable_pending_purchases = true` 以支持待处理的购买

## API 参考 (API Reference)

### BillingClient 主要方法

- `QueryProductDetails()` - 查询可购买的商品详情
- `LaunchPurchaseFlow()` - 启动购买流程
- `ConsumePurchase()` - 消耗购买（用于可重复购买的商品）
- `AcknowledgePurchase()` - 确认购买（用于一次性购买的商品）
- `QueryPurchases()` - 查询用户已购买的商品

## 测试建议 (Testing Recommendations)

1. 测试正常购买流程（输入 y）
2. 测试取消购买（输入 n）
3. 测试重复购买（多次输入 buy）
4. 测试网络断开情况
5. 验证 Play Console 中的购买记录

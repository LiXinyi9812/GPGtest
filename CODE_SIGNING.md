# 代码签名指南

## 准备工作

### 1. 获取证书

**测试环境（自签名）：**
```powershell
# 以管理员身份运行 PowerShell
$cert = New-SelfSignedCertificate -Type CodeSigningCert -Subject "CN=GPG Development" -CertStoreLocation Cert:\CurrentUser\My
$password = ConvertTo-SecureString -String "YourPassword123" -Force -AsPlainText
Export-PfxCertificate -Cert $cert -FilePath "C:\GPGCodeSign.pfx" -Password $password

# 信任证书（避免安全警告）
Export-Certificate -Cert $cert -FilePath "C:\GPGCodeSign.cer"
Import-Certificate -FilePath "C:\GPGCodeSign.cer" -CertStoreLocation Cert:\LocalMachine\Root
```

**生产环境：**
- 从 DigiCert、Sectigo 等 CA 购买代码签名证书
- 收到 .pfx 文件和密码

### 2. 定位 signtool.exe

在你的系统上，signtool 位于：
```
C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe
```

## 使用方法

### 方法 1：使用批处理脚本（推荐）

1. 编辑 `sign_all.bat`，修改配置区域：
   ```batch
   set CERT_FILE=C:\GPGCodeSign.pfx
   set CERT_PASSWORD=YourPassword123
   ```

2. 构建项目（Release 模式）

3. 运行批处理脚本：
   ```cmd
   sign_all.bat
   ```

### 方法 2：手动签名单个文件

```cmd
cd "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64"

signtool sign ^
  /f "C:\GPGCodeSign.pfx" ^
  /p "YourPassword123" ^
  /t http://timestamp.digicert.com ^
  /fd SHA256 ^
  /v ^
  "C:\Users\devre\OneDrive\GPG\bin\Release\GPG.exe"
```

### 方法 3：使用证书存储（无需密码文件）

```cmd
signtool sign ^
  /n "GPG Development" ^
  /t http://timestamp.digicert.com ^
  /fd SHA256 ^
  /v ^
  "C:\Users\devre\OneDrive\GPG\bin\Release\GPG.exe"
```

## 验证签名

```cmd
signtool verify /pa /v "C:\path\to\GPG.exe"
```

或者右键点击文件 → 属性 → 数字签名选项卡

## signtool 参数说明

| 参数 | 说明 |
|------|------|
| `/f <file>` | 证书文件路径（.pfx 或 .p12） |
| `/p <password>` | 证书密码 |
| `/n <name>` | 证书主题名称（从证书存储查找） |
| `/t <url>` | 时间戳服务器 URL（SHA1，旧式） |
| `/tr <url>` | RFC 3161 时间戳服务器 URL（推荐） |
| `/td SHA256` | 时间戳摘要算法 |
| `/fd SHA256` | 文件摘要算法 |
| `/v` | 详细输出 |
| `/pa` | 验证时使用默认认证策略 |

## 时间戳服务器

时间戳很重要！它确保证书过期后签名仍然有效。

**推荐的时间戳服务器：**
- DigiCert: `http://timestamp.digicert.com`
- Sectigo: `http://timestamp.sectigo.com`
- GlobalSign: `http://timestamp.globalsign.com`

**RFC 3161 格式（推荐）：**
```cmd
signtool sign /tr http://timestamp.digicert.com /td SHA256 ...
```

## 集成到 Visual Studio 构建

在项目的 .vcxproj 文件中添加后期生成事件：

```xml
<PostBuildEvent>
  <Command>
    "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe" sign /f "C:\GPGCodeSign.pfx" /p "YourPassword123" /t http://timestamp.digicert.com /fd SHA256 "$(TargetPath)"
  </Command>
  <Message>Signing $(TargetFileName)...</Message>
</PostBuildEvent>
```

## 安全建议

1. **不要将证书和密码提交到版本控制**
2. **使用环境变量存储密码：**
   ```cmd
   set CERT_PASSWORD=YourPassword123
   signtool sign /f cert.pfx /p "%CERT_PASSWORD%" ...
   ```
3. **生产环境使用硬件令牌（USB Token）存储证书**
4. **限制证书文件的访问权限**

## 常见问题

### 错误：SignTool Error: No certificates were found that met all the given criteria

- 检查证书文件路径是否正确
- 检查密码是否正确
- 确认证书类型为代码签名证书

### 警告：签名的文件仍然显示"未知发布者"

- 自签名证书需要导入到"受信任的根证书颁发机构"
- 正式证书不会有此问题

### 时间戳失败

- 尝试更换时间戳服务器
- 检查网络连接

## 检查签名是否成功

```cmd
# 命令行验证
signtool verify /pa /v "GPG.exe"

# 图形界面验证
右键 GPG.exe → 属性 → 数字签名选项卡
```

## 批量签名所有构建输出

使用提供的 `sign_all.bat` 脚本，它会：
1. 签名所有 4 个 exe 文件
2. 自动验证签名
3. 显示详细进度
4. 出错时暂停

## 参考资料

- [Microsoft SignTool 文档](https://docs.microsoft.com/en-us/windows/win32/seccrypto/signtool)
- [代码签名最佳实践](https://docs.microsoft.com/en-us/windows-hardware/drivers/install/code-signing-best-practices)

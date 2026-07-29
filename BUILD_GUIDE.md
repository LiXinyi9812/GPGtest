# 自动化构建和打包脚本使用指南

## 概述

`build_and_package.bat` 是一个一键式自动化脚本，完成以下所有步骤：

1. ✅ 清理旧的构建文件
2. ✅ 编译所有项目（GPG.exe、GPGLauncher.exe、GPGInstaller.exe、GPGUninstaller.exe）
3. ✅ 自动复制 play_pc_sdk.dll 到输出目录
4. ✅ 验证所有必需文件存在
5. ✅ **数字签名所有 EXE 文件（可选）**
6. ✅ 生成 WAB 包
7. ✅ 显示详细的构建信息

## 配置

在使用脚本之前，需要编辑 `build_and_package.bat` 文件的配置区域：

### 1. Visual Studio 路径

```batch
set MSBUILD="C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
```

根据你的 Visual Studio 版本修改路径：
- VS 2022 Community: `C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe`
- VS 2022 Professional: `C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe`
- VS 2019: `C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe`

### 2. SignTool 路径

```batch
set SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe"
```

如果你的 Windows SDK 版本不同，需要修改版本号（如 10.0.22621.0）。

### 3. 证书配置

**方式 1：使用 PFX 证书文件（推荐）**

```batch
set CERT_FILE=C:\GPGCodeSign.pfx
set CERT_PASSWORD=MyPassword123
set SIGN_PARAMS=/f "%CERT_FILE%" /p "%CERT_PASSWORD%"
```

**方式 2：使用证书存储**

如果证书已导入到 Windows 证书存储，可以使用：

```batch
REM 注释掉方式 1 的 3 行
REM set CERT_FILE=C:\GPGCodeSign.pfx
REM set CERT_PASSWORD=MyPassword123
REM set SIGN_PARAMS=/f "%CERT_FILE%" /p "%CERT_PASSWORD%"

REM 取消下面这行的注释
set SIGN_PARAMS=/n "GPG Development"
```

### 4. 其他配置

```batch
REM 时间戳服务器（确保签名即使证书过期后仍有效）
set TIMESTAMP_URL=http://timestamp.digicert.com

REM 构建配置：Debug 或 Release
set BUILD_CONFIG=Debug

REM 是否启用代码签名：YES 或 NO
set ENABLE_SIGNING=YES
```

## 使用方法

### 快速开始

1. 编辑 `build_and_package.bat`，修改配置区域
2. 双击运行 `build_and_package.bat`
3. 等待完成，获得 `GPG.wab`
4. 上传 `GPG.wab` 到 Google Play Games PC

### 详细步骤

#### 步骤 1：准备证书（如果启用签名）

如果 `ENABLE_SIGNING=YES`，你需要先准备代码签名证书。

**测试环境（自签名证书）：**

以管理员身份运行 PowerShell：

```powershell
# 创建自签名证书
$cert = New-SelfSignedCertificate -Type CodeSigningCert -Subject "CN=GPG Development" -CertStoreLocation Cert:\CurrentUser\My

# 导出为 PFX 文件
$password = ConvertTo-SecureString -String "MyPassword123" -Force -AsPlainText
Export-PfxCertificate -Cert $cert -FilePath "C:\GPGCodeSign.pfx" -Password $password

# 信任证书（避免安全警告）
Export-Certificate -Cert $cert -FilePath "C:\GPGCodeSign.cer"
Import-Certificate -FilePath "C:\GPGCodeSign.cer" -CertStoreLocation Cert:\LocalMachine\Root
```

**生产环境：**

从 DigiCert、Sectigo 等 CA 购买正式的代码签名证书。

#### 步骤 2：运行脚本

直接双击 `build_and_package.bat` 或在命令行运行：

```cmd
build_and_package.bat
```

#### 步骤 3：查看输出

脚本执行完成后，你会看到：

```
========================================
SUCCESS!
========================================
Build Configuration: Debug
Code Signing: ENABLED

WAB package created: GPG.wab
-rw-r--r-- 1 devre 197609 166K Jul 28 16:00 GPG.wab

Files in WAB package:
  - GPGInstaller.exe
  - GPGLauncher.exe
  - GPG.exe
  - GPGUninstaller.exe
  - play_pc_sdk.dll
  - All EXE files are digitally signed

You can now upload GPG.wab to Google Play Games PC
========================================
```

## 签名验证

### 验证签名是否成功

**方法 1：使用 SignTool**

```cmd
cd bin\Debug
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe" verify /pa /v GPGInstaller.exe
```

**方法 2：图形界面**

1. 右键点击 `bin\Debug\GPGInstaller.exe`
2. 选择"属性"
3. 查看"数字签名"选项卡
4. 应该能看到签名信息

### 验证 WAB 包内容

解压 WAB 包（本质上是一个 ZIP 文件）：

```cmd
mkdir temp_wab
cd temp_wab
tar -xf ..\GPG.wab
dir
```

检查是否包含所有必需文件：
- GPGInstaller.exe ✅
- GPGLauncher.exe ✅
- GPG.exe ✅
- GPGUninstaller.exe ✅
- play_pc_sdk.dll ✅
- play_publishing_config.xml ✅

## 配置选项说明

### BUILD_CONFIG

- **Debug**：包含调试符号，文件较大，适合开发和测试
- **Release**：优化的发布版本，文件较小，适合生产环境

推荐：
- 开发测试：使用 `Debug`
- 上传到 Google Play：使用 `Release`

### ENABLE_SIGNING

- **YES**：启用代码签名（推荐）
  - ✅ 用户安装时不会看到"未知发布者"警告
  - ✅ 符合 Windows SmartScreen 要求
  - ✅ 更专业，用户信任度更高
  
- **NO**：跳过代码签名
  - ⚠️ 用户可能看到安全警告
  - ⚠️ 适合快速测试
  - ⚠️ 不推荐用于生产环境

## 故障排除

### 错误：MSBuild not found

**原因**：Visual Studio 路径不正确

**解决**：
1. 找到你的 MSBuild.exe 实际位置
2. 更新脚本中的 `MSBUILD` 路径

### 错误：signtool.exe not found

**原因**：Windows SDK 未安装或路径不正确

**解决方案 1**：安装 Windows SDK
- 下载：https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/
- 或在 Visual Studio Installer 中安装"Windows 10 SDK"

**解决方案 2**：禁用签名
```batch
set ENABLE_SIGNING=NO
```

### 错误：Failed to sign (证书相关)

**可能原因**：
1. 证书文件路径不正确
2. 证书密码错误
3. 证书类型不是代码签名证书

**解决**：
1. 检查 `CERT_FILE` 路径
2. 检查 `CERT_PASSWORD`
3. 使用 PowerShell 验证证书：
   ```powershell
   $cert = Get-PfxCertificate -FilePath "C:\GPGCodeSign.pfx"
   $cert | Format-List *
   ```

### 错误：Timestamp server unreachable

**原因**：时间戳服务器不可用或网络问题

**解决**：更换时间戳服务器，编辑脚本：

```batch
REM 备选时间戳服务器
set TIMESTAMP_URL=http://timestamp.sectigo.com
REM set TIMESTAMP_URL=http://timestamp.globalsign.com
REM set TIMESTAMP_URL=http://timestamp.comodoca.com
```

### 错误：play_pc_sdk.dll not found

**原因**：PostBuildEvent 没有正确执行

**解决**：
1. 确保 `imports\x64\play_pc_sdk.dll` 存在
2. 手动复制：
   ```cmd
   copy imports\x64\play_pc_sdk.dll bin\Debug\
   ```
3. 检查项目文件的 PostBuildEvent 配置

## 高级用法

### 仅构建不打包

如果只想编译项目而不生成 WAB：

```batch
%MSBUILD% GPG.sln /t:Build /p:Configuration=Debug /p:Platform=x64
```

### 仅签名不构建

如果已经构建完成，只需要重新签名：

运行 `sign_all.bat` 脚本

### 批量构建多个配置

```batch
REM 构建 Debug 版本
set BUILD_CONFIG=Debug
call build_and_package.bat

REM 构建 Release 版本
set BUILD_CONFIG=Release
call build_and_package.bat
```

## 安全提示

⚠️ **重要安全建议：**

1. **不要将证书文件提交到版本控制（Git）**
   - 将 `*.pfx` 添加到 `.gitignore`
   
2. **不要在脚本中硬编码密码**
   - 使用环境变量：
     ```batch
     set CERT_PASSWORD=%MY_CERT_PASSWORD%
     ```
   
3. **保护证书文件**
   - 存放在安全的位置
   - 设置文件权限，仅管理员可访问
   
4. **使用硬件令牌（生产环境）**
   - USB Token 存储证书
   - 更安全，无法导出私钥

## 与 Google Play Games PC 集成

### 上传流程

1. 运行 `build_and_package.bat` 生成 `GPG.wab`
2. 登录 Google Play Console
3. 进入你的游戏应用
4. 上传 WAB 包
5. 填写发布信息
6. 提交审核

### 更新版本

每次更新时：
1. 修改代码
2. 更新 `play_publishing_config.xml` 中的版本号
3. 重新运行 `build_and_package.bat`
4. 上传新的 WAB 包

### 测试建议

在上传到 Google Play 之前：
1. 在本地测试安装程序
2. 运行 `bin\Debug\GPGInstaller.exe`
3. 检查是否正确安装到目标目录
4. 验证 `play_pc_sdk.dll` 是否被复制
5. 测试启动器是否能正常启动游戏

## 总结

使用 `build_and_package.bat` 的优势：

✅ **一键完成所有步骤** - 无需记住复杂的命令
✅ **自动化代码签名** - 确保每次构建都已签名
✅ **错误检查** - 每个步骤都有验证
✅ **可配置** - 支持 Debug/Release、签名开关等
✅ **可重复** - 每次构建结果一致
✅ **节省时间** - 从几分钟手动操作到几秒钟自动完成

现在你只需要：
1. 修改配置（一次性）
2. 双击运行脚本
3. 等待完成
4. 上传 WAB 包

就这么简单！🚀

# 快速参考 - GPG 项目构建

## 🚀 快速开始（3 步）

1. **配置一次** - 编辑 `build_and_package.bat`：
   ```batch
   set CERT_FILE=C:\GPGCodeSign.pfx
   set CERT_PASSWORD=你的密码
   set BUILD_CONFIG=Debug  (或 Release)
   set ENABLE_SIGNING=YES  (或 NO)
   ```

2. **运行脚本** - 双击 `build_and_package.bat`

3. **上传** - 将生成的 `GPG.wab` 上传到 Google Play

---

## 📁 项目文件结构

```
GPG/
├── GPG.sln                      # Visual Studio 解决方案
├── build_and_package.bat        # 🔥 一键构建脚本（新）
├── sign_all.bat                 # 单独签名脚本
│
├── *.cpp / *.vcxproj            # 源代码和项目文件
│   ├── GPG.vcxproj              # 主程序
│   ├── GPGLauncher.vcxproj      # 启动器
│   ├── GPGInstaller.vcxproj     # 安装程序（已修复）
│   └── GPGUninstaller.vcxproj   # 卸载程序
│
├── imports/x64/
│   └── play_pc_sdk.dll          # Play Games SDK
│
├── bin/Debug/ (或 Release/)
│   ├── GPG.exe                  # ✅ 自动生成
│   ├── GPGLauncher.exe          # ✅ 自动生成
│   ├── GPGInstaller.exe         # ✅ 自动生成
│   ├── GPGUninstaller.exe       # ✅ 自动生成
│   ├── play_pc_sdk.dll          # ✅ 自动复制
│   └── play_publishing_config.xml
│
└── GPG.wab                      # ✅ 最终输出（上传这个）
```

---

## 🔧 已修复的问题

### ✅ installer.cpp
- 添加了复制 `play_pc_sdk.dll`
- 添加到卸载信息列表

### ✅ 所有 .vcxproj 文件
- 统一输出目录到 `bin\Debug` 或 `bin\Release`
- 添加 PostBuildEvent 自动复制 DLL
- 修正头文件和库文件路径

### ✅ build_and_package.bat（新功能）
- ✨ 自动编译
- ✨ 自动代码签名
- ✨ 自动生成 WAB
- ✨ 完整的错误检查

---

## 🎯 构建脚本功能

`build_and_package.bat` 自动执行：

1. 清理旧的构建
2. 编译所有项目
3. 验证文件完整性
4. **数字签名所有 EXE**（可选）
5. 生成 WAB 包

---

## ⚙️ 配置选项

| 选项 | 值 | 说明 |
|------|-----|------|
| `BUILD_CONFIG` | Debug | 包含调试符号，适合开发 |
|  | Release | 优化版本，适合发布 |
| `ENABLE_SIGNING` | YES | 启用数字签名（推荐） |
|  | NO | 跳过签名（快速测试） |

---

## 📝 重要提醒

### 为什么需要重新签名？

❌ **每次重新编译后，签名会丢失！**

EXE 文件的二进制内容改变 → 之前的签名失效 → 必须重新签名

✅ **现在 `build_and_package.bat` 会自动处理签名**

---

## 🐛 常见问题

### Q: 上传后游戏还是闪退？
**A**: 检查 WAB 包是否包含 `play_pc_sdk.dll`
```cmd
mkdir temp && cd temp
tar -xf ..\GPG.wab
dir play_pc_sdk.dll
```

### Q: 签名失败？
**A**: 检查证书配置
```batch
set CERT_FILE=C:\GPGCodeSign.pfx      # 路径正确？
set CERT_PASSWORD=MyPassword123       # 密码正确？
```

### Q: 不想签名怎么办？
**A**: 设置
```batch
set ENABLE_SIGNING=NO
```

### Q: MSBuild 找不到？
**A**: 更新路径，常见位置：
```batch
# VS 2022
C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe

# VS 2019
C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe
```

---

## 📚 文档

详细文档请查看：

- [BUILD_GUIDE.md](BUILD_GUIDE.md) - 完整的构建和签名指南
- [PLAY_SDK_FIX.md](PLAY_SDK_FIX.md) - play_pc_sdk.dll 问题修复说明
- [CODE_SIGNING.md](CODE_SIGNING.md) - 代码签名详细指南
- [INSTALLER_README.md](INSTALLER_README.md) - 安装程序说明

---

## 🎉 完成！

现在你可以：

1. ✅ 一键完成构建、签名、打包
2. ✅ 不再担心缺少 play_pc_sdk.dll
3. ✅ 不需要手动运行签名脚本
4. ✅ 每次构建都是完整且一致的

只需双击 `build_and_package.bat`，剩下的交给脚本！🚀

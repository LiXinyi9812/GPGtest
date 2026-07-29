# play_pc_sdk.dll 问题修复说明

## 问题原因
当用户通过 Google Play Games PC 点击"开始游戏"时，命令窗口闪退的原因是：
1. **安装程序没有复制 `play_pc_sdk.dll`** - 启动器需要这个 DLL 来初始化 Play Games SDK
2. **项目输出路径不统一** - 不同项目输出到不同目录，导致文件分散

## 已修复内容

### 1. 修改了 `installer.cpp`
- 添加了复制 `play_pc_sdk.dll` 的代码
- 在卸载信息文件中添加了 `play_pc_sdk.dll` 记录

### 2. 更新了项目配置文件
修改了以下项目文件，确保它们都：
- 输出到统一的 `bin\Debug` 或 `bin\Release` 目录
- 自动复制 `play_pc_sdk.dll` 到输出目录
- 使用正确的相对路径 `$(SolutionDir)`

修改的文件：
- `GPG.vcxproj` - 主程序
- `GPGLauncher.vcxproj` - 启动器
- `GPGInstaller.vcxproj` - 安装程序

### 3. 创建了自动化构建脚本
创建了 `build_and_package.bat`，自动完成：
1. 清理旧的构建
2. 编译所有项目
3. 验证所有必需文件存在
4. 生成 WAB 包

## 使用方法

### 方式 1：使用自动化脚本（推荐）
1. 打开 `build_and_package.bat`
2. 如果 MSBuild 路径不对，修改脚本中的路径
3. 双击运行脚本
4. 等待完成，获得 `GPG.wab`

### 方式 2：手动构建
1. 在 Visual Studio 中打开 `GPG.sln`
2. 右键解决方案，选择"清理解决方案"
3. 右键解决方案，选择"生成解决方案"
4. 打开命令提示符，运行：
   ```cmd
   cd bin\Debug
   play_publishing_tool_1.0.3.exe build-installer-bundle --input=play_publishing_config.xml --output=..\..\GPG.wab
   ```

## 验证修复

构建完成后，检查 `bin\Debug` 目录应该包含：
- ✅ GPG.exe
- ✅ GPGLauncher.exe
- ✅ GPGInstaller.exe
- ✅ GPGUninstaller.exe
- ✅ **play_pc_sdk.dll** ← 这是关键！

## 上传到 Google Play

1. 将生成的 `GPG.wab` 上传到 Google Play Games PC 后台
2. 用户安装后，游戏目录将包含所有必需文件，包括 `play_pc_sdk.dll`
3. 点击"开始游戏"应该正常启动，不再闪退

## 文件结构

安装后的文件结构：
```
C:\Program Files\GPG\  (或 Google Play 指定的路径)
├── GPGLauncher.exe       # 启动器（由 Google Play 调用）
├── GPG.exe               # 主程序（由启动器调用）
├── GPGUninstaller.exe    # 卸载程序
├── play_pc_sdk.dll       # Play Games SDK 动态库（关键！）
└── uninstall_info.txt    # 卸载信息
```

## 技术细节

### 为什么需要 play_pc_sdk.dll？
启动器 (`launcher.cpp`) 调用了 Play Games SDK 的初始化函数：
```cpp
GooglePlayInitialize([promise](InitializeResult result) {
    promise->set_value(std::move(result));
});
```

如果没有 `play_pc_sdk.dll`，程序会因为找不到 DLL 而立即崩溃，导致命令窗口闪退。

### PostBuildEvent 的作用
每个项目的 `.vcxproj` 文件中都添加了：
```xml
<PostBuildEvent>
  <Command>xcopy /Y /D "$(SolutionDir)imports\x64\play_pc_sdk.dll" "$(OutDir)"</Command>
</PostBuildEvent>
```

这确保每次编译后，DLL 会自动复制到输出目录。

## 故障排除

### 问题：build_and_package.bat 报错找不到 MSBuild
**解决**：在脚本中修改 MSBuild 路径，常见路径：
- VS 2022: `C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe`
- VS 2019: `C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe`

### 问题：编译后还是没有 play_pc_sdk.dll
**解决**：
1. 检查 `imports\x64\play_pc_sdk.dll` 是否存在
2. 在 Visual Studio 中查看"输出"窗口，确认 PostBuildEvent 是否执行
3. 手动复制：`copy imports\x64\play_pc_sdk.dll bin\Debug\`

### 问题：上传后还是闪退
**可能原因**：
1. 检查是否还缺少其他 DLL（VC++ 运行时等）
2. 查看 Windows 事件查看器中的应用程序日志，获取详细错误信息
3. 尝试在本地运行安装程序，测试是否能正常启动

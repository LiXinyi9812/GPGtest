# GPG 安装和卸载程序

此项目包含 GPG 应用程序的安装和卸载工具。

## 项目结构

- **GPGLauncher.exe** - 主启动程序，初始化 Play Games PC SDK 并启动 GPG.exe
- **GPG.exe** - 主应用程序
- **GPGInstaller.exe** - 安装程序
- **GPGUninstaller.exe** - 卸载程序

## 编译

在 Visual Studio 中打开 `GPG.sln`，然后构建以下项目：

1. **GPG** - 生成 GPG.exe
2. **GPGLauncher** - 生成 GPGLauncher.exe
3. **GPGInstaller** - 生成 GPGInstaller.exe
4. **GPGUninstaller** - 生成 GPGUninstaller.exe

所有编译后的文件会输出到 `bin\Release\` 或 `bin\Debug\` 目录。

## 安装程序使用方法

### 方式 1：默认安装
双击 `GPGInstaller.exe`，程序将自动安装到默认位置：
```
C:\Program Files\GPG
```

### 方式 2：自定义安装目录
通过命令行指定安装目录：
```cmd
GPGInstaller.exe "D:\MyApps\GPG"
```

### 安装程序功能
- 将 `GPGLauncher.exe` 和 `GPG.exe` 复制到安装目录
- 复制 `GPGUninstaller.exe` 到安装目录
- 创建卸载信息文件（用于跟踪安装的文件）
- 在注册表中添加卸载条目（可在 Windows 控制面板中看到）

### 注意事项
- 需要管理员权限（GPGInstaller.exe 配置了 UAC 提升）
- 安装前确保以下文件与 GPGInstaller.exe 在同一目录：
  - GPGLauncher.exe
  - GPG.exe
  - GPGUninstaller.exe

## 卸载程序使用方法

### 方式 1：交互式卸载
运行已安装目录中的 `GPGUninstaller.exe`，程序会提示确认：
```
Are you sure you want to uninstall? (y/n):
```

### 方式 2：静默卸载
通过命令行参数进行静默卸载（无需确认）：
```cmd
GPGUninstaller.exe /S
```
或
```cmd
GPGUninstaller.exe /silent
```

### 方式 3：通过 Windows 控制面板
1. 打开"设置" → "应用" → "已安装的应用"
2. 找到"GPG Application"
3. 点击"卸载"

### 卸载程序功能
- 删除所有已安装的文件
- 删除注册表卸载条目
- 如果安装目录为空，则删除该目录

## 文件清单

安装后的文件结构：
```
C:\Program Files\GPG\
├── GPGLauncher.exe      # 启动器
├── GPG.exe              # 主程序
├── GPGUninstaller.exe   # 卸载程序
└── uninstall_info.txt   # 卸载信息（文件列表）
```

## 开发说明

### installer.cpp 主要功能
- `GetInstallerDirectory()` - 获取安装程序所在目录
- `CreateDirectoryRecursive()` - 递归创建目录
- `CopyFileWithCheck()` - 复制文件并验证
- `CreateUninstallInfo()` - 创建卸载信息文件
- `AddUninstallRegistry()` - 添加注册表项

### uninstaller.cpp 主要功能
- `GetUninstallerDirectory()` - 获取卸载程序所在目录
- `ReadUninstallInfo()` - 读取卸载信息文件
- `DeleteFileWithCheck()` - 删除文件并验证
- `RemoveDirectoryIfEmpty()` - 删除空目录
- `RemoveUninstallRegistry()` - 删除注册表项

### 注册表位置
```
HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Uninstall\GPG Application
```

注册表键值：
- `DisplayName` - 显示名称
- `UninstallString` - 卸载程序路径
- `InstallLocation` - 安装位置

## 许可证

根据项目需求添加许可证信息。

@echo off
REM GPG 应用程序代码签名脚本

echo ========================================
echo   GPG Code Signing Script
echo ========================================
echo.

REM 配置区域 - 根据你的实际情况修改这些值
REM ================================================

REM signtool.exe 路径
set SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe"

REM 证书配置 - 选择一种方式，注释掉另一种

REM 方式 1: 使用 PFX 证书文件
set CERT_FILE=C:\GPGCodeSign.pfx
set CERT_PASSWORD=MyPassword123
set SIGN_PARAMS=/f "%CERT_FILE%" /p "%CERT_PASSWORD%"

REM 方式 2: 使用证书存储中的证书（如果使用这种方式，注释掉上面3行，取消下面1行注释）
REM set SIGN_PARAMS=/n "GPG Development"

REM 时间戳服务器（备选）
set TIMESTAMP_URL=http://timestamp.digicert.com
REM set TIMESTAMP_URL=http://timestamp.comodoca.com
REM set TIMESTAMP_URL=http://timestamp.sectigo.com

REM 构建配置（Debug 或 Release）
set BUILD_CONFIG=Debug

REM ================================================

REM 设置输出目录
set OUTPUT_DIR=%~dp0bin\%BUILD_CONFIG%

echo Configuration:
echo   Build: %BUILD_CONFIG%
echo   Output: %OUTPUT_DIR%
echo   Timestamp: %TIMESTAMP_URL%
echo.

REM 检查 signtool 是否存在
if not exist %SIGNTOOL% (
    echo ERROR: signtool.exe not found at %SIGNTOOL%
    echo Please install Windows SDK or update SIGNTOOL path
    pause
    exit /b 1
)

REM 检查输出目录是否存在
if not exist "%OUTPUT_DIR%" (
    echo ERROR: Output directory not found: %OUTPUT_DIR%
    echo Please build the solution first
    pause
    exit /b 1
)

REM 要签名的文件列表
set FILES=GPG.exe GPGLauncher.exe GPGInstaller.exe GPGUninstaller.exe

echo Starting code signing...
echo.

REM 签名每个文件
for %%f in (%FILES%) do (
    echo Signing: %%f

    if not exist "%OUTPUT_DIR%\%%f" (
        echo   WARNING: %%f not found, skipping
        echo.
    ) else (
        %SIGNTOOL% sign %SIGN_PARAMS% /t %TIMESTAMP_URL% /fd SHA256 /v "%OUTPUT_DIR%\%%f"

        if errorlevel 1 (
            echo   ERROR: Failed to sign %%f
            echo.
            pause
            exit /b 1
        ) else (
            echo   SUCCESS: %%f signed
            echo.
        )
    )
)

echo ========================================
echo   All files signed successfully!
echo ========================================
echo.

REM 验证签名
echo Verifying signatures...
echo.

for %%f in (%FILES%) do (
    if exist "%OUTPUT_DIR%\%%f" (
        echo Verifying: %%f
        %SIGNTOOL% verify /pa /v "%OUTPUT_DIR%\%%f"
        echo.
    )
)

echo Done!
pause

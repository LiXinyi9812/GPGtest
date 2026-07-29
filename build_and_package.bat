@echo off
setlocal enabledelayedexpansion
echo ========================================
echo Building GPG Project and Creating WAB
echo ========================================
echo.

REM ================================================
REM 配置区域 - 根据你的实际情况修改这些值
REM ================================================

REM Visual Studio MSBuild 路径
set MSBUILD="C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe"

REM signtool.exe 路径
set SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe"

REM 证书配置 - 选择一种方式
REM 方式 1: 使用 PFX 证书文件
set CERT_FILE=C:\GPGCodeSign.pfx
set CERT_PASSWORD=MyPassword123
set SIGN_PARAMS=/f "%CERT_FILE%" /p "%CERT_PASSWORD%"

REM 方式 2: 使用证书存储中的证书（如果使用这种方式，注释掉上面3行，取消下面1行注释）
REM set SIGN_PARAMS=/n "GPG Development"

REM 时间戳服务器
set TIMESTAMP_URL=http://timestamp.digicert.com

REM 构建配置（Debug 或 Release）
set BUILD_CONFIG=Debug

REM 是否启用代码签名（YES 或 NO）
set ENABLE_SIGNING=YES

REM ================================================

REM 检查 MSBuild 是否存在
if not exist %MSBUILD% (
    echo ERROR: MSBuild not found at %MSBUILD%
    echo Please update the path in this script to match your Visual Studio installation.
    pause
    exit /b 1
)

REM 检查是否需要签名
if /I "%ENABLE_SIGNING%"=="YES" (
    if not exist %SIGNTOOL% (
        echo WARNING: signtool.exe not found at %SIGNTOOL%
        echo Code signing will be skipped.
        echo If you need signing, please install Windows SDK or update SIGNTOOL path.
        set ENABLE_SIGNING=NO
        timeout /t 5
    )
)

echo Step 1: Incrementing version number...
set CONFIG_FILE=play_publishing_config.xml

REM 读取当前版本号
for /f "tokens=2 delims=<>" %%a in ('findstr /r "<version-name>" %CONFIG_FILE%') do set CURRENT_VERSION=%%a
echo Current version: %CURRENT_VERSION%

REM 分解版本号 (major.minor.patch)
for /f "tokens=1,2,3 delims=." %%a in ("%CURRENT_VERSION%") do (
    set MAJOR=%%a
    set MINOR=%%b
    set PATCH=%%c
)

REM 递增 patch 版本号
set /a PATCH+=1
set NEW_VERSION=%MAJOR%.%MINOR%.%PATCH%
echo New version: %NEW_VERSION%

REM 更新 XML 文件中的版本号
powershell -Command "(Get-Content '%CONFIG_FILE%') -replace '<version-name>.*</version-name>', '<version-name>%NEW_VERSION%</version-name>' | Set-Content '%CONFIG_FILE%'"
echo Version updated successfully.
echo.

echo Step 2: Cleaning previous build...
%MSBUILD% GPG.sln /t:Clean /p:Configuration=%BUILD_CONFIG% /p:Platform=x64
if errorlevel 1 goto :error

echo.
echo Step 3: Building all projects...
%MSBUILD% GPG.sln /t:Build /p:Configuration=%BUILD_CONFIG% /p:Platform=x64
if errorlevel 1 goto :error

echo.
echo Step 4: Verifying output files...
cd bin\%BUILD_CONFIG%
if not exist GPGInstaller.exe (
    echo ERROR: GPGInstaller.exe not found
    goto :error
)
if not exist GPGLauncher.exe (
    echo ERROR: GPGLauncher.exe not found
    goto :error
)
if not exist GPG.exe (
    echo ERROR: GPG.exe not found
    goto :error
)
if not exist play_pc_sdk.dll (
    echo ERROR: play_pc_sdk.dll not found
    goto :error
)
echo All required files are present.

REM ================================================
REM 代码签名步骤
REM ================================================
if /I "%ENABLE_SIGNING%"=="YES" (
    echo.
    echo Step 5: Code Signing...
    echo ----------------------------------------

    set FILES=GPG.exe GPGLauncher.exe GPGInstaller.exe GPGUninstaller.exe

    for %%f in (!FILES!) do (
        echo Signing: %%f

        if not exist "%%f" (
            echo   WARNING: %%f not found, skipping
        ) else (
            %SIGNTOOL% sign %SIGN_PARAMS% /t %TIMESTAMP_URL% /fd SHA256 /v "%%f"

            if errorlevel 1 (
                echo   ERROR: Failed to sign %%f
                cd ..\..
                goto :error
            ) else (
                echo   SUCCESS: %%f signed
            )
        )
        echo.
    )

    echo ----------------------------------------
    echo Code signing completed!
    echo.
) else (
    echo.
    echo Step 5: Code Signing... SKIPPED
    echo.
)

echo.
echo Step 6: Removing old WAB package...
if exist GPG.wab del /Q GPG.wab

echo.
echo Step 6.5: Copying config file to output directory...
copy /Y ..\..\play_publishing_config.xml play_publishing_config.xml
echo.
echo Step 7: Building WAB package...
play_publishing_tool_1.0.3.exe build-installer-bundle --input=play_publishing_config.xml --output=GPG.wab
if errorlevel 1 goto :error
cd ..\..
echo.
echo ========================================
echo SUCCESS!
echo ========================================
echo Build Configuration: %BUILD_CONFIG%
if /I "%ENABLE_SIGNING%"=="YES" (
    echo Code Signing: ENABLED
) else (
    echo Code Signing: DISABLED
)
echo.
echo WAB package created: bin\%BUILD_CONFIG%\GPG.wab
dir bin\%BUILD_CONFIG%\GPG.wab
echo.
echo Files in WAB package:
echo   - GPGInstaller.exe %~1
echo   - GPGLauncher.exe %~1
echo   - GPG.exe %~1
echo   - GPGUninstaller.exe %~1
echo   - play_pc_sdk.dll
if /I "%ENABLE_SIGNING%"=="YES" (
    echo   - All EXE files are digitally signed
)
echo.
echo You can now upload bin\%BUILD_CONFIG%\GPG.wab to Google Play Games PC
echo ========================================
pause
exit /b 0

:error
echo.
echo ========================================
echo BUILD FAILED!
echo ========================================
pause
exit /b 1

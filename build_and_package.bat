@echo off
setlocal enabledelayedexpansion
echo ========================================
echo Building GPG Project and Creating WAB
echo ========================================
echo.

REM ================================================
REM Configuration
REM ================================================

REM Visual Studio MSBuild path
set MSBUILD="C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe"

REM signtool.exe path
set SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe"

REM Certificate config - PFX file
set CERT_FILE=C:\GPGCodeSign.pfx
set CERT_PASSWORD=MyPassword123
set SIGN_PARAMS=/f "%CERT_FILE%" /p "%CERT_PASSWORD%"

REM Timestamp server
set TIMESTAMP_URL=http://timestamp.digicert.com

REM Build config (Debug or Release)
set BUILD_CONFIG=Debug

REM Enable code signing (YES or NO)
set ENABLE_SIGNING=YES

REM ================================================

REM Check MSBuild
if not exist %MSBUILD% (
    echo ERROR: MSBuild not found at %MSBUILD%
    pause
    exit /b 1
)

REM Check signtool
if /I "%ENABLE_SIGNING%"=="YES" (
    if not exist %SIGNTOOL% (
        echo WARNING: signtool.exe not found at %SIGNTOOL%
        echo Code signing will be skipped.
        set ENABLE_SIGNING=NO
        timeout /t 5
    )
)

echo.
echo Step 1: Cleaning previous build...
%MSBUILD% GPG.sln /t:Clean /p:Configuration=%BUILD_CONFIG% /p:Platform=x64
if errorlevel 1 goto :error

echo.
echo Step 2: Building GPGUninstaller first (dependency for Installer)...
%MSBUILD% GPGUninstaller.vcxproj /t:Build /p:Configuration=%BUILD_CONFIG% /p:Platform=x64
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
if not exist game.exe (
    echo ERROR: game.exe not found
    goto :error
)
if not exist play_pc_sdk.dll (
    echo ERROR: play_pc_sdk.dll not found
    goto :error
)
echo All required files are present.

REM ================================================
REM Code Signing
REM ================================================
if /I "%ENABLE_SIGNING%"=="YES" (
    echo.
    echo Step 5: Code Signing...
    echo ----------------------------------------

    set FILES=game.exe GPGLauncher.exe GPGInstaller.exe GPGUninstaller.exe

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
echo Step 7: Copying config file to output directory...
copy /Y ..\..\play_publishing_config.xml play_publishing_config.xml

echo.
echo Step 8: Building WAB package...
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

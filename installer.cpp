#include <windows.h>
#include <shlobj.h>
#include <iostream>
#include <string>
#include <fstream>
#include "installer_resources.h"

// 默认安装目录
static const wchar_t* DEFAULT_INSTALL_DIR = L"C:\\Program Files\\GPG";
static const wchar_t* APP_NAME = L"GPG Application";

// 从嵌入资源中提取文件到目标路径
bool ExtractResource(int resourceId, const std::wstring& destPath) {
    HRSRC hRes = FindResourceW(nullptr, MAKEINTRESOURCEW(resourceId), RT_RCDATA);
    if (!hRes) {
        std::wcerr << L"Failed to find resource " << resourceId << std::endl;
        return false;
    }

    HGLOBAL hData = LoadResource(nullptr, hRes);
    if (!hData) {
        std::wcerr << L"Failed to load resource " << resourceId << std::endl;
        return false;
    }

    DWORD size = SizeofResource(nullptr, hRes);
    void* pData = LockResource(hData);
    if (!pData || size == 0) {
        std::wcerr << L"Failed to lock resource " << resourceId << std::endl;
        return false;
    }

    HANDLE hFile = CreateFileW(destPath.c_str(), GENERIC_WRITE, 0, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::wcerr << L"Failed to create file: " << destPath
                   << L" (Error: " << GetLastError() << L")" << std::endl;
        return false;
    }

    DWORD written = 0;
    BOOL writeOk = WriteFile(hFile, pData, size, &written, nullptr);
    CloseHandle(hFile);

    if (!writeOk || written != size) {
        std::wcerr << L"Failed to write file: " << destPath << std::endl;
        return false;
    }

    std::wcout << L"Extracted: " << destPath << std::endl;
    return true;
}

// 创建目录（如果不存在）
bool CreateDirectoryRecursive(const std::wstring& path) {
    DWORD attrs = GetFileAttributesW(path.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        return true; // 目录已存在
    }

    size_t pos = path.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        std::wstring parent = path.substr(0, pos);
        if (!CreateDirectoryRecursive(parent)) {
            return false;
        }
    }

    return CreateDirectoryW(path.c_str(), nullptr) != 0;
}

// 创建卸载信息文件
bool CreateUninstallInfo(const std::wstring& installDir) {
    std::wstring uninstallInfoPath = installDir + L"\\uninstall_info.txt";
    std::wofstream file(uninstallInfoPath);
    if (!file.is_open()) {
        std::wcerr << L"Failed to create uninstall info file" << std::endl;
        return false;
    }

    file << L"GPGLauncher.exe" << std::endl;
    file << L"GPG.exe" << std::endl;
    file << L"GPGUninstaller.exe" << std::endl;
    file << L"play_pc_sdk.dll" << std::endl;
    file << L"manifest.xml" << std::endl;
    file << L"uninstall_info.txt" << std::endl;

    file.close();
    std::wcout << L"Created uninstall info file" << std::endl;
    return true;
}

// 添加到注册表（卸载程序信息）
bool AddUninstallRegistry(const std::wstring& installDir) {
    HKEY hKey;
    std::wstring regPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + std::wstring(APP_NAME);

    LONG result = RegCreateKeyExW(
        HKEY_CURRENT_USER,
        regPath.c_str(),
        0,
        nullptr,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        nullptr,
        &hKey,
        nullptr
    );

    if (result != ERROR_SUCCESS) {
        std::wcerr << L"Failed to create registry key" << std::endl;
        return false;
    }

    // 设置显示名称
    RegSetValueExW(hKey, L"DisplayName", 0, REG_SZ,
        (const BYTE*)APP_NAME, (DWORD)((wcslen(APP_NAME) + 1) * sizeof(wchar_t)));

    // 设置卸载程序路径
    std::wstring uninstallerPath = installDir + L"\\GPGUninstaller.exe";
    RegSetValueExW(hKey, L"UninstallString", 0, REG_SZ,
        (const BYTE*)uninstallerPath.c_str(), (DWORD)((uninstallerPath.length() + 1) * sizeof(wchar_t)));

    // 设置安装位置
    RegSetValueExW(hKey, L"InstallLocation", 0, REG_SZ,
        (const BYTE*)installDir.c_str(), (DWORD)((installDir.length() + 1) * sizeof(wchar_t)));

    RegCloseKey(hKey);
    std::wcout << L"Added uninstall registry entry" << std::endl;
    return true;
}

int wmain(int argc, wchar_t* argv[]) {
    std::wcout << L"========================================" << std::endl;
    std::wcout << L"  GPG Application Installer" << std::endl;
    std::wcout << L"========================================" << std::endl;
    std::wcout << std::endl;

    // 确定安装目标目录
    std::wstring installDir;
    if (argc > 1) {
        installDir = argv[1];
    } else {
        installDir = DEFAULT_INSTALL_DIR;
    }

    std::wcout << L"Install directory: " << installDir << std::endl;
    std::wcout << std::endl;

    // 创建安装目录
    if (!CreateDirectoryRecursive(installDir)) {
        std::wcerr << L"Failed to create install directory" << std::endl;
        system("pause");
        return 1;
    }

    // 从嵌入资源中提取文件
    std::wcout << L"Extracting files..." << std::endl;

    bool success = true;
    success &= ExtractResource(IDR_FILE_LAUNCHER,    installDir + L"\\GPGLauncher.exe");
    success &= ExtractResource(IDR_FILE_GPG,         installDir + L"\\GPG.exe");
    success &= ExtractResource(IDR_FILE_UNINSTALLER, installDir + L"\\GPGUninstaller.exe");
    success &= ExtractResource(IDR_FILE_SDK_DLL,     installDir + L"\\play_pc_sdk.dll");
    success &= ExtractResource(IDR_FILE_MANIFEST,    installDir + L"\\manifest.xml");

    if (!success) {
        std::wcerr << L"Failed to extract all files" << std::endl;
        system("pause");
        return 1;
    }

    // 创建卸载信息文件
    if (!CreateUninstallInfo(installDir)) {
        std::wcerr << L"Warning: Failed to create uninstall info" << std::endl;
    }

    // 添加注册表项
    if (!AddUninstallRegistry(installDir)) {
        std::wcerr << L"Warning: Failed to add registry entry" << std::endl;
    }

    std::wcout << std::endl;
    std::wcout << L"========================================" << std::endl;
    std::wcout << L"  Installation completed successfully!" << std::endl;
    std::wcout << L"========================================" << std::endl;
    std::wcout << std::endl;
    std::wcout << L"Press any key to exit..." << std::endl;
    std::wcin.get();

    return 0;
}

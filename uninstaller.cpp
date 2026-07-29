#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

static const wchar_t* APP_NAME = L"GPG Application";

// 获取当前可执行文件所在目录
std::wstring GetUninstallerDirectory() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring dir(path);
    size_t pos = dir.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        dir = dir.substr(0, pos);
    }
    return dir;
}

// 读取卸载信息文件
std::vector<std::wstring> ReadUninstallInfo(const std::wstring& installDir) {
    std::vector<std::wstring> files;
    std::wstring uninstallInfoPath = installDir + L"\\uninstall_info.txt";

    std::wifstream file(uninstallInfoPath);
    if (!file.is_open()) {
        std::wcerr << L"Warning: Could not open uninstall info file" << std::endl;
        // 返回默认文件列表
        files.push_back(L"GPGLauncher.exe");
        files.push_back(L"GPG.exe");
        files.push_back(L"GPGUninstaller.exe");
        files.push_back(L"uninstall_info.txt");
        return files;
    }

    std::wstring line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            files.push_back(line);
        }
    }

    file.close();
    return files;
}

// 删除文件
bool DeleteFileWithCheck(const std::wstring& filePath) {
    if (DeleteFileW(filePath.c_str())) {
        std::wcout << L"Deleted: " << filePath << std::endl;
        return true;
    } else {
        DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND) {
            std::wcout << L"File not found (skipped): " << filePath << std::endl;
            return true;
        } else {
            std::wcerr << L"Failed to delete " << filePath << L" (Error: " << error << L")" << std::endl;
            return false;
        }
    }
}

// 删除目录（如果为空）
bool RemoveDirectoryIfEmpty(const std::wstring& dirPath) {
    if (RemoveDirectoryW(dirPath.c_str())) {
        std::wcout << L"Removed directory: " << dirPath << std::endl;
        return true;
    } else {
        DWORD error = GetLastError();
        if (error == ERROR_DIR_NOT_EMPTY) {
            std::wcout << L"Directory not empty (kept): " << dirPath << std::endl;
        } else {
            std::wcerr << L"Failed to remove directory (Error: " << error << L")" << std::endl;
        }
        return false;
    }
}

// 从注册表删除卸载信息
bool RemoveUninstallRegistry() {
    std::wstring regPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + std::wstring(APP_NAME);

    LONG result = RegDeleteKeyW(HKEY_CURRENT_USER, regPath.c_str());

    if (result == ERROR_SUCCESS) {
        std::wcout << L"Removed registry entry" << std::endl;
        return true;
    } else if (result == ERROR_FILE_NOT_FOUND) {
        std::wcout << L"Registry entry not found (skipped)" << std::endl;
        return true;
    } else {
        std::wcerr << L"Failed to remove registry entry (Error: " << result << L")" << std::endl;
        return false;
    }
}

// 确认卸载
bool ConfirmUninstall() {
    std::wcout << L"Are you sure you want to uninstall? (y/n): ";
    wchar_t response;
    std::wcin >> response;
    return (response == L'y' || response == L'Y');
}

int wmain(int argc, wchar_t* argv[]) {
    std::wcout << L"========================================" << std::endl;
    std::wcout << L"  GPG Application Uninstaller" << std::endl;
    std::wcout << L"========================================" << std::endl;
    std::wcout << std::endl;

    // 获取安装目录（从当前卸载程序所在位置）
    std::wstring installDir = GetUninstallerDirectory();
    std::wcout << L"Install directory: " << installDir << std::endl;
    std::wcout << std::endl;

    // 静默模式检查（命令行参数 /S 或 /silent）
    bool silent = false;
    if (argc > 1) {
        std::wstring arg = argv[1];
        if (arg == L"/S" || arg == L"/silent") {
            silent = true;
        }
    }

    // 确认卸载
    if (!silent && !ConfirmUninstall()) {
        std::wcout << L"Uninstallation cancelled." << std::endl;
        return 0;
    }

    std::wcout << L"Starting uninstallation..." << std::endl;
    std::wcout << std::endl;

    // 读取要删除的文件列表
    std::vector<std::wstring> filesToDelete = ReadUninstallInfo(installDir);

    // 删除文件
    std::wcout << L"Removing files..." << std::endl;
    bool allDeleted = true;
    for (const auto& fileName : filesToDelete) {
        std::wstring filePath = installDir + L"\\" + fileName;
        if (!DeleteFileWithCheck(filePath)) {
            allDeleted = false;
        }
    }

    // 删除注册表项
    RemoveUninstallRegistry();

    // 尝试删除安装目录（如果为空）
    std::wcout << std::endl;
    RemoveDirectoryIfEmpty(installDir);

    std::wcout << std::endl;
    if (allDeleted) {
        std::wcout << L"========================================" << std::endl;
        std::wcout << L"  Uninstallation completed successfully!" << std::endl;
        std::wcout << L"========================================" << std::endl;
    } else {
        std::wcout << L"========================================" << std::endl;
        std::wcout << L"  Uninstallation completed with warnings" << std::endl;
        std::wcout << L"========================================" << std::endl;
    }

    if (!silent) {
        std::wcout << std::endl;
        std::wcout << L"Press any key to exit..." << std::endl;
        std::wcin.ignore();
        std::wcin.get();
    }

    return 0;
}

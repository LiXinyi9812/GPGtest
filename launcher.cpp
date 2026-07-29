#include <iostream>
#include <future>
#include <string>
#include <vector>
#include <windows.h>
#include "initialization/initialization.h"

using google::play::initialization::GooglePlayInitialize;
using google::play::initialization::InitializeResult;
using google::play::initialization::InitializationError;

// 游戏可执行文件名（与 launcher 位于同一目录）
static constexpr const wchar_t* GAME_EXECUTABLE = L"GPG.exe";

// 构建子进程命令行，转发所有从 GooglePlayGames.exe 收到的参数
std::wstring BuildChildCommandLine(const wchar_t* gameExe, int argc, wchar_t* argv[]) {
    std::wstring cmdLine = L"\"";
    cmdLine += gameExe;
    cmdLine += L"\"";

    // 转发所有参数（跳过 argv[0]，即 launcher 自身路径）
    for (int i = 1; i < argc; ++i) {
        cmdLine += L" ";
        cmdLine += argv[i];
    }
    return cmdLine;
}

// 获取 launcher 所在目录
std::wstring GetLauncherDirectory() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring dir(path);
    size_t pos = dir.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        dir = dir.substr(0, pos + 1);
    }
    return dir;
}

int wmain(int argc, wchar_t* argv[]) {
    // 第 1 步：验证 Play Games PC SDK 初始化
    std::cout << "[Launcher] Initializing Play Games PC SDK..." << std::endl;

    auto promise = std::make_shared<std::promise<InitializeResult>>();
    GooglePlayInitialize(
        [promise](InitializeResult result) {
            promise->set_value(std::move(result));
        });

    auto initResult = promise->get_future().get();

    if (!initResult.ok()) {
        if (initResult.code() == InitializationError::kActionRequiredShutdownClientProcess) {
            std::cerr << "[Launcher] SDK requested immediate shutdown." << std::endl;
        } else {
            std::cerr << "[Launcher] SDK initialization failed, error code: "
                      << static_cast<int>(initResult.code())
                      << ", message: " << initResult.error_message()
                      << std::endl;
        }
        system("pause");
        return 1;
    }

    std::cout << "[Launcher] SDK initialized successfully." << std::endl;

    // 第 2 步：构建子进程命令行，转发所有命令行参数
    std::wstring launcherDir = GetLauncherDirectory();
    std::wstring gameFullPath = launcherDir + GAME_EXECUTABLE;
    std::wstring cmdLine = BuildChildCommandLine(gameFullPath.c_str(), argc, argv);

    std::wcout << L"[Launcher] Starting game process: " << cmdLine << std::endl;

    // 第 3 步：创建子进程
    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    // 使用可写缓冲区（CreateProcessW 要求 lpCommandLine 可写）
    std::vector<wchar_t> cmdLineBuf(cmdLine.begin(), cmdLine.end());
    cmdLineBuf.push_back(L'\0');

    BOOL success = CreateProcessW(
        gameFullPath.c_str(),   // 应用程序路径
        cmdLineBuf.data(),      // 命令行（包含所有转发的参数）
        nullptr,                // 进程安全属性
        nullptr,                // 线程安全属性
        FALSE,                  // 不继承句柄
        0,                      // 创建标志
        nullptr,                // 使用父进程环境变量
        launcherDir.c_str(),    // 工作目录
        &si,
        &pi
    );

    if (!success) {
        std::cerr << "[Launcher] Failed to start game process. Error: "
                  << GetLastError() << std::endl;
        system("pause");
        return 1;
    }

    std::cout << "[Launcher] Game process started (PID: " << pi.dwProcessId << ")." << std::endl;

    // 第 4 步：等待游戏进程退出
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    std::cout << "[Launcher] Game process exited with code: " << exitCode << std::endl;

    // 第 5 步：清理句柄，确保所有进程退出
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return static_cast<int>(exitCode);
}
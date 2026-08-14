#include <iostream>
#include <sstream>
#include <future>
#include <string>
#include <memory>
#include <chrono>
#include <vector>
#include <windows.h>
#include <winhttp.h>
#include <wincrypt.h>
#include <commctrl.h>
#include "initialization/initialization.h"
#include "billing/client.h"
#include "billing/models.h"
#include "billing/enums.h"
#include "integrity/client.h"
#include "integrity/models.h"

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "comctl32.lib")

// 启用 ComCtl32 v6 (TaskDialog 必须)
#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define LOG(msg) do { \
    auto now = std::chrono::system_clock::now(); \
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 100000; \
    std::ostringstream oss; \
    oss << "[" << ms << "][tid:" << GetCurrentThreadId() << "] "; \
    oss << (msg); \
    oss << "\n"; \
    OutputDebugStringA(oss.str().c_str()); \
} while(0)

using namespace google::play::initialization;
using namespace google::play::billing;
using namespace google::play::integrity;

// ============================================================
// 后端服务器配置
// ============================================================
static const wchar_t* BILLING_SERVER_HOST = L"localhost";
static const int BILLING_SERVER_PORT = 3000;
static const char* API_SECRET_KEY = "gpg_billing_secret_key_2024";
static const char* PRODUCT_ID = "100_coins";
static const int64_t CLOUD_PROJECT_NUMBER = 177692096238; // TODO: 填入你的 Google Cloud 项目编号

std::unique_ptr<BillingClient> g_billing_client;
std::unique_ptr<IntegrityClient> g_integrity_client;
std::unique_ptr<PrepareIntegrityTokenResultValue> g_prepare_value; // 缓存的预热结果
bool g_integrity_ready = false;
HWND g_gameWindow = nullptr;
int g_total_coins = 0;  // 当前金币余额
int g_total_score = 0;  // 当前分数
std::string g_device_id; // 设备唯一ID (基于 Windows MachineGuid 生成)

// ============================================================
// SHA256 工具函数 (用于生成 request_hash)
// ============================================================

std::string ComputeSHA256(const std::string& data) {
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    std::string result;

    if (!CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        return "";
    }

    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        return "";
    }

    if (!CryptHashData(hHash, (const BYTE*)data.c_str(), (DWORD)data.size(), 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return "";
    }

    DWORD hashLen = 32;
    BYTE hashValue[32];
    if (CryptGetHashParam(hHash, HP_HASHVAL, hashValue, &hashLen, 0)) {
        static const char hex[] = "0123456789abcdef";
        for (DWORD i = 0; i < hashLen; ++i) {
            result += hex[hashValue[i] >> 4];
            result += hex[hashValue[i] & 0x0F];
        }
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    return result;
}

// ============================================================
// 设备唯一ID: 基于 Windows MachineGuid 生成稳定哈希
// ============================================================

std::string GetDeviceId() {
    // 从注册表读取 MachineGuid (每台 Windows 安装唯一，重装系统才会变)
    HKEY hKey = nullptr;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        LOG("WARNING: Cannot open Cryptography registry key, using fallback.");
        return ComputeSHA256("fallback-device-id");
    }

    char guidBuf[256] = {};
    DWORD bufSize = sizeof(guidBuf);
    DWORD type = 0;
    LONG result = RegQueryValueExA(hKey, "MachineGuid", nullptr, &type,
        (LPBYTE)guidBuf, &bufSize);
    RegCloseKey(hKey);

    if (result != ERROR_SUCCESS || type != REG_SZ) {
        LOG("WARNING: Cannot read MachineGuid, using fallback.");
        return ComputeSHA256("fallback-device-id");
    }

    // 对 MachineGuid 做 SHA256，作为设备ID (避免直接暴露原始 GUID)
    std::string machineGuid(guidBuf);
    std::string deviceId = ComputeSHA256("gpg-device:" + machineGuid);

    std::ostringstream log_msg;
    log_msg << "Device ID: " << deviceId.substr(0, 16) << "...";
    LOG(log_msg.str().c_str());

    return deviceId;
}

// ============================================================
// 前向声明: 带消息泵的等待
// ============================================================
template<typename T>
T WaitWithMessagePump(std::future<T>& future);

// ============================================================
// Play Integrity: 预热 & 请求令牌
// ============================================================

bool PrepareIntegrity() {
    LOG("Preparing Play Integrity token...");

    PrepareIntegrityTokenParams params;
    params.cloud_project_number = CLOUD_PROJECT_NUMBER;

    auto promise = std::make_shared<std::promise<PrepareIntegrityTokenResult>>();
    auto future = promise->get_future();
    g_integrity_client->PrepareIntegrityToken(
        params,
        [promise](PrepareIntegrityTokenResult result) {
            promise->set_value(std::move(result));
        });

    auto result = WaitWithMessagePump(future);
    if (!result.ok()) {
        std::ostringstream log_msg;
        log_msg << "ERROR: PrepareIntegrityToken failed, code=" << static_cast<int>(result.code());
        LOG(log_msg.str().c_str());
        return false;
    }

    // 缓存 PrepareIntegrityTokenResultValue，在整个会话中复用
    g_prepare_value = std::make_unique<PrepareIntegrityTokenResultValue>(result.value());
    g_integrity_ready = true;
    LOG("Play Integrity prepared successfully.");
    return true;
}

/**
 * 请求完整性令牌，绑定到特定请求的 hash
 * @param request_hash 请求体的 SHA256 摘要
 * @return integrity_token 字符串，失败返回空
 */
std::string RequestIntegrityToken(const std::string& request_hash) {
    if (!g_integrity_ready) {
        LOG("WARNING: Integrity not prepared, skipping.");
        return "";
    }

    RequestIntegrityTokenParams params{g_prepare_value->request_token_data, request_hash};

    auto promise = std::make_shared<std::promise<RequestIntegrityTokenResult>>();
    auto future = promise->get_future();
    g_integrity_client->RequestIntegrityToken(
        params,
        [promise](RequestIntegrityTokenResult result) {
            promise->set_value(std::move(result));
        });

    auto result = WaitWithMessagePump(future);
    if (!result.ok()) {
        std::ostringstream log_msg;
        log_msg << "ERROR: RequestIntegrityToken failed, code=" << static_cast<int>(result.code());
        LOG(log_msg.str().c_str());
        return "";
    }

    LOG("Integrity token obtained.");
    return result.value().token_bytes;
}

// ============================================================
// HTTP 工具: 发送 GET 请求查询金币余额
// ============================================================

int QueryCoinsFromServer() {
    // 生成 request_hash (GET 请求无 body，使用路径作为哈希输入)
    std::string request_data = "GET /api/coins";
    std::string request_hash = ComputeSHA256(request_data);
    std::string integrity_token = RequestIntegrityToken(request_hash);

    HINTERNET hSession = WinHttpOpen(L"GPG-BillingClient/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return -1;

    HINTERNET hConnect = WinHttpConnect(hSession, BILLING_SERVER_HOST,
        BILLING_SERVER_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return -1; }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET",
        L"/api/coins", NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return -1;
    }

    // 设置鉴权头 + 设备ID头 + Integrity 头
    std::wstring headers = L"x-api-key: ";
    std::wstring wApiKey(API_SECRET_KEY, API_SECRET_KEY + strlen(API_SECRET_KEY));
    headers += wApiKey;
    headers += L"\r\n";

    // 设备ID
    headers += L"x-device-id: ";
    std::wstring wDeviceId(g_device_id.begin(), g_device_id.end());
    headers += wDeviceId;
    headers += L"\r\n";

    // 添加完整性令牌头
    if (!integrity_token.empty()) {
        headers += L"x-integrity-token: ";
        std::wstring wToken(integrity_token.begin(), integrity_token.end());
        headers += wToken;
        headers += L"\r\n";
        headers += L"x-request-hash: ";
        std::wstring wHash(request_hash.begin(), request_hash.end());
        headers += wHash;
        headers += L"\r\n";
    }

    BOOL bResult = WinHttpSendRequest(hRequest, headers.c_str(),
        (DWORD)headers.length(), WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

    if (!bResult || !WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return -1;
    }

    // 读取响应
    std::string response;
    DWORD dwSize = 0, dwDownloaded = 0;
    do {
        WinHttpQueryDataAvailable(hRequest, &dwSize);
        if (dwSize == 0) break;
        std::vector<char> buffer(dwSize + 1, 0);
        WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded);
        response.append(buffer.data(), dwDownloaded);
    } while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    // 解析 {"coins": N}
    int coins = 0;
    size_t pos = response.find("\"coins\":");
    if (pos != std::string::npos) {
        pos += 8;
        size_t end = response.find_first_not_of("0123456789 ", pos);
        std::string num_str;
        for (size_t i = pos; i < response.size() && (isdigit(response[i]) || response[i] == ' '); ++i) {
            if (isdigit(response[i])) num_str += response[i];
        }
        if (!num_str.empty()) coins = std::stoi(num_str);
    }

    return coins;
}

// ============================================================
// HTTP 工具: 发送 GET 请求查询分数
// ============================================================

int QueryScoreFromServer() {
    HINTERNET hSession = WinHttpOpen(L"GPG-BillingClient/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return -1;

    HINTERNET hConnect = WinHttpConnect(hSession, BILLING_SERVER_HOST,
        BILLING_SERVER_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return -1; }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET",
        L"/api/score", NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return -1;
    }

    // 设置鉴权头 + 设备ID头
    std::wstring headers = L"x-api-key: ";
    std::wstring wApiKey(API_SECRET_KEY, API_SECRET_KEY + strlen(API_SECRET_KEY));
    headers += wApiKey;
    headers += L"\r\n";
    headers += L"x-device-id: ";
    std::wstring wDeviceId(g_device_id.begin(), g_device_id.end());
    headers += wDeviceId;
    headers += L"\r\n";

    BOOL bResult = WinHttpSendRequest(hRequest, headers.c_str(),
        (DWORD)headers.length(), WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

    if (!bResult || !WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return -1;
    }

    // 读取响应
    std::string response;
    DWORD dwSize = 0, dwDownloaded = 0;
    do {
        WinHttpQueryDataAvailable(hRequest, &dwSize);
        if (dwSize == 0) break;
        std::vector<char> buffer(dwSize + 1, 0);
        WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded);
        response.append(buffer.data(), dwDownloaded);
    } while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    // 解析 {"score": N}
    int score = 0;
    size_t pos = response.find("\"score\":");
    if (pos != std::string::npos) {
        pos += 8;
        std::string num_str;
        for (size_t i = pos; i < response.size() && (isdigit(response[i]) || response[i] == ' '); ++i) {
            if (isdigit(response[i])) num_str += response[i];
        }
        if (!num_str.empty()) score = std::stoi(num_str);
    }

    return score;
}

// ============================================================
// HTTP 请求: 将 purchase_token 发送到后端验证
// ============================================================

struct ServerVerifyResult {
    bool success;
    std::string order_id;
    std::string error;
    int coins_added;
    int total_coins;
};

ServerVerifyResult VerifyPurchaseWithServer(const std::string& product_id,
                                           const std::string& purchase_token) {
    ServerVerifyResult result = { false, "", "", 0, 0 };

    // 构造 JSON 请求体
    std::ostringstream json_body;
    json_body << "{\"product_id\":\"" << product_id
              << "\",\"purchase_token\":\"" << purchase_token << "\"}";
    std::string body = json_body.str();

    // 计算 request_hash 并请求完整性令牌
    std::string request_hash = ComputeSHA256(body);
    std::string integrity_token = RequestIntegrityToken(request_hash);

    // 初始化 WinHTTP
    HINTERNET hSession = WinHttpOpen(L"GPG-BillingClient/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        result.error = "WinHttpOpen failed";
        return result;
    }

    HINTERNET hConnect = WinHttpConnect(hSession, BILLING_SERVER_HOST,
        BILLING_SERVER_PORT, 0);
    if (!hConnect) {
        result.error = "WinHttpConnect failed";
        WinHttpCloseHandle(hSession);
        return result;
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST",
        L"/api/verify-and-consume", NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!hRequest) {
        result.error = "WinHttpOpenRequest failed";
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    // 设置请求头 (含 设备ID + Integrity 令牌)
    std::wstring headers = L"Content-Type: application/json\r\n";
    headers += L"x-api-key: ";
    std::wstring wApiKey(API_SECRET_KEY, API_SECRET_KEY + strlen(API_SECRET_KEY));
    headers += wApiKey;
    headers += L"\r\n";
    headers += L"x-device-id: ";
    std::wstring wDeviceId(g_device_id.begin(), g_device_id.end());
    headers += wDeviceId;
    headers += L"\r\n";

    // 添加完整性令牌头
    if (!integrity_token.empty()) {
        headers += L"x-integrity-token: ";
        std::wstring wToken(integrity_token.begin(), integrity_token.end());
        headers += wToken;
        headers += L"\r\n";
        headers += L"x-request-hash: ";
        std::wstring wHash(request_hash.begin(), request_hash.end());
        headers += wHash;
        headers += L"\r\n";
    }

    BOOL bResult = WinHttpSendRequest(hRequest, headers.c_str(),
        (DWORD)headers.length(), (LPVOID)body.c_str(), (DWORD)body.length(),
        (DWORD)body.length(), 0);

    if (!bResult || !WinHttpReceiveResponse(hRequest, NULL)) {
        result.error = "HTTP request failed";
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    // 读取响应
    std::string response;
    DWORD dwSize = 0, dwDownloaded = 0;
    do {
        WinHttpQueryDataAvailable(hRequest, &dwSize);
        if (dwSize == 0) break;
        std::vector<char> buffer(dwSize + 1, 0);
        WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded);
        response.append(buffer.data(), dwDownloaded);
    } while (dwSize > 0);

    // 检查 HTTP 状态码
    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize,
        WINHTTP_NO_HEADER_INDEX);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    std::ostringstream log_msg;
    log_msg << "Backend response (HTTP " << statusCode << "): " << response;
    LOG(log_msg.str().c_str());

    if (statusCode == 200) {
        result.success = true;
        // 简单解析 order_id (生产环境建议用 JSON 库)
        size_t pos = response.find("\"order_id\":\"");
        if (pos != std::string::npos) {
            pos += 12;
            size_t end = response.find("\"", pos);
            if (end != std::string::npos) {
                result.order_id = response.substr(pos, end - pos);
            }
        }
        // 解析 coins_added
        pos = response.find("\"coins_added\":");
        if (pos != std::string::npos) {
            pos += 14;
            size_t end = response.find_first_not_of("0123456789", pos);
            if (end != std::string::npos) {
                result.coins_added = std::stoi(response.substr(pos, end - pos));
            }
        }
        // 解析 total_coins
        pos = response.find("\"total_coins\":");
        if (pos != std::string::npos) {
            pos += 14;
            size_t end = response.find_first_not_of("0123456789", pos);
            if (end != std::string::npos) {
                result.total_coins = std::stoi(response.substr(pos, end - pos));
            }
        }
    } else {
        result.error = "Backend rejected: HTTP " + std::to_string(statusCode);
    }

    return result;
}

// 带消息泵的等待，避免阻塞 UI 线程
template<typename T>
T WaitWithMessagePump(std::future<T>& future) {
    MSG msg;
    while (future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(10);
    }
    return future.get();
}

bool ProcessPurchaseWithBackend(const std::string& purchase_token) {
    std::ostringstream log_msg;
    log_msg << "Processing purchase, token: " << purchase_token.substr(0, 20) << "...";
    LOG(log_msg.str().c_str());

    LOG("Sending to backend for verification, acknowledge and consume...");
    auto result = VerifyPurchaseWithServer(PRODUCT_ID, purchase_token);

    if (!result.success) {
        std::ostringstream err_log;
        err_log << "Backend processing FAILED: " << result.error;
        LOG(err_log.str().c_str());
        return false;
    }

    // 更新本地金币余额
    g_total_coins = result.total_coins;

    std::ostringstream ok_log;
    ok_log << "Backend processed OK, order_id: " << result.order_id
           << ", +" << result.coins_added << " coins, total: " << result.total_coins;
    LOG(ok_log.str().c_str());

    // 显示购买成功提示
    std::ostringstream msg;
    msg << "Purchase successful!\n\n"
        << "+" << result.coins_added << " coins\n"
        << "Total coins: " << result.total_coins;
    MessageBoxA(g_gameWindow, msg.str().c_str(), "Purchase Complete", MB_OK | MB_ICONINFORMATION);

    return true;
}

void ConsumeExistingPurchases() {
    LOG("Querying existing purchases...");
    auto promise = std::make_shared<std::promise<QueryPurchasesResult>>();
    auto future = promise->get_future();
    g_billing_client->QueryPurchases(
        [promise](QueryPurchasesResult result) { promise->set_value(std::move(result)); });

    auto result = WaitWithMessagePump(future);
    if (!result.ok()) {
        std::ostringstream log_msg;
        log_msg << "ERROR: QueryPurchases failed, code=" << static_cast<int>(result.code());
        LOG(log_msg.str().c_str());
        return;
    }
    for (auto& p : result.value().product_purchase_details) {
        if (p.purchase_state == PurchaseState::kPurchaseStatePurchased)
            ProcessPurchaseWithBackend(p.purchase_token);
    }
}

// 返回值: 0=成功, 1=失败, 2=用户取消
int StartPurchaseFlow() {
    if (!g_billing_client) return 1;
    LOG("=== Starting purchase flow ===");

    // Step 1: 查询商品详情
    QueryProductDetailsParams query_params;
    ProductId pid;
    pid.id = "100_coins";
    pid.product_type = ProductType::kTypeInApp;
    query_params.product_ids.push_back(pid);

    auto qp = std::make_shared<std::promise<std::pair<std::string, std::string>>>();
    auto qf = qp->get_future();
    g_billing_client->QueryProductDetails(query_params,
        [qp](auto result) {
            if (!result.ok() || result.value().product_details.empty() ||
                result.value().product_details[0].product_offers.empty()) {
                qp->set_value({"", ""});
                return;
            }
            auto& p = result.value().product_details[0];
            auto& o = p.product_offers[0];
            std::ostringstream log_msg;
            log_msg << "Product: " << p.title << " Price: " << o.formatted_price;
            LOG(log_msg.str().c_str());
            qp->set_value({o.offer_token, p.title + "\n" + o.formatted_price});
        });

    auto offer_result = WaitWithMessagePump(qf);
    if (offer_result.first.empty()) {
        LOG("No valid offer, abort.");
        return 1;
    }
    std::string offer_token = offer_result.first;
    std::string display_info = offer_result.second;

    // Step 2: 确认购买
    std::string confirm_msg = display_info + "\n\nConfirm purchase?";
    if (MessageBoxA(g_gameWindow, confirm_msg.c_str(), "Confirm", MB_YESNO) != IDYES)
        return 2;

    // Step 3: 显示窗口供 overlay 附着
    ShowWindow(g_gameWindow, SW_SHOW);
    UpdateWindow(g_gameWindow);

    // Step 4: 发起购买
    auto pp = std::make_shared<std::promise<std::pair<int, std::string>>>();
    auto pf = pp->get_future();
    LaunchPurchaseFlowParams lp;
    lp.offer_token = offer_token;
    g_billing_client->LaunchPurchaseFlow(lp,
        [pp](auto r) {
            if (r.ok()) {
                auto& pd = r.value().product_purchase_details;
                std::ostringstream log_msg;
                log_msg << "Purchase OK! Order: " << pd.order_id;
                LOG(log_msg.str().c_str());
                pp->set_value({0, pd.purchase_token});
            } else {
                int code = (r.code() == BillingError::kUserCanceled) ? 2 : 1;
                pp->set_value({code, ""});
            }
        });

    auto purchase_result = WaitWithMessagePump(pf);
    int code = purchase_result.first;
    std::string token = purchase_result.second;
    if (code == 0 && !token.empty()) ProcessPurchaseWithBackend(token);
    ShowWindow(g_gameWindow, SW_HIDE);
    return code;
}

// ============================================================
// HTTP 请求: 发送 add-score 到后端 (受 Integrity 保护)
// ============================================================

struct AddScoreResult {
    bool success;
    std::string error;
    int score_added;
    int total_score;
};

AddScoreResult AddScoreToServer(int addscore) {
    AddScoreResult result = { false, "", 0, 0 };

    // 构造 JSON 请求体 (含时间戳，使每次请求的 hash 不同，防重放)
    auto now = std::chrono::system_clock::now();
    auto epoch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    std::ostringstream json_body;
    json_body << "{\"addscore\":" << addscore << ",\"timestamp\":" << epoch_ms << "}";
    std::string body = json_body.str();

    // 计算 request_hash 并请求完整性令牌
    std::string request_hash = ComputeSHA256(body);
    std::string integrity_token = RequestIntegrityToken(request_hash);

    // 初始化 WinHTTP
    HINTERNET hSession = WinHttpOpen(L"GPG-BillingClient/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) { result.error = "WinHttpOpen failed"; return result; }

    HINTERNET hConnect = WinHttpConnect(hSession, BILLING_SERVER_HOST,
        BILLING_SERVER_PORT, 0);
    if (!hConnect) {
        result.error = "WinHttpConnect failed";
        WinHttpCloseHandle(hSession);
        return result;
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST",
        L"/api/add-score", NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!hRequest) {
        result.error = "WinHttpOpenRequest failed";
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    // 设置请求头 (含 设备ID + Integrity 令牌)
    std::wstring headers = L"Content-Type: application/json\r\n";
    headers += L"x-api-key: ";
    std::wstring wApiKey(API_SECRET_KEY, API_SECRET_KEY + strlen(API_SECRET_KEY));
    headers += wApiKey;
    headers += L"\r\n";
    headers += L"x-device-id: ";
    std::wstring wDeviceId(g_device_id.begin(), g_device_id.end());
    headers += wDeviceId;
    headers += L"\r\n";

    if (!integrity_token.empty()) {
        headers += L"x-integrity-token: ";
        std::wstring wToken(integrity_token.begin(), integrity_token.end());
        headers += wToken;
        headers += L"\r\n";
    }

    BOOL bResult = WinHttpSendRequest(hRequest, headers.c_str(),
        (DWORD)headers.length(), (LPVOID)body.c_str(), (DWORD)body.length(),
        (DWORD)body.length(), 0);

    if (!bResult || !WinHttpReceiveResponse(hRequest, NULL)) {
        result.error = "HTTP request failed";
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return result;
    }

    // 读取响应
    std::string response;
    DWORD dwSize = 0, dwDownloaded = 0;
    do {
        WinHttpQueryDataAvailable(hRequest, &dwSize);
        if (dwSize == 0) break;
        std::vector<char> buffer(dwSize + 1, 0);
        WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded);
        response.append(buffer.data(), dwDownloaded);
    } while (dwSize > 0);

    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize,
        WINHTTP_NO_HEADER_INDEX);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    std::ostringstream log_msg;
    log_msg << "AddScore response (HTTP " << statusCode << "): " << response;
    LOG(log_msg.str().c_str());

    if (statusCode == 200) {
        result.success = true;
        // 解析 score_added
        size_t pos = response.find("\"score_added\":");
        if (pos != std::string::npos) {
            pos += 14;
            std::string num_str;
            for (size_t i = pos; i < response.size() && isdigit(response[i]); ++i)
                num_str += response[i];
            if (!num_str.empty()) result.score_added = std::stoi(num_str);
        }
        // 解析 total_score
        pos = response.find("\"total_score\":");
        if (pos != std::string::npos) {
            pos += 14;
            std::string num_str;
            for (size_t i = pos; i < response.size() && isdigit(response[i]); ++i)
                num_str += response[i];
            if (!num_str.empty()) result.total_score = std::stoi(num_str);
        }
    } else {
        result.error = "Backend rejected: HTTP " + std::to_string(statusCode);
    }

    return result;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    std::ostringstream start_log;
    start_log << "=== App starting, PID=" << GetCurrentProcessId() << " ===";
    LOG(start_log.str().c_str());

    // 创建隐藏窗口供 Google Play overlay 附着
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"GPGGameWindow";
    RegisterClassExW(&wc);
    g_gameWindow = CreateWindowExW(0, L"GPGGameWindow", L"Game",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL);

    // 初始化 SDK
    auto promise = std::make_shared<std::promise<InitializeResult>>();
    auto future = promise->get_future();
    GooglePlayInitialize(
        [promise](InitializeResult r) { promise->set_value(std::move(r)); });

    auto init_result = WaitWithMessagePump(future);
    if (!init_result.ok()) {
        std::ostringstream error_log;
        error_log << "ERROR: Init failed, code=" << static_cast<int>(init_result.code());
        LOG(error_log.str().c_str());
        return 1;
    }
    LOG("SDK initialized.");

    // 生成设备唯一ID (基于本机 MachineGuid，一经生成不会变)
    g_device_id = GetDeviceId();
    if (g_device_id.empty()) {
        LOG("FATAL: Failed to generate device ID.");
        return 1;
    }

    // 初始化 IntegrityClient (必须在 SDK 初始化后构造)
    g_integrity_client = std::make_unique<IntegrityClient>();

    // 初始化 Play Integrity (异步预热，减少后续请求延迟)
    PrepareIntegrity();

    // 初始化 BillingClient
    BillingClientParams bp;
    bp.enable_pending_purchases = true;
    g_billing_client = std::make_unique<BillingClient>(bp);

    // 先处理上次未消耗的购买（会在后端入账金币）
    ConsumeExistingPurchases();

    // 查询最新金币余额（包含刚消耗的）
    LOG("Querying coin balance from server...");
    int coins = QueryCoinsFromServer();
    if (coins >= 0) {
        g_total_coins = coins;
        std::ostringstream coin_log;
        coin_log << "Current coin balance: " << g_total_coins;
        LOG(coin_log.str().c_str());
    } else {
        LOG("WARNING: Failed to query coin balance from server.");
    }

    // 查询最新分数
    LOG("Querying score from server...");
    int score = QueryScoreFromServer();
    if (score >= 0) {
        g_total_score = score;
        std::ostringstream score_log;
        score_log << "Current score: " << g_total_score;
        LOG(score_log.str().c_str());
    } else {
        LOG("WARNING: Failed to query score from server.");
    }

    // 主菜单循环 (使用 TaskDialog 自定义按钮文字)
    const int BTN_ADD_SCORE = 100;
    const int BTN_PURCHASE = 101;
    const int BTN_EXIT = 102;

    bool running = true;
    while (running) {
        std::wostringstream content;
        content << L"Coins: " << g_total_coins << L"    Score: " << g_total_score;

        TASKDIALOG_BUTTON buttons[] = {
            { BTN_ADD_SCORE, L"Add Score (+10)" },
            { BTN_PURCHASE,  L"Purchase Coins" },
            { BTN_EXIT,      L"Exit" },
        };

        TASKDIALOGCONFIG tdc = {};
        tdc.cbSize = sizeof(tdc);
        tdc.hwndParent = g_gameWindow;
        tdc.pszWindowTitle = L"Game Menu";
        tdc.pszMainInstruction = L"Choose an action";
        std::wstring contentStr = content.str();
        tdc.pszContent = contentStr.c_str();
        tdc.cButtons = _countof(buttons);
        tdc.pButtons = buttons;
        tdc.nDefaultButton = BTN_ADD_SCORE;

        int clicked = 0;
        HRESULT hr = TaskDialogIndirect(&tdc, &clicked, NULL, NULL);
        if (FAILED(hr)) { running = false; break; }

        if (clicked == BTN_ADD_SCORE) {
            LOG("User clicked Add Score (+10)");
            auto score_result = AddScoreToServer(10);
            if (score_result.success) {
                g_total_score = score_result.total_score;
                std::ostringstream ok_msg;
                ok_msg << "Score +10!\n\nTotal score: " << score_result.total_score;
                MessageBoxA(g_gameWindow, ok_msg.str().c_str(), "Score Added", MB_OK | MB_ICONINFORMATION);
            } else {
                std::ostringstream err_msg;
                err_msg << "Failed to add score:\n" << score_result.error;
                MessageBoxA(g_gameWindow, err_msg.str().c_str(), "Error", MB_OK | MB_ICONERROR);
            }
        } else if (clicked == BTN_PURCHASE) {
            StartPurchaseFlow();
        } else {
            running = false;
        }
    }

    g_billing_client.reset();
    DestroyWindow(g_gameWindow);
    return 0;
}


#include <iostream>
#include <sstream>
#include <future>
#include <string>
#include <memory>
#include <chrono>
#include <windows.h>
#include "initialization/initialization.h"
#include "billing/client.h"
#include "billing/models.h"
#include "billing/enums.h"

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

std::unique_ptr<BillingClient> g_billing_client;
HWND g_gameWindow = nullptr;

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

bool ConsumePurchase(const std::string& purchase_token) {
    std::ostringstream log_msg;
    log_msg << "Consuming purchase, token: " << purchase_token;
    LOG(log_msg.str().c_str());

    ConsumePurchaseParams params;
    params.purchase_token = purchase_token;

    auto promise = std::make_shared<std::promise<bool>>();
    auto future = promise->get_future();
    g_billing_client->ConsumePurchase(params,
        [promise](ConsumePurchaseResult result) {
            std::string log_result = result.ok() ? "Consume succeeded" : "Consume FAILED";
            LOG(log_result.c_str());
            promise->set_value(result.ok());
        });
    return WaitWithMessagePump(future);
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
            ConsumePurchase(p.purchase_token);
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
    if (code == 0 && !token.empty()) ConsumePurchase(token);
    ShowWindow(g_gameWindow, SW_HIDE);
    return code;
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

    // 初始化 BillingClient
    BillingClientParams bp;
    bp.enable_pending_purchases = true;
    g_billing_client = std::make_unique<BillingClient>(bp);

    ConsumeExistingPurchases();
    int result = StartPurchaseFlow();

    g_billing_client.reset();
    DestroyWindow(g_gameWindow);
    return result;
}


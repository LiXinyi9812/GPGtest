#include <iostream>
#include <future>
#include <string>
#include <memory>
#include "initialization/initialization.h"
#include "billing/client.h"
#include "billing/models.h"
#include "billing/enums.h"

using google::play::initialization::GooglePlayInitialize;
using google::play::initialization::InitializeResult;
using google::play::initialization::InitializationError;
using google::play::billing::BillingClient;
using google::play::billing::BillingClientParams;
using google::play::billing::QueryProductDetailsParams;
using google::play::billing::ProductId;
using google::play::billing::ProductType;
using google::play::billing::LaunchPurchaseFlowParams;
using google::play::billing::ConsumePurchaseParams;
using google::play::billing::BillingError;

// Global billing client pointer
std::unique_ptr<BillingClient> g_billing_client;

void InitializeBillingClient() {
    BillingClientParams params;
    params.enable_pending_purchases = true;
    g_billing_client = std::make_unique<BillingClient>(params);
    std::cout << "Billing client initialized." << std::endl;
}

void StartPurchaseFlow() {
    if (!g_billing_client) {
        std::cerr << "Billing client not initialized!" << std::endl;
        return;
    }

    std::cout << "\n=== Starting purchase flow for 100 coins ===" << std::endl;

    // Step 1: Query product details for "100_coins"
    QueryProductDetailsParams query_params;
    ProductId product_id;
    product_id.id = "100_coins";
    product_id.product_type = ProductType::kTypeInApp;
    query_params.product_ids.push_back(product_id);

    auto query_promise = std::make_shared<std::promise<void>>();
    auto query_future = query_promise->get_future();

    g_billing_client->QueryProductDetails(query_params,
        [query_promise](auto result) {
            if (result.ok()) {
                auto& products = result.value().product_details;
                if (products.empty()) {
                    std::cerr << "Product '100_coins' not found!" << std::endl;
                    query_promise->set_value();
                    return;
                }

                auto& product = products[0];
                std::cout << "Product found:" << std::endl;
                std::cout << "  ID: " << product.id << std::endl;
                std::cout << "  Title: " << product.title << std::endl;
                std::cout << "  Description: " << product.description << std::endl;

                if (product.product_offers.empty()) {
                    std::cerr << "No offers available for this product!" << std::endl;
                    query_promise->set_value();
                    return;
                }

                auto& offer = product.product_offers[0];
                std::cout << "  Price: " << offer.formatted_price << std::endl;

                // Ask user if they want to purchase
                std::cout << "\nDo you want to purchase 100 coins? (y/n): ";
                std::string input;
                std::getline(std::cin, input);

                if (input == "y" || input == "Y") {
                    // Step 2: Launch purchase flow
                    std::cout << "Launching purchase flow..." << std::endl;

                    LaunchPurchaseFlowParams purchase_params;
                    purchase_params.offer_token = offer.offer_token;

                    g_billing_client->LaunchPurchaseFlow(purchase_params,
                        [query_promise](auto purchase_result) {
                            if (purchase_result.ok()) {
                                auto& purchase_details = purchase_result.value().product_purchase_details;
                                std::cout << "\n=== Purchase successful! ===" << std::endl;
                                std::cout << "Product ID: " << purchase_details.product_id << std::endl;
                                std::cout << "Purchase token: " << purchase_details.purchase_token << std::endl;
                                std::cout << "Order ID: " << purchase_details.order_id << std::endl;

                                // Step 3: Consume the purchase (since coins are consumable)
                                std::cout << "Consuming purchase..." << std::endl;

                                ConsumePurchaseParams consume_params;
                                consume_params.purchase_token = purchase_details.purchase_token;

                                g_billing_client->ConsumePurchase(consume_params,
                                    [query_promise](auto consume_result) {
                                        if (consume_result.ok()) {
                                            std::cout << "Purchase consumed! You now have 100 coins." << std::endl;
                                        } else {
                                            std::cerr << "Failed to consume purchase: "
                                                      << static_cast<int>(consume_result.code()) << std::endl;
                                        }
                                        query_promise->set_value();
                                    });
                            } else {
                                if (purchase_result.code() == BillingError::kUserCanceled) {
                                    std::cout << "Purchase canceled by user." << std::endl;
                                } else {
                                    std::cerr << "Purchase failed: "
                                              << static_cast<int>(purchase_result.code()) << std::endl;
                                }
                                query_promise->set_value();
                            }
                        });
                } else {
                    std::cout << "Purchase canceled." << std::endl;
                    query_promise->set_value();
                }
            } else {
                std::cerr << "Failed to query product details: "
                          << static_cast<int>(result.code()) << std::endl;
                query_promise->set_value();
            }
        });

    query_future.wait();
}

int main() {
    // Initialize the SDK as part of the startup sequence of your application.
    auto promise = std::make_shared<std::promise<InitializeResult>>();
    GooglePlayInitialize(
        [promise](InitializeResult result) {
            promise->set_value(std::move(result));
        });

    auto initialize_result = promise->get_future().get();
    if (initialize_result.ok()) {
        // The SDK succeeded with initialization. Continue with the startup
        // sequence of the game.
        std::cout << "SDK initialized successfully!" << std::endl;
        std::cout << "Hello, World!" << std::endl;

        // Initialize billing client
        InitializeBillingClient();

        // Start purchase flow
        StartPurchaseFlow();

        // Interactive loop for "buy" command
        std::cout << "\n=== Game loop started ===" << std::endl;
        std::cout << "Type 'buy' to purchase more coins, or 'exit' to quit." << std::endl;

        std::string command;
        while (true) {
            std::cout << "\n> ";
            std::getline(std::cin, command);

            if (command == "buy") {
                StartPurchaseFlow();
            } else if (command == "exit") {
                std::cout << "Exiting game..." << std::endl;
                break;
            } else {
                std::cout << "Unknown command. Type 'buy' or 'exit'." << std::endl;
            }
        }
    } else if (initialize_result.code() == InitializationError::kActionRequiredShutdownClientProcess) {
        // The SDK failed to initialize and has requested that your game process
        // exit as soon as possible.
        std::cerr << "SDK requested immediate shutdown." << std::endl;
        system("pause");
        exit(1);
    } else {
        // The SDK failed to initialize for an alternative reason. It is still
        // generally recommended that you exit the game process as soon as
        // possible, because it won't be possible to access any APIs in the SDK.
        std::cerr << "SDK initialization failed, error code: "
                  << static_cast<int>(initialize_result.code())
                  << ", error message: " << initialize_result.error_message()
                  << std::endl;
        system("pause");
        exit(1);
    }

    return 0;
}


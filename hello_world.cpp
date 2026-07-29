#include <iostream>
#include <future>
#include "initialization/initialization.h"

using google::play::initialization::GooglePlayInitialize;
using google::play::initialization::InitializeResult;
using google::play::initialization::InitializationError;

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

    system("pause");
    return 0;
}


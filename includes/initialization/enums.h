// Copyright 2025 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef PLAY_INITIALIZATION_ERROR_H_
#define PLAY_INITIALIZATION_ERROR_H_

namespace google {
namespace play {
namespace initialization {

/**
 *  @brief Error codes returned by Google Play Initialization.
 */
enum class InitializationError {
  /**
   * The operation completed successfully.
   */
  kOk = 0,

  /**
   * Fatal error during initialization.
   */
  kError = 1,

  /**
   * The calling client process should shutdown as soon as possible.
   *
   * This may be returned when a game is not launched via Google Play Games and
   * is in the process of being relaunched.
   */
  kActionRequiredShutdownClientProcess = 2,

  /**
   * The current version of the SDK runtime installed on the device does not
   * support the calling version of the SDK.
   *
   * The user will need to update Google Play Games and the SDK runtime in order
   * to proceed.
   */
  kSdkRuntimeUpdateRequired = 3,

  /**
   * The SDK runtime is unavailable.
   *
   * The user will need to install Google Play Games and the SDK runtime in
   * order to proceed.
   */
  kSdkRuntimeUnavailable = 4,
};

}  // namespace initialization
}  // namespace play
}  // namespace google

#endif  // PLAY_INITIALIZATION_ERROR_H_

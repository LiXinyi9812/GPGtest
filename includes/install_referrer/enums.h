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

#ifndef THIRD_PARTY_PLAY_PC_SDK_INSTALL_REFERRER_ENUMS_H_
#define THIRD_PARTY_PLAY_PC_SDK_INSTALL_REFERRER_ENUMS_H_

namespace google {
namespace play {
namespace install_referrer {

/**
 * @brief Error codes returned by Install Referrer.
 */
enum class InstallReferrerError {
  /**
   * The operation completed successfully.
   */
  kOk = 0,

  /**
   * Fatal error during the API action.
   */
  kError = 1,
};

}  // namespace install_referrer
}  // namespace play
}  // namespace google

#endif  // THIRD_PARTY_PLAY_PC_SDK_INSTALL_REFERRER_ENUMS_H_

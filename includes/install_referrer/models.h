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

#ifndef THIRD_PARTY_PLAY_PC_SDK_INSTALL_REFERRER_MODELS_H_
#define THIRD_PARTY_PLAY_PC_SDK_INSTALL_REFERRER_MODELS_H_

#include <cstdint>
#include <functional>
#include <string>

#include "install_referrer/enums.h"
#include "libraries/core/result.h"

namespace google {
namespace play {
namespace install_referrer {

/**
 * @brief The result value for the GetInstallReferrer API.
 */
struct GetInstallReferrerResultValue {
  /**
   * The install referrer associated with the installation. Empty if no
   * referrer was provided.
   */
  std::string install_referrer;

  /**
   * The client-side timestamp, seconds since the Unix epoch, when the app
   * was installed.
   */
  int64_t install_time_epoch_seconds;
};

using GetInstallReferrerResult =
    Result<InstallReferrerError, GetInstallReferrerResultValue>;

using GetInstallReferrerContinuation =
    std::function<void(GetInstallReferrerResult)>;

}  // namespace install_referrer
}  // namespace play
}  // namespace google

#endif  // THIRD_PARTY_PLAY_PC_SDK_INSTALL_REFERRER_MODELS_H_

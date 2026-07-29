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

#ifndef THIRD_PARTY_PLAY_PC_SDK_INSTALL_REFERRER_CLIENT_H_
#define THIRD_PARTY_PLAY_PC_SDK_INSTALL_REFERRER_CLIENT_H_

#include <functional>

#include "install_referrer/internal/client.h"
#include "install_referrer/internal/model_conversions.h"
#include "install_referrer/internal/models.h"
#include "install_referrer/models.h"

namespace google {
namespace play {
namespace install_referrer {

/**
 * @brief Entrypoint for the Install Referrer API.
 *
 */
class InstallReferrerClient {
 public:
  InstallReferrerClient() {
    client_ = internal::_InstallReferrerClient_Create();
  };

  ~InstallReferrerClient() {
    internal::_InstallReferrerClient_Destroy(client_);
  };

  /**
   * @brief Get the install referrer associated with the installation.
   *
   * Provides the install referrer associated with the installation (if any) and
   * the time at which the installation was started.
   *
   * If a referrer source was associated with the installation of your
   * application the response will contain it via
   * \ref GetInstallReferrerResultValue::install_referrer.
   *
   * @param[in] continuation The continuation callback.
   */
  void GetInstallReferrer(GetInstallReferrerContinuation continuation) {
    auto context = new GetInstallReferrerContinuation;
    *context = continuation;
    internal::_InstallReferrerClient_GetInstallReferrer(
        client_, reinterpret_cast<void*>(context),
        &InstallReferrerClient::GetInstallReferrerComplete);
  }

 private:
  static void GetInstallReferrerComplete(
      void* context, internal::_InstallReferrerResult result,
      internal::_GetInstallReferrerResultValue resultValue) {
    auto continuation =
        reinterpret_cast<GetInstallReferrerContinuation*>(context);
    (*continuation)(internal::fromInternal(result, resultValue));
    delete continuation;
  }

 private:
  void* client_;
};

}  // namespace install_referrer
}  // namespace play
}  // namespace google

#endif  // THIRD_PARTY_PLAY_PC_SDK_INSTALL_REFERRER_CLIENT_H_

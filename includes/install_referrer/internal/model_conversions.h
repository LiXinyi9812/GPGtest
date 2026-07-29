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

#ifndef THIRD_PARTY_PLAY_PC_SDK_INSTALL_REFERRER_INTERNAL_MODEL_CONVERSIONS_H_
#define THIRD_PARTY_PLAY_PC_SDK_INSTALL_REFERRER_INTERNAL_MODEL_CONVERSIONS_H_

#include <utility>

#include "install_referrer/enums.h"
#include "install_referrer/internal/models.h"
#include "install_referrer/models.h"

namespace google {
namespace play {
namespace install_referrer {
namespace internal {

// GetInstallReferrer

inline GetInstallReferrerResult fromInternal(
    const _InstallReferrerResult& result,
    const internal::_GetInstallReferrerResultValue& _resultValue) {
  if (result.code != InstallReferrerError::kOk) {
    return GetInstallReferrerResult::Error(result.code, result.error_message);
  }

  GetInstallReferrerResultValue resultValue{};
  resultValue.install_time_epoch_seconds =
      _resultValue.install_time_epoch_seconds;

  if (_resultValue.install_referrer != nullptr) {
    resultValue.install_referrer = _resultValue.install_referrer;
  }

  return GetInstallReferrerResult::Ok(std::move(resultValue));
}

}  // namespace internal
}  // namespace install_referrer
}  // namespace play
}  // namespace google

#endif  // THIRD_PARTY_PLAY_PC_SDK_INSTALL_REFERRER_INTERNAL_MODEL_CONVERSIONS_H_

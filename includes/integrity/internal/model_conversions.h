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

#ifndef PLAY_INTEGRITY_CLIENT_INTERNAL_MODEL_CONVERSIONS_H_
#define PLAY_INTEGRITY_CLIENT_INTERNAL_MODEL_CONVERSIONS_H_

#include <string>

#include "integrity/internal/models.h"
#include "integrity/models.h"
#include "integrity/enums.h"

namespace google {
namespace play {
namespace integrity {
namespace internal {

// PrepareIntegrityTokenParams

inline internal::_PrepareIntegrityTokenParams toInternal(
    const PrepareIntegrityTokenParams& params) {
  return internal::_PrepareIntegrityTokenParams({params.cloud_project_number});
}

inline void freeInternal(internal::_PrepareIntegrityTokenParams params) {
  // Nothing to free. If allocations are added, they should be freed here.
}

// PrepareIntegrityTokenResult

inline PrepareIntegrityTokenResult fromInternal(
    const _IntegrityResult& result,
    const internal::_PrepareIntegrityTokenResultValue& resultValue) {
  if (result.code != IntegrityError::kOk) {
    return PrepareIntegrityTokenResult::Error(result.code,
                                              result.error_message);
  }

  return PrepareIntegrityTokenResult::Ok(
      {{resultValue.request_token_data.cloud_project_number,
        std::string(resultValue.request_token_data.warm_up_session_id)}});
}

// RequestIntegrityTokenParams
inline internal::_RequestIntegrityTokenParams toInternal(
    const RequestIntegrityTokenParams& params) {
  return internal::_RequestIntegrityTokenParams(
      {{params.request_token_data.cloud_project_number,
        params.request_token_data.warm_up_session_id.c_str()},
       params.request_hash.c_str()});
}

inline void freeInternal(internal::_RequestIntegrityTokenParams params) {
  // Nothing to free. If allocations are added, they should be freed here.
}

// RequestIntegrityTokenResult
inline RequestIntegrityTokenResult fromInternal(
    const _IntegrityResult& result,
    const internal::_RequestIntegrityTokenResultValue& resultValue) {
  if (result.code != IntegrityError::kOk) {
    return RequestIntegrityTokenResult::Error(result.code,
                                              result.error_message);
  }

  return RequestIntegrityTokenResult::Ok(
      {std::string(resultValue.token_bytes)});
}

}  // namespace internal
}  // namespace integrity
}  // namespace play
}  // namespace google

#endif  // PLAY_INTEGRITY_CLIENT_INTERNAL_MODEL_CONVERSIONS_H_

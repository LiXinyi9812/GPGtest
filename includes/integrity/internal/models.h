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

#ifndef PLAY_INTEGRITY_CLIENT_INTERNAL_MODELS_H_
#define PLAY_INTEGRITY_CLIENT_INTERNAL_MODELS_H_

#include <cstdint>

#include "integrity/enums.h"

namespace google {
  namespace play {
  namespace integrity {
  namespace internal {

struct _IntegrityResult {
  IntegrityError code;
  const char* error_message;
};

// PrepareIntegrityToken

struct _PrepareIntegrityTokenParams {
  // The cloud project number of the app.
  int64_t cloud_project_number;
};

struct _RequestTokenData {
  // The cloud project number of the app.
  int64_t cloud_project_number;
  // The warm up session id of the app.
  const char* warm_up_session_id;
};

struct _PrepareIntegrityTokenResultValue {
  _RequestTokenData request_token_data;
};

using _PrepareIntegrityTokenContinuation =
    void (*)(void*, _IntegrityResult, _PrepareIntegrityTokenResultValue);

// RequestIntegrityToken

struct _RequestIntegrityTokenParams {
  // The data received from PrepareIntegrityToken.
  const _RequestTokenData request_token_data;
  // The hash of the request.
  const char* request_hash;
};

struct _RequestIntegrityTokenResultValue {
  // The encrypted integrity token bytes.
  const char* token_bytes;
};

using _RequestIntegrityTokenContinuation =
    void (*)(void*, _IntegrityResult, _RequestIntegrityTokenResultValue);

}  // namespace internal
}  // namespace integrity
}  // namespace play
}  // namespace google

#endif  // PLAY_INTEGRITY_CLIENT_INTERNAL_MODELS_H_

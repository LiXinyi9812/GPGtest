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

#ifndef PLAY_INTEGRITY_CLIENT_MODELS_H_
#define PLAY_INTEGRITY_CLIENT_MODELS_H_

#include <cstdint>
#include <functional>
#include <string>

#include "integrity/enums.h"
#include "libraries/core/result.h"

namespace google {
namespace play {
namespace integrity {

/**
 * @brief Params for preparing an integrity token.
 *
 * See IntegrityClient::PrepareIntegrityToken for more details.
 */
struct PrepareIntegrityTokenParams {
  /**
   * The Google Cloud project number of your application.
   */
  int64_t cloud_project_number;
};

/**
 * @brief Data needed to request an integrity token.
 */
struct RequestTokenData {
  /**
   * The Google Cloud project number associated with the integrity token.
   */
  int64_t cloud_project_number;

  /**
   * The warm up session id associated with the integrity token.
   */
  const std::string warm_up_session_id;
};

/**
 * @brief %Result value for preparing an integrity token.
 */
struct PrepareIntegrityTokenResultValue {
  /**
   * The data needed to request an integrity token.
   */
  RequestTokenData request_token_data;
};

using PrepareIntegrityTokenResult =
    Result<IntegrityError, PrepareIntegrityTokenResultValue>;

using PrepareIntegrityTokenContinuation =
    std::function<void(PrepareIntegrityTokenResult)>;

/**
 * @brief Params for requesting an integrity token.
 *
 * See IntegrityClient::RequestIntegrityToken for more details.
 */
struct RequestIntegrityTokenParams {
  /**
   * @brief The integrity token request data.
   */
  RequestTokenData request_token_data;

  /**
   * @brief The hash of the request.
   *
   * Use this field to mitigate against tampering attacks.
   *
   * For example, a game may want to report the player's score to the game's
   * backend server, and your server wants to ensure this score has not been
   * tampered with by a proxy server. The Play Integrity API returns the value
   * you set in this field, inside the signed integrity response. Without the
   * request_hash, the integrity token will be bound only to the device, but not
   * to the specific request, which opens up the possibility of attack.
   *
   * To mitigate this when you request an integrity verdict:
   *  - Compute a digest of all relevant request parameters (e.g. SHA256 of a
   *    stable request serialization) from the user action or server request
   *    that is happening.
   *  - Set the \ref RequestIntegrityTokenParams::request_hash field to the
   *    digest.
   */
  const std::string request_hash;
};

/**
 * @brief %Result value of the integrity token request.
 */
struct RequestIntegrityTokenResultValue {
  /**
   * @brief The encrypted integrity token bytes.
   */
  const std::string token_bytes;
};

using RequestIntegrityTokenResult =
    Result<IntegrityError, RequestIntegrityTokenResultValue>;

using RequestIntegrityTokenContinuation =
    std::function<void(RequestIntegrityTokenResult)>;

}  // namespace integrity
}  // namespace play
}  // namespace google

#endif  // PLAY_INTEGRITY_CLIENT_MODELS_H_

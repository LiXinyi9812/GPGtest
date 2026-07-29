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

#ifndef PLAY_INTEGRITY_CLIENT_H_
#define PLAY_INTEGRITY_CLIENT_H_

#include <functional>

#include "integrity/internal/client.h"
#include "integrity/internal/model_conversions.h"
#include "integrity/internal/models.h"
#include "integrity/models.h"

namespace google {
namespace play {
namespace integrity {

/**
 * @brief Entrypoint for the Google Play Integrity API.
 *
 * The Play Integrity API helps you check that interactions and server requests
 * are coming from a genuine PC device. By detecting potentially risky and
 * fraudulent interactions, your app’s backend server can respond with
 * appropriate actions to prevent attacks and reduce abuse.
 */
class IntegrityClient {
 public:
  IntegrityClient() { client_ = internal::_IntegrityClient_Create(); };

  ~IntegrityClient() { internal::_IntegrityClient_Destroy(client_); };

  /**
   * @brief Warms up the Play Integrity API for requesting integrity tokens.
   *
   * Before requesting an integrity token (see \ref RequestIntegrityToken),
   * you must prepare (or "warm up") the Play Integrity API. This allows Google
   * Play to intelligently cache partial attestation information on the device
   * in order to decrease the latency on the critical path when you make a
   * request for an integrity verdict.
   *
   * On success, the continuation will be called with a \ref
   * PrepareIntegrityTokenResultValue containing a \ref RequestTokenData that
   * should be used to request an integrity token. This data should be cached
   * in-memory and reused for the duration of the application's session for
   * calls to \ref RequestIntegrityToken.
   *
   * Only if your application determines that it is necessary to entirely
   * revaluate the integrity verdict should a call to \ref PrepareIntegrityToken
   * be made.
   *
   * @param[in] params Parameters containing a Google Cloud project number.
   * @param[in] continuation The async callback to return the integrity token
   * provider to.
   */
  void PrepareIntegrityToken(const PrepareIntegrityTokenParams& params,
                             PrepareIntegrityTokenContinuation continuation) {
    auto context = new std::function<void(PrepareIntegrityTokenResult)>;
    *context = continuation;
    internal::_PrepareIntegrityTokenParams requestInternal =
        internal::toInternal(params);
    internal::_IntegrityClient_PrepareIntegrityToken(
        client_, requestInternal, reinterpret_cast<void*>(context),
        &IntegrityClient::PrepareIntegrityTokenComplete);
    freeInternal(requestInternal);
  }

  /**
   * @brief Requests an integrity token using a \ref RequestTokenData obtained
   * from a successful \ref PrepareIntegrityToken call.
   *
   * Integrity tokens are a mechanism for your application to verify the device
   * is not tampered with. For example, your backend server can use the
   * integrity token to verify:
   *  - Genuine device: Determine whether your application is running on a
   *    genuine device that contains a genuine instance of Google Play Games
   *    for PC and not tampered with.
   *
   * When you're checking a user action in your app with the Play Integrity API,
   * you can leverage the \ref RequestIntegrityTokenParams::request_hash field
   * to mitigate against tampering attacks.
   *
   * For example, a game may want to report the player's score to the game's
   * backend server, and your server wants to ensure this score has not been
   * tampered with by a proxy server. The Play Integrity API returns the value
   * you set in this field, inside the signed integrity response. Without the
   * requestHash, the integrity token will be bound only to the device, but not
   * to the specific request, which opens up the possibility of attack.
   *
   * To mitigate this when you request an integrity verdict:
   *  - Compute a digest of all relevant request parameters (e.g. SHA256 of a
   *    stable request serialization) from the user action or server request
   *    that is happening.
   *  - Set the \ref RequestIntegrityTokenParams::request_hash field to the
   *    digest.
   *
   * @param[in] params Parameters containing the prepared \ref RequestTokenData
   * and integrity check request hash.
   * @param[in] continuation The async callback to return the data to.
   */
  void RequestIntegrityToken(const RequestIntegrityTokenParams& params,
                             RequestIntegrityTokenContinuation continuation) {
    auto context = new std::function<void(const RequestIntegrityTokenResult&)>;
    *context = continuation;
    internal::_RequestIntegrityTokenParams paramsInternal =
        internal::toInternal(params);
    internal::_IntegrityClient_RequestIntegrityToken(
        client_, paramsInternal, reinterpret_cast<void*>(context),
        &IntegrityClient::RequestTokenComplete);
  }

 private:
  void* client_;

  static void PrepareIntegrityTokenComplete(
      void* context, internal::_IntegrityResult result,
      internal::_PrepareIntegrityTokenResultValue resultValue) {
    auto continuation =
        reinterpret_cast<std::function<void(PrepareIntegrityTokenResult)>*>(
            context);
    (*continuation)(fromInternal(result, resultValue));
    delete continuation;
  }

  static void RequestTokenComplete(
      void* context, internal::_IntegrityResult result,
      internal::_RequestIntegrityTokenResultValue resultValue) {
    auto continuation =
        reinterpret_cast<std::function<void(RequestIntegrityTokenResult)>*>(
            context);
    (*continuation)(fromInternal(result, resultValue));
    delete continuation;
  }
};

}  // namespace integrity
}  // namespace play
}  // namespace google

#endif  // PLAY_INTEGRITY_CLIENT_H_

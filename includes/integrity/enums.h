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

#ifndef PLAY_INTEGRITY_ERROR_H_
#define PLAY_INTEGRITY_ERROR_H_

namespace google {
namespace play {
namespace integrity {

/**
 *  @brief Error codes returned by Google Play Integrity.
 */
enum class IntegrityError {
  /**
   * The operation completed successfully.
   */
  kOk = 0,

  /**
   * Fatal error during the SDK action.
   */
  kError = 1,

  /**
   * The cloud project number is invalid.
   */
  kCloudProjectNumberIsInvalid = 2,

  /**
   * The request hash is too long.
   */
  kRequestHashTooLong = 3,

  /**
   * No valid prepared token found.
   */
  kNoValidPreparedTokenFound = 4,

  /**
   * Network error.
   */
  kNetworkError = 5,

  /**
   * Too many requests.
   */
  kTooManyRequests = 6,

  /**
   * Client transient error.
   */
  kClientTransientError = 7,

  /**
   * The current version of the SDK runtime installed on the device does not
   * support the requested Play Integrity functionality.
   *
   * The user will need to update Google Play Games and the SDK runtime in order
   * to proceed.
   */
  kSdkRuntimeUpdateRequired = 8,
};

}  // namespace integrity
}  // namespace play
}  // namespace google

#endif  // PLAY_INTEGRITY_ERROR_H_

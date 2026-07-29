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

#ifndef PLAY_INITIALIZATION_INITIALIZATION_H_
#define PLAY_INITIALIZATION_INITIALIZATION_H_

#include <functional>

#include "initialization/internal/initialization.h"
#include "initialization/internal/model_conversions.h"
#include "initialization/internal/models.h"
#include "initialization/models.h"

namespace google {
namespace play {
namespace initialization {

namespace internal {
inline void InitializeComplete(void* context, _InitializationResult result,
                               internal::_InitializeResultValue resultValue) {
  auto continuation =
      reinterpret_cast<std::function<void(InitializeResult)>*>(context);
  (*continuation)(fromInternal(result, resultValue));
  delete continuation;
}
}  // namespace internal

/**
 * @brief Establishes a connection to Play Services. Must be called and
 * return successfully before using any other API.
 *
 * If initialization fails your application should exit and close any running
 * processes as soon as possible.
 *
 * Without a successful initialization your application can not use the Play PC
 * SDK and critical checks such as ensuring that the user has a valid license
 * to your application may have failed or could not be performed.
 *
 * @param[in] continuation The async callback to return the data to.
 */
void GooglePlayInitialize(
    std::function<void(InitializeResult)> continuation) {
  auto context = new std::function<void(InitializeResult)>;
  *context = continuation;
  internal::_GooglePlayInitialize(reinterpret_cast<void*>(context),
                                  &internal::InitializeComplete);
}

}  // namespace initialization
}  // namespace play
}  // namespace google

#endif  // PLAY_INITIALIZATION_INITIALIZATION_H_

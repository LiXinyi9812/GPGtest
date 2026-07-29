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

#ifndef PLAY_INITIALIZATION_MODELS_H_
#define PLAY_INITIALIZATION_MODELS_H_

#include <functional>

#include "initialization/enums.h"
#include "libraries/core/result.h"

namespace google {
namespace play {
namespace initialization {

// GooglePlayInitialize

/**
 * @brief %Result value for initialization.
 *
 * See GooglePlayInitialize for more details.
 */
struct InitializeResultValue {};

using InitializeResult = Result<InitializationError, InitializeResultValue>;

using InitializeContinuation = std::function<void(InitializeResult)>;

}  // namespace initialization
}  // namespace play
}  // namespace google

#endif  // PLAY_INITIALIZATION_MODELS_H_

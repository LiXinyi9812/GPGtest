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

#ifndef PLAY_INITIALIZATION_INTERNAL_MODELS_H_
#define PLAY_INITIALIZATION_INTERNAL_MODELS_H_

#include "initialization/enums.h"

namespace google {
namespace play {
namespace initialization {
namespace internal {

struct _InitializationResult {
  InitializationError code;
  const char* error_message;
};

// GooglePlayInitialize

struct _InitializeResultValue {};

using _InitializeContinuation = void (*)(void*, _InitializationResult,
                                         _InitializeResultValue);

}  // namespace internal
}  // namespace initialization
}  // namespace play
}  // namespace google

#endif  // PLAY_INITIALIZATION_INTERNAL_MODELS_H_

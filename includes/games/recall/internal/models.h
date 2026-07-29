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

#ifndef PLAY_GAMES_RECALL_INTERNAL_MODELS_H_
#define PLAY_GAMES_RECALL_INTERNAL_MODELS_H_

#include "games/recall/enums.h"

namespace google {
namespace play {
namespace games {
namespace recall {
namespace internal {

struct _GamesRecallResult {
  GamesRecallError code;
  const char* error_message;
};

// RequestRecallAccess

struct _RequestRecallAccessResultValue {
  const char* recall_session_id;
};

using _RequestRecallAccessContinuation =
    void (*)(void*, _GamesRecallResult, _RequestRecallAccessResultValue);

}  // namespace internal
}  // namespace recall
}  // namespace games
}  // namespace play
}  // namespace google

#endif  // PLAY_GAMES_RECALL_INTERNAL_MODELS_H_

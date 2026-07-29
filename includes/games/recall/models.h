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

#ifndef PLAY_GAMES_RECALL_MODELS_H_
#define PLAY_GAMES_RECALL_MODELS_H_

#include <stdbool.h>

#include <functional>
#include <string>

#include "games/recall/enums.h"
#include "libraries/core/result.h"

namespace google {
namespace play {
namespace games {
namespace recall {

/**
 * @brief The Google Play Games Services recall access.
 */
struct RecallAccessResultValue {
  /**
   * @brief The session ID associated with the active Play Games Services user.
   *
   * This should be passed to the game's server and included in backend calls
   * to Google's servers when performing Recall operations for the user such as
   * querying, linking and unlinking in-game accounts with a Play Games Services
   * user.
   */
  std::string recall_session_id;
};

using RequestRecallAccessResult =
    Result<GamesRecallError, RecallAccessResultValue>;

using RequestRecallAccessContinuation =
    std::function<void(RequestRecallAccessResult)>;

}  // namespace recall
}  // namespace games
}  // namespace play
}  // namespace google

#endif  // PLAY_GAMES_RECALL_MODELS_H_

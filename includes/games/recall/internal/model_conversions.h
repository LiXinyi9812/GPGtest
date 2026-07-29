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

#ifndef PLAY_GAMES_RECALL_INTERNAL_MODEL_CONVERSIONS_H_
#define PLAY_GAMES_RECALL_INTERNAL_MODEL_CONVERSIONS_H_

#include <utility>

#include "games/recall/enums.h"
#include "games/recall/internal/models.h"
#include "games/recall/models.h"

namespace google {
namespace play {
namespace games {
namespace recall {
namespace internal {

// RequestRecallAccess

inline RequestRecallAccessResult fromInternal(
    const _GamesRecallResult& result,
    const internal::_RequestRecallAccessResultValue& _resultValue) {
  if (result.code != GamesRecallError::kOk) {
    return RequestRecallAccessResult::Error(result.code, result.error_message);
  }

  RecallAccessResultValue resultValue{
      _resultValue.recall_session_id,
  };
  return RequestRecallAccessResult::Ok(std::move(resultValue));
}

}  // namespace internal
}  // namespace recall
}  // namespace games
}  // namespace play
}  // namespace google

#endif  // PLAY_GAMES_RECALL_INTERNAL_MODEL_CONVERSIONS_H_

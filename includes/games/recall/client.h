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

#ifndef PLAY_GAMES_RECALL_CLIENT_H_
#define PLAY_GAMES_RECALL_CLIENT_H_

#include <functional>

#include "games/recall/internal/client.h"
#include "games/recall/internal/model_conversions.h"
#include "games/recall/internal/models.h"
#include "games/recall/models.h"

namespace google {
namespace play {
namespace games {
namespace recall {

/**
 * @brief Entrypoint for the Google Play Games Services Recall API.
 *
 * The Recall API lets games with their own sign-in system to offer a more
 * seamless sign-in experience for Google Play Games Services users.
 *
 * It does this by allowing the game to manage links between its own in-game
 * accounts and Google Play Games Services (PGS) users. These links can queried
 * by the game's server to streamline the sign-in process for returning users
 * across different devices or after reinstalling the game on the same device.
 */
class GamesRecallClient {
 public:
  GamesRecallClient() { client_ = internal::_GamesRecallClient_Create(); };

  ~GamesRecallClient() { internal::_GamesRecallClient_Destroy(client_); };

  /**
   * @brief Requests recall access from Play Games Services.
   *
   * If successful, returns a recall session ID which can be used by your
   * backend to make server-side calls to Google for linking & unlinking your
   * in-game accounts with a Play Games Services user.
   *
   * For first-time users, use the recall session ID to link their in-game
   * account with a Play Games Services user. This sets up the user for a
   * seamless sign-in experience when they return to your game across different
   * devices or after reinstalling your game on the same device.
   *
   * For returning users, use the recall session ID to resolve which in-game
   * account is already linked and seamlessly restore the user's progress
   * without requiring them to sign-in to their in-game account again.
   *
   * @param[in] continuation The continuation callback.
   */
  void RequestRecallAccess(RequestRecallAccessContinuation continuation) {
    auto context = new std::function<void(RequestRecallAccessResult)>;
    *context = continuation;
    internal::_GamesRecallClient_RequestRecallAccess(
        client_, reinterpret_cast<void*>(context),
        &GamesRecallClient::RequestRecallAccessComplete);
  }

 private:
  static void RequestRecallAccessComplete(
      void* context, internal::_GamesRecallResult result,
      internal::_RequestRecallAccessResultValue resultValue) {
    auto continuation =
        reinterpret_cast<std::function<void(RequestRecallAccessResult)>*>(
            context);
    (*continuation)(internal::fromInternal(result, resultValue));
    delete continuation;
  }

 private:
  void* client_;
};

}  // namespace recall
}  // namespace games
}  // namespace play
}  // namespace google

#endif  // PLAY_GAMES_RECALL_CLIENT_H_

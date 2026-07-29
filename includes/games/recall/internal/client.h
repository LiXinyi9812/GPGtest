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

#ifndef PLAY_GAMES_RECALL_INTERNAL_CLIENT_H_
#define PLAY_GAMES_RECALL_INTERNAL_CLIENT_H_

#if defined(BUILDING_DLL) && defined(_WIN32)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#include "games/recall/internal/models.h"

namespace google {
namespace play {
namespace games {
namespace recall {
namespace internal {
extern "C" {
DLL_EXPORT void* _GamesRecallClient_Create();
DLL_EXPORT void _GamesRecallClient_Destroy(void* client);

DLL_EXPORT void _GamesRecallClient_RequestRecallAccess(
    void* client, void* context, _RequestRecallAccessContinuation continuation);
}  // extern "C"
}  // namespace internal
}  // namespace recall
}  // namespace games
}  // namespace play
}  // namespace google

#endif  // PLAY_GAMES_RECALL_INTERNAL_CLIENT_H_

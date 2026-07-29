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

#ifndef PLAY_BILLING_INTERNAL_CLIENT_H_
#define PLAY_BILLING_INTERNAL_CLIENT_H_

#if defined(BUILDING_DLL) && defined(_WIN32)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#include "billing/internal/models.h"
#include "billing/models.h"

namespace google {
namespace play {
namespace billing {
namespace internal {
extern "C" {
DLL_EXPORT void* _BillingClient_Create(const BillingClientParams& params);
DLL_EXPORT void _BillingClient_Destroy(void* client);

DLL_EXPORT void _BillingClient_QueryProductDetails(
    void* client, const _QueryProductDetailsParams& params, void* context,
    _QueryProductDetailsContinuation continuation);

DLL_EXPORT void _BillingClient_LaunchPurchaseFlow(
    void* client, const _LaunchPurchaseFlowParams& params, void* context,
    _LaunchPurchaseFlowContinuation continuation);

DLL_EXPORT void _BillingClient_QueryPurchases(
    void* client, void* context, _QueryPurchasesContinuation continuation);

DLL_EXPORT void _BillingClient_AcknowledgePurchase(
    void* client, const _AcknowledgePurchaseParams& params, void* context,
    _AcknowledgePurchaseContinuation continuation);

DLL_EXPORT void _BillingClient_ConsumePurchase(
    void* client, const _ConsumePurchaseParams& params, void* context,
    _ConsumePurchaseContinuation continuation);

DLL_EXPORT void _BillingClient_IsBillingProgramAvailable(
    void* client, const _IsBillingProgramAvailableParams& params, void* context,
    _IsBillingProgramAvailableContinuation continuation);

DLL_EXPORT void _BillingClient_CreateBillingProgramReportingDetails(
    void* client, const _CreateBillingProgramReportingDetailsParams& params,
    void* context,
    _CreateBillingProgramReportingDetailsContinuation continuation);

}  // extern "C"
}  // namespace internal
}  // namespace billing
}  // namespace play
}  // namespace google

#endif  // PLAY_BILLING_INTERNAL_CLIENT_H_

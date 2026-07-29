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

#ifndef PLAY_BILLING_INTERNAL_MODELS_H_
#define PLAY_BILLING_INTERNAL_MODELS_H_

#include <cstddef>
#include <cstdint>

#include "billing/enums.h"

namespace google {
namespace play {
namespace billing {
namespace internal {

struct _BillingResult {
  BillingError code;
  const char* error_message;
};

// QueryProductDetails

struct _ProductId {
  const char* id;
  ProductType product_type;
};

struct _QueryProductDetailsParams {
  size_t product_count;
  _ProductId* ids;
};

struct _ProductOffer {
  int64_t price_micros;
  const char* currency_code;
  const char* formatted_price;
  const char* offer_token;
};

struct _ProductDetails {
  const char* id;
  const char* title;
  const char* name;
  const char* description;
  _ProductOffer* product_offers;
  size_t product_offers_count;
};

struct _QueryProductDetailsResultValue {
  size_t product_details_count;
  _ProductDetails* product_offer_details;
};

using _QueryProductDetailsContinuation =
    void (*)(void*, _BillingResult, _QueryProductDetailsResultValue);

// QueryPurchases

struct _ProductPurchaseDetails {
  const char* obfuscated_account_id;
  const char* obfuscated_profile_id;
  const char* order_id;
  const char* package_name;
  const char* product_id;
  PurchaseState purchase_state;
  int64_t purchase_time_epoch_seconds;
  const char* purchase_token;
  int64_t quantity;
  bool is_acknowledged;
  const char* signature;
  const char* original_json;
};

struct _QueryPurchasesResultValue {
  size_t product_purchase_details_count;
  _ProductPurchaseDetails* product_purchase_details;
};

using _QueryPurchasesContinuation = void (*)(void*, _BillingResult,
                                             _QueryPurchasesResultValue);

// LaunchPurchaseFlow

struct _LaunchPurchaseFlowParams {
  const char* offer_token;
  const char* obfuscated_account_id;
  const char* obfuscated_profile_id;
  bool is_offer_personalized;
  int64_t quantity;
};

struct _LaunchPurchaseFlowResultValue {
  _ProductPurchaseDetails product_purchase_details;
};

using _LaunchPurchaseFlowContinuation =
    void (*)(void*, _BillingResult, _LaunchPurchaseFlowResultValue);

// AcknowledgePurchase

struct _AcknowledgePurchaseParams {
  const char* purchase_token_value;
};

struct _AcknowledgePurchaseResultValue {};

using _AcknowledgePurchaseContinuation =
    void (*)(void*, _BillingResult, _AcknowledgePurchaseResultValue);

// ConsumePurchase

struct _ConsumePurchaseParams {
  const char* purchase_token_value;
};

struct _ConsumePurchaseResultValue {};

using _ConsumePurchaseContinuation = void (*)(void*, _BillingResult,
                                              _ConsumePurchaseResultValue);

// IsBillingProgramAvailable

struct _IsBillingProgramAvailableParams {
  BillingProgram billing_program;
};

struct _IsBillingProgramAvailableResultValue {};

using _IsBillingProgramAvailableContinuation =
    void (*)(void*, _BillingResult, _IsBillingProgramAvailableResultValue);

// CreateBillingProgramReportingDetails

struct _CreateBillingProgramReportingDetailsParams {
  BillingProgram billing_program;
};

struct _CreateBillingProgramReportingDetailsResultValue {
  const char* external_transaction_token;
};

using _CreateBillingProgramReportingDetailsContinuation = void (*)(
    void*, _BillingResult, _CreateBillingProgramReportingDetailsResultValue);

}  // namespace internal
}  // namespace billing
}  // namespace play
}  // namespace google

#endif  // PLAY_BILLING_INTERNAL_MODELS_H_

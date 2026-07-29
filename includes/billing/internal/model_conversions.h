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

#ifndef PLAY_BILLING_INTERNAL_MODEL_CONVERSIONS_H_
#define PLAY_BILLING_INTERNAL_MODEL_CONVERSIONS_H_

#include <string>
#include <utility>

#include "billing/enums.h"
#include "billing/internal/models.h"
#include "billing/models.h"

namespace google {
namespace play {
namespace billing {
namespace internal {

// QueryProductDetails

inline QueryProductDetailsResult fromInternal(
    const _BillingResult& result,
    const internal::_QueryProductDetailsResultValue& _resultValue) {
  if (result.code != BillingError::kOk) {
    return QueryProductDetailsResult::Error(result.code, result.error_message);
  }

  QueryProductDetailsResultValue resultValue;
  for (int detail_index = 0; detail_index < _resultValue.product_details_count;
       detail_index++) {
    const internal::_ProductDetails& detailInternal =
        _resultValue.product_offer_details[detail_index];
    ProductDetails detail{
        std::string(detailInternal.id),
        std::string(detailInternal.title),
        std::string(detailInternal.name),
        std::string(detailInternal.description),
    };

    for (int offer_index = 0; offer_index < detailInternal.product_offers_count;
         offer_index++) {
      const internal::_ProductOffer& offerInternal =
          detailInternal.product_offers[offer_index];
      ProductOffer offer{
          offerInternal.price_micros,
          offerInternal.currency_code,
          offerInternal.formatted_price,
          offerInternal.offer_token,
      };
      detail.product_offers.push_back(std::move(offer));
    }
    resultValue.product_details.push_back(std::move(detail));
  }
  return QueryProductDetailsResult::Ok(std::move(resultValue));
}

inline internal::_QueryProductDetailsParams toInternal(
    const QueryProductDetailsParams& params) {
  internal::_QueryProductDetailsParams params_{
      params.product_ids.size(),
      new internal::_ProductId[params.product_ids.size()]};

  for (int i = 0; i < params.product_ids.size(); i++) {
    internal::_ProductId id = {
        params.product_ids[i].id.c_str(),
        params.product_ids[i].product_type,
    };
    params_.ids[i] = id;
  }
  return params_;
}

inline void freeInternal(internal::_QueryProductDetailsParams& params) {
  if (params.ids != nullptr) {
    delete[] params.ids;
  }
}

// QueryPurchases
inline ProductPurchaseDetails fromInternalProductPurchaseDetails(
    const internal::_ProductPurchaseDetails& detailsInternal) {
  return ProductPurchaseDetails{
      std::string(detailsInternal.product_id),
      std::string(detailsInternal.purchase_token),
      detailsInternal.purchase_state,
      detailsInternal.is_acknowledged,
      detailsInternal.quantity,
      std::string(detailsInternal.order_id),
      std::string(detailsInternal.obfuscated_account_id),
      std::string(detailsInternal.obfuscated_profile_id),
      detailsInternal.purchase_time_epoch_seconds,
      std::string(detailsInternal.package_name),
      std::string(detailsInternal.signature),
      std::string(detailsInternal.original_json)};
}

inline QueryPurchasesResult fromInternal(
    const _BillingResult& result,
    const internal::_QueryPurchasesResultValue& _resultValue) {
  if (result.code != BillingError::kOk) {
    return QueryPurchasesResult::Error(result.code, result.error_message);
  }

  QueryPurchasesResultValue resultValue;
  for (int i = 0; i < _resultValue.product_purchase_details_count; i++) {
    const internal::_ProductPurchaseDetails& detailsInternal =
        _resultValue.product_purchase_details[i];

    resultValue.product_purchase_details.push_back(
        fromInternalProductPurchaseDetails(detailsInternal));
  }
  return QueryPurchasesResult::Ok(std::move(resultValue));
}

// LaunchPurchaseFlow

inline internal::_LaunchPurchaseFlowParams toInternal(
    const LaunchPurchaseFlowParams& params) {
  internal::_LaunchPurchaseFlowParams params_{
      params.offer_token.c_str(),
      !params.obfuscated_account_id.empty()
          ? params.obfuscated_account_id.c_str()
          : nullptr,
      !params.obfuscated_profile_id.empty()
          ? params.obfuscated_profile_id.c_str()
          : nullptr,
      params.is_offer_personalized,
      params.quantity,
  };
  return params_;
}

inline void freeInternal(internal::_LaunchPurchaseFlowParams& params) {
  // Nothing to free. If allocations are added, they should be freed here.
}

inline LaunchPurchaseFlowResult fromInternal(
    const _BillingResult& result,
    const internal::_LaunchPurchaseFlowResultValue& _resultValue) {
  if (result.code != BillingError::kOk) {
    return LaunchPurchaseFlowResult::Error(result.code, result.error_message);
  }

  LaunchPurchaseFlowResultValue resultValue;
  resultValue.product_purchase_details =
      fromInternalProductPurchaseDetails(_resultValue.product_purchase_details);
  return LaunchPurchaseFlowResult::Ok(std::move(resultValue));
}

// AcknowledgePurchase

inline AcknowledgePurchaseResult fromInternal(
    const _BillingResult& result,
    const internal::_AcknowledgePurchaseResultValue& _resultValue) {
  if (result.code != BillingError::kOk) {
    return AcknowledgePurchaseResult::Error(result.code, result.error_message);
  }

  AcknowledgePurchaseResultValue resultValue;
  return AcknowledgePurchaseResult::Ok(std::move(resultValue));
}

inline internal::_AcknowledgePurchaseParams toInternal(
    const AcknowledgePurchaseParams& params) {
  internal::_AcknowledgePurchaseParams params_{
      params.purchase_token.c_str(),
  };
  return params_;
}

inline void freeInternal(internal::_AcknowledgePurchaseParams& params) {
  // Nothing to free. If allocations are added, they should be freed here.
}

// ConsumePurchase

inline ConsumePurchaseResult fromInternal(
    const _BillingResult& result,
    const internal::_ConsumePurchaseResultValue& _resultValue) {
  if (result.code != BillingError::kOk) {
    return ConsumePurchaseResult::Error(result.code, result.error_message);
  }

  ConsumePurchaseResultValue resultValue;
  return ConsumePurchaseResult::Ok(std::move(resultValue));
}

inline internal::_ConsumePurchaseParams toInternal(
    const ConsumePurchaseParams& params) {
  internal::_ConsumePurchaseParams params_{
      params.purchase_token.c_str(),
  };
  return params_;
}

inline void freeInternal(internal::_ConsumePurchaseParams& params) {
  // Nothing to free. If allocations are added, they should be freed here.
}

// IsBillingProgramAvailable

inline IsBillingProgramAvailableResult fromInternal(
    const _BillingResult& result,
    const internal::_IsBillingProgramAvailableResultValue& _resultValue) {
  if (result.code != BillingError::kOk) {
    return IsBillingProgramAvailableResult::Error(result.code,
                                                  result.error_message);
  }

  IsBillingProgramAvailableResultValue resultValue;
  return IsBillingProgramAvailableResult::Ok(std::move(resultValue));
}

inline internal::_IsBillingProgramAvailableParams toInternal(
    const IsBillingProgramAvailableParams& params) {
  internal::_IsBillingProgramAvailableParams internal_params;
  internal_params.billing_program = params.billing_program;
  return internal_params;
}

inline void freeInternal(internal::_IsBillingProgramAvailableParams& params) {
  // Nothing to free. If allocations are added, they should be freed here.
}

// CreateBillingProgramReportingDetails

inline CreateBillingProgramReportingDetailsResult fromInternal(
    const _BillingResult& result,
    const internal::_CreateBillingProgramReportingDetailsResultValue&
        _resultValue) {
  if (result.code != BillingError::kOk) {
    return CreateBillingProgramReportingDetailsResult::Error(
        result.code, result.error_message);
  }

  CreateBillingProgramReportingDetailsResultValue resultValue;
  resultValue.external_transaction_token =
      std::string(_resultValue.external_transaction_token);
  return CreateBillingProgramReportingDetailsResult::Ok(std::move(resultValue));
}

inline internal::_CreateBillingProgramReportingDetailsParams toInternal(
    const CreateBillingProgramReportingDetailsParams& params) {
  internal::_CreateBillingProgramReportingDetailsParams internal_params;
  internal_params.billing_program = params.billing_program;
  return internal_params;
}

inline void freeInternal(
    internal::_CreateBillingProgramReportingDetailsParams& params) {
  // Nothing to free. If allocations are added, they should be freed here.
}

}  // namespace internal
}  // namespace billing
}  // namespace play
}  // namespace google

#endif  // PLAY_BILLING_INTERNAL_MODEL_CONVERSIONS_H_

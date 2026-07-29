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

#ifndef PLAY_BILLING_MODELS_H_
#define PLAY_BILLING_MODELS_H_

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "billing/enums.h"
#include "libraries/core/result.h"

namespace google {
namespace play {
namespace billing {

/**
 * @brief Params for constructing a BillingClient.
 */
struct BillingClientParams {
  /**
   * @brief Enables pending purchase for one-time products.
   *
   * This value must be set to true. Pending purchases are
   * not automatically enabled since your application will need to be updated to
   * ensure entitlement is not granted before payment has been secured. For more
   * information on how to handle pending transactions see
   * https://developer.android.com/google/play/billing/billing_library_overview.
   *
   * Mandatory.
   */
  bool enable_pending_purchases = false;
};

/**
 * @brief The Google Play Store product identifier.
 */
struct ProductId {
  /**
   * The product's unique identifier.
   */
  std::string id;

  /**
   * The type of the product.
   */
  ProductType product_type;
};

/**
 * @brief Params for querying product details.
 *
 * See BillingClient::QueryProductDetails for more details.
 */
struct QueryProductDetailsParams {
  /**
   * Products to query.
   */
  std::vector<ProductId> product_ids;
};

/**
 * @brief Offer details for a product.
 */
struct ProductOffer {
  /**
   * Cost in the \ref currency_code. Price is provided in
   * micro-units, where 1,000,000 micro-units equal one unit of the currency.
   */
  int64_t price_micros;

  /**
   * The ISO 4217 currency code of the price.
   */
  std::string currency_code;

  /**
   * The formatted price for of the offer, including its currency sign.
   */
  std::string formatted_price;

  /**
   * Unique token representing the product offer.
   */
  std::string offer_token;
};

/**
 * @brief Details for a product.
 */
struct ProductDetails {
  /**
   * The product's unique identifier.
   */
  std::string id;

  /**
   * The product's title.
   */
  std::string title;

  /**
   * The product's name.
   */
  std::string name;

  /**
   * The product's description.
   */
  std::string description;

  /**
   * Collection of offers available for the product.
   */
  std::vector<ProductOffer> product_offers;
};

/**
 * @brief %Result value for querying product details.
 *
 * See BillingClient::QueryProductDetails for more details.
 */
struct QueryProductDetailsResultValue {
  std::vector<ProductDetails> product_details;
};

using QueryProductDetailsResult =
    Result<BillingError, QueryProductDetailsResultValue>;

using QueryProductDetailsContinuation =
    std::function<void(QueryProductDetailsResult)>;

/**
 * @brief Details for a purchased product.
 */
struct ProductPurchaseDetails {
  /**
   * Unique identifier for the product.
   */
  std::string product_id;

  /**
   * Token representing the purchase.
   */
  std::string purchase_token;

  /**
   * State of the purchase.
   */
  PurchaseState purchase_state;

  /**
   * The acknowledgement state of this purchase.
   */
  bool is_acknowledged;

  /**
   * The quantity of the purchase.
   */
  int64_t quantity;

  /**
   * @brief A unique order identifier for the transaction.
   *
   * This identifier corresponds to the Google order ID.
   *
   * The order ID will be an empty string if the purchase is in the
   * PurchaseState::kPurchaseStatePending state and populated if the purchase
   * has transitioned to the PurchaseState::kPurchaseStatePurchased state.
   */
  std::string order_id;

  /**
   * @brief The obfuscated account id for the purchase.
   *
   * An obfuscated version of the ID that is uniquely associated with the
   * user's account in the game. Only present (non-empty) if specified when the
   * purchase flow was launched.
   *
   * See LaunchPurchaseFlowParams::obfuscated_account_id for more details.
   */
  std::string obfuscated_account_id;

  /**
   * @brief The obfsucated profile id for the purchase.
   *
   * An obfuscated version of the id that is uniquely associated with the
   * user's profile in the game. Only present (non-empty) if specified when the
   * purchase flow was launched.
   *
   * See LaunchPurchaseFlowParams::obfuscated_profile_id for more details.
   */
  std::string obfuscated_profile_id;

  /**
   * The time the purchase was made in seconds since the epoch.
   */
  int64_t purchase_time_epoch_seconds;

  /**
   * The package name of the game associated with this purchase.
   */
  std::string package_name;

  /**
   * @brief Signature of the `original_json` data that was signed with the
   * private key of the developer.
   * The data signature uses the RSASSA-PKCS1-v1_5 scheme.
   *
   * Note that usage of this parameter requires your application to be
   * allow-listed. Please contact your Google Partner if your game requires
   * access.
   */
  std::string signature;

  /**
   * @brief String in JSON format that contains details about the purchase
   * order.
   *
   * Note that usage of this parameter requires your application to be
   * allow-listed. Please contact your Google Partner if your game requires
   * access.
   */
  std::string original_json;
};

/**
 * @brief %Result value for querying purchases.
 *
 * See BillingClient::QueryPurchases for more details.
 */
struct QueryPurchasesResultValue {
  std::vector<ProductPurchaseDetails> product_purchase_details;
};

using QueryPurchasesResult = Result<BillingError, QueryPurchasesResultValue>;

using QueryPurchasesContinuation = std::function<void(QueryPurchasesResult)>;

/**
 * @brief Params for launching the purchase flow.
 *
 * See BillingClient::LaunchPurchaseFlow for more details.
 */
struct LaunchPurchaseFlowParams {
  /**
   * Unique token representing the product offer that is being purchased.
   */
  std::string offer_token;

  /**
   * @brief The obfuscated account id for the purchase.
   *
   * If non-empty, specifies an optional obfuscated string that is uniquely
   * associated with the purchaser's user account in your game.
   *
   * If you specify this value, Google Play can use it to detect irregular
   * activity, such as many devices making purchases on the same account in a
   * short period of time. Do not use this field to store any Personally
   * Identifiable Information (PII) such as emails in cleartext. Attempting to
   * store PII in this field will result in purchases being blocked. Google Play
   * recommends that you use either encryption or a one-way hash to generate an
   * obfuscated identifier to send to Google Play.
   *
   * This identifier is limited to 64 characters.
   *
   * Leave empty if you do not want to specify an obfuscated account id.
   */
  std::string obfuscated_account_id;

  /**
   * @brief The obfuscated profile id for the purchase.
   *
   * If non-empty, specifies an optional obfuscated string that is uniquely
   * associated with the purchaser's user profile in your game.
   *
   * If you specify this value, Google Play can use it to detect irregular
   * activity, such as many devices making purchases on the same account in a
   * short period of time. Do not use this field to store any Personally
   * Identifiable Information (PII) such as emails in cleartext. Attempting to
   * store PII in this field will result in purchases being blocked. Google Play
   * recommends that you use either encryption or a one-way hash to generate an
   * obfuscated identifier to send to Google Play.
   *
   * This identifier is limited to 64 characters.
   *
   * Leave empty if you do not want to specify an obfuscated profile id.
   */
  std::string obfuscated_profile_id;

  /**
   * Specifies whether the offer is personalized to the buyer.
   *
   * This is used to disclose to the buyer that the product's price was
   * personalized using automated decision-making.
   *
   * You must consult Art. 6 (1) (ea) CRD of the Consumer Rights Directive
   * 2011/83/EU to determine if the price you are offering to users is
   * personalized.
   */
  bool is_offer_personalized = false;

  /**
   * The quantity of the product requested to be purchased.
   *
   * This value defaults to 1 and must be greater than 0. The product must also
   * have multi-quantity enabled.
   *
   * Note that the quantity of the product purchased is not guaranteed to be
   * the quantity requested. The buyer may have the option to change the
   * quantity of the product to be purchased during the checkout process. On a
   * successful purchase, the actual quantity of the product purchased is
   * returned in ProductPurchaseDetails::quantity.
   *
   * Note that support for multi-quantity checkout may not be available in some
   * countries / regions. If the buyer or the product is not eligible for
   * multi-quantity checkout, only a single product will be purchased.
   *
   * Note that usage of this parameter requires your application to be
   * allow-listed. Please contact your Google Partner if your game requires
   * access.
   */
  int64_t quantity = 1;
};

/**
 * @brief %Result value for launching the purchase flow.
 *
 * See BillingClient::LaunchPurchaseFlow for more details.
 */
struct LaunchPurchaseFlowResultValue {
  /**
   * The details of the purchased product.
   */
  ProductPurchaseDetails product_purchase_details;
};

using LaunchPurchaseFlowResult =
    Result<BillingError, LaunchPurchaseFlowResultValue>;

using LaunchPurchaseFlowContinuation =
    std::function<void(LaunchPurchaseFlowResult)>;

/**
 * @brief Params for acknowledging a purchase.
 *
 * See BillingClient::AcknowledgePurchase for more details.
 */
struct AcknowledgePurchaseParams {
  /**
   * @brief Purchase token for purchase being acknowledged.
   *
   * See ProductPurchaseDetails::purchase_token for more details.
   */
  std::string purchase_token;
};

/**
 * @brief %Result value for acknowledging a purchase.
 *
 * See BillingClient::AcknowledgePurchase for more details.
 */
struct AcknowledgePurchaseResultValue {};

using AcknowledgePurchaseResult =
    Result<BillingError, AcknowledgePurchaseResultValue>;

using AcknowledgePurchaseContinuation =
    std::function<void(AcknowledgePurchaseResult)>;

/**
 * @brief Params for consuming a purchase.
 *
 * See BillingClient::ConsumePurchase for more details.
 */
struct ConsumePurchaseParams {
  /**
   * @brief Purchase token for purchase being consumed.
   *
   * See ProductPurchaseDetails::purchase_token for more details.
   */
  std::string purchase_token;
};

/**
 * @brief %Result value for consuming a purchase.
 *
 * See BillingClient::ConsumePurchase for more details.
 */
struct ConsumePurchaseResultValue {};

using ConsumePurchaseResult = Result<BillingError, ConsumePurchaseResultValue>;

using ConsumePurchaseContinuation = std::function<void(ConsumePurchaseResult)>;

/**
 * @brief Params for checking if a billing program is available.
 *
 * See BillingClient::IsBillingProgramAvailable for more details.
 */
struct IsBillingProgramAvailableParams {
  /**
   * The billing program to check availability for.
   */
  BillingProgram billing_program;
};

/**
 * @brief %Result value that represents a billing program is available.
 *
 *
 * See BillingClient::IsBillingProgramAvailable for more details.
 */
struct IsBillingProgramAvailableResultValue {};

using IsBillingProgramAvailableResult =
    Result<BillingError, IsBillingProgramAvailableResultValue>;

using IsBillingProgramAvailableContinuation =
    std::function<void(IsBillingProgramAvailableResult)>;

/**
 * @brief Params for creating billing program reporting details.
 *
 * See BillingClient::CreateBillingProgramReportingDetails for more details.
 */
struct CreateBillingProgramReportingDetailsParams {
  /**
   * The billing program associated with the request.
   */
  BillingProgram billing_program;
};

/**
 * @brief %Result value for creating billing program reporting details.
 *
 * See BillingClient::CreateBillingProgramReportingDetails for more details.
 */
struct CreateBillingProgramReportingDetailsResultValue {
  /**
   * The external transaction token.
   *
   * This token is used to report external transactions to
   * Play Billing using the server-side `Externaltransactions`
   * API.
   */
  std::string external_transaction_token;
};

using CreateBillingProgramReportingDetailsResult =
    Result<BillingError, CreateBillingProgramReportingDetailsResultValue>;

using CreateBillingProgramReportingDetailsContinuation =
    std::function<void(CreateBillingProgramReportingDetailsResult)>;

}  // namespace billing
}  // namespace play
}  // namespace google

#endif  // PLAY_BILLING_MODELS_H_

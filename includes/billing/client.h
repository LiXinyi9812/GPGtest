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

#ifndef PLAY_BILLING_CLIENT_H_
#define PLAY_BILLING_CLIENT_H_

#include <functional>

#include "billing/internal/client.h"
#include "billing/internal/model_conversions.h"
#include "billing/internal/models.h"
#include "billing/models.h"

namespace google {
namespace play {
namespace billing {

/**
 * @brief Entrypoint for the Google Play Billing API.
 */
class BillingClient {
 public:
  explicit BillingClient(const BillingClientParams& params) {
    client_ = internal::_BillingClient_Create(params);
  };

  ~BillingClient() { internal::_BillingClient_Destroy(client_); };

  /**
   * @brief Queries for products available to the user so they may be purchased.
   *
   * Querying for product details is an important step before displaying your
   * products to your users, as it returns localized product information.
   *
   * When querying for product details, pass a list of product identifiers
   * associated with products registered for your application in the Google Play
   * Console.
   *
   * The response contains a list of \ref ProductDetails for each product
   * identifier passed in the request that was available to the user. Each
   * contains a list of offers (see \ref ProductOffer) with pricing details
   * as well as an offer token. To purchase a product, pass the offer token
   * associated with the offer into the \ref LaunchPurchaseFlow API.
   *
   * @param[in] params Parameters containing the collection of product
   * identifiers to be queried.
   * @param[in] continuation The async callback to return the data to.
   */
  void QueryProductDetails(const QueryProductDetailsParams& params,
                           QueryProductDetailsContinuation continuation) {
    auto context = new std::function<void(QueryProductDetailsResult)>;
    *context = continuation;
    internal::_QueryProductDetailsParams paramsInternal =
        internal::toInternal(params);
    internal::_BillingClient_QueryProductDetails(
        client_, paramsInternal, reinterpret_cast<void*>(context),
        &BillingClient::QueryProductDetailsComplete);
    freeInternal(paramsInternal);
  }

  /**
   * @brief Launches the purchase flow for the requested product offers.
   *
   * Start a purchase flow for the given product offers previously queried
   * using the \ref QueryProductDetails API.
   *
   * On a successful purchase, the continuation will be called with a
   * \ref LaunchPurchaseFlowResultValue containing a \ref ProductPurchaseDetail
   * with a purchase token. This purchase token should then be acknowledged
   * once the purchase has been granted to the user in your application
   * (see \ref AcknowledgePurchase) or consumed if it should be possible for the
   * user to purchase the product again (see \ref ConsumePurchase). Failure to
   * do so will cause the purchase to automatically refunded.
   *
   * Purchases can optionally be associated with the details of your
   * application's user profile or account (see \ref
   * LaunchPurchaseFlowParams::obfuscated_account_id and \ref
   * LaunchPurchaseFlowParams::obfuscated_profile_id). These values help Google
   * Play detect irregular activity, such as many devices making purchases on
   * the same account in a short period of time and prevent fraud.
   *
   * If the user does not complete the purchase within a set timeout period, the
   * continuation will be called with a \ref LaunchPurchaseFlowResult
   * containing the error \ref BillingError::kUserCanceled.
   *
   * @param[in] params Parameters specifying the product offer to be purchased.
   * @param[in] continuation The async callback to return the data to.
   */
  void LaunchPurchaseFlow(const LaunchPurchaseFlowParams& params,
                          LaunchPurchaseFlowContinuation continuation) {
    auto context = new std::function<void(LaunchPurchaseFlowResult)>;
    *context = continuation;
    internal::_LaunchPurchaseFlowParams paramsInternal =
        internal::toInternal(params);
    internal::_BillingClient_LaunchPurchaseFlow(
        client_, paramsInternal, reinterpret_cast<void*>(context),
        &BillingClient::LaunchPurchaseFlowComplete);
    freeInternal(paramsInternal);
  }

  /**
   * @brief Returns purchases associated with the current user.
   *
   * Queries for purchases made by the current user. This includes both
   * completed as well as pending purchases. Pending purchases should be
   * processed by your application to ensure that the user is granted
   * entitlement for the purchased product. Note that once a purchase is
   * consumed (see \ref ConsumePurchase) it will no longer be returned by this
   * API.
   *
   * It is recommended to query for purchases when your application start as
   * well as when it is foregrounded to check for purchases that may have been
   * while your application was not running or on a different device.
   *
   * The response contains a list of \ref ProductPurchaseDetail for each
   * purchase made by the user.
   *
   * Check for purchases that are ready to be processed. These are purchases
   * that have not yet been acknowledged or consumed and have a purchase state
   * of
   * \ref PurchaseState::kPurchaseStatePurchased. To process these
   * purchases use the associated purchase token to either acknowledge the
   * purchase (see \ref AcknowledgePurchase) or consume the purchase (see \ref
   * ConsumePurchase).
   *
   * If your application is client-only, you may check for one-time purchases
   * the user has a recorded entitlement for using
   * \ref ProductPurchaseDetails::is_acknowledged. For example, in a single
   * player game without a backend game server, the game could use this to check
   * if the user has purchased a cosmetic item or access to paid game content.
   *
   *
   * @param[in] continuation The async callback to return the data to.
   */
  void QueryPurchases(QueryPurchasesContinuation continuation) {
    auto context = new std::function<void(QueryPurchasesResult)>;
    *context = continuation;
    internal::_BillingClient_QueryPurchases(
        client_, reinterpret_cast<void*>(context),
        &BillingClient::QueryPurchasesComplete);
  }

  /**
   * @brief Acknowledges a purchase with Play Billing.
   *
   * Acknowledges that an entitlement of the purchase has been granted to the
   * user. This updates the Play Billing system's record of the purchase. Future
   * calls to \ref QueryPurchases wll now reflect that purchase as acknowledged.
   *
   * If your application has a secure backend, we recommend that you use the
   * server-side API to acknowledge the purchase instead of this API inside of
   * SDK.
   *
   * If your application is client-only, use this API to acknowledge the
   * purchase. Note that usage of this client-side API requires your application
   * to be allow-listed.
   *
   * Acknowledgement is intended to be used for one-time purchases such as a
   * cosmetic item in a game. If it should be possible for the user to purchase
   * that product again see \ref ConsumePurchase.
   *
   * @param[in] params Parameters containing the purchase token of product to
   * be acknowledged.
   * @param[in] continuation The async callback to return the data to.
   */
  void AcknowledgePurchase(const AcknowledgePurchaseParams& params,
                           AcknowledgePurchaseContinuation continuation) {
    auto context = new std::function<void(AcknowledgePurchaseResult)>;
    *context = continuation;
    internal::_AcknowledgePurchaseParams paramsInternal =
        internal::toInternal(params);
    internal::_BillingClient_AcknowledgePurchase(
        client_, paramsInternal, reinterpret_cast<void*>(context),
        &BillingClient::AcknowledgePurchaseComplete);
    freeInternal(paramsInternal);
  }

  /**
   * @brief Consumes a purchase with Play Billing.
   *
   * Consuming a purchased product removes the purchase from future purchase
   * queries (see \ref QueryPurchases) and allows a user to purchase the same
   * product again.
   *
   * If your application is client-only, use this API to consume the
   * purchase. Note that usage of this client-side API requires your application
   * to be allow-listed.
   *
   * Consuming is intended to be used for products that can be purchased
   * multiple times by the user such as a virtual currency. For purchase that
   * should only be purchased once, see \ref AcknowledgePurchase.
   *
   * @param[in] request Parameters containing the purchase token of the product
   * to be consumed.
   * @param[in] continuation The async callback to return the data to.
   */
  void ConsumePurchase(const ConsumePurchaseParams& params,
                       ConsumePurchaseContinuation continuation) {
    auto context = new std::function<void(ConsumePurchaseResult)>;
    *context = continuation;
    internal::_ConsumePurchaseParams paramsInternal =
        internal::toInternal(params);
    internal::_BillingClient_ConsumePurchase(
        client_, paramsInternal, reinterpret_cast<void*>(context),
        &BillingClient::ConsumePurchaseComplete);
    freeInternal(paramsInternal);
  }

  /**
   * @brief Checks if a billing program is available.
   *
   * Determines if the current user is eligible for the specified billing
   * program. Before offering programs outside of Google Play Billing,
   * your game must complete the required enrollment steps.
   *
   * If a billing program is available for the current user, and your
   * application performs an external transaction, you must report this
   * transaction using \ref CreateBillingProgramReportingDetails.
   *
   * @param[in] params Parameters containing the billing program.
   * @param[in] continuation The async callback to return the data to.
   */
  void IsBillingProgramAvailable(
      const IsBillingProgramAvailableParams& params,
      IsBillingProgramAvailableContinuation continuation) {
    auto context = new std::function<void(IsBillingProgramAvailableResult)>;
    *context = continuation;
    internal::_IsBillingProgramAvailableParams paramsInternal =
        internal::toInternal(params);
    internal::_BillingClient_IsBillingProgramAvailable(
        client_, paramsInternal, reinterpret_cast<void*>(context),
        &BillingClient::IsBillingProgramAvailableComplete);
    freeInternal(paramsInternal);
  }

  /**
   * @brief Creates billing program reporting details.
   *
   * Returns an external transaction token that may be used to report
   * transactions happening outside Google Play's billing system in supported
   * countries, such as alternative billing transactions.
   *
   * The external transaction token is a required parameter when reporting
   * an external transaction using the server-side `Externaltransactions`
   * Play Billing API.
   *
   * Before requesting billing program reporting details, ensure that
   * the current user is eligible using \ref IsBillingProgramAvailable.
   *
   * @param[in] params Parameters containing the billing program.
   * @param[in] continuation The async callback to return the data to.
   */
  void CreateBillingProgramReportingDetails(
      const CreateBillingProgramReportingDetailsParams& params,
      CreateBillingProgramReportingDetailsContinuation continuation) {
    auto context =
        new std::function<void(CreateBillingProgramReportingDetailsResult)>;
    *context = continuation;
    internal::_CreateBillingProgramReportingDetailsParams paramsInternal =
        internal::toInternal(params);
    internal::_BillingClient_CreateBillingProgramReportingDetails(
        client_, paramsInternal, reinterpret_cast<void*>(context),
        &BillingClient::CreateBillingProgramReportingDetailsComplete);
    freeInternal(paramsInternal);
  }

 private:
  static void QueryProductDetailsComplete(
      void* context, internal::_BillingResult result,
      internal::_QueryProductDetailsResultValue resultValue) {
    auto continuation =
        reinterpret_cast<std::function<void(QueryProductDetailsResult)>*>(
            context);
    (*continuation)(internal::fromInternal(result, resultValue));
    delete continuation;
  }

  static void LaunchPurchaseFlowComplete(
      void* context, internal::_BillingResult result,
      internal::_LaunchPurchaseFlowResultValue resultValue) {
    auto continuation =
        reinterpret_cast<std::function<void(LaunchPurchaseFlowResult)>*>(
            context);
    (*continuation)(internal::fromInternal(result, resultValue));
    delete continuation;
  }

  static void QueryPurchasesComplete(
      void* context, internal::_BillingResult result,
      internal::_QueryPurchasesResultValue resultValue) {
    auto continuation =
        reinterpret_cast<std::function<void(QueryPurchasesResult)>*>(context);
    (*continuation)(internal::fromInternal(result, resultValue));
    delete continuation;
  }

  static void AcknowledgePurchaseComplete(
      void* context, internal::_BillingResult result,
      internal::_AcknowledgePurchaseResultValue resultValue) {
    auto continuation =
        reinterpret_cast<std::function<void(AcknowledgePurchaseResult)>*>(
            context);
    (*continuation)(internal::fromInternal(result, resultValue));
    delete continuation;
  }

  static void ConsumePurchaseComplete(
      void* context, internal::_BillingResult result,
      internal::_ConsumePurchaseResultValue resultValue) {
    auto continuation =
        reinterpret_cast<std::function<void(ConsumePurchaseResult)>*>(context);
    (*continuation)(internal::fromInternal(result, resultValue));
    delete continuation;
  }

  static void IsBillingProgramAvailableComplete(
      void* context, internal::_BillingResult result,
      internal::_IsBillingProgramAvailableResultValue resultValue) {
    auto continuation =
        reinterpret_cast<std::function<void(IsBillingProgramAvailableResult)>*>(
            context);
    (*continuation)(internal::fromInternal(result, resultValue));
    delete continuation;
  }

  static void CreateBillingProgramReportingDetailsComplete(
      void* context, internal::_BillingResult result,
      internal::_CreateBillingProgramReportingDetailsResultValue resultValue) {
    auto continuation = reinterpret_cast<
        std::function<void(CreateBillingProgramReportingDetailsResult)>*>(
        context);
    (*continuation)(internal::fromInternal(result, resultValue));
    delete continuation;
  }

 private:
  void* client_;
};

}  // namespace billing
}  // namespace play
}  // namespace google

#endif  // PLAY_BILLING_CLIENT_H_

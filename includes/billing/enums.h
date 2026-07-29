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

#ifndef PLAY_BILLING_ERROR_H_
#define PLAY_BILLING_ERROR_H_

namespace google {
namespace play {
namespace billing {

/**
 * @brief Error codes returned by Google Play Billing.
 */
enum class BillingError {
  /**
   * The operation completed successfully.
   */
  kOk = 0,

  /**
   * A user billing error occurred during processing.
   */
  kBillingUnavailable = 3,

  /**
   * Error resulting from incorrect usage of the API.
   */
  kDeveloperError = 5,

  /**
   * Fatal error during the API action.
   */
  kError = 6,

  /**
   * The purchase failed because the item is already owned.
   */
  kItemAlreadyOwned = 7,

  /**
   * Requested action on the item failed since it is not owned by the user.
   */
  kItemNotOwned = 8,

  /**
   * The requested product is not available for purchase.
   */
  kItemUnavailable = 4,

  /**
   * A network error occurred during the operation.
   */
  kNetworkError = 12,

  /**
   * The app is not connected to the Google Play Billing Library.
   */
  kServiceDisconnected = -1,

  /**
   * The service is currently unavailable.
   */
  kServiceUnavailable = 2,

  /**
   * Transaction was canceled by the user. This may be the result of the user
   * explicitly aborting the transaction or implicitly cancelling the
   * transaction by timing out.
   */
  kUserCanceled = 1,
};

/**
 * @brief The type of the product.
 */
enum ProductType {
  /**
   * The type of the product is an in-app item.
   */
  kTypeInApp = 1,
};

/**
 * @brief The purchase state of the order.
 */
enum PurchaseState {
  /**
   * Purchase with unknown state.
   */
  kPurchaseStateUnspecified = 0,

  /**
   * Purchased is completed.
   */
  kPurchaseStatePurchased = 1,

  /**
   * Purchase is pending and not yet completed to be processed by your app.
   */
  kPurchaseStatePending = 2,
};

/**
 * @brief The type of billing program available for the product.
 */
enum BillingProgram {
  /**
   * Alternative billing only (without user choice).
   */
  kBillingProgramAlternativeBillingOnly = 1,

  /**
   * Alternative billing with user choice.
   */
  kBillingProgramUserChoice = 2,
};

}  // namespace billing
}  // namespace play
}  // namespace google

#endif  // PLAY_BILLING_ERROR_H_

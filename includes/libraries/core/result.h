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

#ifndef PLAY_LIBRARIES_CORE_RESULT_H_
#define PLAY_LIBRARIES_CORE_RESULT_H_

#include <string>

namespace google {
namespace play {

/**
 * @brief Represents the outcome of an operation.
 *
 * This contains either a successful result with a value, or an error result
 * with a error code and error message.
 */
template <typename R, typename V>
class Result {
 public:
  Result(R code, const char* error_message)
      : code_(code), error_message_(error_message), value_(V()) {}
  explicit Result(V value)
      : code_(R::kOk),
        error_message_(""),
        value_(value) {}
  Result(R code, const char* error_message, V value)
      : code_(code), error_message_(error_message), value_(value) {}

  static Result Ok(V value) { return Result<R, V>(value); }
  static Result Error(R code, const char* error_message) {
    return Result<R, V>(code, error_message);
  }

  /**
   * @brief Returns the error code associated with the result.
   */
  R code() const { return code_; }

  /**
   * @brief Returns the error message.
   */
  const std::string& error_message() const { return error_message_; }

  /**
   * @brief Returns the value associated with the result. Only valid if the
   * result is OK.
   */
  const V& value() const { return value_; }

  /**
   * @brief Returns true if the result is OK.
   */
  bool ok() const { return code_ == R::kOk; }

 private:
  // The error code.
  R code_;

  // The error message.
  std::string error_message_;

  // The value associated with the result.
  V value_;
};

}  // namespace play
}  // namespace google

#endif  // PLAY_LIBRARIES_CORE_RESULT_H_

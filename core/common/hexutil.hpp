/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_HEXUTIL_HPP
#define KAGOME_HEXUTIL_HPP

#include <string_view>
#include <vector>

#include "common/result.hpp"
#include "common/unhex_errors.hpp"

namespace kagome::common {

  /**
   * @brief Converts bytes to hex representation
   * @tparam ToUpper - true, if conversion is to result in uppercase string,
   * false, if in lowercase
   * @param array bytes
   * @param len length of bytes
   * @return hexstring
   */
  template <bool ToUpper = true>
  std::string hex(const uint8_t *array, size_t len) noexcept;
  template <bool ToUpper = true>
  std::string hex(const std::vector<uint8_t> &bytes) noexcept;

  /**
   * @brief Converts hex representation to bytes
   * @param array individual chars
   * @param len length of chars
   * @return Result containing array of bytes if input string is hex encoded and
   * has even length. Otherwise Result containing error is returned
   *
   * @note reads both uppercase and lowercase hexstrings
   *
   * @see
   * https://www.boost.org/doc/libs/1_51_0/libs/algorithm/doc/html/the_boost_algorithm_library/Misc/hex.html
   */
  expected::Result<std::vector<uint8_t>, UnhexError> unhex(
      std::string_view hex);

}  // namespace kagome::common

#endif  // KAGOME_HEXUTIL_HPP

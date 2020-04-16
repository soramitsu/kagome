/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 *
 * Kagome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Kagome is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef KAGOME_TEST_TESTUTIL_TESTPARAM_HPP
#define KAGOME_TEST_TESTUTIL_TESTPARAM_HPP

#include <cstdint>
#include <vector>

namespace testutil {
  template <typename T>
  struct TestParam {
    std::vector<uint8_t> encoded_value{};
    bool should_fail{false};
    T value{};
  };

  template <typename T>
  TestParam<T> make_param(std::vector<uint8_t> &&buffer,
                          bool should_fail,
                          T &&value) {
    return {buffer, should_fail, std::forward<T>(value)};
  };
}  // namespace testutil

#endif  // KAGOME_TEST_TESTUTIL_TESTPARAM_HPP

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

#ifndef KAGOME_CRYPTO_TWOX_HPP
#define KAGOME_CRYPTO_TWOX_HPP

#include "common/blob.hpp"

namespace kagome::crypto {

  // TODO(warchant): PRE-357 refactor to span

  common::Hash64 make_twox64(gsl::span<const uint8_t> buf);

  common::Hash128 make_twox128(gsl::span<const uint8_t> buf);

  common::Hash256 make_twox256(gsl::span<const uint8_t> buf);

}  // namespace kagome::crypto

#endif  // KAGOME_CRYPTO_TWOX_HPP

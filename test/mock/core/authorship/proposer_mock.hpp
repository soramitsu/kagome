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

#ifndef KAGOME_PROPOSER_MOCK_HPP
#define KAGOME_PROPOSER_MOCK_HPP

#include "authorship/proposer.hpp"

#include <gmock/gmock.h>

namespace kagome::authorship {
  class ProposerMock : public Proposer {
   public:
    MOCK_METHOD3(
        propose,
        outcome::result<primitives::Block>(const primitives::BlockId &,
                                           const primitives::InherentData &,
                                           const primitives::Digest &));
  };
}  // namespace kagome::authorship

#endif  // KAGOME_PROPOSER_MOCK_HPP

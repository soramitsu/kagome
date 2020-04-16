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

#ifndef KAGOME_BABE_LOTTERY_MOCK_HPP
#define KAGOME_BABE_LOTTERY_MOCK_HPP

#include "consensus/babe/babe_lottery.hpp"

#include <gmock/gmock.h>

namespace kagome::consensus {
  struct BabeLotteryMock : public BabeLottery {
    MOCK_CONST_METHOD3(slotsLeadership,
                       SlotsLeadership(const Epoch &,
                                       const Threshold &,
                                       const crypto::SR25519Keypair &));

    MOCK_METHOD2(computeRandomness, Randomness(const Randomness &, EpochIndex));

    MOCK_METHOD1(submitVRFValue, void(const crypto::VRFPreOutput &));
  };
}  // namespace kagome::consensus

#endif  // KAGOME_BABE_LOTTERY_MOCK_HPP

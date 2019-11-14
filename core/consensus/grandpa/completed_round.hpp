/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_CONSENSUS_GRANDPA_COMPLETED_ROUND_HPP
#define KAGOME_CORE_CONSENSUS_GRANDPA_COMPLETED_ROUND_HPP

#include "consensus/grandpa/round_state.hpp"

namespace kagome::consensus::grandpa {

  struct CompletedRound {
    RoundNumber round_number;
    RoundState state;
  };

  template <class Stream,
            typename = std::enable_if_t<Stream::is_encoder_stream>>
  Stream &operator<<(Stream &s, const CompletedRound &round) {
    return s << round.round_number << round.state;
  }

  template <class Stream,
            typename = std::enable_if_t<Stream::is_decoder_stream>>
  Stream &operator>>(Stream &s, CompletedRound &round) {
    return s >> round.round_number >> round.state;
  }

}  // namespace kagome::consensus::grandpa

#endif  // KAGOME_CORE_CONSENSUS_GRANDPA_COMPLETED_ROUND_HPP

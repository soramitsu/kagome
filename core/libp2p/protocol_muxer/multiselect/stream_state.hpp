/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_STREAM_STATE_HPP
#define KAGOME_STREAM_STATE_HPP

#include <functional>

#include "libp2p/protocol_muxer/protocol_muxer.hpp"
#include "libp2p/stream/stream.hpp"

namespace libp2p::protocol_muxer {
  /**
   * Stores current state of protocol negotiation over the stream
   */
  struct StreamState {
    enum class NegotiationStatus {
      OPENING_SENT,
      PROTOCOL_SENT,
      PROTOCOLS_SENT,
      LS_SENT,
      NA_SENT
    };

    std::reference_wrapper<const stream::Stream> stream_;
    ProtocolMuxer::ChosenProtocolCallback proto_callback_;
    NegotiationStatus status_;
  };
}  // namespace libp2p::protocol_muxer

#endif  // KAGOME_STREAM_STATE_HPP

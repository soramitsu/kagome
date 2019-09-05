/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_SYNCHRONIZER_HPP
#define KAGOME_SYNCHRONIZER_HPP

#include "network/sync_protocol_client.hpp"
#include "network/sync_protocol_observer.hpp"
#include "primitives/block_header.hpp"

namespace kagome::consensus {
  /**
   * Synchronizer, which retrieves blocks from other peers; mostly used to
   * *synchronize* this node's state with the state of other nodes, for example,
   * if this node does not have some blocks
   */
  struct Synchronizer : public network::SyncProtocolClient,
                        public network::SyncProtocolObserver {
    // TODO(akvinikym) PRE-304: move it to BabeGossiper
    /**
     * Announce the network about a new block
     * @param block_header to be announced
     */
    virtual void announce(const primitives::BlockHeader &block_header) = 0;
  };
}  // namespace kagome::consensus

#endif  // KAGOME_SYNCHRONIZER_HPP

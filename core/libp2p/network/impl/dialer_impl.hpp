/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_DIALER_IMPL_HPP
#define KAGOME_DIALER_IMPL_HPP

#include "libp2p/connection/capable_connection.hpp"
#include "libp2p/multi/multiaddress.hpp"
#include "libp2p/network/connection_manager.hpp"
#include "libp2p/network/dialer.hpp"
#include "libp2p/network/listener.hpp"
#include "libp2p/network/network.hpp"
#include "libp2p/network/transport_manager.hpp"
#include "libp2p/peer/identity_manager.hpp"
#include "libp2p/peer/peer_id.hpp"
#include "libp2p/peer/peer_repository.hpp"
#include "libp2p/protocol_muxer/protocol_muxer.hpp"
#include "libp2p/transport/transport_listener.hpp"
#include "libp2p/transport/upgrader.hpp"

namespace libp2p::network {

  class DialerImpl : public Dialer {
   public:
    ~DialerImpl() override = default;

    DialerImpl(std::shared_ptr<protocol_muxer::ProtocolMuxer> multiselect,
               std::shared_ptr<TransportManager> tmgr,
               std::shared_ptr<ConnectionManager> cmgr);

    // Establishes a connection to a given peer
    void dial(const peer::PeerInfo &p, DialResultFunc cb) override;

    // NewStream returns a new stream to given peer p.
    // If there is no connection to p, attempts to create one.
    void newStream(const peer::PeerInfo &p, const peer::Protocol &protocol,
                   StreamResultFunc cb) override;

   private:
    std::shared_ptr<protocol_muxer::ProtocolMuxer> multiselect_;
    std::shared_ptr<TransportManager> tmgr_;
    std::shared_ptr<ConnectionManager> cmgr_;
  };

}  // namespace libp2p::network

#endif  // KAGOME_DIALER_IMPL_HPP

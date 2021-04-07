/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_NETWORK_GOSSIPPROTOCOL
#define KAGOME_NETWORK_GOSSIPPROTOCOL

#include <memory>

#include <libp2p/connection/stream.hpp>
#include <libp2p/host/host.hpp>

#include "application/chain_spec.hpp"
#include "blockchain/block_storage.hpp"
#include "blockchain/block_tree.hpp"
#include "consensus/grandpa/grandpa_observer.hpp"
#include "crypto/hasher.hpp"
#include "log/logger.hpp"
#include "network/babe_observer.hpp"
#include "network/gossiper.hpp"
#include "network/impl/loopback_stream.hpp"
#include "network/impl/stream_engine.hpp"
#include "network/peer_manager.hpp"
#include "network/protocol_base.hpp"
#include "network/types/block_announce.hpp"
#include "network/types/gossip_message.hpp"
#include "network/types/own_peer_info.hpp"
#include "network/types/status.hpp"
#include "outcome/outcome.hpp"

namespace kagome::network {

  using Stream = libp2p::connection::Stream;
  using Protocol = libp2p::peer::Protocol;
  using PeerId = libp2p::peer::PeerId;
  using PeerInfo = libp2p::peer::PeerInfo;

  class GossipProtocol final
      : public ProtocolBase,
        public std::enable_shared_from_this<GossipProtocol> {
   public:
    enum class Error { CAN_NOT_CREATE_STATUS = 1, GONE };

    GossipProtocol() = delete;
    GossipProtocol(GossipProtocol &&) noexcept = delete;
    GossipProtocol(const GossipProtocol &) = delete;
    virtual ~GossipProtocol() = default;
    GossipProtocol &operator=(GossipProtocol &&) noexcept = delete;
    GossipProtocol &operator=(GossipProtocol const &) = delete;

    GossipProtocol(
        libp2p::Host &host,
        std::shared_ptr<consensus::grandpa::GrandpaObserver> grandpa_observer,
        const OwnPeerInfo &own_info,
        std::shared_ptr<Gossiper> gossiper,
        std::shared_ptr<StreamEngine> stream_engine);

    const Protocol &protocol() const override {
      return protocol_;
    }

    bool start() override;
    bool stop() override;

    void onIncomingStream(std::shared_ptr<Stream> stream) override;
    void newOutgoingStream(
        const PeerInfo &peer_info,
        std::function<void(outcome::result<std::shared_ptr<Stream>>)> &&cb)
        override;

   private:
    enum class Direction { INCOMING, OUTGOING };

    void readGossipMessage(std::shared_ptr<Stream> stream);
    void writeGossipMessage(std::shared_ptr<Stream> stream,
                            const GossipMessage &gossip_message);

    libp2p::Host &host_;
    std::shared_ptr<consensus::grandpa::GrandpaObserver> grandpa_observer_;
    const OwnPeerInfo &own_info_;
    std::shared_ptr<StreamEngine> stream_engine_;
    const libp2p::peer::Protocol protocol_;
    log::Logger log_ = log::createLogger("GossipProtocol");
  };

}  // namespace kagome::network

OUTCOME_HPP_DECLARE_ERROR(kagome::network, GossipProtocol::Error);

#endif  // KAGOME_NETWORK_GOSSIPPROTOCOL

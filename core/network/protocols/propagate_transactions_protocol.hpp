/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_NETWORK_PROPAGATETRANSACTIONSPROTOCOL
#define KAGOME_NETWORK_PROPAGATETRANSACTIONSPROTOCOL

#include <memory>

#include <libp2p/connection/stream.hpp>
#include <libp2p/host/host.hpp>

#include "application/chain_spec.hpp"
#include "blockchain/block_storage.hpp"
#include "blockchain/block_tree.hpp"
#include "crypto/hasher.hpp"
#include "log/logger.hpp"
#include "network/babe_observer.hpp"
#include "network/extrinsic_observer.hpp"
#include "network/impl/stream_engine.hpp"
#include "network/peer_manager.hpp"
#include "network/protocol_base.hpp"
#include "network/types/block_announce.hpp"
#include "network/types/propagate_transactions.hpp"
#include "network/types/status.hpp"
#include "outcome/outcome.hpp"

namespace kagome::network {

  using Stream = libp2p::connection::Stream;
  using Protocol = libp2p::peer::Protocol;
  using PeerId = libp2p::peer::PeerId;
  using PeerInfo = libp2p::peer::PeerInfo;

  class PropagateTransactionsProtocol final
      : public ProtocolBase,
        public std::enable_shared_from_this<PropagateTransactionsProtocol> {
   public:
    enum class Error { CAN_NOT_CREATE_STATUS = 1, GONE };

    PropagateTransactionsProtocol() = delete;
    PropagateTransactionsProtocol(PropagateTransactionsProtocol &&) noexcept =
        delete;
    PropagateTransactionsProtocol(const PropagateTransactionsProtocol &) =
        delete;
    virtual ~PropagateTransactionsProtocol() = default;
    PropagateTransactionsProtocol &operator=(
        PropagateTransactionsProtocol &&) noexcept = delete;
    PropagateTransactionsProtocol &operator=(
        PropagateTransactionsProtocol const &) = delete;

    PropagateTransactionsProtocol(libp2p::Host &host,
                                  const application::ChainSpec &chain_spec,
                                  std::shared_ptr<ExtrinsicObserver> extrinsic_observer,
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
    void readPropagatedExtrinsics(std::shared_ptr<Stream> stream);

    void writePropagatedExtrinsics(
        std::shared_ptr<Stream> stream,
        const PropagatedExtrinsics &msg,
        std::function<void(outcome::result<std::shared_ptr<Stream>>)> &&cb);

    libp2p::Host &host_;
    std::shared_ptr<ExtrinsicObserver> extrinsic_observer_;
    std::shared_ptr<StreamEngine> stream_engine_;
    const libp2p::peer::Protocol protocol_;
    log::Logger log_ = log::createLogger("PropagateTransactionsProtocol");
  };

}  // namespace kagome::network

OUTCOME_HPP_DECLARE_ERROR(kagome::network,
                          PropagateTransactionsProtocol::Error);

#endif  // KAGOME_NETWORK_PROPAGATETRANSACTIONSPROTOCOL

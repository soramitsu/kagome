/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "network/impl/gossiper_broadcast.hpp"

#include <atomic>
#include <memory>

#include "application/chain_spec.hpp"

namespace kagome::network {
  KAGOME_DEFINE_CACHE(stream_engine);

  GossiperBroadcast::GossiperBroadcast(
      std::shared_ptr<application::AppStateManager> app_state_manager,
      StreamEngine::StreamEnginePtr stream_engine,
      std::shared_ptr<primitives::events::ExtrinsicSubscriptionEngine>
          extrinsic_events_engine,
      std::shared_ptr<subscription::ExtrinsicEventKeyRepository>
          ext_event_key_repo,
      std::shared_ptr<network::Router> router)
      : logger_{log::createLogger("GossiperBroadcast", "network")},
        stream_engine_{std::move(stream_engine)},
        extrinsic_events_engine_{std::move(extrinsic_events_engine)},
        ext_event_key_repo_{std::move(ext_event_key_repo)},
        router_{std::move(router)} {
    BOOST_ASSERT(stream_engine_ != nullptr);
    BOOST_ASSERT(extrinsic_events_engine_ != nullptr);
    BOOST_ASSERT(ext_event_key_repo_ != nullptr);
    BOOST_ASSERT(router_ != nullptr);

    BOOST_ASSERT(app_state_manager);
    app_state_manager->takeControl(*this);
  }

  bool GossiperBroadcast::prepare() {
    block_announce_protocol_ = router_->getBlockAnnounceProtocol();
    gossip_protocol_ = router_->getGossipProtocol();
    grandpa_protocol_ = router_->getGrandpaProtocol();
    propagate_transaction_protocol_ =
        router_->getPropagateTransactionsProtocol();
    return true;
  }

  bool GossiperBroadcast::start() {
    return true;
  }

  void GossiperBroadcast::stop() {
    block_announce_protocol_.reset();
    gossip_protocol_.reset();
    grandpa_protocol_.reset();
    propagate_transaction_protocol_.reset();
  }

  void GossiperBroadcast::propagateTransactions(
      gsl::span<const primitives::Transaction> txs) {
    SL_DEBUG(logger_, "Propagate transactions : {} extrinsics", txs.size());

    std::vector<libp2p::peer::PeerId> peers;
    stream_engine_->forEachPeer(
        [&peers](const libp2p::peer::PeerId &peer_id, const auto &peer_map) {
          peers.push_back(peer_id);
        });
    if (peers.size() > 1) {
      for (const auto &tx : txs) {
        if (auto key = ext_event_key_repo_->getEventKey(tx); key.has_value()) {
          extrinsic_events_engine_->notify(
              key.value(),
              primitives::events::ExtrinsicLifecycleEvent::Broadcast(
                  key.value(), std::move(peers)));
        }
      }
    }

    PropagatedExtrinsics exts;
    exts.extrinsics.resize(txs.size());
    std::transform(
        txs.begin(), txs.end(), exts.extrinsics.begin(), [](auto &tx) {
          return tx.ext;
        });
    broadcast(propagate_transaction_protocol_, exts);
  }

  void GossiperBroadcast::blockAnnounce(const BlockAnnounce &announce) {
    SL_DEBUG(
        logger_, "Block announce: block number {}", announce.header.number);
    broadcast(block_announce_protocol_, announce);
  }

  void GossiperBroadcast::vote(const network::GrandpaVote &vote_message) {
    SL_DEBUG(logger_,
             "Gossip vote message: grandpa round number {}",
             vote_message.round_number);
    broadcast(grandpa_protocol_, GrandpaMessage(vote_message));
  }

  void GossiperBroadcast::finalize(const FullCommitMessage &msg) {
    SL_DEBUG(logger_,
             "Gossip fin message: grandpa round number {}",
             msg.round);
    broadcast(grandpa_protocol_, GrandpaMessage(msg));
  }

  void GossiperBroadcast::catchUpRequest(
      const libp2p::peer::PeerId &peer_id,
      const CatchUpRequest &catch_up_request) {
    SL_DEBUG(logger_,
             "Gossip catch-up request: grandpa round number {}",
             catch_up_request.round_number);
    broadcast(grandpa_protocol_, GrandpaMessage(catch_up_request));
  }

  void GossiperBroadcast::catchUpResponse(
      const libp2p::peer::PeerId &peer_id,
      const CatchUpResponse &catch_up_response) {
    SL_DEBUG(logger_,
             "Gossip catch-up response: grandpa round number {}",
             catch_up_response.round_number);
    broadcast(grandpa_protocol_, GrandpaMessage(catch_up_response));
  }
}  // namespace kagome::network

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "network/impl/peer_manager_impl.hpp"

#include <libp2p/host/host.hpp>
#include <libp2p/multi/content_identifier_codec.hpp>
#include <libp2p/protocol/kademlia/kademlia.hpp>
#include <memory>

#include "network/common.hpp"
#include "outcome/outcome.hpp"

namespace kagome::network {
  PeerManagerImpl::PeerManagerImpl(
      std::shared_ptr<application::AppStateManager> app_state_manager,
      std::shared_ptr<libp2p::Host> host,
      std::shared_ptr<libp2p::protocol::kademlia::Kademlia> kademlia,
      std::shared_ptr<libp2p::protocol::Scheduler> scheduler,
      std::shared_ptr<StreamEngine> stream_engine,
      std::shared_ptr<application::ChainSpec> config,
      const BootstrapNodes &bootstrap_nodes,
      const OwnPeerInfo &own_peer_info)
      : app_state_manager_(std::move(app_state_manager)),
        host_(std::move(host)),
        kademlia_(std::move(kademlia)),
        scheduler_(std::move(scheduler)),
        stream_engine_(std::move(stream_engine)),
        config_(std::move(config)),
        bootstrap_nodes_(bootstrap_nodes),
        own_peer_info_(own_peer_info),
        log_(common::createLogger("PeerManager")) {
    BOOST_ASSERT(app_state_manager_ != nullptr);
    BOOST_ASSERT(host_ != nullptr);
    BOOST_ASSERT(kademlia_ != nullptr);
    BOOST_ASSERT(scheduler_ != nullptr);
    BOOST_ASSERT(stream_engine_ != nullptr);
    BOOST_ASSERT(config_ != nullptr);

    app_state_manager_->takeControl(*this);
  }

  bool PeerManagerImpl::prepare() {
    // Add themselves into peer routing
    kademlia_->addPeer(own_peer_info_, true);

    if (bootstrap_nodes_.empty()) {
      log_->critical("Have not any bootstrap nodes");
      return false;
    }

    // Add bootstrap nodes into peer routing
    for (const auto &bootstrap_node : bootstrap_nodes_) {
      kademlia_->addPeer(bootstrap_node, true);
    }

    content_id_ = libp2p::protocol::kademlia::ContentId(
        "kagome");  // TODO(xDimon): use value from config

    //    if (peer_info.id != own_peer_info_.id) {
    //      gossiper_->reserveStream(peer_info, transactions_protocol_, {});
    //      gossiper_->reserveStream(peer_info, block_announces_protocol_, {});
    //      gossiper_->reserveStream(peer_info, kGossipProtocol, {});
    //    } else {
    //      auto stream = std::make_shared<LoopbackStream>(own_peer_info);
    //      loopback_stream_ = stream;
    //      gossiper_->reserveStream(
    //          own_peer_info, kGossipProtocol, std::move(stream));
    //    }
    return true;
  }

  bool PeerManagerImpl::start() {
    // Infer peer_id from content_id
    auto cid =
        libp2p::multi::ContentIdentifierCodec::decode(content_id_.data).value();
    auto peer_id = PeerId::fromHash(cid.content_address).value();

    // Doing first peer finding with peer_id inferred from content_id
    auto res = kademlia_->findPeer(peer_id, [&](auto) {
      // Say to world about his providing
      announce();

      // Ask provider from world
      discovery();

      // Start Kademlia (processing incoming message and random walking)
      kademlia_->start();

      // Up/dowm current connection
      align();
    });
    if (not res.has_value()) {
      log_->error(
          "Can't start PeerManager: can't execute initial find peer: {}",
          res.error().message());
      return false;
    }

    // Watch new connection
    new_connection_handle_ = host_->setOnNewConnectionHandler(
        [wp = weak_from_this()](auto &&peer_info) {
          if (auto self = wp.lock()) {
            // Add to active peer list
            if (auto [ap_it, ok] = self->active_peers_.emplace(peer_info.id);
                ok) {
              auto &peer_id = *ap_it;

              // And remove from queue
              if (auto piq_it = self->peers_in_queue_.find(peer_id);
                  piq_it != self->peers_in_queue_.end()) {
                auto qtc_it = std::find_if(self->queue_to_connect_.cbegin(),
                                           self->queue_to_connect_.cend(),
                                           [&peer_id](const auto &item) {
                                             return peer_id == item.get();
                                           });
                self->queue_to_connect_.erase(qtc_it);
              }

              // Reserve streams
              self->stream_engine_->add(
                  peer_id,
                  fmt::format(kPropagateTransactionsProtocol.data(),
                              self->config_->protocolId()));
              self->stream_engine_->add(
                  peer_id,
                  fmt::format(kBlockAnnouncesProtocol.data(),
                              self->config_->protocolId()));

              self->stream_engine_->add(peer_id, kGossipProtocol);
            }
          }
        });

    return true;
  }

  void PeerManagerImpl::stop() {
    new_connection_handle_.unsubscribe();
    announce_timer_.cancel();
    discovery_timer_.cancel();
  }

  void PeerManagerImpl::connectToPeer(PeerManager::PeerInfo peer_info) {
    auto res =
        host_->getPeerRepository().getAddressRepository().upsertAddresses(
            peer_info.id, peer_info.addresses, libp2p::peer::ttl::kDay);
    if (res) {
      connectToPeer(peer_info.id);
    }
  }

  void PeerManagerImpl::forEachPeer(std::function<void(PeerId)> func) {
    std::for_each(active_peers_.begin(), active_peers_.end(), func);
  }

  void PeerManagerImpl::forOnePeer(PeerManager::PeerId peer_id,
                                   std::function<void()> func) {
    auto i = active_peers_.find(peer_id);
    if (i != active_peers_.end()) {
      func();
    }
  }

  void PeerManagerImpl::announce() {
    bool passive_mode = false;  // TODO(xDimon): use value from config

    announce_timer_.cancel();

    auto res = kademlia_->provide(content_id_, not passive_mode);
    if (not res) {
      log_->error("Can't announce himself: {}", res.error().message());
    }

    announce_timer_ =
        scheduler_->schedule(libp2p::protocol::scheduler::toTicks(
                                 std::chrono::seconds(res ? 300 : 30)),
                             [wp = weak_from_this()] {
                               if (auto self = wp.lock()) {
                                 self->announce();
                               }
                             });
  }

  void PeerManagerImpl::discovery() {
    discovery_timer_.cancel();

    auto res = kademlia_->findProviders(
        content_id_,
        0,
        [wp = weak_from_this()](outcome::result<std::vector<PeerInfo>> res) {
          auto self = wp.lock();
          if (not self) {
            return;
          }
          self->discovery_timer_.cancel();
          self->discovery_timer_ = self->scheduler_->schedule(
              libp2p::protocol::scheduler::toTicks(std::chrono::seconds(300)),
              [wp] {
                if (auto self = wp.lock()) {
                  self->discovery();
                }
              });

          if (not res) {
            return;
          }

          auto &providers = res.value();
          for (auto &provider : providers) {
            if (auto [it, ok] =
                    self->peers_in_queue_.emplace(std::move(provider.id));
                ok) {
              self->queue_to_connect_.emplace_back(*it);
            }
          }
        });

    if (not res) {
      log_->error("Can't discovery new peers: {}", res.error().message());
    }
  }

  void PeerManagerImpl::align() {
    size_t target_count = 4;  // TODO(xDimon): use value from config
    size_t soft_limit = 8;    // TODO(xDimon): use value from config
    size_t hard_limit = 8;    // TODO(xDimon): use value from config

    align_timer_.cancel();

    size_t cur_active_peer = active_peers_.size();

    // Not enough active peers
    if (cur_active_peer < target_count and not queue_to_connect_.empty()) {
      auto peer_id =
          std::move(peers_in_queue_.extract(queue_to_connect_.front()).value());
      queue_to_connect_.pop_front();

      connectToPeer(peer_id);
    }

    // Soft limit is exceeded
    if (cur_active_peer > soft_limit) {
      // Get oldest peer
      PeerId peer_id = *active_peers_.begin();

      // Hard limit is exceeded OR peer is inactive long time
      if (cur_active_peer > hard_limit or false) {
        // Disconnect from peer
        disconnectFromPeer(peer_id);
      }
    }

    align_timer_ = scheduler_->schedule(
        libp2p::protocol::scheduler::toTicks(std::chrono::seconds(5)),
        [wp = weak_from_this()] {
          if (auto self = wp.lock()) {
            self->align();
          }
        });
  }

  void PeerManagerImpl::connectToPeer(PeerId peer_id) {
    auto peer_info = host_->getPeerRepository().getPeerInfo(peer_id);

    if (peer_info.addresses.empty()) {
      return;
    }

    auto connectedness =
        host_->getNetwork().getConnectionManager().connectedness(peer_info);
    if (connectedness
        == libp2p::network::ConnectionManager::Connectedness::CAN_NOT_CONNECT) {
      return;
    }

    host_->connect(peer_info);
  }

  void PeerManagerImpl::disconnectFromPeer(PeerId peer_id) {
    // TODO(xDimon): Find connection and close him
  }

}  // namespace kagome::network

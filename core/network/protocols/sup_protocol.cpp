/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "network/protocols/sup_protocol.hpp"

#include "network/common.hpp"
#include "network/protocols/protocol_error.hpp"

namespace kagome::network {

  SupProtocol::SupProtocol(libp2p::Host &host,
                           const application::AppConfiguration &app_config,
                           std::shared_ptr<StreamEngine> stream_engine,
                           std::shared_ptr<blockchain::BlockTree> block_tree,
                           std::shared_ptr<blockchain::BlockStorage> storage,
                           std::shared_ptr<PeerManager> peer_manager)
      : host_(host),
        app_config_(app_config),
        stream_engine_(std::move(stream_engine)),
        block_tree_(std::move(block_tree)),
        storage_(std::move(storage)),
        peer_manager_(std::move(peer_manager)) {
    BOOST_ASSERT(stream_engine_ != nullptr);
    BOOST_ASSERT(block_tree_ != nullptr);
    BOOST_ASSERT(storage_ != nullptr);
    BOOST_ASSERT(peer_manager_ != nullptr);
    const_cast<Protocol &>(protocol_) = kSupProtocol;
  }

  bool SupProtocol::start() {
    host_.setProtocolHandler(protocol_, [wp = weak_from_this()](auto &&stream) {
      if (auto self = wp.lock()) {
        if (auto peer_id = stream->remotePeerId()) {
          SL_TRACE(self->log_,
                   "Handled {} protocol stream from: {}",
                   self->protocol_,
                   peer_id.value().toBase58());
          self->onIncomingStream(std::forward<decltype(stream)>(stream));
          return;
        }
        self->log_->warn("Handled {} protocol stream from unknown peer",
                         self->protocol_);
        stream->reset();
      }
    });
    return true;
  }

  bool SupProtocol::stop() {
    return true;
  }

  outcome::result<Status> SupProtocol::createStatus() const {
    /// Roles
    Roles roles = app_config_.roles();

    /// Best block info
    BlockInfo best_block;
    const auto &last_finalized = block_tree_->getLastFinalized().hash;
    if (auto best_res =
            block_tree_->getBestContaining(last_finalized, boost::none);
        best_res.has_value()) {
      best_block = best_res.value();
    } else {
      log_->error("Could not get best block info: {}",
                  best_res.error().message());
      return ProtocolError::CAN_NOT_CREATE_STATUS;
    }

    /// Genesis hash
    BlockHash genesis_hash;
    if (auto genesis_res = storage_->getGenesisBlockHash();
        genesis_res.has_value()) {
      genesis_hash = std::move(genesis_res.value());
    } else {
      log_->error("Could not get genesis block hash: {}",
                  genesis_res.error().message());
      return ProtocolError::CAN_NOT_CREATE_STATUS;
    }

    return Status{
        .roles = roles, .best_block = best_block, .genesis_hash = genesis_hash};
  }

  void SupProtocol::onIncomingStream(std::shared_ptr<Stream> stream) {
    BOOST_ASSERT(stream->remotePeerId().has_value());

    readHandshake(
        stream,
        Direction::INCOMING,
        [wp = weak_from_this(), stream](outcome::result<void> res) {
          auto self = wp.lock();
          if (not self) {
            stream->reset();
            return;
          }

          auto peer_id = stream->remotePeerId().value();

          if (not res.has_value()) {
            SL_VERBOSE(self->log_,
                       "Handshake failed on incoming {} stream with {}: {}",
                       self->protocol_,
                       peer_id.toBase58(),
                       res.error().message());
            stream->reset();
            return;
          }

          res = self->stream_engine_->addIncoming(stream, self);
          if (not res.has_value()) {
            SL_VERBOSE(self->log_,
                       "Can't register incoming {} stream with {}: {}",
                       self->protocol_,
                       peer_id.toBase58(),
                       res.error().message());
            stream->reset();
            return;
          }

          SL_VERBOSE(self->log_,
                     "Fully established incoming {} stream with {}",
                     self->protocol_,
                     peer_id.toBase58());
        });
  }

  void SupProtocol::newOutgoingStream(
      const PeerInfo &peer_info,
      std::function<void(outcome::result<std::shared_ptr<Stream>>)> &&cb) {
    host_.newStream(
        peer_info.id,
        protocol_,
        [wp = weak_from_this(), peer_id = peer_info.id, cb = std::move(cb)](
            auto &&stream_res) mutable {
          auto self = wp.lock();
          if (not self) {
            cb(ProtocolError::GONE);
            return;
          }

          if (not stream_res.has_value()) {
            SL_VERBOSE(self->log_,
                       "Can't create outgoing {} stream with {}: {}",
                       self->protocol_,
                       peer_id.toBase58(),
                       stream_res.error().message());
            cb(stream_res.as_failure());
            return;
          }
          auto &stream = stream_res.value();

          auto cb2 = [wp, stream, cb = std::move(cb)](
                         outcome::result<void> res) {
            auto self = wp.lock();
            if (not self) {
              cb(ProtocolError::GONE);
              return;
            }

            if (not res.has_value()) {
              SL_VERBOSE(self->log_,
                         "Handshake failed on outgoing {} stream with {}: {}",
                         self->protocol_,
                         stream->remotePeerId().value().toBase58(),
                         res.error().message());
              stream->reset();
              cb(res.as_failure());
              return;
            }

            res = self->stream_engine_->addOutgoing(stream, self);
            if (not res.has_value()) {
              SL_VERBOSE(self->log_,
                         "Can't register outgoing {} stream with {}: {}",
                         self->protocol_,
                         stream->remotePeerId().value().toBase58(),
                         res.error().message());
              stream->reset();
              cb(res.as_failure());
              return;
            }

            SL_VERBOSE(self->log_,
                       "Fully established outgoing {} stream with {}",
                       self->protocol_,
                       stream->remotePeerId().value().toBase58());
            cb(std::move(stream));
          };

          self->writeHandshake(
              std::move(stream), Direction::OUTGOING, std::move(cb2));
        });
  }

  void SupProtocol::readHandshake(
      std::shared_ptr<Stream> stream,
      Direction direction,
      std::function<void(outcome::result<void>)> &&cb) {
    auto read_writer = std::make_shared<ScaleMessageReadWriter>(stream);

    read_writer->read<Status>(
        [stream, direction, wp = weak_from_this(), cb = std::move(cb)](
            auto &&remote_status_res) mutable {
          auto self = wp.lock();
          if (not self) {
            stream->reset();
            cb(ProtocolError::GONE);
            return;
          }

          if (not remote_status_res.has_value()) {
            SL_VERBOSE(self->log_,
                       "Can't read handshake from {}: {}",
                       stream->remotePeerId().value().toBase58(),
                       remote_status_res.error().message());
            stream->reset();
            cb(remote_status_res.as_failure());
            return;
          }
          auto &remote_status = remote_status_res.value();

          SL_TRACE(self->log_,
                   "Handshake has received from {}",
                   stream->remotePeerId().value().toBase58());

          if (auto genesis_res = self->storage_->getGenesisBlockHash();
              genesis_res.has_value()) {
            if (remote_status.genesis_hash != genesis_res.value()) {
              SL_VERBOSE(self->log_,
                         "Error while processing status: {}",
                         genesis_res.error().message());
              stream->reset();
              cb(ProtocolError::GENESIS_NO_MATCH);
              return;
            }
          } else {
            SL_VERBOSE(self->log_,
                       "Error while processing status: {}",
                       genesis_res.error().message());
            stream->reset();
            cb(ProtocolError::GENESIS_NO_MATCH);
            return;
          }

          auto peer_id = stream->remotePeerId().value();
          SL_TRACE(self->log_,
                   "Received status from peer_id={} (best block {})",
                   peer_id.toBase58(),
                   remote_status.best_block.number);
          self->peer_manager_->updatePeerStatus(peer_id, remote_status);

          switch (direction) {
            case Direction::OUTGOING:
              cb(outcome::success());
              break;
            case Direction::INCOMING:
              self->writeHandshake(
                  std::move(stream), Direction::INCOMING, std::move(cb));
              break;
          }
        });
  }

  void SupProtocol::writeHandshake(
      std::shared_ptr<Stream> stream,
      Direction direction,
      std::function<void(outcome::result<void>)> &&cb) {
    auto read_writer = std::make_shared<ScaleMessageReadWriter>(stream);

    auto status_res = createStatus();
    if (not status_res.has_value()) {
      stream->reset();
      cb(ProtocolError::CAN_NOT_CREATE_STATUS);
      return;
    }

    read_writer->write(
        status_res.value(),
        [stream = std::move(stream),
         direction,
         wp = weak_from_this(),
         cb = std::move(cb)](auto &&write_res) mutable {
          auto self = wp.lock();
          if (not self) {
            stream->reset();
            cb(ProtocolError::GONE);
            return;
          }

          if (not write_res.has_value()) {
            SL_VERBOSE(self->log_,"Can't send handshake to {}: {}",
                              stream->remotePeerId().value().toBase58(),
                              write_res.error().message());
            stream->reset();
            cb(write_res.as_failure());
            return;
          }

          SL_TRACE(self->log_,
                   "Handshake has sent to {}",
                   stream->remotePeerId().value().toBase58());

          switch (direction) {
            case Direction::OUTGOING:
              self->readHandshake(
                  std::move(stream), Direction::OUTGOING, std::move(cb));
              break;
            case Direction::INCOMING:
              cb(outcome::success());
              self->readStatus(std::move(stream));
              break;
          }
        });
  }

  void SupProtocol::readStatus(std::shared_ptr<Stream> stream) {
    auto read_writer = std::make_shared<ScaleMessageReadWriter>(stream);

    read_writer->read<Status>(
        [stream = std::move(stream),
         wp = weak_from_this()](auto &&remote_status_res) mutable {
          auto self = wp.lock();
          if (not self) {
            stream->reset();
            return;
          }

          if (not remote_status_res.has_value()) {
            SL_VERBOSE(self->log_,"Can't read status message from {}: {}",
                              stream->remotePeerId().value().toBase58(),
                              remote_status_res.error().message());
            stream->reset();
            return;
          }

          auto peer_id = stream->remotePeerId().value();
          auto &remote_status = remote_status_res.value();

          SL_VERBOSE(self->log_, "Received status from {}", peer_id.toBase58());

          self->peer_manager_->updatePeerStatus(peer_id, remote_status);

          self->readStatus(std::move(stream));
        });
  }

  //  void SupProtocol::writeStatus(
  //      std::shared_ptr<Stream> stream,
  //      const Status &status,
  //      std::function<void(outcome::result<std::shared_ptr<Stream>>)> &&cb) {
  //    auto read_writer = std::make_shared<ScaleMessageReadWriter>(stream);
  //
  //    read_writer->write(status,
  //                       [stream = std::move(stream),
  //                        wp = weak_from_this(),
  //                        cb = std::move(cb)](auto &&write_res) mutable {
  //                         auto self = wp.lock();
  //                         if (not self) {
  //                           stream->reset();
  //                           if (cb) cb(ProtocolError::GONE);
  //                           return;
  //                         }
  //
  //                         if (not write_res.has_value()) {
  //                           SL_VERBOSE(self->log_,
  //                                      "Error while writing own status: {}",
  //                                      write_res.error().message());
  //                           stream->reset();
  //                           if (cb) cb(write_res.as_failure());
  //                           return;
  //                         }
  //
  //                         if (cb) cb(std::move(stream));
  //                       });
  //  }

}  // namespace kagome::network

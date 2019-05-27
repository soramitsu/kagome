/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "libp2p/protocol_muxer/multiselect/message_writer.hpp"

#include "libp2p/protocol_muxer/multiselect/message_manager.hpp"
#include "libp2p/protocol_muxer/multiselect/multiselect.hpp"

namespace libp2p::protocol_muxer {
  using peer::Protocol;

  void MessageWriter::writeMessage(
      std::shared_ptr<ConnectionState> connection_state,
      ConnectionState::NegotiationStatus success_status,
      const std::function<std::string(const std::error_code &ec)> &error) {
    auto multiselect = connection_state->multiselect_;

    auto write_res = connection_state->write();
    if (!write_res) {
      const auto &err = write_res.error();
      multiselect->onError(std::move(connection_state), error(err), err);
      return;
    }

    connection_state->status_ = success_status;
    multiselect->onWriteCompleted(std::move(connection_state));
  }

  void MessageWriter::sendOpeningMsg(
      std::shared_ptr<ConnectionState> connection_state) {
    *connection_state->write_buffer_ = MessageManager::openingMsg();
    writeMessage(std::move(connection_state),
                 ConnectionState::NegotiationStatus::OPENING_SENT,
                 [](const std::error_code &ec) {
                   return "cannot send an opening message: " + ec.message();
                 });
  }

  void MessageWriter::sendProtocolMsg(
      const Protocol &protocol,
      std::shared_ptr<ConnectionState> connection_state) {
    *connection_state->write_buffer_ = MessageManager::protocolMsg(protocol);
    writeMessage(std::move(connection_state),
                 ConnectionState::NegotiationStatus::PROTOCOL_SENT,
                 [](const std::error_code &ec) {
                   return "cannot send a protocol message: " + ec.message();
                 });
  }

  void MessageWriter::sendProtocolsMsg(
      gsl::span<const Protocol> protocols,
      std::shared_ptr<ConnectionState> connection_state) {
    *connection_state->write_buffer_ = MessageManager::protocolsMsg(protocols);

    writeMessage(std::move(connection_state),
                 ConnectionState::NegotiationStatus::PROTOCOLS_SENT,
                 [](const std::error_code &ec) {
                   return "cannot send protocols message: " + ec.message();
                 });
  }

  void MessageWriter::sendLsMsg(
      std::shared_ptr<ConnectionState> connection_state) {
    *connection_state->write_buffer_ = MessageManager::lsMsg();
    writeMessage(std::move(connection_state),
                 ConnectionState::NegotiationStatus::LS_SENT,
                 [](const std::error_code &ec) {
                   return "cannot send an ls message: " + ec.message();
                 });
  }

  void MessageWriter::sendNaMsg(
      std::shared_ptr<ConnectionState> connection_state) {
    *connection_state->write_buffer_ = MessageManager::naMsg();
    writeMessage(std::move(connection_state),
                 ConnectionState::NegotiationStatus::NA_SENT,
                 [](const std::error_code &ec) {
                   return "cannot send an na message: " + ec.message();
                 });
  }

  void MessageWriter::sendProtocolAck(
      std::shared_ptr<ConnectionState> connection_state,
      const peer::Protocol &protocol) {
    auto multiselect = connection_state->multiselect_;

    *connection_state->write_buffer_ = MessageManager::naMsg();
    auto write_res = connection_state->write();
    if (!write_res) {
      const auto &err = write_res.error();
      multiselect->onError(std::move(connection_state),
                           "cannot write ack message: " + err.message(), err);
      return;
    }

    connection_state->status_ =
        ConnectionState::NegotiationStatus::PROTOCOL_SENT;
    multiselect->onWriteAckCompleted(std::move(connection_state), protocol);
  }

}  // namespace libp2p::protocol_muxer

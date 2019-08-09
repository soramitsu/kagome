/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_MESSAGE_MANAGER_HPP
#define KAGOME_MESSAGE_MANAGER_HPP

#include <memory>
#include <optional>
#include <vector>

#include <gsl/span>
#include <outcome/outcome.hpp>
#include "common/buffer.hpp"
#include "libp2p/multi/uvarint.hpp"
#include "libp2p/protocol_muxer/protocol_muxer.hpp"

namespace libp2p::protocol_muxer {
  /**
   * Creates and parses Multiselect messages to be sent over the network
   */
  class MessageManager {
   public:
    struct MultiselectMessage {
      enum class MessageType { OPENING, PROTOCOL, PROTOCOLS, LS, NA };

      /// type of the message
      MessageType type;
      /// zero or more protocols in that message
      std::vector<std::string> protocols{};
    };

    struct ProtocolsMessageHeader {
      uint64_t size_of_protocols;
      uint64_t number_of_protocols;
    };

    enum class ParseError {
      VARINT_IS_EXPECTED = 1,
      MSG_LENGTH_IS_INCORRECT,
      MSG_IS_ILL_FORMED
    };

    static outcome::result<MultiselectMessage> parseConstantMsg(
        gsl::span<const uint8_t> bytes);

    static outcome::result<ProtocolsMessageHeader> parseProtocolsHeader(
        gsl::span<const uint8_t> bytes);

    static outcome::result<MultiselectMessage> parseProtocols(
        gsl::span<const uint8_t> bytes, uint64_t expected_protocols_number);

    static outcome::result<MultiselectMessage> parseProtocol(
        gsl::span<const uint8_t> bytes);

    /**
     * Create an opening message
     * @return created message
     */
    static kagome::common::Buffer openingMsg();

    /**
     * Create a message with an ls command
     * @return created message
     */
    static kagome::common::Buffer lsMsg();

    /**
     * Create a message telling the protocol is not supported
     * @return created message
     */
    static kagome::common::Buffer naMsg();

    /**
     * Create a response message with a single protocol
     * @param protocol to be sent
     * @return created message
     */
    static kagome::common::Buffer protocolMsg(const peer::Protocol &protocol);

    /**
     * Create a response message with a list of protocols
     * @param protocols to be sent
     * @return created message
     */
    static kagome::common::Buffer protocolsMsg(
        gsl::span<const peer::Protocol> protocols);
  };
}  // namespace libp2p::protocol_muxer

OUTCOME_HPP_DECLARE_ERROR_2(libp2p::protocol_muxer, MessageManager::ParseError)

#endif  // KAGOME_MESSAGE_MANAGER_HPP

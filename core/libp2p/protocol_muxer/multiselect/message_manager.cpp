/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "libp2p/protocol_muxer/multiselect/message_manager.hpp"

#include <string_view>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include "common/hexutil.hpp"

OUTCOME_CPP_DEFINE_CATEGORY(libp2p::protocol_muxer, MessageManager::ParseError,
                            e) {
  using Error = libp2p::protocol_muxer::MessageManager::ParseError;
  switch (e) {
    case Error::MSG_IS_TOO_SHORT:
      return "message size is less than a minimum one";
    case Error::VARINT_IS_EXPECTED:
      return "expected varint, but not found";
    case Error::MSG_LENGTH_IS_INCORRECT:
      return "incorrect message length";
    case Error::MSG_IS_ILL_FORMED:
      return "format of the message does not meet the protocol spec";
  }
  return "unknown error";
}

namespace {
  using kagome::common::Buffer;
  using libp2p::multi::UVarint;
  using MultiselectMessage =
      libp2p::protocol_muxer::MessageManager::MultiselectMessage;

  /// header of Multiselect protocol
  constexpr std::string_view kMultiselectHeaderString =
      "/multistream-select/1.0.0\n";

  /// string of ls message
  constexpr std::string_view kLsString = "ls\n";

  /// string of na message
  constexpr std::string_view kNaString = "na\n";

  /// ls message, ready to be sent
  const kagome::common::Buffer kLsMsg =
      kagome::common::Buffer{}
          .put(UVarint{kLsString.size()}.toBytes())
          .put(kLsString);

  /// na message, ready to be sent
  const kagome::common::Buffer kNaMsg =
      kagome::common::Buffer{}
          .put(UVarint{kNaString.size()}.toBytes())
          .put(kNaString);

  /**
   * Retrieve a varint from a bytes buffer
   * @param buffer to be seeked
   * @param pos - position, from which the retrieval should start; after the
   * function execution it is set to the position AFTER the found varint
   * @return varint, if it was retrieved; error otherwise
   */
  outcome::result<UVarint> getVarint(gsl::span<const uint8_t> buffer,
                                     size_t &pos) {
    using ParseError = libp2p::protocol_muxer::MessageManager::ParseError;

    if (buffer.empty()) {
      return ParseError::VARINT_IS_EXPECTED;
    }
    auto varint_opt = UVarint::create(buffer.subspan(pos));
    if (!varint_opt) {
      return ParseError::VARINT_IS_EXPECTED;
    }
    pos += varint_opt->size();
    return *varint_opt;
  }

  /**
   * Retrieve a line from a buffer, starting from the specified position
   * @param buffer, from which the line is to be retrieved
   * @param current_position, from which to get the line; after the execution is
   * set to a position after the line
   * @return line in case of success, error otherwise
   */
  outcome::result<std::string> lineToString(gsl::span<const uint8_t> buffer,
                                            size_t &current_position) {
    using ParseError = libp2p::protocol_muxer::MessageManager::ParseError;

    // firstly, a varint, showing length of this line (and thus a whole message)
    // without itself
    OUTCOME_TRY(msg_length, getVarint(buffer, current_position));
    auto prev_position = current_position;
    current_position += msg_length.toUInt64();

    if (current_position > static_cast<size_t>(buffer.size())) {
      return ParseError::MSG_LENGTH_IS_INCORRECT;
    }

    assert(msg_length.size() < current_position);          // NOLINT
    return std::string{buffer.data() + prev_position,      // NOLINT
                       buffer.data() + current_position};  // NOLINT
  }

  /**
   * Get a protocol from a string and check it meets specification requirements
   * @param msg, which must contain a protocol, ending with \n
   * @return pure protocol string with cutted \n or error
   */
  outcome::result<std::string> parseProtocolLine(std::string_view msg) {
    using ParseError = libp2p::protocol_muxer::MessageManager::ParseError;

    auto new_line_byte = msg.find('\n');
    if (new_line_byte != msg.size() - 1) {
      return ParseError::MSG_IS_ILL_FORMED;
    }

    return std::string{msg.substr(0, new_line_byte)};
  }
}  // namespace

namespace libp2p::protocol_muxer {
  using kagome::common::Buffer;
  using MultiselectMessage = MessageManager::MultiselectMessage;

  outcome::result<MultiselectMessage> MessageManager::parseConstantMsg(
      gsl::span<const uint8_t> bytes) {
    static const std::string kLsMsgHex{"036C730A"};  // '3ls\n'
    static const std::string kNaMsgHex{"036E610A"};  // '3na\n'

    static constexpr int64_t kShortestMessageLength{3};
    static constexpr int64_t kConstMsgsLength{4};

    auto buffer_size = bytes.size();
    // shortest messages are LS, NA and (sometimes) a header for LS response
    if (buffer_size < kShortestMessageLength) {
      return ParseError::MSG_IS_TOO_SHORT;
    }

    // check the message against the constant ones
    if (buffer_size == kConstMsgsLength) {
      auto msg_hex = kagome::common::hex_upper(bytes);
      if (msg_hex == kLsMsgHex) {
        return MultiselectMessage{MultiselectMessage::MessageType::LS};
      }
      if (msg_hex == kNaMsgHex) {
        return MultiselectMessage{MultiselectMessage::MessageType::NA};
      }
    }

    return ParseError::MSG_IS_ILL_FORMED;
  }

  outcome::result<MessageManager::ProtocolsMessageHeader>
  MessageManager::parseProtocolsHeader(gsl::span<const uint8_t> bytes) {
    // this header consists of three varints, one of which is assumed to be
    // already parsed; try to parse the other two

    size_t current_position = 0;
    // next varint shows, how much bytes list of protocols take
    OUTCOME_TRY(protocols_bytes_size, getVarint(bytes, current_position));

    // next varint shows, how much protocols are expected in the message
    OUTCOME_TRY(protocols_number, getVarint(bytes, current_position));

    return ProtocolsMessageHeader{protocols_bytes_size.toUInt64(),
                                  protocols_number.toUInt64()};
  }

  outcome::result<MultiselectMessage> MessageManager::parseProtocols(
      gsl::span<const uint8_t> bytes, uint64_t expected_protocols_number) {
    // parse protocols, which are after the header
    size_t current_position = 0;
    MultiselectMessage parsed_msg{MultiselectMessage::MessageType::PROTOCOLS};
    for (uint64_t i = 0; i < expected_protocols_number; ++i) {
      OUTCOME_TRY(current_line, lineToString(bytes, current_position));
      OUTCOME_TRY(protocol, parseProtocolLine(current_line));
      parsed_msg.protocols_.push_back(std::move(protocol));
    }
    return parsed_msg;
  }

  outcome::result<MultiselectMessage> MessageManager::parseProtocol(
      gsl::span<const uint8_t> bytes) {
    if (bytes.empty()) {
      return ParseError::MSG_LENGTH_IS_INCORRECT;
    }

    auto current_line =
        std::string{bytes.data(), bytes.data() + bytes.size()};  // NOLINT
    if (current_line == kMultiselectHeaderString) {
      return MultiselectMessage{MultiselectMessage::MessageType::OPENING};
    }
    OUTCOME_TRY(protocol, parseProtocolLine(current_line));

    MultiselectMessage parsed_msg{MultiselectMessage::MessageType::PROTOCOL};
    parsed_msg.protocols_.push_back(std::move(protocol));
    return parsed_msg;
  }

  Buffer MessageManager::openingMsg() {
    return kagome::common::Buffer{}
        .put(multi::UVarint{kMultiselectHeaderString.size()}.toBytes())
        .put(kMultiselectHeaderString);
  }

  Buffer MessageManager::lsMsg() {
    return kLsMsg;
  }

  Buffer MessageManager::naMsg() {
    return kNaMsg;
  }

  Buffer MessageManager::protocolMsg(const peer::Protocol &protocol) {
    return kagome::common::Buffer{}
        .put(multi::UVarint{protocol.size() + 1}.toBytes())
        .put(protocol)
        .put("\n");
  }

  Buffer MessageManager::protocolsMsg(
      gsl::span<const peer::Protocol> protocols) {
    // FIXME: naive approach, involving a tremendous amount of copies

    Buffer protocols_buffer{};
    for (const auto &protocol : protocols) {
      protocols_buffer.put(multi::UVarint{protocol.size() + 1}.toBytes())
          .put(protocol)
          .put("\n");
    }

    auto header_buffer =
        Buffer{}
            .put(multi::UVarint{protocols_buffer.size()}.toBytes())
            .put(multi::UVarint{static_cast<uint64_t>(protocols.size())}
                     .toBytes())
            .put("\n");

    return Buffer{}
        .put(multi::UVarint{header_buffer.size()}.toBytes())
        .putBuffer(header_buffer)
        .putBuffer(protocols_buffer);
  }
}  // namespace libp2p::protocol_muxer

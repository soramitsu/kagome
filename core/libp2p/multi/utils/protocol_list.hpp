/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_PROTOCOLLIST_HPP
#define KAGOME_PROTOCOLLIST_HPP

#include <map>
#include <functional>

namespace libp2p::multi {

  struct Protocol {
  public:

    static const int kVarLen = -1;

    enum class Code: std::size_t {
      ip4 = 4,
      tcp = 6,
      udp = 273,
      dccp = 33,
      ip6 = 41,
      ip6zone = 42,
      dns = 53,
      dns4 = 54,
      dns6 = 55,
      dnsaddr = 56,
      sctp = 132,
      udt = 301,
      utp = 302,
      unix = 400,
      p2p = 421,
      ipfs = 421,
      onion = 444,
      onion3 = 445,
      garlic64 = 446,
      quic = 460,
      http = 480,
      https = 443,
      ws = 477,
      wss = 478,
      p2p_websocket_star = 479,
      p2p_stardust = 277,
      p2p_webrtc_star = 275,
      p2p_webrtc_direct = 276,
      p2p_circuit = 290,
    };

    const Code dec_code;
    const ssize_t size;
    const std::string_view name;
  };

  class ProtocolList {
  public:

    static const std::size_t kProtocolsNum = 29;

    static constexpr auto get(std::string_view name)
        -> Protocol const* {
      for(auto& protocol: protocols_) {
        if (protocol.name == name) {
          return &protocol;
        }
      }
      return nullptr;
    }

    static constexpr auto get(Protocol::Code code)
        -> Protocol const* {
      for(auto& protocol: protocols_) {
        if (protocol.dec_code == code) {
          return &protocol;
        }
      }
      return nullptr;
    }


  private:
    static constexpr const std::array<Protocol, kProtocolsNum> protocols_= {
        Protocol {Protocol::Code::ip4, 32, "ip4"},
        Protocol {Protocol::Code::tcp, 16, "tcp"},
        Protocol {Protocol::Code::udp, 16, "udp"},
        {Protocol::Code::dccp, 16, "dccp"},
        {Protocol::Code::ip6, 128, "ip6"},
        {Protocol::Code::ip6zone, Protocol::kVarLen, "ip6zone"},
        {Protocol::Code::dns, Protocol::kVarLen, "dns"},
        {Protocol::Code::dns4, Protocol::kVarLen, "dns64"},
        {Protocol::Code::dns6, Protocol::kVarLen, "dns6"},
        {Protocol::Code::dnsaddr, Protocol::kVarLen, "dnsaddr"},
        {Protocol::Code::sctp, 16, "sctp"},
        {Protocol::Code::udt, 0, "udt"},
        {Protocol::Code::utp, 0, "utp"},
        {Protocol::Code::unix, Protocol::kVarLen, "unix"},
        {Protocol::Code::p2p, Protocol::kVarLen, "p2p"},
        {Protocol::Code::ipfs, Protocol::kVarLen, "ipfs"},
        {Protocol::Code::onion, 96, "onion"},
        {Protocol::Code::onion3, 296, "onion3"},
        {Protocol::Code::garlic64, Protocol::kVarLen, "garlic64"},
        {Protocol::Code::quic, 0, "quic"},
        {Protocol::Code::http, 0, "http"},
        {Protocol::Code::https, 0, "https"},
        {Protocol::Code::ws, 0, "ws"},
        {Protocol::Code::wss, 0, "wss"},
        {Protocol::Code::p2p_websocket_star, 0, "p2p-websocket-star"},
        {Protocol::Code::p2p_stardust, 0, "p2p-stardust"},
        {Protocol::Code::p2p_webrtc_star, 0, "p2p-webrtc-star"},
        {Protocol::Code::p2p_webrtc_direct, 0, "p2p-webrtc-direct"},
        {Protocol::Code::p2p_circuit, 0, "p2p-circuit"},
    };

  };

}
#endif //KAGOME_PROTOCOLLIST_HPP

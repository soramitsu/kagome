/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "scale/libp2p_types.hpp"

#include <algorithm>
#include <exception>

namespace kagome::scale {

  PeerInfoSerializable::PeerInfoSerializable() : PeerInfo{dummyPeerId(), {}} {}

  libp2p::peer::PeerId PeerInfoSerializable::dummyPeerId() {
    // some valid dummy peer id
    auto res = libp2p::peer::PeerId::fromBase58(
        "12D3KooWFN2mhgpkJsDBuNuE5427AcDrsib8EoqGMZmkxWwx3Md4");
    BOOST_ASSERT(res);
    return res.value();
  }

  ScaleEncoderStream &operator<<(ScaleEncoderStream &s,
                                 const libp2p::peer::PeerInfo &peer_info) {
    std::vector<std::string> addresses;
    for (const auto &addr : peer_info.addresses) {
      addresses.emplace_back(addr.getStringAddress());
    }
    return s << peer_info.id.toBase58() << addresses;
  }

  ScaleDecoderStream &operator>>(ScaleDecoderStream &s,
                                 libp2p::peer::PeerInfo &peer_info) {
    std::string peer_id_base58;
    std::vector<std::string> addresses;
    s >> peer_id_base58 >> addresses;
    auto peer_id_res = libp2p::peer::PeerId::fromBase58(peer_id_base58);
    if (not peer_id_res) {
      throw std::runtime_error(peer_id_res.error().message());
    }
    peer_info.id = std::move(peer_id_res.value());
    std::vector<libp2p::multi::Multiaddress> multi_addrs;
    std::transform(addresses.begin(),
                   addresses.end(),
                   std::back_inserter(multi_addrs),
                   [](const auto &addr) {
                     auto res = libp2p::multi::Multiaddress::create(addr);
                     if (not res) {
                       throw std::runtime_error(res.error().message());
                     }
                     return res.value();
                   });
    peer_info.addresses = std::move(multi_addrs);
    return s;
  }
}  // namespace kagome::scale

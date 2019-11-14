/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_IDENTITY_MANAGER_HPP
#define KAGOME_IDENTITY_MANAGER_HPP

#include "libp2p/crypto/key.hpp"
#include "libp2p/event/bus.hpp"
#include "libp2p/peer/peer_id.hpp"

namespace libp2p::peer {

  namespace event {
    struct KeyPairChanged {};
    using KeyPairChangedChannel =
        libp2p::event::channel_decl<KeyPairChanged, crypto::KeyPair>;
  }  // namespace event

  /**
   * @brief Component, which "owns" information about current Host identity.
   */
  struct IdentityManager {
    virtual ~IdentityManager() = default;

    virtual const peer::PeerId &getId() const = 0;

    virtual const crypto::KeyPair &getKeyPair() const = 0;
  };

}  // namespace libp2p::peer

#endif  // KAGOME_IDENTITY_MANAGER_HPP
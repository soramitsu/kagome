/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_PEER_ID_HPP
#define KAGOME_PEER_ID_HPP

#include <memory>
#include <optional>
#include <string_view>

#include "common/buffer.hpp"
#include "common/result.hpp"
#include "libp2p/crypto/crypto_provider.hpp"
#include "libp2p/crypto/private_key.hpp"
#include "libp2p/crypto/public_key.hpp"
#include "libp2p/multi/multibase_codec.hpp"

namespace libp2p::peer {
  /**
   * Peer of the network
   */
  class PeerId {
    /// so that private ctor could be called from the factory
    friend class PeerIdFactory;

   public:
    PeerId() = delete;

    /**
     * Get hex representation of the Peer's id
     * @return hex string with the id
     */
    std::string_view toHex() const;

    /**
     * Get bytes representation of the Peer's id
     * @return bytes with the id
     */
    const kagome::common::Buffer &toBytes() const;

    /**
     * Get base58 representation of the Peer's id
     * @return base58-encoded string with the id
     */
    std::string_view toBase58() const;

    /**
     * Get public key of the Peer
     * @return pointer to key; can be NULL
     */
    std::shared_ptr<crypto::PublicKey> publicKey() const;

    /**
     * Set public key of the Peer
     * @param public_key to be set
     * @return true, if it was set, false otherwise
     * @note if private key is set, this public key must be derived from it
     */
    bool setPublicKey(std::shared_ptr<crypto::PublicKey> public_key);

    /**
     * Get private key of the Peer
     * @return pointer to key; can be NULL
     */
    std::shared_ptr<crypto::PrivateKey> privateKey() const;

    /**
     * Set private key of the Peer
     * @param private_key to be set
     * @return true, if it was set, false otherwise
     * @note if public key is set, this private key must derive the public key
     */
    bool setPrivateKey(std::shared_ptr<crypto::PrivateKey> private_key);

    /**
     * Get a Protobuf representation of the public key
     * @return optional with bytes of the key
     */
    std::optional<kagome::common::Buffer> marshalPublicKey() const;

    /**
     * Get a Protobuf representation of the private key
     * @return optional with bytes of the key
     */
    std::optional<kagome::common::Buffer> marshalPrivateKey() const;

    /**
     * Get a string representation of that Peer
     * @return stringified Peer
     */
    std::string_view toString() const;

    bool operator==(const PeerId &other) const;

   private:
    PeerId(kagome::common::Buffer id,
           const multi::MultibaseCodec &multibase_codec,
           const crypto::CryptoProvider &crypto_provider);
    PeerId(kagome::common::Buffer id,
           std::shared_ptr<crypto::PublicKey> public_key,
           std::shared_ptr<crypto::PrivateKey> private_key,
           const multi::MultibaseCodec &multibase_codec,
           const crypto::CryptoProvider &crypto_provider);

    /**
     * Set a public key without performing any checks
     * @param public_key to be set
     */
    void unsafeSetPublicKey(std::shared_ptr<crypto::PublicKey> public_key);

    /**
     * Set a private key without peforming any checks
     * @param private_key to be set
     */
    void unsafeSetPrivateKey(std::shared_ptr<crypto::PrivateKey> private_key);

    kagome::common::Buffer id_;
    std::shared_ptr<crypto::PublicKey> public_key_;
    std::shared_ptr<crypto::PrivateKey> private_key_;

    const multi::MultibaseCodec &multibase_codec_;
    const crypto::CryptoProvider &crypto_provider_;
  };
}  // namespace libp2p::peer

#endif  // KAGOME_PEER_HPP

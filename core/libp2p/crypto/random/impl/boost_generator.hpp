/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_LIBP2P_CRYPTO_RANDOM_BOOST_GENERATOR_HPP
#define KAGOME_CORE_LIBP2P_CRYPTO_RANDOM_BOOST_GENERATOR_HPP

#include <boost/nondet_random.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include "libp2p/crypto/random/cryptographic_safe_generator.hpp"

namespace libp2p::crypto::random {
  /**
   * @class BoostRandomGenerator provides implementation
   * of cryptographic-secure random bytes generator
   * on systems which don't provide true random numbers source
   * it may not compile, so you will need to implement
   * your own random bytes generator
   */
  class BoostRandomGenerator : public CryptographicallySecureRandomGenerator {
    using Buffer = kagome::common::Buffer;

   public:
    /**
     * @brief generators random bytes
     * @param out pointer to buffer
     * @param len number of bytes
     */
    void randomBytes(unsigned char *out, size_t len) override;

   private:
    /// boost cryptographic-secure random generator
    boost::random_device generator_;
    /// uniform distribution tool
    boost::random::uniform_int_distribution<uint8_t> distribution_;
  };
}  // namespace libp2p::crypto::random

#endif  // KAGOME_CORE_LIBP2P_CRYPTO_RANDOM_BOOST_GENERATOR_HPP

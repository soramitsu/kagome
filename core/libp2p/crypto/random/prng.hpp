/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_LIBP2P_CRYPTO_RANDOM_PSEUDO_RANDOM_GENERATOR_HPP
#define KAGOME_CORE_LIBP2P_CRYPTO_RANDOM_PSEUDO_RANDOM_GENERATOR_HPP

#include "libp2p/crypto/random/random_generator.hpp"

namespace libp2p::crypto::random {

  /// class PseudoRandomGenerator provides interface for pseudo-random generator
  class PRNG : public RandomGenerator {
   public:
    /**
     * @brief generators random bytes
     * @param out pointer to buffer
     * @param len number of bytes
     */
    void randomBytes(unsigned char *out, size_t len) override  = 0;
  };
}  // namespace libp2p::crypto::random

#endif  // KAGOME_CORE_LIBP2P_CRYPTO_RANDOM_PSEUDO_RANDOM_GENERATOR_HPP

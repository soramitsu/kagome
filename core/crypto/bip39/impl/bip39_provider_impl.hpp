/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_BIP39_PROVIDER_IMPL_HPP
#define KAGOME_BIP39_PROVIDER_IMPL_HPP

#include "crypto/bip39/bip39_provider.hpp"

#include "crypto/pbkdf2/pbkdf2_provider.hpp"

namespace kagome::crypto {
  class Bip39ProviderImpl : public Bip39Provider {
   public:
    ~Bip39ProviderImpl() override = default;

    explicit Bip39ProviderImpl(std::shared_ptr<Pbkdf2Provider> pbkdf2_provider);

    outcome::result<bip39::Bip39Seed> makeSeed(std::string_view phrase);

   private:
    std::shared_ptr<Pbkdf2Provider> pbkdf2_provider_;
  };
}  // namespace kagome::crypto

#endif  // KAGOME_BIP39_PROVIDER_IMPL_HPP

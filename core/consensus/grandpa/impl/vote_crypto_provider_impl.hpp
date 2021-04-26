/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_CONSENSUS_GRANDPA_IMPL_VOTE_CRYPTO_PROVIDER_IMPL_HPP
#define KAGOME_CORE_CONSENSUS_GRANDPA_IMPL_VOTE_CRYPTO_PROVIDER_IMPL_HPP

#include "consensus/grandpa/vote_crypto_provider.hpp"

#include "consensus/authority/authority_manager.hpp"
#include "consensus/grandpa/voter_set.hpp"
#include "crypto/ed25519_provider.hpp"

namespace kagome::consensus::grandpa {

  class VoteCryptoProviderImpl : public VoteCryptoProvider {
   public:
    ~VoteCryptoProviderImpl() override = default;

    VoteCryptoProviderImpl(
        boost::optional<crypto::Ed25519Keypair> keypair,
        std::shared_ptr<crypto::Ed25519Provider> ed_provider,
        RoundNumber round_number,
        std::shared_ptr<authority::AuthorityManager> authority_manager);

    bool verifyPrimaryPropose(
        const SignedMessage &primary_propose) const override;
    bool verifyPrevote(const SignedMessage &prevote) const override;
    bool verifyPrecommit(const SignedMessage &precommit) const override;

    boost::optional<SignedMessage> signPrimaryPropose(
        const PrimaryPropose &primary_propose) const override;
    boost::optional<SignedMessage> signPrevote(
        const Prevote &prevote) const override;
    boost::optional<SignedMessage> signPrecommit(
        const Precommit &precommit) const override;

   private:
    boost::optional<SignedMessage> sign(Vote vote) const;
    bool verify(const SignedMessage &vote, RoundNumber number) const;

    boost::optional<crypto::Ed25519Keypair> keypair_;
    std::shared_ptr<crypto::Ed25519Provider> ed_provider_;
    RoundNumber round_number_;
    std::shared_ptr<authority::AuthorityManager> authority_manager_;
  };

}  // namespace kagome::consensus::grandpa

#endif  // KAGOME_CORE_CONSENSUS_GRANDPA_IMPL_VOTE_CRYPTO_PROVIDER_IMPL_HPP

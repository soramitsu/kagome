/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 *
 * Kagome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Kagome is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef KAGOME_BABE_BLOCK_VALIDATOR_HPP
#define KAGOME_BABE_BLOCK_VALIDATOR_HPP

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "blockchain/block_tree.hpp"
#include "common/logger.hpp"
#include "consensus/babe/types/babe_block_header.hpp"
#include "consensus/babe/types/seal.hpp"
#include "consensus/validation/block_validator.hpp"
#include "crypto/hasher.hpp"
#include "crypto/vrf_provider.hpp"
#include "primitives/authority.hpp"
#include "runtime/tagged_transaction_queue.hpp"

namespace kagome::crypto {
  class SR25519Provider;
}

namespace kagome::consensus {
  /**
   * Validation of blocks in BABE system. Based on the algorithm described here:
   * https://research.web3.foundation/en/latest/polkadot/BABE/Babe/#2-normal-phase
   */
  class BabeBlockValidator : public BlockValidator {
   public:
    ~BabeBlockValidator() override = default;

    /**
     * Create an instance of BabeBlockValidator
     * @param block_tree to be used by this instance
     * @param tx_queue to validate the extrinsics
     * @param hasher to take hashes
     * @param vrf_provider for VRF-specific operations
     */
    BabeBlockValidator(
        std::shared_ptr<blockchain::BlockTree> block_tree,
        std::shared_ptr<runtime::TaggedTransactionQueue> tx_queue,
        std::shared_ptr<crypto::Hasher> hasher,
        std::shared_ptr<crypto::VRFProvider> vrf_provider,
        std::shared_ptr<crypto::SR25519Provider> sr25519_provider);

    enum class ValidationError {
      NO_AUTHORITIES = 1,
      INVALID_SIGNATURE,
      INVALID_VRF,
      TWO_BLOCKS_IN_SLOT,
      INVALID_TRANSACTIONS
    };

    outcome::result<void> validateBlock(
        const primitives::Block &block,
        const primitives::AuthorityId &authority_id,
        const Threshold &threshold,
        const Randomness &randomness) const override;

    outcome::result<void> validateHeader(
        const primitives::BlockHeader &header,
        const primitives::AuthorityId &authority_id,
        const Threshold &threshold,
        const Randomness &randomness) const override;

   private:
    /**
     * Verify that block is signed by valid signature
     * @param header Header to be checked
     * @param babe_header BabeBlockHeader corresponding to (fetched from) header
     * @param seal Seal corresponding to (fetched from) header
     * @param public_key public key that corresponds to the authority by
     * authority index
     * @return true if signature is valid, false otherwise
     */
    bool verifySignature(const primitives::BlockHeader &header,
                         const BabeBlockHeader &babe_header,
                         const Seal &seal,
                         const primitives::SessionKey &public_key) const;

    /**
     * Verify that vrf value contained in babe_header is less than threshold and
     * was generated by the creator of the block
     * @param babe_header BabeBlockHeader corresponding to (fetched from) header
     * @param public_key Public key of creator of the block
     * @param threshold threshold value for that epoch
     * @param randomness randomness for that epoch
     * @return true if vrf is valid, false otherwise
     */
    bool verifyVRF(const BabeBlockHeader &babe_header,
                   const primitives::SessionKey &public_key,
                   const Threshold &threshold,
                   const Randomness &randomness) const;

    /**
     * Check, if the peer has produced a block in this slot and memorize, if the
     * peer hasn't
     * @param peer to be checked
     * @return true, if the peer has not produced any blocks in this slot, false
     * otherwise
     */
    bool verifyProducer(const BabeBlockHeader &babe_header) const;

    /**
     * Check, if all transactions in the block are valid
     * @return true, if all transactions have passed verification, false
     * otherwise
     */
    bool verifyTransactions(const primitives::BlockBody &block_body) const;

    std::shared_ptr<blockchain::BlockTree> block_tree_;
    mutable std::unordered_map<BabeSlotNumber,
                               std::unordered_set<primitives::AuthorityIndex>>
        blocks_producers_;

    std::shared_ptr<runtime::TaggedTransactionQueue> tx_queue_;

    std::shared_ptr<crypto::Hasher> hasher_;

    std::shared_ptr<crypto::VRFProvider> vrf_provider_;
    std::shared_ptr<crypto::SR25519Provider> sr25519_provider_;

    common::Logger log_;
  };
}  // namespace kagome::consensus

OUTCOME_HPP_DECLARE_ERROR(kagome::consensus,
                          BabeBlockValidator::ValidationError)

#endif  // KAGOME_BABE_BLOCK_VALIDATOR_HPP

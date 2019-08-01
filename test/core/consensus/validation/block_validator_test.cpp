/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/validation/babe_block_validator.hpp"

#include <gtest/gtest.h>
#include "crypto/util.hpp"
#include "mock/core/blockchain/block_tree_mock.hpp"
#include "mock/core/crypto/vrf_provider_mock.hpp"
#include "mock/core/runtime/tagged_transaction_queue_mock.hpp"
#include "mock/crypto/hasher.hpp"
#include "scale/scale.hpp"
#include "testutil/literals.hpp"

using namespace kagome;
using namespace blockchain;
using namespace consensus;
using namespace runtime;
using namespace primitives;
using namespace common;
using namespace crypto;

using testing::_;
using testing::Return;

class BlockValidatorTest : public testing::Test {
 public:
  std::pair<Seal, Blob<SR25519_PUBLIC_SIZE>> sealBlock(
      Block &block, Hash256 block_hash) const {
    // generate a new keypair
    std::array<uint8_t, SR25519_SIGNATURE_SIZE> sr25519_signature{};

    std::array<uint8_t, SR25519_KEYPAIR_SIZE> sr25519_keypair{};
    std::array<uint8_t, SR25519_SEED_SIZE> seed{};
    sr25519_keypair_from_seed(sr25519_keypair.data(), seed.data());

    gsl::span<uint8_t, SR25519_KEYPAIR_SIZE> keypair_span(sr25519_keypair);
    auto secret = keypair_span.subspan(0, SR25519_SECRET_SIZE);
    auto pub_key =
        keypair_span.subspan(SR25519_SECRET_SIZE, SR25519_PUBLIC_SIZE);

    // create a signature
    sr25519_sign(sr25519_signature.data(),
                 pub_key.data(),
                 secret.data(),
                 block_hash.data(),
                 Hash256::size());

    // seal the block
    Seal seal{sr25519_signature};
    block.header.digests.emplace_back(scale::encode(seal).value());

    Blob<SR25519_PUBLIC_SIZE> pubkey_blob{};
    std::copy(pub_key.begin(), pub_key.end(), pubkey_blob.begin());
    return {seal, pubkey_blob};
  }

  // fields for validator
  std::shared_ptr<BlockTreeMock> tree_ = std::make_shared<BlockTreeMock>();
  std::shared_ptr<TaggedTransactionQueueMock> tx_queue_ =
      std::make_shared<TaggedTransactionQueueMock>();
  std::shared_ptr<HasherMock> hasher_ = std::make_shared<HasherMock>();
  std::shared_ptr<VRFProviderMock> vrf_provider_ =
      std::make_shared<VRFProviderMock>();

  BabeBlockValidator validator_{tree_, tx_queue_, hasher_, vrf_provider_};

  // fields for block
  Hash256 parent_hash_ =
      Hash256::fromString("c30ojfn4983u4093jv3894j3f034ojs3").value();

  BabeSlotNumber slot_number_ = 2;
  VRFValue vrf_value_ = 1488228;
  VRFProof vrf_proof_{};
  AuthorityIndex authority_index_ = 1;
  BabeBlockHeader babe_header_{
      slot_number_, {vrf_value_, vrf_proof_}, authority_index_};
  Buffer encoded_babe_header_{scale::encode(babe_header_).value()};

  BlockHeader block_header_{
      .parent_hash = parent_hash_,
      .digests = std::vector<Digest>{encoded_babe_header_}};
  Extrinsic ext_{Buffer{0x11, 0x22}};
  BlockBody block_body_{ext_};
  Block valid_block_{block_header_, block_body_};

  // fields for validation
  libp2p::peer::PeerId peer_id_ = "my_peer"_peerid;

  Epoch babe_epoch_{.randomness = 475995757021, .threshold = 3820948573};
};

/**
 * @given block validator
 * @when validating a valid block
 * @then success
 */
TEST_F(BlockValidatorTest, Success) {
  // verifySignature
  // get an encoded pre-seal part of the block's header
  auto block_copy = valid_block_;
  block_copy.header.digests.pop_back();
  auto encoded_block_copy = scale::encode(block_copy.header).value();
  Hash256 encoded_block_copy_hash{};  // not a real hash, but don't want to
                                      // actually take it
  std::copy(encoded_block_copy.begin(),
            encoded_block_copy.begin() + Hash256::size(),
            encoded_block_copy_hash.begin());

  auto [seal, pubkey] = sealBlock(valid_block_, encoded_block_copy_hash);

  EXPECT_CALL(*hasher_, blake2s_256(_))
      .WillOnce(Return(encoded_block_copy_hash));
  babe_epoch_.authorities.emplace_back();
  babe_epoch_.authorities.emplace_back(Authority{pubkey, 42});

  // verifyVRF
  auto randomness_with_slot =
      Buffer{}
          .put(util::uint256_t_to_bytes(babe_epoch_.randomness))
          .put(util::uint256_t_to_bytes(babe_epoch_.threshold));
  EXPECT_CALL(*vrf_provider_, verify(randomness_with_slot, _, pubkey))
      .WillOnce(Return(true));

  // verifyTransactions
  EXPECT_CALL(*tx_queue_, validate_transaction(ext_)).WillOnce(Return(Valid{}));

  // addBlock
  EXPECT_CALL(*tree_, addBlock(valid_block_))
      .WillOnce(Return(outcome::success()));

  ASSERT_TRUE(validator_.validate(valid_block_, peer_id_, babe_epoch_));
}

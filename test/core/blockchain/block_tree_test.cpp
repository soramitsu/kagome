/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "blockchain/impl/level_db_block_tree.hpp"

#include <gtest/gtest.h>
#include "blockchain/impl/level_db_util.hpp"
#include "common/blob.hpp"
#include "common/buffer.hpp"
#include "crypto/hasher/hasher_impl.hpp"
#include "mock/core/storage/persistent_map_mock.hpp"
#include "primitives/block_id.hpp"
#include "primitives/justification.hpp"
#include "scale/scale.hpp"
#include "testutil/outcome.hpp"

using namespace kagome;
using namespace storage;
using namespace common;
using namespace primitives;
using namespace blockchain;

using prefix::Prefix;
using testing::_;
using testing::Return;

struct BlockTreeTest : public testing::Test {
  void SetUp() override {
    // for LevelDbBlockTree::create(..)
    EXPECT_CALL(db_, get(_))
        .WillOnce(Return(kFinalizedBlockLookupKey))
        .WillOnce(Return(Buffer{encoded_finalized_block_header_}));

    block_tree_ =
        LevelDbBlockTree::create(db_, kLastFinalizedBlockId, hasher_).value();
  }

  /**
   * Add a block with some data, which is a child of the top-most block
   * @return block, which was added, along with its hash
   */
  std::pair<Block, BlockHash> addChildBlock() {
    BlockHeader header{.parent_hash = kFinalizedBlockHash,
                       .number = 1,
                       .digest = Buffer{0x66, 0x44}};
    BlockBody body{{Buffer{0x55, 0x55}}};
    Block new_block{header, body};

    EXPECT_CALL(db_, put(_, _))
        .Times(4)
        .WillRepeatedly(Return(outcome::success()));
    EXPECT_CALL(db_, put(_, Buffer{scale::encode(header).value()}))
        .WillOnce(Return(outcome::success()));
    EXPECT_CALL(db_, put(_, Buffer{scale::encode(body).value()}))
        .WillOnce(Return(outcome::success()));

    EXPECT_TRUE(block_tree_->addBlock(new_block));

    auto encoded_block = scale::encode(new_block).value();
    auto hash = hasher_->blake2_256(encoded_block);

    return {new_block, hash};
  }

  const Buffer kFinalizedBlockLookupKey{0x12, 0x85};
  const Buffer kFinalizedBlockHashWithKey =
      Buffer{}.putUint8(Prefix::ID_TO_LOOKUP_KEY).put(kFinalizedBlockHash);
  const Buffer kFinalizedBlockHashWithKeyAndHeader =
      Buffer{}.putUint8(Prefix::HEADER).putBuffer(kFinalizedBlockLookupKey);
  const Buffer kFinalizedBlockHashWithKeyAndBody =
      Buffer{}.putUint8(Prefix::BODY).putBuffer(kFinalizedBlockLookupKey);

  const BlockHash kFinalizedBlockHash =
      BlockHash::fromString("andj4kdn4odnfkslfn3k4jdnbmeodkv4").value();

  face::PersistentMapMock<Buffer, Buffer> db_;
  const BlockId kLastFinalizedBlockId = kFinalizedBlockHash;
  std::shared_ptr<hash::Hasher> hasher_ = std::make_shared<hash::HasherImpl>();

  std::unique_ptr<LevelDbBlockTree> block_tree_;

  BlockHeader finalized_block_header_{.number = 0,
                                      .digest = Buffer{0x11, 0x33}};
  std::vector<uint8_t> encoded_finalized_block_header_ =
      scale::encode(finalized_block_header_).value();

  BlockBody finalized_block_body_{{Buffer{0x22, 0x44}}, {Buffer{0x55, 0x66}}};
  std::vector<uint8_t> encoded_finalized_block_body_ =
      scale::encode(finalized_block_body_).value();
};

/**
 * @given block tree with at least one block inside
 * @when requesting body of that block
 * @then body is returned
 */
TEST_F(BlockTreeTest, GetBody) {
  EXPECT_CALL(db_, get(_))
      .WillOnce(Return(kFinalizedBlockLookupKey))
      .WillOnce(Return(Buffer{encoded_finalized_block_body_}));

  EXPECT_OUTCOME_TRUE(body, block_tree_->getBlockBody(kLastFinalizedBlockId))
  ASSERT_EQ(body, finalized_block_body_);
}

/**
 * @given block tree with at least one block inside
 * @when adding a new block, which is a child of that block
 * @then block is added
 */
TEST_F(BlockTreeTest, AddBlock) {
  auto [block, hash] = addChildBlock();
}

/**
 * @given block tree with at least one block inside
 * @when adding a new block, which is not a child of any block inside
 * @then corresponding error is returned
 */
TEST_F(BlockTreeTest, AddBlockNoParent) {
  BlockHeader header{.digest = Buffer{0x66, 0x44}};
  BlockBody body{{Buffer{0x55, 0x55}}};
  Block new_block{header, body};

  EXPECT_OUTCOME_FALSE(err, block_tree_->addBlock(new_block));
  ASSERT_EQ(err, LevelDbBlockTree::Error::NO_PARENT);
}

TEST_F(BlockTreeTest, Finalize) {
  auto [block, hash] = addChildBlock();

  Justification justification{{0x45, 0xF4}};
  auto encoded_justification = scale::encode(justification).value();
  EXPECT_CALL(db_, put(_, _))
      .Times(2)
      .WillRepeatedly(Return(outcome::success()));
  EXPECT_CALL(db_, put(_, Buffer{encoded_justification}))
      .WillOnce(Return(outcome::success()));

  ASSERT_TRUE(block_tree_->finalize(hash, justification));
  ASSERT_EQ(block_tree_->getLastFinalized(), hash);
}

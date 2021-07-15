/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "runtime/runtime_api/impl/grandpa_api.hpp"

#include <gtest/gtest.h>

#include "core/runtime/binaryen/runtime_test.hpp"
#include "host_api/impl/host_api_impl.hpp"
#include "mock/core/blockchain/block_header_repository_mock.hpp"
#include "runtime/runtime_api/impl/metadata.hpp"
#include "runtime/binaryen/wasm_memory_impl.hpp"
#include "testutil/outcome.hpp"
#include "testutil/prepare_loggers.hpp"

using ::testing::_;
using ::testing::Return;

using kagome::blockchain::BlockHeaderRepositoryMock;
using kagome::runtime::Metadata;
using kagome::runtime::MetadataImpl;

namespace fs = boost::filesystem;

class MetadataTest : public RuntimeTest {
 public:
  static void SetUpTestCase() {
    testutil::prepareLoggers();
  }

  void SetUp() override {
    RuntimeTest::SetUp();
    prepareEphemeralStorageExpects();

    api_ = std::make_shared<MetadataImpl>(
        std::make_shared<BlockHeaderRepositoryMock>(), executor_);
  }

 protected:
  std::shared_ptr<Metadata> api_;
};

/**
 * @given initialized Metadata api
 * @when metadata() is invoked
 * @then successful result is returned
 */
TEST_F(MetadataTest, metadata) {
  EXPECT_CALL(*storage_provider_, rollbackTransaction());
  ASSERT_TRUE(api_->metadata({}));
}

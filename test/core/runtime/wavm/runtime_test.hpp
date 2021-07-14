/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_RUNTIME_TEST_HPP
#define KAGOME_RUNTIME_TEST_HPP

#include <gtest/gtest.h>

#include <boost/filesystem/path.hpp>
#include <fstream>
#include <memory>

#include "crypto/bip39/impl/bip39_provider_impl.hpp"
#include "crypto/crypto_store/crypto_store_impl.hpp"
#include "crypto/ed25519/ed25519_provider_impl.hpp"
#include "crypto/hasher/hasher_impl.hpp"
#include "crypto/pbkdf2/impl/pbkdf2_provider_impl.hpp"
#include "crypto/random_generator/boost_generator.hpp"
#include "crypto/secp256k1/secp256k1_provider_impl.hpp"
#include "crypto/sr25519/sr25519_provider_impl.hpp"
#include "host_api/impl/host_api_factory_impl.hpp"
#include "mock/core/blockchain/block_header_repository_mock.hpp"
#include "mock/core/runtime/trie_storage_provider_mock.hpp"
#include "mock/core/storage/changes_trie/changes_tracker_mock.hpp"
#include "mock/core/storage/trie/trie_batches_mock.hpp"
#include "mock/core/storage/trie/trie_storage_mock.hpp"
#include "primitives/block.hpp"
#include "primitives/block_header.hpp"
#include "primitives/block_id.hpp"
#include "runtime/common/module_repository_impl.hpp"
#include "runtime/common/runtime_transaction_error.hpp"
#include "runtime/common/runtime_upgrade_tracker_impl.hpp"
#include "runtime/core_api_factory.hpp"
#include "runtime/executor.hpp"
#include "runtime/runtime_environment_factory.hpp"
#include "runtime/wavm/compartment_wrapper.hpp"
#include "runtime/wavm/core_api_factory.hpp"
#include "runtime/wavm/intrinsics/intrinsic_module.hpp"
#include "runtime/wavm/intrinsics/intrinsic_resolver_impl.hpp"
#include "runtime/wavm/module_factory_impl.hpp"
#include "runtime/wavm/wavm_memory_provider.hpp"
#include "testutil/literals.hpp"
#include "testutil/outcome.hpp"
#include "testutil/runtime/common/basic_wasm_provider.hpp"

class RuntimeTest : public ::testing::Test {
 public:
  using Buffer = kagome::common::Buffer;
  using Block = kagome::primitives::Block;
  using BlockId = kagome::primitives::BlockId;
  using BlockHeader = kagome::primitives::BlockHeader;
  using Extrinsic = kagome::primitives::Extrinsic;
  using Digest = kagome::primitives::Digest;

  void SetUp() override {
    using kagome::storage::trie::EphemeralTrieBatchMock;
    using kagome::storage::trie::PersistentTrieBatch;
    using kagome::storage::trie::PersistentTrieBatchMock;
    using kagome::storage::trie::TopperTrieBatchMock;

    batch_mock_ = std::make_shared<PersistentTrieBatchMock>();
    ON_CALL(*batch_mock_, get(testing::_))
        .WillByDefault(testing::Return(kagome::common::Buffer()));
    ON_CALL(*batch_mock_, batchOnTop()).WillByDefault(testing::Invoke([] {
      return std::make_unique<TopperTrieBatchMock>();
    }));
    storage_provider_ =
        std::make_shared<kagome::runtime::TrieStorageProviderMock>();
    ON_CALL(*storage_provider_, getCurrentBatch())
        .WillByDefault(testing::Return(batch_mock_));
    ON_CALL(*storage_provider_, tryGetPersistentBatch())
        .WillByDefault(testing::Invoke(
            [&]() -> boost::optional<std::shared_ptr<PersistentTrieBatch>> {
              return std::shared_ptr<PersistentTrieBatch>(batch_mock_);
            }));
    ON_CALL(*storage_provider_, setToPersistent())
        .WillByDefault(testing::Return(outcome::success()));
    ON_CALL(*storage_provider_, setToEphemeral())
        .WillByDefault(testing::Return(outcome::success()));
    ON_CALL(*storage_provider_, rollbackTransaction())
        .WillByDefault(testing::Return(
            outcome::failure(kagome::runtime::RuntimeTransactionError::
                                 NO_TRANSACTIONS_WERE_STARTED)));
    ON_CALL(*storage_provider_, getLatestRootMock())
        .WillByDefault(testing::Return("42"_hash256));

    auto random_generator =
        std::make_shared<kagome::crypto::BoostRandomGenerator>();
    auto sr25519_provider =
        std::make_shared<kagome::crypto::Sr25519ProviderImpl>(random_generator);
    auto ed25519_provider =
        std::make_shared<kagome::crypto::Ed25519ProviderImpl>(random_generator);
    auto secp256k1_provider =
        std::make_shared<kagome::crypto::Secp256k1ProviderImpl>();
    auto hasher = std::make_shared<kagome::crypto::HasherImpl>();
    auto pbkdf2_provider =
        std::make_shared<kagome::crypto::Pbkdf2ProviderImpl>();
    auto bip39_provider =
        std::make_shared<kagome::crypto::Bip39ProviderImpl>(pbkdf2_provider);
    auto keystore_path =
        boost::filesystem::temp_directory_path()
        / boost::filesystem::unique_path("kagome_keystore_test_dir");
    auto crypto_store = std::make_shared<kagome::crypto::CryptoStoreImpl>(
        std::make_shared<kagome::crypto::Ed25519Suite>(ed25519_provider),
        std::make_shared<kagome::crypto::Sr25519Suite>(sr25519_provider),
        bip39_provider,
        kagome::crypto::KeyFileStorage::createAt(keystore_path).value());
    changes_tracker_ =
        std::make_shared<kagome::storage::changes_trie::ChangesTrackerMock>();

    auto extension_factory =
        std::make_shared<kagome::host_api::HostApiFactoryImpl>(
            changes_tracker_,
            sr25519_provider,
            ed25519_provider,
            secp256k1_provider,
            hasher,
            crypto_store,
            bip39_provider);

    auto compartment =
        std::make_shared<kagome::runtime::wavm::CompartmentWrapper>(
            "Test Compartment");
    auto intrinsic_module =
        std::make_shared<kagome::runtime::wavm::IntrinsicModule>(compartment);
    std::shared_ptr intrinsic_module_instance = intrinsic_module->instantiate();
    auto resolver =
        std::make_shared<kagome::runtime::wavm::IntrinsicResolverImpl>(
            intrinsic_module_instance, compartment);

    auto module_factory =
        std::make_shared<kagome::runtime::wavm::ModuleFactoryImpl>(compartment,
                                                                   resolver);

    auto memory_provider =
        std::make_shared<kagome::runtime::wavm::WavmMemoryProvider>(
            intrinsic_module_instance);

    auto header_repo_mock =
        std::make_shared<kagome::blockchain::BlockHeaderRepositoryMock>();

    auto core_factory = std::make_shared<kagome::runtime::wavm::CoreApiFactory>(
        compartment,
        intrinsic_module,
        std::make_shared<kagome::storage::trie::TrieStorageMock>(),
        header_repo_mock,
        changes_tracker_,
        extension_factory);

    auto host_api = extension_factory->make(
        core_factory, memory_provider, storage_provider_);

    auto wasm_path = boost::filesystem::path(__FILE__).parent_path().string()
                     + "/wasm/sub2dev.wasm";
    wasm_provider_ =
        std::make_shared<kagome::runtime::BasicCodeProvider>(wasm_path);

    auto upgrade_tracker =
        std::make_shared<kagome::runtime::RuntimeUpgradeTrackerImpl>(
            header_repo_mock);

    auto module_repo = std::make_shared<kagome::runtime::ModuleRepositoryImpl>(
        upgrade_tracker, module_factory);

    runtime_env_factory_ =
        std::make_shared<kagome::runtime::RuntimeEnvironmentFactory>(
            storage_provider_,
            host_api,
            std::move(memory_provider),
            std::move(wasm_provider_),
            std::move(module_repo),
            header_repo_mock);

    executor_ = std::make_shared<kagome::runtime::Executor>(
        header_repo_mock, runtime_env_factory_);
  }

  void preparePersistentStorageExpects() {
    EXPECT_CALL(*storage_provider_, setToPersistent());
    EXPECT_CALL(*storage_provider_, tryGetPersistentBatch());
    prepareCommonStorageExpects();
  }

  void prepareEphemeralStorageExpects() {
    EXPECT_CALL(*storage_provider_, setToEphemeral());
    prepareCommonStorageExpects();
  }

  kagome::primitives::BlockHeader createBlockHeader() {
    kagome::common::Hash256 parent_hash;
    parent_hash.fill('p');

    kagome::primitives::BlockNumber number = 1;

    kagome::common::Hash256 state_root;
    state_root.fill('s');

    kagome::common::Hash256 extrinsics_root;
    extrinsics_root.fill('e');

    Digest digest{};

    return BlockHeader{
        parent_hash, number, state_root, extrinsics_root, digest};
  }

  kagome::primitives::Block createBlock() {
    auto header = createBlockHeader();

    std::vector<Extrinsic> xts;

    xts.emplace_back(Extrinsic{Buffer{'a', 'b', 'c'}});
    xts.emplace_back(Extrinsic{Buffer{'1', '2', '3'}});

    return Block{header, xts};
  }

  kagome::primitives::BlockId createBlockId() {
    BlockId res = kagome::primitives::BlockNumber{0};
    return res;
  }

 private:
  void prepareCommonStorageExpects() {
    auto heappages_key_res =
        kagome::common::Buffer::fromString(std::string(":heappages"));
    if (not heappages_key_res.has_value()) {
      GTEST_FAIL() << heappages_key_res.error().message();
    }
    auto &&heappages_key = heappages_key_res.value();
    EXPECT_CALL(*storage_provider_, getLatestRootMock());
    EXPECT_CALL(*storage_provider_, getCurrentBatch());
    EXPECT_CALL(*batch_mock_, get(heappages_key));
  }

 protected:
  std::shared_ptr<kagome::runtime::TrieStorageProviderMock> storage_provider_;
  std::shared_ptr<kagome::storage::trie::PersistentTrieBatchMock> batch_mock_;
  std::shared_ptr<kagome::runtime::RuntimeCodeProvider> wasm_provider_;
  std::shared_ptr<kagome::runtime::RuntimeEnvironmentFactory>
      runtime_env_factory_;
  std::shared_ptr<kagome::runtime::Executor> executor_;
  std::shared_ptr<kagome::storage::changes_trie::ChangesTracker>
      changes_tracker_;
};

#endif  // KAGOME_RUNTIME_TEST_HPP

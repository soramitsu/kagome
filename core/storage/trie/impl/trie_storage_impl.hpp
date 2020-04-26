/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_STORAGE_TRIE_IMPL_TRIE_STORAGE_IMPL
#define KAGOME_STORAGE_TRIE_IMPL_TRIE_STORAGE_IMPL

#include "storage/changes_trie/changes_tracker.hpp"
#include "storage/trie/codec.hpp"
#include "storage/trie/impl/polkadot_trie_factory.hpp"
#include "storage/trie/impl/trie_serializer.hpp"
#include "storage/trie/trie_storage.hpp"

namespace kagome::storage::trie {

  class TrieStorageImpl : public TrieStorage {
   public:
    TrieStorageImpl(common::Hash256 root_hash_,
                    std::shared_ptr<Codec> codec,
                    std::shared_ptr<TrieSerializer> serializer,
                    std::shared_ptr<changes_trie::ChangesTracker> changes);

    ~TrieStorageImpl() override = default;

    outcome::result<std::unique_ptr<PersistentTrieBatch>> getPersistentBatch()
        override;
    outcome::result<std::unique_ptr<EphemeralTrieBatch>> getEphemeralBatch()
        const override;
    common::Hash256 getRootHash() const override;

   private:
    common::Hash256 root_hash_;
    std::shared_ptr<Codec> codec_;
    std::shared_ptr<TrieSerializer> serializer_;
    std::shared_ptr<changes_trie::ChangesTracker> changes_;
  };

}  // namespace kagome::storage::trie

#endif  // KAGOME_STORAGE_TRIE_IMPL_TRIE_STORAGE_IMPL

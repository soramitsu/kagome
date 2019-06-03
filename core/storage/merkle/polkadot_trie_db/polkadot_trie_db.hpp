/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_STORAGE_MERKLE_POLKADOT_TRIE_DB_POLKADOT_TRIE_DB_HPP_
#define KAGOME_CORE_STORAGE_MERKLE_POLKADOT_TRIE_DB_POLKADOT_TRIE_DB_HPP_

#include <map>
#include <memory>

#include "crypto/hasher.hpp"
#include "storage/merkle/polkadot_trie_db/polkadot_codec.hpp"
#include "storage/merkle/polkadot_trie_db/polkadot_node.hpp"
#include "storage/merkle/trie_db.hpp"

namespace kagome::storage::merkle {

  class PolkadotTrieDb : public TrieDb {
    using MapCursor = face::MapCursor<common::Buffer, common::Buffer>;
    using WriteBatch = face::WriteBatch<common::Buffer, common::Buffer>;
    using Codec = scale::ScaleCodec<common::Buffer>;
    using NodePtr = std::shared_ptr<PolkadotNode>;
    using BranchPtr = std::shared_ptr<BranchNode>;

    template <typename Stream>
    friend Stream &operator<<(Stream &s, const PolkadotTrieDb &trie);

   public:
    enum class Error { INVALID_NODE_TYPE = 1 };

   public:
    PolkadotTrieDb(std::unique_ptr<PersistedBufferMap> db,
                   std::shared_ptr<Codec> codec,
                   std::shared_ptr<hash::Hasher> hasher);
    ~PolkadotTrieDb() override = default;

    common::Buffer getRootHash() const override;
    void clearPrefix(const common::Buffer &buf) override;

    std::unique_ptr<WriteBatch> batch() override;

    outcome::result<void> put(const common::Buffer &key,
                              const common::Buffer &value) override;
    outcome::result<void> remove(const common::Buffer &key) override;
    outcome::result<common::Buffer> get(
        const common::Buffer &key) const override;
    bool contains(const common::Buffer &key) const override;

    std::unique_ptr<MapCursor> cursor() override;

   private:
    outcome::result<void> insertRoot(const common::Buffer &key_nibbles,
                                     const common::Buffer &value);

    outcome::result<NodePtr> insert(const NodePtr& parent,
                                    const common::Buffer &key_nibbles,
                                    NodePtr node);

    outcome::result<NodePtr> updateBranch(BranchPtr parent,
                                          const common::Buffer &key_nibbles,
                                          const NodePtr& node);

    outcome::result<NodePtr> deleteNode(NodePtr parent,
                                        const common::Buffer &key_nibbles);
    outcome::result<NodePtr> handleDeletion(const BranchPtr& parent, NodePtr node,
                                            const common::Buffer &key_nibbles);

    outcome::result<NodePtr> getNode(NodePtr parent,
                                     const common::Buffer &key_nibbles) const;

    uint32_t getCommonPrefixLength(const common::Buffer &pref1,
                                   const common::Buffer &pref2) const;

    outcome::result<common::Buffer> storeNode(PolkadotNode &node);
    outcome::result<NodePtr> retrieveNode(const common::Buffer &db_key) const;
    outcome::result<NodePtr> retrieveChild(const BranchPtr& parent, uint8_t idx) const;

    std::unique_ptr<PersistedBufferMap> db_;
    std::shared_ptr<hash::Hasher> hasher_;
    PolkadotCodec codec_;
    std::optional<common::Buffer> root_;
  };

}  // namespace kagome::storage::merkle

OUTCOME_HPP_DECLARE_ERROR(kagome::storage::merkle, PolkadotTrieDb::Error);

#endif  // KAGOME_CORE_STORAGE_MERKLE_POLKADOT_TRIE_DB_POLKADOT_TRIE_DB_HPP_

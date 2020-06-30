/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_STORAGE_TRIE_POLKADOT_TRIE_HPP
#define KAGOME_STORAGE_TRIE_POLKADOT_TRIE_HPP

#include "storage/face/generic_maps.hpp"

#include "storage/trie/polkadot_trie/polkadot_node.hpp"

namespace kagome::storage::trie {

  /**
   * For specification see Polkadot Runtime Environment Protocol Specification
   * '2.1.2 The General Tree Structure' and further
   */
  class PolkadotTrie : public face::GenericMap<common::Buffer, common::Buffer> {
   public:
    using NodePtr = std::shared_ptr<PolkadotNode>;
    using BranchPtr = std::shared_ptr<BranchNode>;

    /**
     * Remove all trie entries which key begins with the supplied prefix
     */
    virtual outcome::result<void> clearPrefix(const common::Buffer &prefix) = 0;

    /**
     * @return the root node of the trie
     */
    virtual NodePtr getRoot() const = 0;

    /**
     * @returns a child node pointer of a provided \arg parent node
     * at the index \idx
     */
    virtual outcome::result<NodePtr> retrieveChild(BranchPtr parent,
                                                   uint8_t idx) const = 0;

    /**
     * @returns a node which is a descendant of \arg parent found by following
     * \arg key_nibbles
     */
    virtual outcome::result<NodePtr> getNode(
        NodePtr parent, const common::Buffer &key_nibbles) const = 0;

    /**
     * @returns a sequence of nodes in between \arg parent and the node found by
     * following \arg key_nibbles. The parent is included, the end node isn't.
     */
    virtual outcome::result<std::list<std::pair<BranchPtr, uint8_t>>> getPath(
        NodePtr parent, const common::Buffer &key_nibbles) const = 0;
  };

}  // namespace kagome::storage::trie

#endif  // KAGOME_STORAGE_TRIE_POLKADOT_TRIE_HPP

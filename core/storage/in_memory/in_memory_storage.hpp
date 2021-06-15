/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_STORAGE_IN_MEMORY_IN_MEMORY_STORAGE_HPP
#define KAGOME_STORAGE_IN_MEMORY_IN_MEMORY_STORAGE_HPP

#include <memory>

#include "common/buffer.hpp"
#include "metrics/metrics.hpp"
#include "outcome/outcome.hpp"
#include "storage/buffer_map_types.hpp"

namespace kagome::storage {

  /**
   * Simple storage that conforms PersistentMap interface
   * Mostly needed to have an in-memory trie in tests to avoid integration with
   * LevelDB
   */
  class InMemoryStorage : public storage::BufferStorage {
   public:
    InMemoryStorage();
    ~InMemoryStorage() override = default;

    outcome::result<common::Buffer> get(
        const common::Buffer &key) const override;

    outcome::result<void> put(const common::Buffer &key,
                              const common::Buffer &value) override;

    outcome::result<void> put(const common::Buffer &key,
                              common::Buffer &&value) override;

    bool contains(const common::Buffer &key) const override;

    bool empty() const override;

    outcome::result<void> remove(const common::Buffer &key) override;

    std::unique_ptr<
        kagome::storage::face::WriteBatch<common::Buffer, common::Buffer>>
    batch() override;

    std::unique_ptr<
        kagome::storage::face::MapCursor<common::Buffer, common::Buffer>>
    cursor() override;

   private:
    std::map<std::string, common::Buffer> storage;
    metrics::RegistryPtr registry_ = metrics::createRegistry();
    metrics::Gauge *in_memory_db_size_;
    metrics::Gauge *in_memory_db_index_size_;
  };

}  // namespace kagome::storage

#endif  // KAGOME_STORAGE_IN_MEMORY_IN_MEMORY_STORAGE_HPP

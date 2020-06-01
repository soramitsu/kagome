#ifndef KAGOME_STORAGE_CHANGES_TRIE_STORAGE_CHANGES_TRACKER_IMPL
#define KAGOME_STORAGE_CHANGES_TRIE_STORAGE_CHANGES_TRACKER_IMPL

#include "storage/changes_trie/changes_tracker.hpp"

namespace kagome::storage::trie {
  class Codec;
  class PolkadotTrieFactory;
}  // namespace kagome::storage::trie

namespace kagome::storage::changes_trie {

  class StorageChangesTrackerImpl : public ChangesTracker {
   public:
    enum class Error {
      EXTRINSIC_IDX_GETTER_UNINITIALIZED = 1,
      INVALID_PARENT_HASH
    };

    StorageChangesTrackerImpl(
        std::shared_ptr<storage::trie::PolkadotTrieFactory> trie_factory,
        std::shared_ptr<storage::trie::Codec> codec);

    /**
     * Functor that returns the current extrinsic index, which is supposed to
     * be stored in the state trie
     */
    ~StorageChangesTrackerImpl() override = default;

    void setExtrinsicIdxGetter(GetExtrinsicIndexDelegate f) override;

    outcome::result<void> onBlockChange(
        primitives::BlockHash new_parent_hash,
        primitives::BlockNumber new_parent_number) override;

    outcome::result<void> onChange(const common::Buffer &key) override;

    outcome::result<common::Hash256> constructChangesTrie(
        const primitives::BlockHash &parent,
        const ChangesTrieConfig &conf) override;

   private:
    std::shared_ptr<storage::trie::PolkadotTrieFactory> trie_factory_;
    std::shared_ptr<storage::trie::Codec> codec_;

    std::map<common::Buffer, std::vector<primitives::ExtrinsicIndex>>
        extrinsics_changes_;
    primitives::BlockHash parent_hash_;
    primitives::BlockNumber parent_number_;
    GetExtrinsicIndexDelegate get_extrinsic_index_;
  };

}  // namespace kagome::storage::changes_trie

OUTCOME_HPP_DECLARE_ERROR(kagome::storage::changes_trie,
                          StorageChangesTrackerImpl::Error);

#endif  // KAGOME_STORAGE_CHANGES_TRIE_STORAGE_CHANGES_TRACKER_IMPL

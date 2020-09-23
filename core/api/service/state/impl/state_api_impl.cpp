/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "api/service/state/impl/state_api_impl.hpp"

#include <utility>

namespace kagome::api {

  StateApiImpl::StateApiImpl(
      std::shared_ptr<blockchain::BlockHeaderRepository> block_repo,
      std::shared_ptr<const storage::trie::TrieStorage> trie_storage,
      std::shared_ptr<blockchain::BlockTree> block_tree,
      std::shared_ptr<runtime::Core> runtime_core)
      : block_repo_{std::move(block_repo)},
        storage_{std::move(trie_storage)},
        block_tree_{std::move(block_tree)},
        runtime_core_{std::move(runtime_core)} {
    BOOST_ASSERT(nullptr != block_repo_);
    BOOST_ASSERT(nullptr != storage_);
    BOOST_ASSERT(nullptr != block_tree_);
    BOOST_ASSERT(nullptr != runtime_core_);
  }

  outcome::result<std::vector<common::Buffer>> StateApiImpl::getKeysPaged(
      const boost::optional<common::Buffer> &prefix_opt,
      uint32_t keys_amount,
      const boost::optional<common::Buffer> &prev_key_opt,
      const boost::optional<primitives::BlockHash> &block_hash_opt) const {
    auto prefix = prefix_opt.value_or(common::Buffer{});
    auto prev_key = prev_key_opt.value_or(prefix);
    auto block_hash =
        block_hash_opt.value_or(block_tree_->getLastFinalized().block_hash);

    OUTCOME_TRY(header, block_repo_->getBlockHeader(block_hash));
    OUTCOME_TRY(initial_trie_reader,
                storage_->getEphemeralBatchAt(header.state_root));
    auto cursor = initial_trie_reader->trieCursor();

    // if prev_key is bigger than prefix, then set cursor to the next key after
    // prev_key
    if (prev_key > prefix) {
      OUTCOME_TRY(cursor->seekUpperBound(prev_key));
    }
    // otherwise set cursor to key that is next to or equal to prefix
    else {
      OUTCOME_TRY(cursor->seekLowerBound(prefix));
    }

    std::vector<common::Buffer> result{};
    result.reserve(keys_amount);
    for (uint32_t i = 0; i < keys_amount; i++) {
      if (!cursor->isValid()) {
        break;
      }
      BOOST_ASSERT(cursor->key());
      auto key = cursor->key().value();
      BOOST_ASSERT_MSG(initial_trie_reader->get(key).has_value(),
                       "Found key does not exist");

      // make sure our key begins with prefix
      if (not std::equal(prefix.begin(), prefix.end(), key.begin())) {
        break;
      }
      BOOST_ASSERT(cursor->value());
      result.push_back(cursor->key().value());
      OUTCOME_TRY(cursor->next());
    }

    return result;
  }

  outcome::result<common::Buffer> StateApiImpl::getStorage(
      const common::Buffer &key) const {
    auto last_finalized = block_tree_->getLastFinalized();
    return getStorage(key, last_finalized.block_hash);
  }

  outcome::result<common::Buffer> StateApiImpl::getStorage(
      const common::Buffer &key, const primitives::BlockHash &at) const {
    OUTCOME_TRY(header, block_repo_->getBlockHeader(at));
    OUTCOME_TRY(trie_reader, storage_->getEphemeralBatchAt(header.state_root));
    return trie_reader->get(key);
  }

  outcome::result<primitives::Version> StateApiImpl::getRuntimeVersion(
      const boost::optional<primitives::BlockHash> &at) const {
    return runtime_core_->version(at);
  }

  void StateApiImpl::setApiService(
      std::shared_ptr<api::ApiService> const &api_service) {
    BOOST_ASSERT(api_service != nullptr);
    api_service_ = api_service;
  }

  outcome::result<uint32_t> StateApiImpl::subscribeStorage(
      const std::vector<common::Buffer> &keys) {
    if (auto api_service = api_service_.lock())
      return api_service->subscribeSessionToKeys(keys);

    throw jsonrpc::InternalErrorFault(
        "Internal error. Api service not initialized.");
  }

  outcome::result<void> StateApiImpl::unsubscribeStorage(
      const std::vector<uint32_t> &subscription_id) {
    if (auto api_service = api_service_.lock())
      return api_service->unsubscribeSessionFromIds(subscription_id);

    throw jsonrpc::InternalErrorFault(
        "Internal error. Api service not initialized.");
  }
}  // namespace kagome::api

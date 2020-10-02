/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CHAIN_API_IMPL_HPP
#define KAGOME_CHAIN_API_IMPL_HPP

#include <memory>

#include "api/service/chain/chain_api.hpp"
#include "blockchain/block_header_repository.hpp"
#include "blockchain/block_tree.hpp"

namespace kagome::api {

  class ChainApiImpl : public ChainApi {
   public:
    ~ChainApiImpl() override = default;

    ChainApiImpl(std::shared_ptr<blockchain::BlockHeaderRepository> block_repo,
                 std::shared_ptr<blockchain::BlockTree> block_tree);

    void setApiService(
        std::shared_ptr<api::ApiService> const &api_service) override;

    outcome::result<BlockHash> getBlockHash() const override;

    outcome::result<BlockHash> getBlockHash(BlockNumber value) const override;

    outcome::result<BlockHash> getBlockHash(
        std::string_view value) const override;

    outcome::result<std::vector<BlockHash>> getBlockHash(
        gsl::span<const ValueType> values) const override;

    outcome::result<primitives::BlockHeader> getHeader(std::string_view hash) override {
      OUTCOME_TRY(h, primitives::BlockHash::fromHexWithPrefix(hash));
      return block_repo_->getBlockHeader(h);
    }

    outcome::result<primitives::BlockHeader> getHeader() override {
      auto last = block_tree_->getLastFinalized();
      return block_repo_->getBlockHeader(last.block_hash);
    }

    outcome::result<uint32_t> subscribeNewHeads() override;

   private:
    std::shared_ptr<blockchain::BlockHeaderRepository> block_repo_;
    std::shared_ptr<blockchain::BlockTree> block_tree_;
    std::weak_ptr<api::ApiService> api_service_;
  };
}  // namespace kagome::api

#endif  // KAGOME_CHAIN_API_IMPL_HPP

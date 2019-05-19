/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_RUNTIME_IMPL_GRANDPA_IMPL_HPP
#define KAGOME_CORE_RUNTIME_IMPL_GRANDPA_IMPL_HPP

#include "runtime/grandpa.hpp"

#include "primitives/scale_codec.hpp"
#include "primitives/scheduled_change.hpp"
#include "runtime/impl/wasm_executor.hpp"
#include "runtime/wasm_memory.hpp"

namespace kagome::runtime {
  class GrandpaImpl : public Grandpa {
   public:
    ~GrandpaImpl() override = default;

    /**
     * @brief constructor
     * @param state_code error or result code
     * @param extension extension instance
     * @param codec scale codec instance
     */
    GrandpaImpl(common::Buffer state_code,
                std::shared_ptr<extensions::Extension> extension,
                std::shared_ptr<primitives::ScaleCodec> codec);

    outcome::result<std::optional<ScheduledChange>> pendingChange(
        BlockId block_id, const Digest &digest) override;

    outcome::result<std::optional<ForcedChangeType>> forcedChange(
        BlockId block_id, const Digest &digest) override;

    outcome::result<std::vector<WeightedAuthority>> authorities(BlockId block_id) override;

   protected:
    std::shared_ptr<WasmMemory> memory_;
    std::shared_ptr<primitives::ScaleCodec> codec_;
    WasmExecutor executor_;
    common::Buffer state_code_;
  };
}  // namespace kagome::runtime

#endif  // KAGOME_CORE_RUNTIME_IMPL_GRANDPA_IMPL_HPP

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "authorship/impl/block_builder_impl.hpp"

#include "common/visitor.hpp"
#include "authorship/impl/block_builder_error.hpp"

namespace kagome::authorship {

  using primitives::events::ExtrinsicEventType;
  using primitives::events::ExtrinsicSubscriptionEngine;
  using primitives::events::InBlockEventParams;

  BlockBuilderImpl::BlockBuilderImpl(
      primitives::BlockHeader block_header,
      std::shared_ptr<runtime::BlockBuilder> block_builder_api)
      : block_header_(std::move(block_header)),
        block_builder_api_(std::move(block_builder_api)),
        logger_{common::createLogger("BlockBuilder")} {
    BOOST_ASSERT(block_builder_api_ != nullptr);
  }

  outcome::result<void> BlockBuilderImpl::pushExtrinsic(
      const primitives::Extrinsic &extrinsic) {
    auto apply_res = block_builder_api_->apply_extrinsic(extrinsic);
    if (not apply_res) {
      logger_->warn(
          "Extrinsic {} was not pushed to block. Error during xt application: "
          "{}",
          extrinsic.data.toHex(),
          apply_res.error().message());
      return apply_res.error();
    }

    const static std::string logger_error_template =
        "Extrinsic {} was not pushed to block. Extrinsic cannot be "
        "applied";
    return visit_in_place(
        apply_res.value(),
        [this, &extrinsic](
            primitives::ApplyOutcome apply_outcome) -> outcome::result<void> {
          switch (apply_outcome) {
            case primitives::ApplyOutcome::SUCCESS:
              extrinsics_.push_back(extrinsic);
              return outcome::success();
            case primitives::ApplyOutcome::FAIL:
              logger_->warn(logger_error_template, extrinsic.data.toHex());
              return BlockBuilderError::EXTRINSIC_APPLICATION_FAILED;
          }
        },
        [this, &extrinsic](primitives::ApplyError) -> outcome::result<void> {
          logger_->warn(logger_error_template, extrinsic.data.toHex());
          return BlockBuilderError::EXTRINSIC_APPLICATION_FAILED;
        });
  }

  outcome::result<primitives::Block> BlockBuilderImpl::bake() const {
    OUTCOME_TRY(finalised_header, block_builder_api_->finalise_block());
    return primitives::Block{finalised_header, extrinsics_};
  }

}  // namespace kagome::authorship

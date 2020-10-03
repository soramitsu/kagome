/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_API_STATE_API_HPP
#define KAGOME_API_STATE_API_HPP

#include <boost/optional.hpp>

#include "api/service/api_service.hpp"
#include "common/buffer.hpp"
#include "outcome/outcome.hpp"
#include "primitives/common.hpp"
#include "primitives/version.hpp"

namespace kagome::api {

  class StateApi {
   public:
    virtual ~StateApi() = default;

    virtual void setApiService(
        const std::shared_ptr<api::ApiService> &api_service) = 0;

    virtual outcome::result<std::vector<common::Buffer>> getKeysPaged(
        const boost::optional<common::Buffer> &prefix,
        uint32_t keys_amount,
        const boost::optional<common::Buffer> &prev_key,
        const boost::optional<primitives::BlockHash> &block_hash_opt) const = 0;

    virtual outcome::result<common::Buffer> getStorage(
        const common::Buffer &key) const = 0;
    virtual outcome::result<common::Buffer> getStorage(
        const common::Buffer &key, const primitives::BlockHash &at) const = 0;
    virtual outcome::result<primitives::Version> getRuntimeVersion(
        const boost::optional<primitives::BlockHash> &at) const = 0;
    virtual outcome::result<uint32_t> subscribeStorage(
        const std::vector<common::Buffer> &keys) = 0;
    virtual outcome::result<void> unsubscribeStorage(
        const std::vector<uint32_t> &subscription_id) = 0;
    virtual outcome::result<std::vector<uint8_t>> getMetadata() = 0;
  };

}  // namespace kagome::api

#endif  // KAGOME_API_STATE_API_HPP

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_RUNTIME_BINARYEN_RUNTIME_ENVIRONMENT_FACTORY_HPP
#define KAGOME_CORE_RUNTIME_BINARYEN_RUNTIME_ENVIRONMENT_FACTORY_HPP

#include "storage/trie/types.hpp"

namespace kagome::common {
  class Buffer;
}

namespace kagome::runtime::binaryen {

  class RuntimeEnvironment;

  /**
   * @brief RuntimeEnvironmentFactory is a mechanism to prepare environment for
   * launching execute() function of runtime APIs. It supports in-memory cache
   * to reuse existing environments, avoid hi-load operations.
   */
  class RuntimeEnvironmentFactory {
   public:

    virtual ~RuntimeEnvironmentFactory() = default;

    virtual outcome::result<RuntimeEnvironment> makeIsolated() = 0;

    virtual outcome::result<RuntimeEnvironment> makePersistent() = 0;

    virtual outcome::result<RuntimeEnvironment> makeEphemeral() = 0;

    virtual void setIsolatedCode(const kagome::common::Buffer& code) = 0;

    virtual outcome::result<RuntimeEnvironment> makeIsolatedAt(
        const storage::trie::RootHash &state_root) = 0;

    /**
     * @warning calling this with an \arg state_root older than the current root
     * will reset the storage to an older state once changes are committed
     */
    virtual outcome::result<RuntimeEnvironment> makePersistentAt(
        const storage::trie::RootHash &state_root) = 0;

    virtual outcome::result<RuntimeEnvironment> makeEphemeralAt(
        const storage::trie::RootHash &state_root) = 0;
  };

}  // namespace kagome::runtime::binaryen

#endif  // KAGOME_CORE_RUNTIME_BINARYEN_RUNTIME_ENVIRONMENT_FACTORY_HPP
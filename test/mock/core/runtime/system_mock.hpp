/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_RUNTIME_SYSTEM_MOCK
#define KAGOME_RUNTIME_SYSTEM_MOCK

#include "runtime/system.hpp"

#include <gmock/gmock.h>

namespace kagome::runtime {

  class SystemMock : public System {
   public:
    MOCK_METHOD1(account_nonce,
                 outcome::result<primitives::AccountNonce>(
                     const primitives::AccountId &));
  };

}  // namespace kagome::runtime

#endif  // KAGOME_RUNTIME_SYSTEM_MOCK

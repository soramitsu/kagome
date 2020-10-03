/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_RUNTIME_METADATAMOCK
#define KAGOME_RUNTIME_METADATAMOCK

#include "runtime/metadata.hpp"

#include <gmock/gmock.h>

namespace kagome::runtime {

  class MetadataMock : public Metadata {
   public:
    MOCK_METHOD0(metadata, outcome::result<Metadata::OpaqueMetadata>());
  };

}  // namespace kagome::runtime

#endif  // KAGOME_RUNTIME_METADATAMOCK

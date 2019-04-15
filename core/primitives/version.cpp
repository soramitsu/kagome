/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "primitives/version.hpp"

namespace kagome::primitives {

  Version::Version(const std::string &spec_name, const std::string &impl_name,
                   uint32_t authoring_version, uint32_t impl_version,
                   ApisVec apis)
      : spec_name_(spec_name),
        impl_name_(impl_name),
        authoring_version_(authoring_version),
        impl_version_(impl_version),
        apis_(std::move(apis)) {}

  const std::string &Version::specName() {
    return spec_name_;
  }

  const std::string &Version::implName() {
    return impl_name_;
  }

  uint32_t Version::authoringVersion() {
    return authoring_version_;
  }

  uint32_t Version::implVersion() {
    return impl_version_;
  }

  const ApisVec &Version::apis() {
    return apis_;
  }

}  // namespace kagome::primitives

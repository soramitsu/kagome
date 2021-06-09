/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_RUNTIME_WAVM_IMPL_MODULE_INSTANCE_HPP
#define KAGOME_CORE_RUNTIME_WAVM_IMPL_MODULE_INSTANCE_HPP

#include <string_view>

#include <WAVM/Runtime/Runtime.h>
#include <boost/optional.hpp>

#include "runtime/ptr_size.hpp"

namespace WAVM {
  namespace Runtime {
    struct Instance;
    struct Compartment;
  }  // namespace Runtime
  namespace IR {
    struct Value;
  }
}  // namespace WAVM

namespace kagome::runtime::wavm {

  class ModuleInstance {
   public:
    explicit ModuleInstance(WAVM::Runtime::Instance *instance,
                            WAVM::Runtime::Compartment *compartment);

    PtrSize callExportFunction(std::string_view name, PtrSize args);

    boost::optional<WAVM::IR::Value> getGlobal(std::string_view name);

   private:
    WAVM::Runtime::GCPointer<WAVM::Runtime::Instance> instance_;
    WAVM::Runtime::Compartment *compartment_;
  };

}  // namespace kagome::runtime::wavm

#endif  // KAGOME_CORE_RUNTIME_WAVM_IMPL_MODULE_INSTANCE_HPP

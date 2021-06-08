/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_RUNTIME_WAVM_IMPL_INTRINSIC_RESOLVER_IMPL_HPP
#define KAGOME_CORE_RUNTIME_WAVM_IMPL_INTRINSIC_RESOLVER_IMPL_HPP

#include "runtime/wavm/intrinsic_resolver.hpp"

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Linker.h>

namespace WAVM::Runtime {
  struct Instance;
  struct Compartment;
  struct ContextRuntimeData;
}  // namespace WAVM::Runtime

namespace WAVM::Intrinsics {
  struct Module;
  struct Function;
}  // namespace WAVM::Intrinsics

namespace kagome::runtime::wavm {

  class Memory;

  class IntrinsicResolverImpl final : public IntrinsicResolver {
   public:
    explicit IntrinsicResolverImpl();
    ~IntrinsicResolverImpl() noexcept override;

    bool resolve(const std::string &moduleName,
                 const std::string &exportName,
                 WAVM::IR::ExternType type,
                 WAVM::Runtime::Object *&outObject) override;

    //template <typename R, typename... Args>
    void addIntrinsic(
        std::string_view name,
        WAVM::Intrinsics::Function* func) {
      // side-effect of Function constructor is that this function is registered
      // in the module by pointer

     /* R (*dummy_pointer)(WAVM::Runtime::ContextRuntimeData *, Args...) {};
      auto type = WAVM::Intrinsics::inferIntrinsicFunctionType(dummy_pointer);
      auto function = std::make_unique<WAVM::Intrinsics::Function>(
          module_.get(), name.data(), static_cast<void *>(&func), type);*/
      functions_.emplace(name, func);
    }

    WAVM::Runtime::Memory *getMemory() const;

    WAVM::Runtime::Compartment * getCompartment() const override {
      return compartment_;
    }

    std::unique_ptr<IntrinsicResolver> clone() const override;

   private:
    WAVM::Intrinsics::Module* module_;
    // TODO(Harrm) cleanup
    WAVM::Runtime::Instance *module_instance_;
    WAVM::Runtime::GCPointer<WAVM::Runtime::Compartment> compartment_;
    std::unordered_map<std::string_view,
                       WAVM::Intrinsics::Function*>
        functions_;
  };

}  // namespace kagome::runtime::wavm

#endif  // KAGOME_CORE_RUNTIME_WAVM_IMPL_INTRINSIC_RESOLVER_IMPL_HPP

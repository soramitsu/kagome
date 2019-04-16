/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "runtime/impl/runtime_external_interface.hpp"

namespace kagome::runtime {

  const static wasm::Name env = "env";

  const static wasm::Name ext_malloc = "ext_malloc";
  const static wasm::Name ext_free = "ext_free";

  const static wasm::Name ext_clear_prefix = "ext_clear_prefix";
  const static wasm::Name ext_clear_storage = "ext_clear_storage";
  const static wasm::Name ext_exists_storage = "ext_exists_storage";
  const static wasm::Name ext_get_allocated_storage =
      "ext_get_allocated_storage";
  const static wasm::Name ext_get_storage_into = "ext_get_storage_into";
  const static wasm::Name ext_set_storage = "ext_set_storage";
  const static wasm::Name ext_blake2_256_enumerated_trie_root =
      "ext_blake2_256_enumerated_trie_root";
  const static wasm::Name ext_storage_changes_root = "ext_storage_changes_root";
  const static wasm::Name ext_storage_root = "ext_storage_root";
  const static wasm::Name ext_child_storage_root = "ext_child_storage_root";
  const static wasm::Name ext_clear_child_storage = "ext_clear_child_storage";
  const static wasm::Name ext_exists_child_storage = "ext_exists_child_storage";
  const static wasm::Name ext_get_allocated_child_storage =
      "ext_get_allocated_child_storage";
  const static wasm::Name ext_get_child_storage_into =
      "ext_get_child_storage_into";
  const static wasm::Name ext_kill_child_storage = "ext_kill_child_storage";
  const static wasm::Name ext_set_child_storage = "ext_set_child_storage";

  RuntimeExternalInterface::RuntimeExternalInterface(
      std::shared_ptr<extensions::Extension> extension,
      std::shared_ptr<WasmMemory> memory)
      : extension_(std::move(extension)), memory_(std::move(memory)) {}

  void RuntimeExternalInterface::init(wasm::Module &wasm,
                                      wasm::ModuleInstance &instance) {
    using wasm::Address;
    using wasm::ConstantExpressionRunner;
    using wasm::TrivialGlobalManager;

    memory_->resize(wasm.memory.initial * wasm::Memory::kPageSize);
    // apply memory segments
    for (auto &segment : wasm.memory.segments) {
      Address offset = (uint32_t)ConstantExpressionRunner<TrivialGlobalManager>(
                           instance.globals)
                           .visit(segment.offset)
                           .value.geti32();
      if (offset + segment.data.size()
          > wasm.memory.initial * wasm::Memory::kPageSize) {
        trap("invalid offset when initializing memory");
      }
      for (size_t i = 0; i != segment.data.size(); ++i) {
        memory_->store8(offset + i, segment.data[i]);
      }
    }

    table.resize(wasm.table.initial);
    for (auto &segment : wasm.table.segments) {
      Address offset = (uint32_t)ConstantExpressionRunner<TrivialGlobalManager>(
                           instance.globals)
                           .visit(segment.offset)
                           .value.geti32();
      if (offset + segment.data.size() > wasm.table.initial) {
        trap("invalid offset when initializing table");
      }
      for (size_t i = 0; i != segment.data.size(); ++i) {
        table[offset + i] = segment.data[i];
      }
    }
  }

  wasm::Literal RuntimeExternalInterface::callImport(
      wasm::Function *import, wasm::LiteralList &arguments) {
    if (import->module == env) {
      /// memory externals

      /// ext_malloc
      if (import->base == ext_malloc) {
        auto ptr = extension_->ext_malloc(arguments.at(0).geti32());
        return wasm::Literal(ptr);
      }
      /// ext_free
      if (import->base == ext_free) {
        extension_->ext_free(arguments.at(0).geti32());
        return wasm::Literal();
      }
      /// storage externals

      /// ext_clear_prefix
      if (import->base == ext_clear_prefix) {
        extension_->ext_clear_prefix(arguments.at(0).geti32(),
                                     arguments.at(1).geti32());
        return wasm::Literal();
      }
      /// ext_clear_storage
      if (import->base == ext_clear_storage) {
        extension_->ext_clear_storage(arguments.at(0).geti32(),
                                      arguments.at(1).geti32());
        return wasm::Literal();
      }
      /// ext_exists_storage
      if (import->base == ext_exists_storage) {
        auto storage_exists = extension_->ext_exists_storage(
            arguments.at(0).geti32(), arguments.at(1).geti32());
        return wasm::Literal(storage_exists);
      }
      /// ext_get_allocated_storage
      if (import->base == ext_get_allocated_storage) {
        auto ptr = extension_->ext_get_allocated_storage(
            arguments.at(0).geti32(), arguments.at(1).geti32(),
            arguments.at(2).geti32());
        return wasm::Literal(ptr);
      }
      /// ext_get_storage_into
      if (import->base == ext_get_storage_into) {
        auto res = extension_->ext_get_storage_into(
            arguments.at(0).geti32(), arguments.at(1).geti32(),
            arguments.at(2).geti32(), arguments.at(3).geti32(),
            arguments.at(4).geti32());
        return wasm::Literal(res);
      }
      /// ext_set_storage
      if (import->base == ext_set_storage) {
        extension_->ext_set_storage(
            arguments.at(0).geti32(), arguments.at(1).geti32(),
            arguments.at(2).geti32(), arguments.at(3).geti32());
        return wasm::Literal();
      }
      /// ext_blake2_256_enumerated_trie_root
      if (import->base == ext_blake2_256_enumerated_trie_root) {
        extension_->ext_blake2_256_enumerated_trie_root(
            arguments.at(0).geti32(), arguments.at(1).geti32(),
            arguments.at(2).geti32(), arguments.at(3).geti32());
        return wasm::Literal();
      }
      /// ext_storage_changes_root
      if (import->base == ext_storage_changes_root) {
        auto res = extension_->ext_storage_changes_root(
            arguments.at(0).geti32(), arguments.at(1).geti32(),
            arguments.at(2).geti32(), arguments.at(3).geti32());
        return wasm::Literal(res);
      }
      /// ext_storage_root
      if (import->base == ext_storage_root) {
        extension_->ext_storage_root(arguments.at(0).geti32());
        return wasm::Literal();
      }
      /// ext_child_storage_root
      if (import->base == ext_child_storage_root) {
        auto res = extension_->ext_child_storage_root(arguments.at(0).geti32(),
                                                      arguments.at(1).geti32(),
                                                      arguments.at(2).geti32());
        return wasm::Literal(res);
      }
      /// ext_clear_child_storage
      if (import->base == ext_clear_child_storage) {
        extension_->ext_clear_child_storage(
            arguments.at(0).geti32(), arguments.at(1).geti32(),
            arguments.at(2).geti32(), arguments.at(3).geti32());
        return wasm::Literal();
      }
      /// ext_exists_child_storage
      if (import->base == ext_exists_child_storage) {
        auto res = extension_->ext_exists_child_storage(
            arguments.at(0).geti32(), arguments.at(1).geti32(),
            arguments.at(2).geti32(), arguments.at(3).geti32());
        return wasm::Literal(res);
      }
      /// ext_get_allocated_child_storage
      if (import->base == ext_get_allocated_child_storage) {
        auto res = extension_->ext_get_allocated_child_storage(
            arguments.at(0).geti32(), arguments.at(1).geti32(),
            arguments.at(2).geti32(), arguments.at(3).geti32(),
            arguments.at(4).geti32());
        return wasm::Literal(res);
      }
      /// ext_get_child_storage_into
      if (import->base == ext_get_child_storage_into) {
        auto res = extension_->ext_get_child_storage_into(
            arguments.at(0).geti32(), arguments.at(1).geti32(),
            arguments.at(2).geti32(), arguments.at(3).geti32(),
            arguments.at(4).geti32(), arguments.at(5).geti32(),
            arguments.at(6).geti32());
        return wasm::Literal(res);
      }
      /// ext_kill_child_storage
      if (import->base == ext_kill_child_storage) {
        extension_->ext_kill_child_storage(arguments.at(0).geti32(),
                                           arguments.at(1).geti32());
        return wasm::Literal();
      }
      /// ext_set_child_storage
      if (import->base == ext_set_child_storage) {
        extension_->ext_set_child_storage(
            arguments.at(0).geti32(), arguments.at(1).geti32(),
            arguments.at(2).geti32(), arguments.at(3).geti32(),
            arguments.at(4).geti32(), arguments.at(5).geti32());
        return wasm::Literal();
      }
    }
    wasm::Fatal() << "callImport: unknown import: " << import->module.str << "."
                  << import->name.str;
  }

  /**
   * Load integers from provided address
   */
  int8_t RuntimeExternalInterface::load8s(wasm::Address addr) {
    return memory_->load8s(addr);
  }
  uint8_t RuntimeExternalInterface::load8u(wasm::Address addr) {
    return memory_->load8u(addr);
  }
  int16_t RuntimeExternalInterface::load16s(wasm::Address addr) {
    return memory_->load16s(addr);
  }
  uint16_t RuntimeExternalInterface::load16u(wasm::Address addr) {
    return memory_->load16u(addr);
  }
  int32_t RuntimeExternalInterface::load32s(wasm::Address addr) {
    return memory_->load32s(addr);
  }
  uint32_t RuntimeExternalInterface::load32u(wasm::Address addr) {
    return memory_->load32u(addr);
  }
  int64_t RuntimeExternalInterface::load64s(wasm::Address addr) {
    return memory_->load64s(addr);
  }
  uint64_t RuntimeExternalInterface::load64u(wasm::Address addr) {
    return memory_->load64u(addr);
  }
  std::array<uint8_t, 16> RuntimeExternalInterface::load128(
      wasm::Address addr) {
    return memory_->load128(addr);
  }

  /**
   * Store integers at given address of the wasm memory
   */
  void RuntimeExternalInterface::store8(wasm::Address addr, int8_t value) {
    memory_->store8(addr, value);
  }
  void RuntimeExternalInterface::store16(wasm::Address addr, int16_t value) {
    memory_->store16(addr, value);
  }
  void RuntimeExternalInterface::store32(wasm::Address addr, int32_t value) {
    memory_->store32(addr, value);
  }
  void RuntimeExternalInterface::store64(wasm::Address addr, int64_t value) {
    memory_->store64(addr, value);
  }
  void RuntimeExternalInterface::store128(
      wasm::Address addr, const std::array<uint8_t, 16> &value) {
    memory_->store128(addr, value);
  }

  void RuntimeExternalInterface::growMemory(wasm::Address,
                                            wasm::Address newSize) {
    memory_->resize(newSize);
  }

}  // namespace kagome::runtime

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "crypto/typed_key_storage/typed_key_storage_impl.hpp"
namespace kagome::crypto::storage {

  TypedKeyStorageImpl::EDKeys TypedKeyStorageImpl::getEd25519Keys(
      KeyTypeId key_type) {
    EDKeys keys;
    auto &map = ed_keys_[key_type];
    if (map.empty()) {
      return {};
    }
    keys.reserve(map.size());
    for (auto &k : map) {
      keys.emplace_back(k.first);
    }
    return keys;
  }

  TypedKeyStorageImpl::SRKeys TypedKeyStorageImpl::getSr25519Keys(
      KeyTypeId key_type) {
    SRKeys keys;
    auto &map = sr_keys_[key_type];
    if (map.empty()) {
      return {};
    }
    keys.reserve(map.size());
    for (auto &k : map) {
      keys.emplace_back(k.first);
    }
    return keys;
  }

  // these methods are implemented to enable other crypto api methods,
  // which depend on putting keys into storage
  // this implementation is a stub
  // probably final implementation will differ
  void TypedKeyStorageImpl::addEd25519KeyPair(KeyTypeId key_type,
                                         const ED25519Keypair &key_pair) {
    ed_keys_[key_type][key_pair.public_key] = key_pair.private_key;
  }

  void TypedKeyStorageImpl::addSr25519KeyPair(KeyTypeId key_type,
                                         const SR25519Keypair &key_pair) {
    sr_keys_[key_type][key_pair.public_key] = key_pair.secret_key;
  }

  boost::optional<ED25519Keypair> TypedKeyStorageImpl::findEd25519Keypair(
      KeyTypeId key_type, const ED25519PublicKey &pk) {
    auto &map = ed_keys_[key_type];
    if (map.empty()) {
      return boost::none;
    }
    return ED25519Keypair{pk, map[pk]};
  }

  boost::optional<SR25519Keypair> TypedKeyStorageImpl::findSr25519Keypair(
      KeyTypeId key_type, const SR25519PublicKey &pk) {
    auto &map = sr_keys_[key_type];
    if (map.empty()) {
      return boost::none;
    }
    return SR25519Keypair{map[pk], pk};
  }

}  // namespace kagome::crypto::storage

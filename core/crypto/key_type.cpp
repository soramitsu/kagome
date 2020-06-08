/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "crypto/key_type.hpp"

#include <boost/endian/arithmetic.hpp>

namespace kagome::crypto {

  outcome::result<KeyType> getKeyTypeById(KeyTypeId key_type_id) {
    static const std::map<KeyTypeId, KeyType> map = {
        {KeyTypeId::BABE, KeyType::fromString("babe").value()},
        {KeyTypeId::GRAN, KeyType::fromString("gran").value()},
        {KeyTypeId::ACCO, KeyType::fromString("acco").value()},
        {KeyTypeId::IMON, KeyType::fromString("imon").value()},
        {KeyTypeId::AUDI, KeyType::fromString("audi").value()}};

    if (map.count(key_type_id) > 0) {
      return map.at(key_type_id);
    }

    return KeyTypeError::UNSUPPORTED_KEY_TYPE_ID;
  }

  outcome::result<KeyTypeId> getKeyIdByType(const KeyType &key_type) {
    static const std::map<KeyType, KeyTypeId> map = {
        {KeyType::fromString("babe").value(), KeyTypeId::BABE},
        {KeyType::fromString("gran").value(), KeyTypeId::GRAN},
        {KeyType::fromString("acco").value(), KeyTypeId::ACCO},
        {KeyType::fromString("imon").value(), KeyTypeId::IMON},
        {KeyType::fromString("audi").value(), KeyTypeId::AUDI}};
    if (map.count(key_type) > 0) {
      return map.at(key_type);
    }

    return KeyTypeError::UNSUPPORTED_KEY_TYPE;
  }

  outcome::result<crypto::KeyTypeId> decodeKeyTypeId(KeyTypeRepr param) {
    constexpr unsigned size = sizeof(KeyTypeRepr);
    constexpr unsigned bits = size * 8u;
    crypto::KeyType key_type{};

    boost::endian::endian_buffer<boost::endian::order::big, KeyTypeRepr, bits>
        buf{};
    buf = param;  // cannot initialize, only assign
    for (size_t i = 0; i < size; ++i) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      key_type[i] = buf.data()[i];
    }

    return getKeyIdByType(key_type);
  }
}  // namespace kagome::crypto

OUTCOME_CPP_DEFINE_CATEGORY(kagome::crypto, KeyTypeError, e) {
  using Error = kagome::crypto::KeyTypeError;
  switch (e) {
    case Error::UNSUPPORTED_KEY_TYPE:
      return "key type is not supported";
    case Error::UNSUPPORTED_KEY_TYPE_ID:
      return "key type id is not supported";
  }

  return "Unknown KeyTypeError";
}
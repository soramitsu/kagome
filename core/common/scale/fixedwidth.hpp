/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_SCALE_FIXEDWIDTH_HPP
#define KAGOME_SCALE_FIXEDWIDTH_HPP

#include "common/result.hpp"
#include "common/scale/types.hpp"

namespace kagome::common::scale::fixedwidth {
  // 8 bit
  /**
   * @brief encodeInt8 encodes int8_t from host- to little- endian
   * @param value source value
   * @return encoded value
   */
  ByteArray encodeInt8(int8_t value);

  /**
   * @brief encodeUInt8 encodes uint8_t from host- to little- endian
   * @param value source value
   * @return byte array representing encoded value
   */
  ByteArray encodeUInt8(uint8_t value);

  // 16 bit
  /**
   * @brief encodeInt16 encodes int16_t from host- to little- endian
   * @param value source value
   * @return byte array representing encoded value
   */
  ByteArray encodeInt16(int16_t value);

  /**
   * @brief encodeUint16 encodes uint16_t from host- to little- endian
   * @param value source value
   * @return byte array representing encoded value
   */
  ByteArray encodeUint16(uint16_t value);

  // 32 bit
  /**
   * @brief encodeInt32 encodes int32_t from host- to little- endian
   * @param value source value
   * @return byte array representing encoded value
   */
  ByteArray encodeInt32(int32_t value);

  /**
   * @brief encodeUint32 encodes uint32_t from host- to little- endian
   * @param value source value
   * @return byte array representing encoded value
   */
  ByteArray encodeUint32(uint32_t value);

  // 64 bit
  /**
   * @brief encodeInt64 encodes int64_t from host- to little- endian
   * @param value source value
   * @return byte array representing encoded value
   */
  ByteArray encodeInt64(int64_t value);

  /**
   * @brief encodeInt64 encodes int64_t from host- to little- endian
   * @param value source value
   * @return byte array representing encoded value
   */
  ByteArray encodeUint64(uint64_t value);

  // 8 bit
  /**
   * @brief decodeInt8 decodes int8_t from host- to little- endian
   * @param stream source byte stream reference
   * @return optional decoded value
   */
  std::optional<int8_t> decodeInt8(Stream &stream);

  /**
   * @brief decodeUint8 decodes uint8_t from host- to little- endian
   * @param stream source byte stream reference
   * @return optional decoded value
   */
  std::optional<uint8_t> decodeUint8(Stream &stream);

  // 16 bit
  /**
   * @brief decodeInt16 decodes int16_t from host- to little- endian
   * @param stream source byte stream reference
   * @return optional decoded value
   */
  std::optional<int16_t> decodeInt16(Stream &stream);

  /**
   * @brief decodeUint16 decodes uint16_t from host- to little- endian
   * @param stream source byte stream reference
   * @return optional decoded value
   */
  std::optional<uint16_t> decodeUint16(Stream &stream);

  // 32 bit
  /**
   * @brief decodeInt32 decodes int32_t from host- to little- endian
   * @param stream source byte stream reference
   * @return optional decoded value
   */
  std::optional<int32_t> decodeInt32(Stream &stream);

  /**
   * @brief decodeUint32 decodes uint32_t from host- to little- endian
   * @param stream source byte stream reference
   * @return optional decoded value
   */
  std::optional<uint32_t> decodeUint32(Stream &stream);

  // 64 bit
  /**
   * @brief decodeInt64 decodes int64_t from host- to little- endian
   * @param stream source byte stream reference
   * @return optional decoded value
   */
  std::optional<int64_t> decodeInt64(Stream &stream);

  /**
   * @brief decodeUint64 decodes uint64_t from host- to little- endian
   * @param stream source byte stream reference
   * @return optional decoded value
   */
  std::optional<uint64_t> decodeUint64(Stream &stream);
}  // namespace kagome::common::scale::fixedwidth

#endif  // KAGOME_SCALE_FIXEDWIDTH_HPP

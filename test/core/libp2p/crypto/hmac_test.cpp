/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "libp2p/crypto/hmac_provider/hmac_provider.hpp"

#include <gtest/gtest.h>
#include <outcome/outcome.hpp>
#include "common/buffer.hpp"
#include "libp2p/crypto/common.hpp"
#include "libp2p/crypto/error.hpp"

using kagome::common::Buffer;
using namespace libp2p::crypto;

class HmacTest : public testing::Test {
 protected:
  void SetUp() override {
    // use the same message for all tests
    message.put("The fly got to the jam that's all the poem");
  }

  /// hash provider
  hmac::HmacProvider provider;
  /// message to be hashed
  Buffer message;

  Buffer sha1_key{0x55, 0xcd, 0x43, 0x3b, 0xe9, 0x56, 0x8e,
                  0xe7, 0x95, 0x25, 0xa0, 0x91, 0x9c, 0xf4,
                  0xb3, 0x1c, 0x28, 0x10, 0x8c, 0xee};  // 20 bytes

  Buffer sha256_key{0xa1, 0x99, 0x0a, 0xeb, 0x68, 0xef, 0xb1, 0xb5, 0x9d,
                    0x31, 0x65, 0x79, 0x5f, 0x63, 0x38, 0x96, 0x0a, 0xa7,
                    0x23, 0x8b, 0xa7, 0x47, 0x79, 0xea, 0x5d, 0xf3, 0xa4,
                    0x35, 0xfd, 0xbb, 0x8d, 0x4c};  // 32 bytes

  Buffer sha512_key{
      0xdd, 0x11, 0x4c, 0x73, 0x51, 0xb2, 0x18, 0x6a, 0xeb, 0xa2,  // 64 bytes
      0xd3, 0xfb, 0x4d, 0x96, 0x49, 0x6d, 0xa9, 0xe1, 0x68, 0x1a, 0xe6,
      0x27, 0x2d, 0xf5, 0x53, 0xa8, 0x23, 0x5a, 0x05, 0xe6, 0xf1, 0xae,
      0x66, 0xd5, 0xc4, 0xef, 0xa3, 0x2c, 0xdf, 0xbf, 0x1b, 0x0f, 0x3b,
      0x95, 0x42, 0xc1, 0x44, 0x44, 0xa5, 0x23, 0x85, 0x9c, 0xde, 0x43,
      0x73, 0x6c, 0x7b, 0x5b, 0x89, 0x98, 0x03, 0xd1, 0xa9, 0x6a};

  Buffer sha1_dgst{0x42, 0x98, 0x56, 0x01, 0xb3, 0xd6, 0x11,
                   0x25, 0xe0, 0x2b, 0xcc, 0xa5, 0xa4, 0xdc,
                   0xb9, 0xe3, 0x76, 0x3b, 0xc9, 0x42};  // 20 bytes

  Buffer sha256_dgst{0xbd, 0xb5, 0xa9, 0xc8, 0xf3, 0xe0, 0x8f, 0xdb, 0x8c,
                     0x0e, 0xe7, 0x18, 0x9d, 0x76, 0xfd, 0x6c, 0x48, 0x7d,
                     0x57, 0x89, 0xe0, 0x14, 0x18, 0x50, 0xbc, 0xc9, 0x45,
                     0x55, 0x84, 0x88, 0x09, 0x7a};  // 32 bytes

  Buffer sha512_dgst{0x0f, 0x5b, 0xf6, 0xaf, 0x49, 0x43, 0xb3, 0x5b, 0x76, 0xd7,
                     0xd8, 0x97, 0x14, 0xb6, 0x81, 0x90, 0x0e, 0x03, 0x26, 0x2e,
                     0x99, 0x7f, 0x25, 0x19, 0xbe, 0xfd, 0x7b, 0x1c, 0xb0, 0xcb,
                     0x56, 0xe8, 0xe6, 0x48, 0xfa, 0x29, 0x7b, 0xa1, 0x85, 0x53,
                     0x82, 0x12, 0x32, 0x40, 0xf6, 0xcd, 0xed, 0x44, 0x17, 0x4b,
                     0x85, 0x1b, 0x94, 0x66, 0x5b, 0x9a, 0x56, 0xb2, 0x49, 0xd4,
                     0xd8, 0x8d, 0xeb, 0x63};  // 64 bytes
};

/**
 * @given 20 bytes key, default message
 * @when hmacDigest is applied with hash = kSha1
 * @then obtained digest matches predefined one
 */
TEST_F(HmacTest, HashSha1Success) {
  auto &&digest =
      provider.calculateDigest(common::HashType::kSHA1, sha1_key, message);
  ASSERT_TRUE(digest);
  ASSERT_EQ(digest.value().size(), 20);
  ASSERT_EQ(digest.value(), sha1_dgst);
}

/**
 * @given 32 bytes key, default message
 * @when hmacDigest is applied with hash = kSha256
 * @then obtained digest matches predefined one
 */
TEST_F(HmacTest, HashSha256Success) {
  auto &&digest =
      provider.calculateDigest(common::HashType::kSHA256, sha256_key, message);
  ASSERT_TRUE(digest);
  ASSERT_EQ(digest.value().size(), 32);
  ASSERT_EQ(digest.value(), sha256_dgst);
}

/**
 * @given 64 bytes key, default message
 * @when hmacDigest is applied with hash = kSha512
 * @then obtained digest matches predefined one
 */
TEST_F(HmacTest, HashSha512Success) {
  auto &&digest =
      provider.calculateDigest(common::HashType::kSHA512, sha512_key, message);
  ASSERT_TRUE(digest);
  ASSERT_EQ(digest.value().size(), 64);
  ASSERT_EQ(digest.value(), sha512_dgst);
}

/**
 * @given invalid HashType value
 * @when hmacDigest is applied with given value
 * @then error is returned
 */
TEST_F(HmacTest, HashInvalidFails) {
  auto &&digest = provider.calculateDigest(static_cast<common::HashType>(15),
                                           sha1_key, message);
  ASSERT_FALSE(digest);
  ASSERT_EQ(digest.error().value(),
            static_cast<int>(HmacProviderError::UNSUPPORTED_HASH_METHOD));
}

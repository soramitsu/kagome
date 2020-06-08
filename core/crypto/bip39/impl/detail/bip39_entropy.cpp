/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "crypto/bip39/impl/detail/bip39_entropy.hpp"

#include "crypto/sha/sha256.hpp"

OUTCOME_CPP_DEFINE_CATEGORY(kagome::crypto::bip39, Bip39EntropyError, error) {
  using E = kagome::crypto::bip39::Bip39EntropyError;
  switch (error) {
    case E::WRONG_WORDS_COUNT:
      return "invalid or unsupported words count";
    case E::STORAGE_NOT_COMPLETE:
      return "cannot get info from storage while it is still not complete";
    case E::STORAGE_IS_FULL:
      return "cannot put more data into storage, it is full";
  }

  return "unknown Bip39EntropyError error";
}

namespace kagome::crypto::bip39 {
  outcome::result<Bip39Entropy> Bip39Entropy::create(size_t words_count) {
    switch (words_count) {
      case 12:
        return Bip39Entropy(132, 4);
      case 15:
        return Bip39Entropy(165, 5);
      case 18:
        return Bip39Entropy(198, 6);
      case 21:
        return Bip39Entropy(231, 7);
      case 24:
        return Bip39Entropy(264, 8);
      default:
        break;
    }
    return Bip39EntropyError::WRONG_WORDS_COUNT;
  }

  outcome::result<std::vector<uint8_t>> Bip39Entropy::getEntropy() const {
    if (bits_.size() != total_bits_count_) {
      return Bip39EntropyError::STORAGE_NOT_COMPLETE;
    }

    // convert data
    size_t bytes_count = (total_bits_count_ - checksum_bits_count_) / 8;
    std::vector<uint8_t> res;
    res.reserve(bytes_count);
    auto it = bits_.begin();
    for (size_t i = 0; i < bytes_count; ++i) {
      uint8_t byte = 0;
      for (size_t j = 0; j < 8u; ++j) {
        byte <<= 1;
        byte += *it;
      }

      res.push_back(byte);
    }

    return res;
  }

  outcome::result<uint8_t> Bip39Entropy::getChecksum() const {
    if (bits_.size() != total_bits_count_) {
      return Bip39EntropyError::STORAGE_NOT_COMPLETE;
    }
    uint8_t mask = 0xFFu;
    mask >>= 8 - checksum_bits_count_;
    auto checksum = *bits_.rbegin() >> mask;

    return checksum;
  }

  outcome::result<void> Bip39Entropy::append(const EntropyToken &value) {
    if (bits_.size() + value.size() > total_bits_count_) {
      return Bip39EntropyError::STORAGE_IS_FULL;
    }
    for (size_t i = 0; i < value.size(); ++i) {
      uint8_t v = value[i] ? 1 : 0;
      bits_.push_back(v);
    }
    return outcome::success();
  }

  outcome::result<uint8_t> Bip39Entropy::calculateChecksum() const {
    OUTCOME_TRY(entropy, getEntropy());
    auto hash = sha256(entropy);
    return hash[0];
  }

}  // namespace kagome::crypto::bip39

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_PRIMITIVES_DIGEST
#define KAGOME_CORE_PRIMITIVES_DIGEST

#include <boost/variant.hpp>

#include "common/buffer.hpp"
#include "common/unused.hpp"
#include "primitives/scheduled_change.hpp"
#include "scale/scale.hpp"

namespace kagome::primitives {
  // from
  // https://github.com/paritytech/substrate/blob/39094c764a0bc12134d2a2ed8ab494a9ebfeba88/core/sr-primitives/src/generic/digest.rs#L77-L102

  /// Consensus engine unique ID.
  using ConsensusEngineId = common::Blob<4>;

  inline const auto kBabeEngineId =
      ConsensusEngineId::fromString("BABE").value();

  inline const auto kGrandpaEngineId =
      ConsensusEngineId::fromString("FRNK").value();

  /// System digest item that contains the root of changes trie at given
  /// block. It is created for every block iff runtime supports changes
  /// trie creation.
  struct ChangesTrieRoot : public common::Hash256 {};

  namespace detail {
    struct DigestItemCommon {
      ConsensusEngineId consensus_engine_id;
      common::Buffer data;

      bool operator==(const DigestItemCommon &rhs) const {
        return consensus_engine_id == rhs.consensus_engine_id
               and data == rhs.data;
      }

      bool operator!=(const DigestItemCommon &rhs) const {
        return !operator==(rhs);
      }
    };
  }  // namespace detail

  /// A pre-runtime digest.
  ///
  /// These are messages from the consensus engine to the runtime, although
  /// the consensus engine can (and should) read them itself to avoid
  /// code and state duplication. It is erroneous for a runtime to produce
  /// these, but this is not (yet) checked.
  struct PreRuntime : public detail::DigestItemCommon {};

  /// A message from the runtime to the consensus engine. This should *never*
  /// be generated by the native code of any consensus engine, but this is not
  /// checked (yet).
  struct Consensus : public detail::DigestItemCommon {
    using BabeDigest =
        /// Note: order of types in variant matters
        boost::variant<Unused<0>,
                       NextEpochData,    // 1: (Auth C; R)
                       OnDisabled,       // 2: Auth ID
                       NextConfigData>;  // 3: c, S2nd

    using GrandpaDigest =
        /// Note: order of types in variant matters
        boost::variant<Unused<0>,
                       ScheduledChange,  // 1: (Auth C; N delay)
                       ForcedChange,     // 2: (Auth C; N delay)
                       OnDisabled,       // 3: Auth ID
                       Pause,            // 4: N delay
                       Resume>;          // 5: N delay

    Consensus() = default;

    // Note: this ctor is needed only for tests
    template <class A>
    Consensus(const A &a) {
      // clang-format off
      if constexpr (std::is_same_v<A, NextEpochData>
                 or std::is_same_v<A, NextConfigData>) {
        consensus_engine_id = primitives::kBabeEngineId;
        data = common::Buffer(scale::encode(BabeDigest(a)).value());
      } else if constexpr (std::is_same_v<A, ScheduledChange>
                        or std::is_same_v<A, ForcedChange>
                        or std::is_same_v<A, OnDisabled>
                        or std::is_same_v<A, Pause>
                        or std::is_same_v<A, Resume>) {
        consensus_engine_id = primitives::kGrandpaEngineId;
        data = common::Buffer(scale::encode(GrandpaDigest(a)).value());
      } else {
        BOOST_UNREACHABLE_RETURN();
      }
      // clang-format on
    }

    outcome::result<void> decode() const {
      if (consensus_engine_id == primitives::kBabeEngineId) {
        if (not decoded) {
          OUTCOME_TRY(payload, scale::decode<BabeDigest>(data));
          digest = std::move(payload);
          decoded = true;
        }
      } else {
        if (not decoded) {
          OUTCOME_TRY(payload, scale::decode<GrandpaDigest>(data));
          digest = std::move(payload);
          decoded = true;
        }
      }
      return outcome::success();
    }

    const BabeDigest &asBabeDigest() const {
      BOOST_ASSERT(consensus_engine_id == primitives::kBabeEngineId);
      return boost::relaxed_get<BabeDigest>(digest);
    }

    const GrandpaDigest &asGrandpaDigest() const {
      BOOST_ASSERT(consensus_engine_id == primitives::kGrandpaEngineId);
      return boost::relaxed_get<GrandpaDigest>(digest);
    }

   private:
    mutable boost::variant<BabeDigest, GrandpaDigest> digest{};
    mutable bool decoded = false;
  };

  /// Put a Seal on it.
  /// This is only used by native code, and is never seen by runtimes.
  struct Seal : public detail::DigestItemCommon {};

  template <class Stream,
            typename = std::enable_if_t<Stream::is_encoder_stream>>
  Stream &operator<<(Stream &s, const detail::DigestItemCommon &dic) {
    return s << dic.consensus_engine_id << dic.data;
  }

  template <class Stream,
            typename = std::enable_if_t<Stream::is_decoder_stream>>
  Stream &operator>>(Stream &s, detail::DigestItemCommon &dic) {
    return s >> dic.consensus_engine_id >> dic.data;
  }

  /// Digest item that is able to encode/decode 'system' digest items and
  /// provide opaque access to other items.
  /// Note: order of types in variant matters. Should match type ids from here:
  /// https://github.com/paritytech/substrate/blob/39094c764a0bc12134d2a2ed8ab494a9ebfeba88/core/sr-primitives/src/generic/digest.rs#L155-L161
  using DigestItem = boost::variant<Unused<0>,        // 0
                                    Unused<1>,        // 1
                                    ChangesTrieRoot,  // 2
                                    Unused<3>,        // 3
                                    Consensus,        // 4
                                    Seal,             // 5
                                    PreRuntime>;      // 6

  /**
   * Digest is an implementation- and usage-defined entity, for example,
   * information, needed to verify the block
   */
  using Digest = std::vector<DigestItem>;
}  // namespace kagome::primitives

#endif  // KAGOME_CORE_PRIMITIVES_DIGEST

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_STREAM_MOCK_HPP
#define KAGOME_STREAM_MOCK_HPP

#include <gmock/gmock.h>
#include "libp2p/connection/stream.hpp"

namespace libp2p::connection {
  class StreamMock : public Stream {
   public:
    StreamMock() = default;
    StreamMock(uint8_t id) : stream_id{id} {}

    /// this field here is for easier testing
    uint8_t stream_id = 137;

    ~StreamMock() override = default;

    MOCK_CONST_METHOD0(isClosed, bool(void));

    MOCK_METHOD0(close, outcome::result<void>(void));

    MOCK_METHOD2(read, void(gsl::span<uint8_t>, Reader::ReadCallbackFunc));
    MOCK_METHOD2(readSome, void(gsl::span<uint8_t>, Reader::ReadCallbackFunc));
    MOCK_METHOD2(write, void(gsl::span<const uint8_t>, Writer::WriteCallbackFunc));
    MOCK_METHOD2(writeSome, void(gsl::span<const uint8_t>, Writer::WriteCallbackFunc));

    MOCK_METHOD0(reset, void(void));

    MOCK_CONST_METHOD0(isClosedForRead, bool(void));

    MOCK_CONST_METHOD0(isClosedForWrite, bool(void));
  };
}  // namespace libp2p::connection

#endif  // KAGOME_STREAM_MOCK_HPP

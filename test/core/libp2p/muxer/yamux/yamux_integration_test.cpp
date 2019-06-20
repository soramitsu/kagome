/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "libp2p/connection/yamux/yamuxed_connection.hpp"

#include <gtest/gtest.h>
#include "core/libp2p/transport_fixture/transport_fixture.hpp"
#include "libp2p/connection/yamux/yamux_frame.hpp"
#include "libp2p/connection/yamux/yamux_stream.hpp"
#include "libp2p/multi/multiaddress.hpp"
#include "libp2p/muxer/yamux.hpp"
#include "libp2p/security/plaintext.hpp"
#include "testutil/outcome.hpp"

using namespace libp2p::connection;
using namespace libp2p::transport;
using namespace kagome::common;
using namespace libp2p::multi;
using namespace libp2p::basic;
using namespace libp2p::security;
using namespace libp2p::muxer;

using std::chrono_literals::operator""ms;

class YamuxIntegrationTest : public libp2p::testing::TransportFixture {
 public:
  void SetUp() override {
    libp2p::testing::TransportFixture::SetUp();

    this->server([this](auto &&conn_res) mutable {
      EXPECT_OUTCOME_TRUE(conn, conn_res)
      EXPECT_OUTCOME_TRUE(sec_conn,
                          security_adaptor_->secureInbound(std::move(conn)))
      EXPECT_OUTCOME_TRUE(mux_conn,
                          muxer_adaptor_->muxConnection(
                              std::move(sec_conn),
                              [this](auto &&stream_res) {
                                EXPECT_OUTCOME_TRUE(stream, stream_res)
                                accepted_streams_.push_back(std::move(stream));
                              },
                              YamuxConfig{}))
      yamuxed_connection_ =
          std::move(std::static_pointer_cast<YamuxedConnection>(mux_conn));
      yamuxed_connection_->start();
      return outcome::success();
    });
  }

  /**
   * Get a pointer to a new stream
   * @param expected_stream_id - id, which is expected to be assigned to that
   * stream
   * @return pointer to the stream
   */
  std::shared_ptr<Stream> getNewStream(
      std::shared_ptr<ReadWriteCloser> conn,
      YamuxedConnection::StreamId expected_stream_id =
          kDefaulExpectedStreamId) {
    auto new_stream_msg = newStreamMsg(expected_stream_id);
    auto rcvd_msg = std::make_shared<Buffer>(new_stream_msg.size(), 0);

    std::shared_ptr<Stream> stream;
    yamuxed_connection_->newStream([c = std::move(conn), &stream,
                                    &new_stream_msg,
                                    rcvd_msg](auto &&stream_res) mutable {
      ASSERT_TRUE(stream_res);
      stream = std::move(stream_res.value());

      c->read(*rcvd_msg, new_stream_msg.size(),
              [&new_stream_msg, rcvd_msg](auto &&res) {
                ASSERT_TRUE(res);
                ASSERT_EQ(*rcvd_msg, new_stream_msg);
              });
    });

    context_.run_for(10ms);

    return stream;
  }

  std::shared_ptr<YamuxedConnection> yamuxed_connection_;
  std::vector<std::shared_ptr<Stream>> accepted_streams_;

  std::shared_ptr<SecurityAdaptor> security_adaptor_ =
      std::make_shared<Plaintext>();
  std::shared_ptr<MuxerAdaptor> muxer_adaptor_ = std::make_shared<Yamux>();

  bool client_finished = false;

  static constexpr YamuxedConnection::StreamId kDefaulExpectedStreamId = 2;
};

/**
 * @given initialized Yamux
 * @when creating a new stream from the client's side
 * @then stream is created @and corresponding ack message is sent to the client
 */
TEST_F(YamuxIntegrationTest, StreamFromClient) {
  constexpr YamuxedConnection::StreamId created_stream_id = 1;

  auto new_stream_ack_msg_rcv =
      std::make_shared<Buffer>(YamuxFrame::kHeaderLength, 0);
  auto new_stream_msg = newStreamMsg(created_stream_id);

  this->client([this, created_stream_id, &new_stream_msg,
                new_stream_ack_msg_rcv](auto &&conn_res) {
    EXPECT_OUTCOME_TRUE(conn, conn_res)
    conn->write(
        new_stream_msg, new_stream_msg.size(),
        [this, conn, created_stream_id, new_stream_ack_msg_rcv](auto &&res) {
          ASSERT_TRUE(res);
          conn->read(*new_stream_ack_msg_rcv, YamuxFrame::kHeaderLength,
                     [this, created_stream_id, new_stream_ack_msg_rcv,
                      conn](auto &&res) {
                       ASSERT_TRUE(res);

                       // check a new stream is in our 'accepted_streams'
                       ASSERT_EQ(accepted_streams_.size(), 1);

                       // check our yamux has sent an ack message for that
                       // stream
                       auto parsed_ack_opt =
                           parseFrame(new_stream_ack_msg_rcv->toVector());
                       ASSERT_TRUE(parsed_ack_opt);
                       ASSERT_EQ(parsed_ack_opt->stream_id, created_stream_id);

                       client_finished = true;
                     });
        });
  });

  launchContext();
  ASSERT_TRUE(client_finished);
}
//
///**
// * @given initialized Yamux
// * @when creating a new stream from the server's side
// * @then stream is created @and corresponding new stream message is received
// by
// * the client
// */
// TEST_F(YamuxIntegrationTest, StreamFromServer) {
//  constexpr YamuxedConnection::StreamId expected_stream_id = 2;
//
//  auto stream_res = yamux_->newStream();
//  ASSERT_TRUE(stream_res);
//
//  auto stream = std::move(stream_res.value());
//  ASSERT_FALSE(stream->isClosedForRead());
//  ASSERT_FALSE(stream->isClosedForWrite());
//  ASSERT_FALSE(stream->isClosedEntirely());
//
//  auto expected_new_stream_msg = newStreamMsg(expected_stream_id);
//  auto new_stream_msg_buf =
//      std::make_shared<Buffer>(YamuxFrame::kHeaderLength, 0);
//  connection_->asyncRead(
//      boost::asio::buffer(new_stream_msg_buf->toVector()),
//      YamuxFrame::kHeaderLength,
//      [new_stream_msg_buf, &expected_new_stream_msg](auto &&ec, auto &&n) {
//        CHECK_IO_SUCCESS(ec, n, YamuxFrame::kHeaderLength)
//
//        ASSERT_EQ(*new_stream_msg_buf, expected_new_stream_msg);
//      });
//
//  launchContext();
//}
//
///**
// * @given initialized Yamux @and streams, multiplexed by that Yamux
// * @When writing to that stream
// * @then the operation is succesfully executed
// */
// TEST_F(YamuxIntegrationTest, StreamWrite) {
//  auto stream = getNewStream();
//
//  Buffer data{{0x12, 0x34, 0xAA}};
//  auto expected_data_msg = dataMsg(kDefaulExpectedStreamId, data);
//  auto received_data_msg =
//      std::make_shared<Buffer>(expected_data_msg.size(), 0);
//
//  stream->writeAsync(
//      data,
//      [this, &data, &expected_data_msg, received_data_msg](auto &&ec,
//                                                           auto &&n) {
//        CHECK_IO_SUCCESS(ec, n, data.size())
//
//        // check that our written data has achieved the destination
//        connection_->asyncRead(
//            boost::asio::buffer(received_data_msg->toVector()),
//            expected_data_msg.size(),
//            [&expected_data_msg, received_data_msg](auto &&ec, auto &&n) {
//              CHECK_IO_SUCCESS(ec, n, expected_data_msg.size())
//
//              ASSERT_EQ(*received_data_msg, expected_data_msg);
//            });
//      });
//
//  launchContext();
//}
//
///**
// * @given initialized Yamux @and streams, multiplexed by that Yamux
// * @when reading from that stream
// * @then the operation is successfully executed
// */
// TEST_F(YamuxIntegrationTest, StreamRead) {
//  auto stream = getNewStream();
//
//  Buffer data{{0x12, 0x34, 0xAA}};
//  auto written_data_msg = dataMsg(kDefaulExpectedStreamId, data);
//
//  connection_->asyncWrite(
//      boost::asio::buffer(written_data_msg.toVector()),
//      [&written_data_msg, &stream, &data](auto &&ec, auto &&n) mutable {
//        CHECK_IO_SUCCESS(ec, n, written_data_msg.size())
//
//        stream->readAsync([&data](Stream::NetworkMessageOutcome msg_res) {
//          ASSERT_TRUE(msg_res);
//          ASSERT_EQ(msg_res.value(), data);
//        });
//      });
//
//  launchContext();
//}
//
///**
// * @given initialized Yamux @and stream over it
// * @when closing that stream for writes
// * @then the stream is closed for writes @and corresponding message is
// received
// * on the other side
// */
// TEST_F(YamuxIntegrationTest, CloseForWrites) {
//  auto stream = getNewStream();
//
//  ASSERT_FALSE(stream->isClosedForWrite());
//  stream->close();
//  ASSERT_TRUE(stream->isClosedForWrite());
//
//  auto expected_close_stream_msg = closeStreamMsg(kDefaulExpectedStreamId);
//  auto close_stream_msg_rcv =
//      std::make_shared<Buffer>(YamuxFrame::kHeaderLength, 0);
//
//  connection_->asyncRead(
//      boost::asio::buffer(close_stream_msg_rcv->toVector()),
//      YamuxFrame::kHeaderLength,
//      [&expected_close_stream_msg, close_stream_msg_rcv](auto &&ec, auto &&n)
//      {
//        CHECK_IO_SUCCESS(ec, n, YamuxFrame::kHeaderLength)
//
//        ASSERT_EQ(*close_stream_msg_rcv, expected_close_stream_msg);
//      });
//
//  launchContext();
//}
//
///**
// * @given initialized Yamux @and stream over it
// * @when the other side sends a close message for that stream
// * @then the stream is closed for reads
// */
// TEST_F(YamuxIntegrationTest, CloseForReads) {
//  auto stream = getNewStream();
//
//  ASSERT_FALSE(stream->isClosedForRead());
//
//  auto sent_close_stream_msg = closeStreamMsg(kDefaulExpectedStreamId);
//
//  connection_->asyncWrite(boost::asio::buffer(sent_close_stream_msg.toVector()),
//                          [&sent_close_stream_msg](auto &&ec, auto &&n) {
//                            CHECK_IO_SUCCESS(ec, n,
//                                             sent_close_stream_msg.size())
//                          });
//
//  launchContext();
//  ASSERT_TRUE(stream->isClosedForRead());
//}
//
///**
// * @given initialized Yamux @and stream over it
// * @when close message is sent over the stream @and the other side responses
// * with a close message as well
// * @then the stream is closed entirely - removed from Yamux
// */
// TEST_F(YamuxIntegrationTest, CloseEntirely) {
//  auto stream = getNewStream();
//
//  ASSERT_FALSE(stream->isClosedForWrite());
//  stream->close();
//  ASSERT_TRUE(stream->isClosedForWrite());
//
//  auto expected_close_stream_msg = closeStreamMsg(kDefaulExpectedStreamId);
//  auto close_stream_msg_rcv =
//      std::make_shared<Buffer>(YamuxFrame::kHeaderLength, 0);
//
//  connection_->asyncRead(
//      boost::asio::buffer(close_stream_msg_rcv->toVector()),
//      YamuxFrame::kHeaderLength,
//      [this, &expected_close_stream_msg, close_stream_msg_rcv](auto &&ec,
//                                                               auto &&n) {
//        CHECK_IO_SUCCESS(ec, n, YamuxFrame::kHeaderLength)
//        ASSERT_EQ(*close_stream_msg_rcv, expected_close_stream_msg);
//
//        connection_->asyncWrite(
//            boost::asio::buffer(expected_close_stream_msg.toVector()),
//            [&expected_close_stream_msg](auto &&ec, auto &&n) {
//              CHECK_IO_SUCCESS(ec, n, expected_close_stream_msg.size())
//            });
//      });
//
//  launchContext();
//  ASSERT_TRUE(stream->isClosedEntirely());
//}
//
///**
// * @given initialized Yamux
// * @when a ping message arrives to Yamux
// * @then Yamux sends a ping response back
// */
// TEST_F(YamuxIntegrationTest, Ping) {
//  static constexpr uint32_t ping_value = 42;
//
//  auto ping_in_msg = pingOutMsg(ping_value);
//  auto ping_out_msg = pingResponseMsg(ping_value);
//  auto received_ping = std::make_shared<Buffer>(ping_out_msg.size(), 0);
//
//  connection_->asyncWrite(boost::asio::buffer(ping_in_msg.toVector()),
//                          [&ping_in_msg](auto &&ec, auto &&n) {
//                            CHECK_IO_SUCCESS(ec, n, ping_in_msg.size())
//                          });
//  connection_->asyncRead(boost::asio::buffer(received_ping->toVector()),
//                         ping_out_msg.size(),
//                         [&ping_out_msg, received_ping](auto &&ec, auto &&n) {
//                           CHECK_IO_SUCCESS(ec, n, ping_out_msg.size())
//                           ASSERT_EQ(*received_ping, ping_out_msg);
//                         });
//  launchContext();
//}
//
///**
// * @given initialized Yamux @and stream over it
// * @when a reset message is sent over that stream
// * @then the stream is closed entirely - removed from Yamux @and the other
// side
// * receives a corresponding message
// */
// TEST_F(YamuxIntegrationTest, Reset) {
//  auto stream = getNewStream();
//
//  ASSERT_FALSE(stream->isClosedEntirely());
//  stream->reset();
//  ASSERT_TRUE(stream->isClosedEntirely());
//
//  auto expected_reset_msg = resetStreamMsg(kDefaulExpectedStreamId);
//  auto rcvd_msg = std::make_shared<Buffer>(expected_reset_msg.size(), 0);
//
//  connection_->asyncRead(boost::asio::buffer(rcvd_msg->toVector()),
//                         expected_reset_msg.size(),
//                         [&expected_reset_msg, rcvd_msg](auto &&ec, auto &&n)
//                         {
//                           CHECK_IO_SUCCESS(ec, n, rcvd_msg->size())
//                           ASSERT_EQ(*rcvd_msg, expected_reset_msg);
//                         });
//
//  launchContext();
//}
//
///**
// * @given initialized Yamux
// * @when Yamux is closed
// * @then an underlying connection is closed @and the other side receives a
// * corresponding message
// */
// TEST_F(YamuxIntegrationTest, GoAway) {
//  yamux_->close();
//  ASSERT_TRUE(yamux_->isClosed());
//}

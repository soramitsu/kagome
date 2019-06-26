/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "libp2p/muxer/yamux/yamuxed_connection.hpp"

#include <gtest/gtest.h>
#include "common/buffer.hpp"
#include "libp2p/connection/stream.hpp"
#include "libp2p/muxer/yamux.hpp"
#include "libp2p/security/plaintext.hpp"
#include "libp2p/transport/tcp.hpp"
#include "mock/libp2p/transport/upgrader_mock.hpp"
#include "testutil/literals.hpp"
#include "testutil/outcome.hpp"

using namespace libp2p::connection;
using namespace libp2p::transport;
using namespace kagome::common;
using namespace libp2p::multi;
using namespace libp2p::basic;
using namespace libp2p::security;
using namespace libp2p::muxer;

using std::chrono_literals::operator""ms;

const Buffer kPingBytes = Buffer{}.put("PING");
const Buffer kPongBytes = Buffer{}.put("PONG");

struct ServerStream : std::enable_shared_from_this<ServerStream> {
  explicit ServerStream(std::shared_ptr<Stream> s)
      : stream{std::move(s)}, read_buffer(kPingBytes.size(), 0) {}

  std::shared_ptr<Stream> stream;
  Buffer read_buffer;

  void doRead() {
    if (stream->isClosedForRead()) {
      return;
    }
    stream->read(read_buffer, read_buffer.size(),
                 [self = shared_from_this()](auto &&res) {
                   ASSERT_TRUE(res);
                   self->readCompleted();
                 });
  }

  void readCompleted() {
    ASSERT_EQ(read_buffer, kPingBytes) << "expected to received a PING message";
    doWrite();
  }

  void doWrite() {
    if (stream->isClosedForWrite()) {
      return;
    }
    stream->write(kPongBytes, kPongBytes.size(),
                  [self = shared_from_this()](auto &&res) {
                    ASSERT_TRUE(res);
                    self->doRead();
                  });
  }
};

class YamuxAcceptanceTest : public testing::Test {
 public:
  void SetUp() override {
    transport_ = std::make_shared<TcpTransport>(
        context_, std::make_shared<DefaultUpgrader>());
    ASSERT_TRUE(transport_) << "cannot create transport";

    auto ma = "/ip4/127.0.0.1/tcp/40009"_multiaddr;
    multiaddress_ = std::make_shared<Multiaddress>(std::move(ma));

    // set up a Yamux server - that lambda is to be called, when a new
    // connection is received
    transport_listener_ =
        transport_->createListener([this](auto &&conn_res) mutable {
          EXPECT_OUTCOME_TRUE(conn, conn_res)
          server_connection_ = std::move(conn);
          server_connection_->start();
          server_connection_->onStream([this](auto &&stream) {
            // wrap each received stream into a server structure and start
            // reading
            ASSERT_TRUE(stream);
            auto server = std::make_shared<ServerStream>(
                std::forward<decltype(stream)>(stream));
            server->doRead();
            server_streams_.push_back(std::move(server));
          });
          return outcome::success();
        });
    ASSERT_TRUE(transport_listener_->listen(*multiaddress_))
        << "is port 40009 busy?";
  }

  boost::asio::io_context context_;

  std::shared_ptr<libp2p::transport::Transport> transport_;
  std::shared_ptr<libp2p::transport::TransportListener> transport_listener_;
  std::shared_ptr<libp2p::multi::Multiaddress> multiaddress_;

  std::shared_ptr<SecurityAdaptor> security_adaptor_ =
      std::make_shared<Plaintext>();

  std::shared_ptr<MuxerAdaptor> server_muxer_adaptor_ =
      std::make_shared<Yamux>();
  std::shared_ptr<MuxerAdaptor> client_muxer_adaptor_ =
      std::make_shared<Yamux>();

  std::shared_ptr<CapableConnection> server_connection_;
  std::shared_ptr<CapableConnection> client_connection_;

  std::vector<std::shared_ptr<ServerStream>> server_streams_;
};

/**
 * @given Yamuxed server, which is setup to write 'PONG' for any received 'PING'
 * message @and Yamuxed client, connected to that server
 * @when the client sets up a listener on that server @and writes 'PING'
 * @then the 'PONG' message is received by the client
 */
TEST_F(YamuxAcceptanceTest, PingPong) {
  transport_->dial(*multiaddress_, [this](auto &&conn_res) {
    EXPECT_OUTCOME_TRUE(conn, conn_res)
    client_connection_ = std::move(conn);
    client_connection_->start();
    return outcome::success();
  });

  // let both client and server be created
  context_.run_for(100ms);
  ASSERT_TRUE(server_connection_);
  ASSERT_TRUE(client_connection_);

  auto stream_read = false, stream_wrote = false;
  Buffer stream_read_buffer(kPongBytes.size(), 0);
  client_connection_->newStream([&stream_read_buffer, &stream_read,
                                 &stream_wrote](auto &&stream_res) mutable {
    EXPECT_OUTCOME_TRUE(stream, stream_res)

    // proof our streams have parallelism: set up both read and write on the
    // stream and make sure they are successfully executed
    stream->read(stream_read_buffer, stream_read_buffer.size(),
                 [&stream_read_buffer, &stream_read](auto &&res) {
                   ASSERT_EQ(stream_read_buffer, kPongBytes);
                   stream_read = true;
                 });
    stream->write(kPingBytes, kPingBytes.size(), [&stream_wrote](auto &&res) {
      ASSERT_TRUE(res);
      stream_wrote = true;
    });
  });

  // let the streams make their jobs
  context_.run_for(100ms);

  ASSERT_TRUE(stream_read);
  ASSERT_TRUE(stream_wrote);
}

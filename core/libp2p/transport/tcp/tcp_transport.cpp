/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "libp2p/transport/tcp/tcp_transport.hpp"

#include <boost/lexical_cast.hpp>
#include <functional>

#include "libp2p/transport/tcp/tcp_connection.hpp"
#include "libp2p/transport/tcp/tcp_listener.hpp"

using boost::asio::ip::address;
using boost::asio::ip::make_address;
using libp2p::multi::Multiaddress;
using namespace boost;            // NOLINT
using namespace boost::asio::ip;  // NOLINT

namespace libp2p::transport {

  outcome::result<std::shared_ptr<Connection>> TcpTransport::dial(
      const multi::Multiaddress &address) {
    OUTCOME_TRY(addr,
                address.getFirstValueForProtocol(Multiaddress::Protocol::kIp4));

    OUTCOME_TRY(port,
                address.getFirstValueForProtocol(Multiaddress::Protocol::kTcp));

    try {
      tcp::resolver resolver(context_);
      auto iterator = resolver.resolve(addr, port);
      tcp::socket socket(context_);
      boost::asio::connect(socket, iterator);
      return std::make_shared<TcpConnection>(std::move(socket));
    } catch (const boost::system::system_error &e) {
      return e.code();
    }
  }

  std::shared_ptr<TransportListener> TcpTransport::createListener(
      TransportListener::HandlerFunc handler) {
    auto m = std::make_shared<TcpListener>(context_, std::move(handler));
    listeners_.insert(m);
    return m;
  }

  std::error_code TcpTransport::close() {
    for (auto &&l : listeners_) {
      OUTCOME_TRY_EC(l->close());
    }

    listeners_.clear();
    return {};  // success
  }

  bool TcpTransport::isClosed() const {
    return listeners_.empty()
        || std::all_of(listeners_.begin(), listeners_.end(),
                       [](auto &&l) { return l->isClosed(); });
  }

  TcpTransport::TcpTransport(boost::asio::io_context &context)
      : context_(context) {}
}  // namespace libp2p::transport

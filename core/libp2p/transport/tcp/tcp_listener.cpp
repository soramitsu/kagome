/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "libp2p/transport/tcp/tcp_listener.hpp"
#include "libp2p/multi/multiaddress.hpp"
#include "libp2p/transport/tcp/tcp_connection.hpp"

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

using boost::asio::ip::tcp;
using boost::asio::ip::address;
using boost::asio::ip::make_address;
using libp2p::multi::Multiaddress;

namespace libp2p::transport {

  outcome::result<void> TcpListener::listen(const Multiaddress &address) {
    // TODO(warchant): there must be a better way to extract correct values
    OUTCOME_TRY(addr,
                address.getFirstValueForProtocol<boost::asio::ip::address>(
                    Multiaddress::Protocol::kIp4, [](const std::string &val) {
                      return make_address(val);
                    }));

    OUTCOME_TRY(port,
                address.getFirstValueForProtocol<int>(
                    Multiaddress::Protocol::kTcp, [](const std::string &val) {
                      return boost::lexical_cast<int>(val);
                    }));

    tcp::endpoint endpoint(addr, port);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    // TODO(@warchant): maybe we need to store not the whole multiaddress, but
    // only ip4/tcp part?
    listening_on_.push_back(address);
    signal_start_listening_(address);

    // start async recv loop
    doAccept();

    return outcome::success();
  }

  std::error_code TcpListener::close() noexcept {
    boost::system::error_code ec;
    try {
      // TODO(warchant): should we execute signal_error_ in case of error?
      acceptor_.close(ec);
      if (ec) {
        signal_error_(ec);
        return ec;
      }

      listening_on_.clear();

      signal_close_();
    } catch (const boost::system::system_error &e) {
      signal_error_(e.code());
      return e.code();
    }

    return ec;
  }

  const std::vector<multi::Multiaddress> &TcpListener::getAddresses() const {
    return listening_on_;
  }

  TcpListener::TcpListener(boost::asio::io_context &io_context,
                           HandlerFunc handler)
      : context_(io_context),
        acceptor_(io_context),
        handler_(std::move(handler)) {}

  void TcpListener::doAccept() {
    // async accept loop
    if (acceptor_.is_open()) {
      boost::asio::ip::tcp::socket socket(context_);
      auto session = std::make_shared<TcpConnection>(std::move(socket));
      acceptor_.async_accept(
          session->socket(),
          [s = std::move(session), this](boost::system::error_code ec) {
            if (!ec) {
              try {
                handler_(s);
                signal_new_connection_(s);
              } catch (const std::exception &e) {
                // TODO(warchant): how to cast exception to std::error_code?
                // define policy for exception handling
                std::cerr << e.what() << '\n';
              }
            } else {
              signal_error_(ec);
            }

            doAccept();
          });
    }
  }

  TcpListener::~TcpListener() {
    if (!isClosed()) {
      close();
    }
  }

  bool TcpListener::isClosed() const noexcept {
    return !acceptor_.is_open();
  }

}  // namespace libp2p::transport

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "api/transport/impl/http_session.hpp"

#include <iostream>

#include <boost/config.hpp>
#include <outcome/outcome.hpp>

namespace kagome::api {

  HttpSession::HttpSession(Socket socket, Configuration config)
      : config_{config}, stream_(std::move(socket)) {
    connectOnError([](auto ec, auto &&message) {
      std::cerr << "http session error " << ec << ": " << message << " "
                << outcome::failure(ec).error().message() << std::endl;
    });
    connectOnResponse([this](auto &&message) {
      sendResponse(message);
    });
  }

  void HttpSession::start() {
    acyncRead();
  }

  void HttpSession::stop() {
    boost::system::error_code ec;
    stream_.socket().shutdown(Socket::shutdown_both, ec);
    boost::ignore_unused(ec);
  }

  auto HttpSession::makeBadRequest(std::string_view message,
                                   unsigned version,
                                   bool keep_alive) {
    Response<StringBody> res{boost::beast::http::status::bad_request, version};
    res.set(boost::beast::http::field::server, kServerName);
    res.set(boost::beast::http::field::content_type, "text/html");
    res.keep_alive(keep_alive);
    res.body() = message;
    res.prepare_payload();
    return res;
  }

  template <class Body>
  void HttpSession::handleRequest(boost::beast::http::request<Body> &&req) {
    // allow only POST method
    if (req.method() != boost::beast::http::verb::post) {
      return asyncWrite(makeBadRequest(
          "Unsupported HTTP-method", req.version(), req.keep_alive()));
    }

    std::cout << "request received: " << std::endl << req << std::endl;

    processRequest(req.body(), shared_from_this());
  }

  void HttpSession::acyncRead() {
    parser_ = std::make_unique<Parser>();
    parser_->body_limit(config_.max_request_size);
    stream_.expires_after(config_.operation_timeout);

    boost::beast::http::async_read(
        stream_,
        buffer_,
        parser_->get(),
        [self = shared_from_this()](auto ec, auto count) {
          auto* s = dynamic_cast<HttpSession*>(self.get());
          BOOST_ASSERT_MSG(s != nullptr, "cannot cast to HttpSession");
          s->onRead(ec, count);
        });
  }

  template <class Message>
  void HttpSession::asyncWrite(Message &&message) {
    // we need to put message into a shared ptr for async operations
    using message_type = decltype(message);
    auto m = std::make_shared<std::decay_t<message_type>>(
        std::forward<message_type>(message));

    // write response
    boost::beast::http::async_write(
        stream_, *m, [self = shared_from_this(), m](auto ec, auto size) {
          auto* s = dynamic_cast<HttpSession*>(self.get());
          BOOST_ASSERT_MSG(s != nullptr, "cannot cast to HttpSession");
          s->onWrite(ec, size, m->need_eof());
        });
  }

  void HttpSession::sendResponse(std::string_view response) {
    StringBody::value_type body;
    body.assign(response);

    const auto size = body.size();

    // send response
    Response<StringBody> res(std::piecewise_construct,
                             std::make_tuple(std::move(body)));
    res.set(HttpField::server, kServerName);
    res.set(HttpField::content_type, "text/html");
    res.content_length(size);
    res.keep_alive(true);

    return asyncWrite(std::move(res));
  }

  void HttpSession::onRead(boost::system::error_code ec, std::size_t) {
    if (ec) {
      auto error_message = (HttpError::end_of_stream == ec)
                               ? "connection was closed"
                               : "unknown error occurred";
      reportError(ec, error_message);
      stop();
    }

    handleRequest(parser_->release());
  }

  void HttpSession::onWrite(boost::system::error_code ec,
                            std::size_t,
                            bool should_stop) {
    if (ec) {
      reportError(ec, "failed to write message");
      return stop();
    }

    if (should_stop) {
      return stop();
    }

    // read next request
    acyncRead();
  }
}  // namespace kagome::api

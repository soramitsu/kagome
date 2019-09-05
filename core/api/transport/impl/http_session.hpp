/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_API_TRANSPORT_BEAST_HTTP_SESSION_HPP
#define KAGOME_CORE_API_TRANSPORT_BEAST_HTTP_SESSION_HPP

#include <chrono>
#include <cstdlib>
#include <memory>

#include <boost/beast.hpp>
#include <boost/optional.hpp>
#include "api/transport/session.hpp"

namespace kagome::api {
  /**
   * @brief HTTP session for api service
   */
  class HttpSession : public std::enable_shared_from_this<HttpSession>,
                      public Session {
   public:
    struct Configuration {
      static constexpr size_t kDefaultRequestSize = 10000u;
      static constexpr Duration kDefaultTimeout = std::chrono::seconds(30);
      size_t max_request_size{kDefaultRequestSize};
      Duration operation_timeout{kDefaultTimeout};
    };

    ~HttpSession() override = default;

    /**
     * @brief constructor
     * @param socket socket instance
     * @param config session configuration
     */
    HttpSession(boost::asio::ip::tcp::socket socket, Configuration config);

    /**
     * @brief starts session
     */
    void start() override;

    /**
     * @brief stops session
     */
    void stop() override;

   private:
    /**
     * @brief process http request, compose and execute response
     * @tparam Body request body type
     * @tparam Allocator allocator type
     * @tparam Send sender lambda
     * @param req request
     * @param send sender function
     */
    template <class Body>
    void handleRequest(boost::beast::http::request<Body> &&req);

    /**
     * @brief asynchronously read http message
     */
    void acyncRead();

    /**
     * @brief sends http message
     * @tparam Message http message type
     * @param message http message
     */
    template <class Message>
    void asyncWrite(Message &&message);

    /**
     * @brief sends response wrapped by http message
     * @param response message to send
     */
    void sendResponse(std::string_view response);

    void onRead(boost::system::error_code ec, std::size_t);
    void onWrite(boost::system::error_code ec, std::size_t, bool close);

    static constexpr std::string_view kServerName = "Kagome extrinsic api";

    Configuration config_;              ///< session configuration
    boost::beast::tcp_stream stream_;   ///< stream
    boost::beast::flat_buffer buffer_;  ///< read buffer

    using Parser =
        boost::beast::http::request_parser<boost::beast::http::string_body>;

    std::unique_ptr<Parser> parser_;  ///< http parser
  };

}  // namespace kagome::api

#endif  // KAGOME_CORE_API_TRANSPORT_BEAST_HTTP_SESSION_HPP

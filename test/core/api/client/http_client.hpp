/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 *
 * Kagome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Kagome is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef KAGOME_TEST_CORE_API_CLIENT_HTTP_CLIENT_HPP
#define KAGOME_TEST_CORE_API_CLIENT_HTTP_CLIENT_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <outcome/outcome.hpp>

namespace test {

  enum class HttpClientError {
    CONNECTION_FAILED = 1,
    HTTP_ERROR,
    NETWORK_ERROR
  };

  /**
   * Simple synchronous client for api service
   * it allows making synchronous http queries to api service
   */
  class HttpClient {
    using Socket = boost::asio::ip::tcp::socket;
    using FlatBuffer = boost::beast::flat_buffer;
    using HttpField = boost::beast::http::field;
    using HttpError = boost::beast::http::error;
    using HttpMethods = boost::beast::http::verb;
    using StringBody = boost::beast::http::string_body;
    using DynamicBody = boost::beast::http::dynamic_body;
    using QueryCallback = void(outcome::result<std::string>);
    using Context = boost::asio::io_context;

    template <typename Body>
    using HttpRequest = boost::beast::http::request<Body>;

    template <typename Body>
    using HttpResponse = boost::beast::http::response<Body>;

    template <class Body>
    using RequestParser = boost::beast::http::request_parser<Body>;

    static constexpr auto kUserAgent = "Kagome test api client 0.1";

   public:
    /**
     * @param context reference to io context instance
     */
    explicit HttpClient(Context &context) : stream_(context) {}

    HttpClient(const HttpClient &other) = delete;
    HttpClient &operator=(const HttpClient &other) = delete;
    HttpClient(HttpClient &&other) noexcept = delete;
    HttpClient &operator=(HttpClient &&other) noexcept = delete;
    ~HttpClient();

    /**
     * @brief connects to endpoint
     * @param endpoint address to connect
     * @return error code as outcome::result if failed or success
     */
    outcome::result<void> connect(boost::asio::ip::tcp::endpoint endpoint);

    /**
     * @brief make synchronous query to api service
     * @param message api query message
     * @param callback instructions to execute on completion
     */
    void query(std::string_view message,
               std::function<void(outcome::result<std::string>)> &&callback);

    /**
     * @brief disconnects stream
     */
    void disconnect();

   private:
    boost::beast::tcp_stream stream_;
    boost::asio::ip::tcp::endpoint endpoint_;
  };
}  // namespace test

OUTCOME_HPP_DECLARE_ERROR(test, HttpClientError)

#endif  // KAGOME_TEST_CORE_API_CLIENT_HTTP_CLIENT_HPP

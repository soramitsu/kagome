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

#ifndef KAGOME_VISITOR_HPP
#define KAGOME_VISITOR_HPP

#include <type_traits>  // for std::decay
#include <utility>      // for std::forward

#include <boost/variant/apply_visitor.hpp>  // for boost::apply_visitor

namespace kagome {

  template <typename... Lambdas>
  struct lambda_visitor;

  template <typename Lambda, typename... Lambdas>
  struct lambda_visitor<Lambda, Lambdas...>
      : public Lambda, public lambda_visitor<Lambdas...> {
    using Lambda::operator();
    using lambda_visitor<Lambdas...>::operator();

    // NOLINTNEXTLINE(google-explicit-constructor)
    lambda_visitor(Lambda lambda, Lambdas... lambdas)
        : Lambda(lambda), lambda_visitor<Lambdas...>(lambdas...) {}
  };

  template <typename Lambda>
  struct lambda_visitor<Lambda> : public Lambda {
    using Lambda::operator();

    // NOLINTNEXTLINE(google-explicit-constructor)
    lambda_visitor(Lambda lambda) : Lambda(lambda) {}
  };

  /**
   * @brief Convenient in-place compile-time visitor creation, from a set of
   * lambdas
   *
   * @code
   * make_visitor([](int a){ return 1; },
   *              [](std::string b) { return 2; });
   * @nocode
   *
   * is essentially the same as
   *
   * @code
   * struct visitor : public boost::static_visitor<int> {
   *   int operator()(int a) { return 1; }
   *   int operator()(std::string b) { return 2; }
   * }
   * @nocode
   *
   * @param lambdas
   * @return visitor
   */
  template <class... Fs>
  constexpr auto make_visitor(Fs &&... fs) {
    using visitor_type = lambda_visitor<std::decay_t<Fs>...>;
    return visitor_type(std::forward<Fs>(fs)...);
  }

  /**
   * @brief Inplace visitor for boost::variant.
   * @code
   *   boost::variant<int, std::string> value = "1234";
   *   ...
   *   visit_in_place(value,
   *                  [](int v) { std::cout << "(int)" << v; },
   *                  [](std::string v) { std::cout << "(string)" << v;}
   *                  );
   * @nocode
   *
   * @param variant
   * @param lambdas
   * @param lambdas
   */
  template <typename TVariant, typename... TVisitors>
  constexpr decltype(auto) visit_in_place(TVariant &&variant,
                                          TVisitors &&... visitors) {
    return boost::apply_visitor(
        make_visitor(std::forward<TVisitors>(visitors)...),
        std::forward<TVariant>(variant));
  }

  /// apply Matcher to optional T
  template <typename T, typename Matcher>
  constexpr decltype(auto) match(T &&t, Matcher &&m) {
    return std::forward<T>(t) ? std::forward<Matcher>(m)(*std::forward<T>(t))
                              : std::forward<Matcher>(m)();
  }

  /// construct visitor from Fs and apply it to optional T
  template <typename T, typename... Fs>
  constexpr decltype(auto) match_in_place(T &&t, Fs &&... fs) {
    return match(std::forward<T>(t), make_visitor(std::forward<Fs>(fs)...));
  }
}  // namespace kagome

#endif  // KAGOME_VISITOR_HPP

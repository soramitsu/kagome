/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_OUTCOME_REGISTER_HPP
#define KAGOME_OUTCOME_REGISTER_HPP

#include <boost/config.hpp>  // for BOOST_SYMBOL_EXPORT
#include <string>
#include <system_error>  // bring in std::error_code et al

#ifndef KAGOME_EXPORT
#if defined(BOOST_SYMBOL_EXPORT)
#define KAGOME_EXPORT BOOST_SYMBOL_EXPORT
#else
#define KAGOME_EXPORT
#endif
#endif

#define OUTCOME_USE_STD_IN_PLACE_TYPE 1

namespace __kagome {

  template <typename T>
  class Category : public std::error_category {
   public:
    const char *name() const noexcept final {
      return typeid(T).name();  // enum Errc -> 4Errc
    }

    std::string message(int c) const final {
      return toString(static_cast<T>(c));
    }

    static std::string toString(T t) {
      static_assert(
          !std::is_same<T, T>::value,
          "toString<T>() was not specialised for the type T supplied");
      return "";
    }

    KAGOME_EXPORT static const Category<T> &get() {
      static const Category<T> c;
      return c;
    }

   private:
    Category() = default;
    Category(const Category &) = delete;
    Category(Category &&) = delete;
  }; /* end of class */

}  // namespace __kagome

#define __OUTCOME_DEFINE_MAKE_ERROR_CODE(Enum)                     \
  extern std::error_code make_error_code(Enum e) {                 \
    return {static_cast<int>(e), __kagome::Category<Enum>::get()}; \
  }

#define __OUTCOME_DECLARE_MAKE_ERROR_CODE(Enum) \
  std::error_code make_error_code(Enum e);

/// MUST BE EXECUTED A FILE LEVEL (no namespace) in HPP
// ns - fully qualified enum namespace. Example: kagome::common::scale
// Enum - enum name. Example: EncodeError
#define OUTCOME_HPP_DECLARE_ERROR_2(ns, Enum) \
  namespace ns {                              \
    __OUTCOME_DECLARE_MAKE_ERROR_CODE(Enum)   \
  }                                           \
                                              \
  template <>                                 \
  struct std::is_error_code_enum<ns::Enum> : std::true_type {};

/// MUST BE EXECUTED A FILE LEVEL (global namespace) in HPP
// Enum - enum name. Example: EncodeError
#define OUTCOME_HPP_DECLARE_ERROR_1(Enum) \
  __OUTCOME_DECLARE_MAKE_ERROR_CODE(Enum) \
  template <>                             \
  struct std::is_error_code_enum<Enum> : std::true_type {};

/// MUST BE EXECUTED AT FILE LEVEL(no namespace) IN CPP
// ns - fully qualified enum namespace. Example: kagome::common::scale
// Enum - enum name. Example: EncodeError
// Name - variable name. Example: e
#define OUTCOME_CPP_DEFINE_CATEGORY_3(ns, Enum, Name) \
  namespace ns {                                      \
    __OUTCOME_DEFINE_MAKE_ERROR_CODE(Enum)            \
  };                                                  \
  template <>                                         \
  std::string __kagome::Category<ns::Enum>::toString(ns::Enum Name)

/// MUST BE EXECUTED AT FILE LEVEL(global namespace) IN CPP
// Enum - enum name. Example: EncodeError
// Name - variable name. Example: e
#define OUTCOME_CPP_DEFINE_CATEGORY_2(Enum, Name) \
  __OUTCOME_DEFINE_MAKE_ERROR_CODE(Enum)          \
  template <>                                     \
  std::string __kagome::Category<Enum>::toString(Enum Name)

// kind of "macro overloading"
#define __GET_MACRO_3(_1, _2, _3, NAME, ...) NAME
#define __GET_MACRO_2(_1, _2, NAME, ...) NAME


/// with 3 args: OUTCOME_CPP_DEFINE_CATEGORY_3
/// with 2 args: OUTCOME_CPP_DEFINE_CATEGORY_2
#define OUTCOME_CPP_DEFINE_CATEGORY(...)                    \
  __GET_MACRO_3(__VA_ARGS__, OUTCOME_CPP_DEFINE_CATEGORY_3, \
                OUTCOME_CPP_DEFINE_CATEGORY_2)              \
  (__VA_ARGS__)

/// with 2 args: OUTCOME_CPP_DEFINE_CATEGORY_2
/// with 1 arg : OUTCOME_CPP_DEFINE_CATEGORY_1
#define OUTCOME_HPP_DECLARE_ERROR(...)                    \
  __GET_MACRO_2(__VA_ARGS__, OUTCOME_HPP_DECLARE_ERROR_2, \
                OUTCOME_HPP_DECLARE_ERROR_1)              \
  (__VA_ARGS__)

#endif  // KAGOME_OUTCOME_REGISTER_HPP

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_GENERIC_LIST_ITERATOR_HPP
#define KAGOME_GENERIC_LIST_ITERATOR_HPP

namespace kagome::face {

  template <typename Container>
  class GeneriIterator {
   public:
    using value_type = typename Container::value_type;

    virtual ~GeneriIterator() = default;

    virtual value_type *get() = 0;
    virtual value_type const *get() const = 0;

    virtual value_type &operator*() = 0;
    virtual value_type const &operator*() const = 0;

    virtual GeneriIterator<value_type> &operator++() = 0;

    value_type &operator->() {
      return **this;
    }

    virtual bool operator!=(GeneriIterator<Container> &other) const {
      return get() != other.get();
    }
  };

}  // namespace kagome::face

#endif  // KAGOME_GENERIC_LIST_ITERATOR_HPP

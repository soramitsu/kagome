/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_TRANSPORT_MANAGER_IMPL_HPP
#define KAGOME_TRANSPORT_MANAGER_IMPL_HPP

#include <vector>

#include "libp2p/network/transport_manager.hpp"

namespace libp2p::network {
  class TransportManagerImpl : public TransportManager {
   public:
    /**
     * Initialize a transport manager with no supported transports
     */
    TransportManagerImpl() = default;

    /**
     * Initialize a transport manager from a collection of transports
     * @param transports, which this manager is going to support
     */
    explicit TransportManagerImpl(std::vector<TransportSPtr> transports);

    ~TransportManagerImpl() override = default;

    void add(TransportSPtr t) override;

    void add(gsl::span<const TransportSPtr> t) override;

    gsl::span<const TransportSPtr> getAll() const override;

    void clear() override;

    TransportSPtr findBest(const multi::Multiaddress &ma) override;

   private:
    std::vector<TransportSPtr> transports_;
  };
}  // namespace libp2p::network

#endif  // KAGOME_TRANSPORT_MANAGER_IMPL_HPP

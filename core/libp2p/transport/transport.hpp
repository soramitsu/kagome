/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_TRANSPORT_HPP
#define KAGOME_TRANSPORT_HPP

#include <chrono>
#include <functional>
#include <memory>
#include <system_error>

#include <outcome/outcome.hpp>  // for outcome::result
#include "libp2p/connection/raw_connection.hpp"
#include "libp2p/event/emitter.hpp"
#include "libp2p/multi/multiaddress.hpp"
#include "libp2p/peer/peer_id.hpp"
#include "libp2p/transport/transport_listener.hpp"

namespace libp2p::transport {

  /**
   * Allows to establish connections with other peers and react to received
   * attempts to do so; can be implemented, for example, as TCP, UDP etc
   */
  class Transport {
   public:
    using ConnectionCallback =
        outcome::result<void>(std::shared_ptr<connection::RawConnection>);
    using HandlerFunc = std::function<ConnectionCallback>;
    using ErrorFunc = std::function<void(const std::error_code &)>;

    virtual ~Transport() = default;

    /**
     * Try to establish connection with a peer
     * @param address of the peer
     * @return connection in case of success, error otherwise
     */
    virtual void dial(const multi::Multiaddress &address, HandlerFunc onSuccess,
                      ErrorFunc onError) const = 0;

    /**
     * Create a listener for incoming connections of this Transport; in case
     * it was already created, return it
     * @return pointer to the created listener
     */
    virtual std::shared_ptr<TransportListener> createListener(
        TransportListener::HandlerFunc onSuccess,
        TransportListener::ErrorFunc onError) const = 0;

    /**
     * Check if this transport supports a given multiaddress
     * @param ma to be checked against
     * @return true, if transport supports that multiaddress, false otherwise
     * @note example: '/tcp/...' on tcp transport will return true
     */
    virtual bool canDial(const multi::Multiaddress &ma) const = 0;
  };
}  // namespace libp2p::transport

#endif  // KAGOME_TRANSPORT_HPP

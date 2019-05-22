/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_ROUTER_HPP
#define KAGOME_ROUTER_HPP

#include <functional>

#include <outcome/outcome.hpp>
#include "libp2p/connection/stream.hpp"
#include "libp2p/peer/protocol.hpp"

namespace libp2p::network {

  /**
   * Manager for application-level protocols; when a new stream arrives for a
   * specific protocol, the corresponding handler is called
   * @note analog of Go's switch:
   * https://github.com/libp2p/go-libp2p-core/blob/consolidate-skeleton/host/host.go#L37
   */
  struct Router {
    using ProtoHandler = std::function<connection::Stream::Handler>;
    using ProtoPredicate = std::function<bool(const peer::Protocol &)>;

    virtual ~Router() = default;

    /**
     * Set handler for the {@param protocol}; only the perfect handler match
     * will be invoked
     * @param protocol to be handled
     * @param handler to be called, when a new stream for this protocol arrives
     */
    virtual void setProtocolHandler(const peer::Protocol &protocol,
                                    const ProtoHandler &handler) = 0;

    /**
     * Set handler for the <protocol, predicate> pair. First, searches all
     * handlers by prefix of the given protocol, then executes handler callback
     * for all matches when {@param predicate} returns true
     * @param protocol, for which pairs <protocol, handler> are to be retrieved
     * @param handler to be executed over the protocols, for which the predicate
     * was evaluated to true
     * @param predicate to be executed over the found protocols
     */
    virtual void setProtocolHandler(const peer::Protocol &protocol,
                                    const ProtoHandler &handler,
                                    const ProtoPredicate &predicate) = 0;

    /**
     * Get a list of handled protocols
     * @return supported protocols; may also include protocol prefixes
     */
    virtual std::vector<peer::Protocol> getSupportedProtocols() const = 0;

    /**
     * Remove handlers, associated with the given protocol prefix
     * @param protocol prefix, for which the handlers are to be removed
     */
    virtual void removeProtocolHandlers(const peer::Protocol &protocol) = 0;

    /**
     * Remove all handlers
     */
    virtual void removeAll() = 0;

   protected:
    /**
     * Execute stored handler for given protocol {@param p}; if several
     * handlers can be found (for example, if exact protocol match failed, and
     * prefix+predicate search returned more than one handlers, the random one
     * will be invoked)
     * @param p handler for this protocol will be invoked
     * @param stream with this stream
     */
    virtual outcome::result<void> handle(
        const peer::Protocol &p,
        std::shared_ptr<connection::Stream> stream) = 0;
  };

}  // namespace libp2p::network

#endif  // KAGOME_ROUTER_HPP

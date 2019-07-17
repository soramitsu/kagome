/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_DEFAULT_NETWORK_HPP
#define KAGOME_DEFAULT_NETWORK_HPP

// implementations
#include "libp2p/crypto/key_generator/key_generator_impl.hpp"
#include "libp2p/crypto/marshaller/key_marshaller_impl.hpp"
#include "libp2p/crypto/random_generator/boost_generator.hpp"
#include "libp2p/muxer/yamux.hpp"
#include "libp2p/network/impl/connection_manager_impl.hpp"
#include "libp2p/network/impl/dialer_impl.hpp"
#include "libp2p/network/impl/listener_manager_impl.hpp"
#include "libp2p/network/impl/network_impl.hpp"
#include "libp2p/network/impl/router_impl.hpp"
#include "libp2p/network/impl/transport_manager_impl.hpp"
#include "libp2p/peer/impl/identity_manager_impl.hpp"
#include "libp2p/protocol_muxer/multiselect.hpp"
#include "libp2p/security/plaintext.hpp"
#include "libp2p/transport/impl/upgrader_impl.hpp"
#include "libp2p/transport/tcp.hpp"

#endif //KAGOME_DEFAULT_NETWORK_HPP

#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

add_library(gossiper_broadcast
    gossiper_broadcast.cpp
    gossiper_broadcast.hpp
    )
target_link_libraries(gossiper_broadcast
    scale_message_read_writer
    logger
    )

add_library(kagome_router
    router_libp2p.cpp
    router_libp2p.hpp
    )
target_link_libraries(kagome_router
    logger
    outcome
    )

add_library(extrinsic_observer
    extrinsic_observer_impl.hpp
    extrinsic_observer_impl.cpp
    )
target_link_libraries(extrinsic_observer
    logger
    )

add_library(remote_sync_protocol_client
    remote_sync_protocol_client.cpp
    remote_sync_protocol_client.hpp
    )
target_link_libraries(remote_sync_protocol_client
    logger
    node_api_proto
    adapter_errors
    )

add_library(sync_protocol_observer
    sync_protocol_observer_impl.hpp
    sync_protocol_observer_impl.cpp
    )
target_link_libraries(sync_protocol_observer
    block_header_repository
    logger
    p2p::p2p_peer_id
    )

add_library(kademlia_storage_backend
    kademlia_storage_backend.cpp
    )
target_link_libraries(kademlia_storage_backend
    logger
    )

add_library(peer_manager
    peer_manager_impl.cpp
    )
target_link_libraries(peer_manager
    logger
    )

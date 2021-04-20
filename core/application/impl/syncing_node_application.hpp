/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_CORE_APPLICATION_IMPL_SYNCING_NODE_APPLICATION_HPP
#define KAGOME_CORE_APPLICATION_IMPL_SYNCING_NODE_APPLICATION_HPP

#include "application/kagome_application.hpp"

#include "application/app_configuration.hpp"
#include "application/app_state_manager.hpp"
#include "application/chain_spec.hpp"
#include "injector/application_injector.hpp"

namespace kagome::application {

  class SyncingNodeApplication : public KagomeApplication {
    using Babe = consensus::babe::Babe;
    using Grandpa = consensus::grandpa::Grandpa;

    template <class T>
    using sptr = std::shared_ptr<T>;

    template <class T>
    using uptr = std::unique_ptr<T>;

   public:
    ~SyncingNodeApplication() override = default;

    explicit SyncingNodeApplication(const AppConfiguration &app_config);

    void run() override;

   private:
    const AppConfiguration &app_config_;
    uptr<injector::SyncingNodeInjector> injector_;
    log::Logger logger_;

    sptr<boost::asio::io_context> io_context_;
    sptr<AppStateManager> app_state_manager_;
    sptr<ChainSpec> chain_spec_;
    sptr<network::Router> router_;
    sptr<network::PeerManager> peer_manager_;
    sptr<api::ApiService> jrpc_api_service_;
    sptr<Babe> babe_;
    sptr<Grandpa> grandpa_;
    sptr<network::SyncProtocolObserver> sync_observer_;
    const std::string node_name_;
  };

}  // namespace kagome::application

#endif  // KAGOME_CORE_APPLICATION_IMPL_SYNCING_NODE_APPLICATION_HPP

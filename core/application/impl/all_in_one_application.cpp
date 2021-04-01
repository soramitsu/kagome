/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "application/impl/all_in_one_application.hpp"

#include "application/impl/util.hpp"

namespace kagome::application {

  AllInOneApplication::AllInOneApplication(const AppConfiguration &app_config)
      : injector_{injector::makeValidatingNodeInjector(app_config)},
        logger_(log::createLogger("AllInOneApplication", "application")) {
    log::setLevelOfGroup("main", app_config.verbosity());

    if (app_config.isAlreadySynchronized()) {
      babe_execution_strategy_ = Babe::ExecutionStrategy::START;
    } else {
      babe_execution_strategy_ = Babe::ExecutionStrategy::SYNC_FIRST;
    }

    // keep important instances, the must exist when injector destroyed
    // some of them are requested by reference and hence not copied
    chain_spec_ = injector_.create<sptr<ChainSpec>>();
    BOOST_ASSERT(chain_spec_ != nullptr);

    app_state_manager_ = injector_.create<std::shared_ptr<AppStateManager>>();

    chain_path_ = app_config.chainPath(chain_spec_->id());

    io_context_ = injector_.create<sptr<boost::asio::io_context>>();
    clock_ = injector_.create<sptr<clock::SystemClock>>();
    babe_ = injector_.create<sptr<Babe>>();
    grandpa_ = injector_.create<sptr<Grandpa>>();
    router_ = injector_.create<sptr<network::Router>>();
    peer_manager_ = injector_.create<sptr<network::PeerManager>>();
    jrpc_api_service_ = injector_.create<sptr<api::ApiService>>();
  }

  void AllInOneApplication::run() {
    logger_->info("Start as ValidatingNode with PID {}", getpid());

    auto res = util::init_directory(chain_path_);
    if (not res) {
      logger_->critical("Error initalizing chain directory {}: {}",
                        chain_path_.native(),
                        res.error().message());
      exit(EXIT_FAILURE);
    }

    babe_->setExecutionStrategy(babe_execution_strategy_);

    app_state_manager_->atLaunch([ctx{io_context_}] {
      std::thread asio_runner([ctx{ctx}] { ctx->run(); });
      asio_runner.detach();
      return true;
    });

    app_state_manager_->atShutdown([ctx{io_context_}] { ctx->stop(); });

    app_state_manager_->run();
  }

}  // namespace kagome::application
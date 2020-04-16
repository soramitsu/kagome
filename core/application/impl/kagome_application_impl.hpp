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

#ifndef KAGOME_CORE_APPLICATION_IMPL_KAGOME_APPLICATION_IMPL_HPP
#define KAGOME_CORE_APPLICATION_IMPL_KAGOME_APPLICATION_IMPL_HPP

#include "application/kagome_application.hpp"

#include "application/configuration_storage.hpp"
#include "application/impl/local_key_storage.hpp"
#include "api/service/api_service.hpp"

namespace kagome::application {

  /**
   * @class KagomeApplicationImpl implements kagome application
   */
  class KagomeApplicationImpl : public KagomeApplication {
    using AuthorityIndex = primitives::AuthorityIndex;
    using Babe = consensus::Babe;
    using BabeGossiper = network::BabeGossiper;
    using BabeLottery = consensus::BabeLottery;
    using BlockBuilderFactory = authorship::BlockBuilderFactory;
    using BlockTree = blockchain::BlockTree;
    using Epoch = consensus::Epoch;
    using Hasher = crypto::Hasher;
    using ListenerImpl = api::WsListenerImpl;
    using Proposer = authorship::Proposer;
    using SR25519Keypair = crypto::SR25519Keypair;
    using Synchronizer = consensus::Synchronizer;
    using SystemClock = clock::SystemClock;
    using GrandpaLauncher = consensus::grandpa::Launcher;
    using Timer = clock::Timer;
    using InjectorType = decltype(injector::makeApplicationInjector(
        std::string{}, std::string{}, std::string{}, uint16_t{}, uint16_t{}, uint16_t{}));

    template <class T>
    using sptr = std::shared_ptr<T>;

    template <class T>
    using uptr = std::unique_ptr<T>;

   public:
    ~KagomeApplicationImpl() override = default;

    /**
     * @param kagome_config kagome configuration parameters
     * @param keys_config keys parameters
     */
    KagomeApplicationImpl(const std::string &config_path,
                          const std::string &keystore_path,
                          const std::string &leveldb_path,
                          uint16_t p2p_port,
                          uint16_t rpc_http_port,
                          uint16_t rpc_ws_port,
                          bool is_genesis_epoch,
                          uint8_t verbosity);

    void run() override;

   private:
    // need to keep all of these instances, since injector itself is destroyed
    InjectorType injector_;
    sptr<boost::asio::io_context> io_context_;
    sptr<ConfigurationStorage> config_storage_;
    sptr<KeyStorage> key_storage_;
    sptr<clock::SystemClock> clock_;
    sptr<api::ApiService> jrpc_api_service_;
    sptr<Babe> babe_;
    sptr<GrandpaLauncher> grandpa_launcher_;
    sptr<network::Router> router_;

    bool is_genesis_epoch_;
    common::Logger logger_;
  };

}  // namespace kagome::application

#endif  // KAGOME_CORE_APPLICATION_IMPL_KAGOME_APPLICATION_IMPL_HPP

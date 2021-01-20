/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "api/service/system/impl/system_api_impl.hpp"

#include <gtest/gtest.h>

#include "mock/core/application/chain_spec_mock.hpp"
#include "mock/core/consensus/babe/babe_mock.hpp"
#include "mock/core/network/gossiper_mock.hpp"
#include "mock/core/transaction_pool/transaction_pool_mock.hpp"
#include "testutil/outcome.hpp"

using kagome::api::SystemApi;
using kagome::api::SystemApiImpl;
using kagome::application::ChainSpecMock;
using kagome::common::Hash256;
using kagome::consensus::babe::BabeMock;
using kagome::network::GossiperMock;
using kagome::primitives::Transaction;
using kagome::transaction_pool::TransactionPoolMock;

using testing::_;
using testing::Return;

class SystemApiTest : public ::testing::Test {
 public:
  void SetUp() {
    chain_spec_mock_ = std::make_shared<ChainSpecMock>();
    babe_mock_ = std::make_shared<BabeMock>();
    gossiper_mock_ = std::make_shared<GossiperMock>();
    transaction_pool_mock_ = std::make_shared<TransactionPoolMock>();

    system_api_ = std::make_unique<SystemApiImpl>(
        chain_spec_mock_, babe_mock_, gossiper_mock_, transaction_pool_mock_);
  }

 protected:
  std::unique_ptr<SystemApi> system_api_;

  std::shared_ptr<ChainSpecMock> chain_spec_mock_;
  std::shared_ptr<BabeMock> babe_mock_;
  std::shared_ptr<GossiperMock> gossiper_mock_;
  std::shared_ptr<TransactionPoolMock> transaction_pool_mock_;

  static constexpr auto kSs58Account =
      "FJaSzBUAJ1Nwa1u5TbKAFZG5MBtcUouTixdP7hAkmce2SDS";
};

/**
 * @given an account id and no pending txs from that account
 * @when querying the account nonce
 * @then the nonce is equal to the value returned from runtime
 */
TEST_F(SystemApiTest, GetNonceNoPendingTxs) {
  constexpr auto kInitialNonce = 42;

  EXPECT_CALL(*transaction_pool_mock_, getReadyTransactions());

  EXPECT_OUTCOME_TRUE(nonce, system_api_->getNonceFor(kSs58Account))
  ASSERT_EQ(nonce, kInitialNonce);
}

/**
 * @given an account id and pending txs from that account
 * @when querying the account nonce
 * @then the nonce is equal to the value returned from runtime PLUS the number
 * of txs from the account
 */
TEST_F(SystemApiTest, GetNonceWithPendingTxs) {
  constexpr auto kInitialNonce = 42;
  constexpr auto kReadyTxNum = 5;
  std::array<std::vector<uint8_t>, kReadyTxNum> encoded_nonces;
  std::map<Transaction::Hash, std::shared_ptr<Transaction>> ready_txs;
  for (size_t i = 0; i < kReadyTxNum; i++) {
    EXPECT_OUTCOME_TRUE(enc_nonce, kagome::scale::encode(0, i))
    encoded_nonces[i] = std::move(enc_nonce);
    ready_txs[Hash256{{static_cast<uint8_t>(i)}}] = std::make_shared<Transaction>(
        Transaction{.provides{encoded_nonces[i]}});
  }

  EXPECT_CALL(*transaction_pool_mock_, getReadyTransactions())
      .WillOnce(Return(ready_txs));

  EXPECT_OUTCOME_TRUE(nonce, system_api_->getNonceFor(kSs58Account));
  ASSERT_EQ(nonce, kInitialNonce + kReadyTxNum);
}

/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thread>

#include "application/impl/app_state_manager_impl.hpp"

using kagome::AppStateException;
using kagome::AppStateManager;
using kagome::AppStateManagerImpl;

using testing::Return;
using testing::Sequence;

class CallbackMock {
 public:
  MOCK_METHOD0(call, void());
  void operator()() {
    return call();
  }
};

class AppStateManagerTest : public AppStateManagerImpl, public testing::Test {
 public:
  void SetUp() override {
    reset();
    prepare_cb = std::make_shared<CallbackMock>();
    launch_cb = std::make_shared<CallbackMock>();
    shutdown_cb = std::make_shared<CallbackMock>();
  }
  void TearDown() override {
    prepare_cb.reset();
    launch_cb.reset();
    shutdown_cb.reset();
  }

  std::shared_ptr<CallbackMock> prepare_cb;
  std::shared_ptr<CallbackMock> launch_cb;
  std::shared_ptr<CallbackMock> shutdown_cb;
};

/**
 * @given new created AppStateManager
 * @when switch stages in order
 * @then state changes according to the order
 */
TEST_F(AppStateManagerTest, StateSequence_Normal) {
  ASSERT_EQ(state(), AppStateManager::State::Init);
  ASSERT_NO_THROW(prepare());
  ASSERT_EQ(state(), AppStateManager::State::ReadyToStart);
  ASSERT_NO_THROW(launch());
  ASSERT_EQ(state(), AppStateManager::State::Works);
  ASSERT_NO_THROW(shuttingdown());
  ASSERT_EQ(state(), AppStateManager::State::ReadyToStop);
}

/**
 * @given AppStateManager in state 'ReadyToStart' (after stage 'prepare')
 * @when trying to run stage 'prepare' again
 * @then thrown exception, state wasn't change. Can to run stages 'launch' and
 * 'shutdown'
 */
TEST_F(AppStateManagerTest, StateSequence_Abnormal_1) {
  prepare();
  EXPECT_THROW(prepare(), AppStateException);
  EXPECT_EQ(state(), AppStateManager::State::ReadyToStart);
  EXPECT_NO_THROW(launch());
  EXPECT_NO_THROW(shuttingdown());
}

/**
 * @given AppStateManager in state 'Works' (after stage 'launch')
 * @when trying to run stages 'prepare' and 'launch' again
 * @then thrown exceptions, state wasn't change. Can to run stages 'launch' and
 * 'shutdown'
 */
TEST_F(AppStateManagerTest, StateSequence_Abnormal_2) {
  prepare();
  launch();
  EXPECT_THROW(prepare(), AppStateException);
  EXPECT_THROW(launch(), AppStateException);
  EXPECT_EQ(state(), AppStateManager::State::Works);
  EXPECT_NO_THROW(shuttingdown());
}

/**
 * @given AppStateManager in state 'ReadyToStop' (after stage 'shutdown')
 * @when trying to run any stages 'prepare' and 'launch' again
 * @then thrown exceptions, except shutdown. State wasn't change.
 */
TEST_F(AppStateManagerTest, StateSequence_Abnormal_3) {
  prepare();
  launch();
  shuttingdown();
  EXPECT_THROW(prepare(), AppStateException);
  EXPECT_THROW(launch(), AppStateException);
  EXPECT_NO_THROW(shuttingdown());
  EXPECT_EQ(state(), AppStateManager::State::ReadyToStop);
}

/**
 * @given new created AppStateManager
 * @when add callbacks for each stages
 * @then done without exceptions
 */
TEST_F(AppStateManagerTest, AddCallback_Initial) {
  EXPECT_NO_THROW(atPrepare([] {}));
  EXPECT_NO_THROW(atLaunch([] {}));
  EXPECT_NO_THROW(atShuttingdown([] {}));
}

/**
 * @given AppStateManager in state 'ReadyToStart' (after stage 'prepare')
 * @when add callbacks for each stages
 * @then thrown exception only for 'prepare' stage callback
 */
TEST_F(AppStateManagerTest, AddCallback_AfterPrepare) {
  prepare();
  EXPECT_THROW(atPrepare([] {}), AppStateException);
  EXPECT_NO_THROW(atLaunch([] {}));
  EXPECT_NO_THROW(atShuttingdown([] {}));
}

/**
 * @given AppStateManager in state 'Works' (after stage 'launch')
 * @when add callbacks for each stages
 * @then done without exception only for 'shutdown' stage callback
 */
TEST_F(AppStateManagerTest, AddCallback_AfterLaunch) {
  prepare();
  launch();
  EXPECT_THROW(atPrepare([] {}), AppStateException);
  EXPECT_THROW(atLaunch([] {}), AppStateException);
  EXPECT_NO_THROW(atShuttingdown([] {}));
}

/**
 * @given AppStateManager in state 'ReadyToStop' (after stage 'shutdown')
 * @when add callbacks for each stages
 * @then trown exceptions for each calls
 */
TEST_F(AppStateManagerTest, AddCallback_AfterShutdown) {
  prepare();
  launch();
  shuttingdown();
  EXPECT_THROW(atPrepare([] {}), AppStateException);
  EXPECT_THROW(atLaunch([] {}), AppStateException);
  EXPECT_THROW(atShuttingdown([] {}), AppStateException);
}

/**
 * @given new created AppStateManager
 * @when register callbacks by reg() method
 * @then each callcack registered for appropriate state
 */
TEST_F(AppStateManagerTest, RegCallbacks) {
  int tag = 0;

  reg(
      [&] {
        (*prepare_cb)();
        tag = 1;
      },
      [&] {
        (*launch_cb)();
        tag = 2;
      },
      [&] {
        (*shutdown_cb)();
        tag = 3;
      });

  EXPECT_CALL(*prepare_cb, call()).WillOnce(Return());
  EXPECT_CALL(*launch_cb, call()).WillOnce(Return());
  EXPECT_CALL(*shutdown_cb, call()).WillOnce(Return());

  EXPECT_NO_THROW(prepare());
  EXPECT_EQ(tag, 1);
  EXPECT_NO_THROW(launch());
  EXPECT_EQ(tag, 2);
  EXPECT_NO_THROW(shuttingdown());
  EXPECT_EQ(tag, 3);
}

/**
 * @given new created AppStateManager
 * @when register callbacks by reg() method and run() AppStateManager
 * @then each callcack execudted according to the stages order
 */
TEST_F(AppStateManagerTest, Run_CallSequence) {
  EXPECT_THROW(run(), std::logic_error);

  auto app_state_manager = std::make_shared<AppStateManagerImpl>();

  app_state_manager->reg([&] { (*prepare_cb)(); },
                         [&] { (*launch_cb)(); },
                         [&] { (*shutdown_cb)(); });

  Sequence seq;
  EXPECT_CALL(*prepare_cb, call()).InSequence(seq).WillOnce(Return());
  EXPECT_CALL(*launch_cb, call()).InSequence(seq).WillOnce(Return());
  EXPECT_CALL(*shutdown_cb, call()).InSequence(seq).WillOnce(Return());

  app_state_manager->atLaunch([] {
    std::thread terminator([] { raise(SIGQUIT); });
    terminator.detach();
  });

  EXPECT_NO_THROW(app_state_manager->run());
}

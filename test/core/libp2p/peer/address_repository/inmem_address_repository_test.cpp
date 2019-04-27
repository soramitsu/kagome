/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thread>

#include "libp2p/peer/address_repository.hpp"
#include "libp2p/peer/address_repository/inmem_address_repository.hpp"
#include "libp2p/peer/errors.hpp"
#include "testutil/outcome.hpp"

using namespace libp2p::peer;
using namespace libp2p::multi;
using std::literals::chrono_literals::operator""ms;

struct InmemAddressRepository_Test : public ::testing::Test {
  void SetUp() override {
    db = std::make_unique<InmemAddressRepository>();
    db->onAddressAdded([](const PeerId &p, const Multiaddress &ma) {
      std::cout << "added  : <" << p.toHex() << "> " << ma.getStringAddress()
                << '\n';
    });

    db->onAddressRemoved([](const PeerId &p, const Multiaddress &ma) {
      std::cout << "removed: <" << p.toHex() << "> " << ma.getStringAddress()
                << '\n';
    });
  }

  void collectGarbage() {
    std::cout << "[collectGarbage started...  ]\n";
    db->collectGarbage();
    std::cout << "[collectGarbage finished... ]\n";
  }

  std::unique_ptr<AddressRepository> db;

  const PeerId p1 = Multihash::createFromHex("1203020304").value();
  const PeerId p2 = Multihash::createFromHex("1203ffffff").value();

  const Multiaddress ma1 =
      Multiaddress::create("/ip4/127.0.0.1/tcp/8080").value();
  const Multiaddress ma2 =
      Multiaddress::create("/ip4/127.0.0.1/tcp/8081").value();
  const Multiaddress ma3 =
      Multiaddress::create("/ip4/127.0.0.1/tcp/8082").value();
  const Multiaddress ma4 =
      Multiaddress::create("/ip4/127.0.0.1/tcp/8083").value();
};

TEST_F(InmemAddressRepository_Test, GarbageCollection) {
  // @given address repository that has 2 peers, and some addresses
  EXPECT_OUTCOME_TRUE_1(
      db->addAddresses(p1, std::vector<Multiaddress>{ma1, ma2}, 10ms));
  EXPECT_OUTCOME_TRUE_1(
      db->addAddresses(p1, std::vector<Multiaddress>{ma3, ma4}, 100ms));
  EXPECT_OUTCOME_TRUE_1(
      db->upsertAddresses(p2, std::vector<Multiaddress>{ma4}, 10ms));

  // @when no collectGarbage is called
  {
    EXPECT_OUTCOME_TRUE_2(v1, db->getAddresses(p1));
    EXPECT_OUTCOME_TRUE_2(v2, db->getAddresses(p2));

    // @then initial state is initial
    EXPECT_EQ(v1.size(), 4);
    EXPECT_EQ(v2.size(), 1);
  }

  // @when first collect garbage is called
  collectGarbage();

  {
    EXPECT_OUTCOME_TRUE_2(v1, db->getAddresses(p1));
    EXPECT_OUTCOME_TRUE_2(v2, db->getAddresses(p2));

    // @then no addresses are evicted
    EXPECT_EQ(v1.size(), 4);
    EXPECT_EQ(v2.size(), 1);
  }

  // @when second collect garbage is called in 50
  std::this_thread::sleep_for(50ms);
  collectGarbage();
  // ma1 and ma2 for p1 should be evicted by now

  {
    // @then p1 has evicted 2 addresses
    EXPECT_OUTCOME_TRUE_2(v1, db->getAddresses(p1));
    EXPECT_EQ(v1.size(), 2);

    // @and p2 has been evicted completely
    auto v2 = db->getAddresses(p2);
    EXPECT_FALSE(v2);
    // peers without addresses are removed... so we can't find this peer
    EXPECT_EQ(v2.error().value(), (int)PeerError::NotFound);
  }

  // @when clear p1 addresses
  db->clear(p1);

  {
    // @then p1 is not evicted, but all its addresses are
    // since we intentionally cleared addresses of this peer, we do not evict
    // this peer from the list of known peers up to the next garbage collection
    EXPECT_OUTCOME_TRUE_2(v1, db->getAddresses(p1));
    EXPECT_EQ(v1.size(), 0);

    // @and p2 is still evicted
    auto v2 = db->getAddresses(p2);
    EXPECT_FALSE(v2);
    EXPECT_EQ(v2.error().value(), (int)PeerError::NotFound);
  }

  // @when third collect garbage is called
  collectGarbage();

  {
    // @then both p1 and p2 have been evicted completely
    // last garbage collection removed all peers that do not have addresses
    for (const auto &it : {p1, p2}) {
      auto v = db->getAddresses(it);
      EXPECT_FALSE(v);
      EXPECT_EQ(v.error().value(), (int)PeerError::NotFound);
    }
  }
}

/**
 * @given Peer p1 has address m1 with ttl 100ms
 * @when update ttl with 1000ms, then execute collectGarbage
 * @then ttl is updated, ma1 is not evicted
 */
TEST_F(InmemAddressRepository_Test, UpdateAddress) {
  EXPECT_OUTCOME_TRUE_1(
      db->addAddresses(p1, std::vector<Multiaddress>{ma1}, 10ms));
  EXPECT_OUTCOME_TRUE_1(
      db->upsertAddresses(p1, std::vector<Multiaddress>{ma1}, 100ms));

  {
    EXPECT_OUTCOME_TRUE_2(v, db->getAddresses(p1));
    EXPECT_EQ(v.size(), 1);
  }

  std::this_thread::sleep_for(50ms);
  collectGarbage();

  // ma1 is updated
  EXPECT_OUTCOME_TRUE_2(v, db->getAddresses(p1));
  EXPECT_EQ(v.size(), 1);
}

/**
 * @given Peer p1 has address m1 with ttl 100ms
 * @when upsert ma2 with ttl=1000ms, and execute collectGarbage
 * @then ttl of ma1 is not updated, ma1 is evicted. ma2 is inserted.
 */
TEST_F(InmemAddressRepository_Test, InsertAddress) {
  EXPECT_OUTCOME_TRUE_1(
      db->addAddresses(p1, std::vector<Multiaddress>{ma1}, 10ms));
  EXPECT_OUTCOME_TRUE_1(
      db->upsertAddresses(p1, std::vector<Multiaddress>{ma2}, 100ms));

  {
    EXPECT_OUTCOME_TRUE_2(v, db->getAddresses(p1));
    EXPECT_EQ(v.size(), 2);
  }

  std::this_thread::sleep_for(50ms);
  collectGarbage();

  // ma1 is evicted, ma2 is not
  EXPECT_OUTCOME_TRUE_2(v, db->getAddresses(p1));
  EXPECT_EQ(v.size(), 1);
  EXPECT_EQ(v.front(), ma2);
}

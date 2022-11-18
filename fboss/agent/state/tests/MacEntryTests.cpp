/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "fboss/agent/state/MacEntry.h"
#include "fboss/agent/test/TestUtils.h"

#include <gtest/gtest.h>

using namespace facebook::fboss;
namespace {
const auto kTestMac = folly::MacAddress("01:02:03:04:05:06");
}

TEST(MacEntryTest, toFromFollyDynamic) {
  MacEntry entryDynamic(
      kTestMac,
      PortDescriptor(PortID(1)),
      std::optional<cfg::AclLookupClass>(
          cfg::AclLookupClass::CLASS_QUEUE_PER_HOST_QUEUE_0));
  EXPECT_EQ(
      *MacEntry::fromFollyDynamic(entryDynamic.toFollyDynamic()), entryDynamic);
  MacEntry entryStatic(
      kTestMac,
      PortDescriptor(PortID(1)),
      std::optional<cfg::AclLookupClass>(
          cfg::AclLookupClass::CLASS_QUEUE_PER_HOST_QUEUE_0),
      MacEntryType::STATIC_ENTRY);
  EXPECT_EQ(
      *MacEntry::fromFollyDynamic(entryStatic.toFollyDynamic()), entryStatic);
  validateNodeSerialization(entryStatic);
}

TEST(MacEntryTest, Compare) {
  MacEntry entryDynamic(
      kTestMac,
      PortDescriptor(PortID(1)),
      std::optional<cfg::AclLookupClass>(
          cfg::AclLookupClass::CLASS_QUEUE_PER_HOST_QUEUE_0));

  MacEntry entryStatic(
      kTestMac,
      PortDescriptor(PortID(1)),
      std::optional<cfg::AclLookupClass>(
          cfg::AclLookupClass::CLASS_QUEUE_PER_HOST_QUEUE_0),
      MacEntryType::STATIC_ENTRY);
  EXPECT_NE(entryStatic, entryDynamic);
}

TEST(MacEntryTest, fromJSONWithType) {
  std::string jsonStrMissingEntryType = R"(
  {
      "mac": "01:02:03:04:05:06",
      "portId": {
            "portId": 1,
            "portType": 0
      }
  })";

  auto fields = MacEntryFields::fromJson(jsonStrMissingEntryType);
  auto entry = std::make_shared<MacEntry>();
  entry->fromThrift(fields.toThrift());
  EXPECT_EQ(
      *entry,
      MacEntry(
          kTestMac,
          PortDescriptor(PortID(1)),
          std::optional<cfg::AclLookupClass>(std::nullopt),
          MacEntryType::DYNAMIC_ENTRY));
}

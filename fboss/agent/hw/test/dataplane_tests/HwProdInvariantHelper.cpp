/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "fboss/agent/hw/test/dataplane_tests/HwProdInvariantHelper.h"

#include <folly/logging/xlog.h>
#include "fboss/agent/hw/test/ConfigFactory.h"
#include "fboss/agent/hw/test/HwTestCoppUtils.h"
#include "fboss/agent/hw/test/HwTestPacketUtils.h"
#include "fboss/agent/hw/test/LoadBalancerUtils.h"
#include "fboss/agent/hw/test/dataplane_tests/HwEcmpDataPlaneTestUtil.h"
#include "fboss/agent/hw/test/dataplane_tests/HwTestOlympicUtils.h"
#include "fboss/agent/hw/test/dataplane_tests/HwTestQosUtils.h"
#include "fboss/agent/hw/test/dataplane_tests/HwTestQueuePerHostUtils.h"
#include "fboss/agent/state/Interface.h"
#include "fboss/agent/test/EcmpSetupHelper.h"

#include "fboss/agent/hw/test/gen-cpp2/validated_shell_commands_constants.h"

#include "folly/IPAddressV4.h"

namespace {
auto constexpr kEcmpWidth = 4;
auto constexpr kTopLabel = 5000;
} // namespace
namespace facebook::fboss {

void HwProdInvariantHelper::setupEcmp() {
  ecmpHelper_ = std::make_unique<utility::HwIpV6EcmpDataPlaneTestUtil>(
      ensemble_, RouterID(0));
  ecmpHelper_->programRoutes(
      kEcmpWidth, std::vector<NextHopWeight>(kEcmpWidth, 1));
}

std::vector<PortDescriptor> HwProdInvariantHelper::getUplinksForEcmp() {
  auto hwSwitch = ensemble_->getHwSwitch();
  auto ecmpPorts = utility::getUplinksForEcmp(
      hwSwitch, initialConfig(), kEcmpWidth, is_mmu_lossless_mode());
  return ecmpPorts;
}

void HwProdInvariantHelper::setupEcmpWithNextHopMac(
    const folly::MacAddress& nextHopMac) {
  ecmpHelper_ = std::make_unique<utility::HwIpV6EcmpDataPlaneTestUtil>(
      ensemble_, nextHopMac, RouterID(0));

  ecmpPorts_ = getUplinksForEcmp();
  ecmpHelper_->programRoutesVecHelper(
      ecmpPorts_, std::vector<NextHopWeight>(kEcmpWidth, 1));
}

void HwProdInvariantHelper::setupEcmpOnUplinks() {
  ecmpHelper_ = std::make_unique<utility::HwIpV6EcmpDataPlaneTestUtil>(
      ensemble_, RouterID(0));

  ecmpPorts_ = getUplinksForEcmp();
  ecmpHelper_->programRoutesVecHelper(
      ecmpPorts_, std::vector<NextHopWeight>(kEcmpWidth, 1));
}

void HwProdInvariantHelper::sendTraffic() {
  CHECK(ecmpHelper_);
  ecmpHelper_->pumpTrafficThroughPort(getDownlinkPort());
}

/*
 * "On Downlink" really just means "On Not-an-ECMP-port". ECMP is only setup
 * on uplinks, so using a downlink is a safe way to do it.
 */
void HwProdInvariantHelper::sendTrafficOnDownlink() {
  CHECK(ecmpHelper_);
  // This function is mostly used for verifying load balancing, where packets
  // will end up on ECMP-enabled uplinks and then be verified.
  // Since the traffic ends up on uplinks, we send the traffic through
  // downlink.
  auto downlink = getDownlinkPort();
  ecmpHelper_->pumpTrafficThroughPort(downlink);
}

void HwProdInvariantHelper::verifyLoadBalacing() {
  CHECK(ecmpHelper_);
  bool loadBalanced = utility::pumpTrafficAndVerifyLoadBalanced(
      [=]() { sendTrafficOnDownlink(); },
      [=]() {
        auto ports = std::make_unique<std::vector<int32_t>>();
        auto ecmpPortIds = getEcmpPortIds();
        for (auto ecmpPortId : ecmpPortIds) {
          ports->push_back(static_cast<int32_t>(ecmpPortId));
        }
        getHwSwitchEnsemble()->getHwSwitch()->clearPortStats(ports);
      },
      [=]() {
        return ecmpHelper_->isLoadBalanced(
            ecmpPorts_, std::vector<NextHopWeight>(kEcmpWidth, 1), 25);
      });
  EXPECT_TRUE(loadBalanced);
}

std::shared_ptr<SwitchState> HwProdInvariantHelper::getProgrammedState() const {
  return ensemble_->getProgrammedState();
}

PortID HwProdInvariantHelper::getDownlinkPort() {
  // pick the first downlink in the list
  return utility::getAllUplinkDownlinkPorts(
             ensemble_->getHwSwitch(),
             initialConfig(),
             kEcmpWidth,
             is_mmu_lossless_mode())
      .second[0];
}

void HwProdInvariantHelper::verifyCopp() {
  utility::verifyCoppInvariantHelper(
      ensemble_->getHwSwitch(),
      ensemble_->getPlatform()->getAsic(),
      getProgrammedState(),
      getDownlinkPort());
}

// two ways to get the ECMP ports
// either they are populated in the ecmpPorts_
// or we pick them from the ecmpHelper
// since we have some tests using both
// support both for now and phase out the one using kEcmpWidth
std::vector<PortID> HwProdInvariantHelper::getEcmpPortIds() {
  std::vector<PortID> ecmpPortIds{};
  for (auto portDesc : ecmpPorts_) {
    EXPECT_TRUE(portDesc.isPhysicalPort());
    auto portId = portDesc.phyPortID();
    ecmpPortIds.emplace_back(portId);
  }

  return ecmpPortIds;
}

void HwProdInvariantHelper::verifyDscpToQueueMapping() {
  if (!ensemble_->getAsic()->isSupported(HwAsic::Feature::L3_QOS)) {
    return;
  }
  auto portId = getDownlinkPort();
  // lambda that returns HwPortStats for the given port
  auto getPortStats = [this]() {
    return ensemble_->getLatestPortStats(ensemble_->masterLogicalPortIds());
  };

  auto q2dscpMap = utility::getOlympicQosMaps(initialConfig());
  EXPECT_TRUE(utility::verifyQueueMappingsInvariantHelper(
      q2dscpMap,
      ensemble_->getHwSwitch(),
      getProgrammedState(),
      getPortStats,
      getEcmpPortIds(),
      portId));
}

void HwProdInvariantHelper::verifySafeDiagCmds() {
  std::set<std::string> diagCmds;
  switch (ensemble_->getAsic()->getAsicType()) {
    case HwAsic::AsicType::ASIC_TYPE_FAKE:
    case HwAsic::AsicType::ASIC_TYPE_MOCK:
    case HwAsic::AsicType::ASIC_TYPE_TAJO:
    case HwAsic::AsicType::ASIC_TYPE_ELBERT_8DD:
      break;

    case HwAsic::AsicType::ASIC_TYPE_TRIDENT2:
      diagCmds = validated_shell_commands_constants::TD2_TESTED_CMDS();
    case HwAsic::AsicType::ASIC_TYPE_TOMAHAWK:
      diagCmds = validated_shell_commands_constants::TH_TESTED_CMDS();
    case HwAsic::AsicType::ASIC_TYPE_TOMAHAWK3:
      diagCmds = validated_shell_commands_constants::TH3_TESTED_CMDS();
    case HwAsic::AsicType::ASIC_TYPE_TOMAHAWK4:
      diagCmds = validated_shell_commands_constants::TH4_TESTED_CMDS();
      break;
  }
  if (diagCmds.size()) {
    for (auto i = 0; i < 10; ++i) {
      for (auto cmd : diagCmds) {
        std::string out;
        ensemble_->runDiagCommand(cmd + "\n", out);
      }
    }
    std::string out;
    ensemble_->runDiagCommand("quit\n", out);
  }
}

void HwProdInvariantHelper::verifyNoDiscards() {
  auto portId = ensemble_->masterLogicalPortIds()[0];
  auto outDiscards = *ensemble_->getLatestPortStats(portId).outDiscards__ref();
  auto inDiscards = *ensemble_->getLatestPortStats(portId).inDiscards__ref();
  EXPECT_EQ(outDiscards, 0);
  EXPECT_EQ(inDiscards, 0);
}

void HwProdInvariantHelper::disableTtl() {
  for (const auto& nhop : ecmpHelper_->getNextHops()) {
    if (std::find(ecmpPorts_.begin(), ecmpPorts_.end(), nhop.portDesc) !=
        ecmpPorts_.end()) {
      utility::disableTTLDecrements(
          ensemble_->getHwSwitch(), RouterID(0), nhop);
    }
  }
}

void HwProdInvariantHelper::verifyQueuePerHostMapping() {
  auto vlanId = utility::firstVlanID(getProgrammedState());
  auto intfMac =
      utility::getInterfaceMac(ensemble_->getProgrammedState(), vlanId);
  auto srcMac = utility::MacAddressGenerator().get(intfMac.u64NBO());

  utility::verifyQueuePerHostMapping(
      ensemble_->getHwSwitch(),
      ensemble_,
      vlanId,
      srcMac,
      intfMac,
      folly::IPAddressV4("1.0.0.1"),
      folly::IPAddressV4("10.10.1.2"),
      true /* useFrontPanel */,
      false /* blockNeighbor */,
      std::nullopt, /* l4SrcPort */
      std::nullopt, /* l4DstPort */
      std::nullopt); /* dscp */
}

void HwProdInvariantHelper::verifyMpls() {
  verifyMplsEntry(kTopLabel, LabelForwardingAction::LabelForwardingType::SWAP);
}

void HwProdInvariantHelper::verifyMplsEntry(
    int label,
    LabelForwardingAction::LabelForwardingType action) {
  auto state = getProgrammedState();
  auto entry =
      state->getLabelForwardingInformationBase()->getLabelForwardingEntryIf(
          label);
  EXPECT_NE(entry, nullptr);
  auto nhops = entry->getForwardInfo().getNextHopSet();
  EXPECT_NE(nhops.size(), 0);
  for (const auto& nhop : nhops) {
    EXPECT_TRUE(nhop.labelForwardingAction().has_value());
    EXPECT_EQ(nhop.labelForwardingAction()->type(), action);
  }
}

} // namespace facebook::fboss

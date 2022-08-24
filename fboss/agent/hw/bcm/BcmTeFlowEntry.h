/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#pragma once

extern "C" {
#include <bcm/types.h>
}

#include "fboss/agent/FbossError.h"
#include "fboss/agent/hw/bcm/BcmAddressFBConvertors.h"
#include "fboss/agent/hw/bcm/types.h"
#include "fboss/agent/state/RouteNextHopEntry.h"
#include "fboss/agent/state/TeFlowEntry.h"

namespace facebook::fboss {

class BcmSwitch;
class BcmMultiPathNextHop;

/**
 * BcmTeFlowEntry is the class to abstract an TeFlow's resources in BcmSwitch
 */
class BcmTeFlowEntry {
 public:
  BcmTeFlowEntry(
      BcmSwitch* hw,
      int gid,
      const std::shared_ptr<TeFlowEntry>& teFlow);
  ~BcmTeFlowEntry();
  BcmTeFlowEntryHandle getHandle() const {
    return handle_;
  }

  TeFlow getID() const;

 private:
  void createTeFlowEntry();

  BcmSwitch* hw_;
  int gid_;
  std::shared_ptr<TeFlowEntry> teFlow_;
  BcmTeFlowEntryHandle handle_;
  std::shared_ptr<BcmMultiPathNextHop> redirectNexthop_;
};

} // namespace facebook::fboss
// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#pragma once

#include <folly/IPAddress.h>
#include "fboss/cli/fboss2/utils/CmdUtils.h"

namespace facebook::fboss {

class HostInfo {
 public:
  explicit HostInfo(const std::string& hostName)
      : HostInfo(hostName, utils::getIPFromHost(hostName)) {}

  HostInfo(const std::string& hostName, const folly::IPAddress& ip)
      : name_(hostName), ip_(ip) {}

  const std::string& getName() const {
    return name_;
  }

  const folly::IPAddress& getIp() const {
    return ip_;
  }

  const std::string getIpStr() const {
    return ip_.str();
  }

 private:
  const std::string name_;
  const folly::IPAddress ip_;
};

} // namespace facebook::fboss

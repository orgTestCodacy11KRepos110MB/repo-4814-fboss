// Copyright 2004-present Facebook. All Rights Reserved.

#include "common/services/cpp/ServiceFrameworkLight.h"
#include "fboss/platform/helpers/Init.h"

#include <fb303/FollyLoggingHandler.h>
#include <folly/logging/Init.h>

#include "fboss/platform/rackmon/RackmonThriftHandler.h"

using namespace facebook;
using namespace facebook::services;
using namespace facebook::fboss::platform;

FOLLY_INIT_LOGGING_CONFIG("fboss=DBG2; default:async=true");

DEFINE_int32(port, 7910, "Port for the thrift service");

int main(int argc, char** argv) {
  // Init FB and export build values
  helpers::fbInit(argc, argv);
  fb303::registerFollyLoggingOptionHandlers();

  // Setup thrift handler and server
  auto [server, handler] = helpers::setupThrift(
      std::make_shared<rackmonsvc::ThriftHandler>(), FLAGS_port);

  services::ServiceFrameworkLight service("Rackmon Service");
  service.addThriftService(server, handler.get(), FLAGS_port);
  helpers::addCommonModules(service);
  service.go();

  return 0;
}

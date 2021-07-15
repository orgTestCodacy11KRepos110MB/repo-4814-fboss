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

#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include "fboss/agent/if/gen-cpp2/FbossCtrl.h"
#include "fboss/cli/fboss2/CmdGlobalOptions.h"
#include "fboss/qsfp_service/if/gen-cpp2/QsfpService.h"

#include <memory>
#include <string>

namespace facebook::fboss::utils {

static auto constexpr kConnTimeout = 1000;
static auto constexpr kRecvTimeout = 45000;
static auto constexpr kSendTimeout = 5000;

template <typename T>
std::unique_ptr<T> createClient(const std::string& ip);

template <typename Client>
std::unique_ptr<Client> createPlaintextClient(
    const std::string& ip,
    const int port) {
  auto eb = folly::EventBaseManager::get()->getEventBase();
  auto addr = folly::SocketAddress(ip, port);
  auto sock = folly::AsyncSocket::newSocket(eb, addr, kConnTimeout);
  sock->setSendTimeout(kSendTimeout);
  auto channel =
      apache::thrift::HeaderClientChannel::newChannel(std::move(sock));
  channel->setTimeout(kRecvTimeout);
  return std::make_unique<Client>(std::move(channel));
}

std::unique_ptr<facebook::fboss::FbossCtrlAsyncClient> createAgentClient(
    const std::string& ip);

std::unique_ptr<facebook::fboss::QsfpServiceAsyncClient> createQsfpClient(
    const std::string& ip);
} // namespace facebook::fboss::utils

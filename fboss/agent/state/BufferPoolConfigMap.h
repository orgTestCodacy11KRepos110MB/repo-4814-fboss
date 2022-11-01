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

#include "fboss/agent/gen-cpp2/switch_state_types.h"
#include "fboss/agent/state/BufferPoolConfig.h"
#include "fboss/agent/state/NodeMap.h"
#include "fboss/agent/state/Thrifty.h"
#include "fboss/agent/types.h"

namespace facebook::fboss {

struct BufferPoolCfgMapThriftTraits
    : public ThriftyNodeMapTraits<std::string, state::BufferPoolFields> {
  static inline const std::string& getThriftKeyName() {
    static const std::string _key = "id";
    return _key;
  }
  static const KeyType parseKey(const folly::dynamic& key) {
    return key.asString();
  }
};

using BufferPoolCfgMapTypeClass = apache::thrift::type_class::map<
    apache::thrift::type_class::string,
    apache::thrift::type_class::structure>;
using BufferPoolCfgMapThriftType =
    std::map<std::string, state::BufferPoolFields>;

using BufferPoolCfgMapTraits = thrift_cow::
    ThriftMapTraits<BufferPoolCfgMapTypeClass, BufferPoolCfgMapThriftType>;
ADD_THRIFT_MAP_RESOLVER_MAPPING(BufferPoolCfgMapTraits, BufferPoolCfgMap);

/*
 * A container for the set of collectors.
 */
class BufferPoolCfgMap
    : public ThriftMapNode<BufferPoolCfgMap, BufferPoolCfgMapTraits> {
 public:
  using Base = ThriftMapNode<BufferPoolCfgMap, BufferPoolCfgMapTraits>;
  BufferPoolCfgMap() = default;
  virtual ~BufferPoolCfgMap() override = default;

 private:
  // Inherit the constructors required for clone()
  using Base::Base;
  friend class CloneAllocator;
};

} // namespace facebook::fboss

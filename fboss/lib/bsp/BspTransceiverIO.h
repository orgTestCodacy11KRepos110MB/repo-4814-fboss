// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include "fboss/lib/bsp/gen-cpp2/bsp_platform_mapping_types.h"
#include "fboss/lib/i2c/I2cController.h"
#include "fboss/lib/i2c/I2cDevIo.h"
#include "fboss/lib/usb/TransceiverI2CApi.h"

namespace facebook {
namespace fboss {

class BspTransceiverIOError : public I2cError {
 public:
  explicit BspTransceiverIOError(const std::string& what) : I2cError(what) {}
};

class BspTransceiverIO : public I2cController {
 public:
  BspTransceiverIO(uint32_t tcvr, BspTransceiverMapping& tcvrMapping);
  ~BspTransceiverIO() {}
  void read(uint8_t addr, uint8_t offset, uint8_t* buf, int len);
  void write(uint8_t addr, uint8_t offset, const uint8_t* buf, int len);

 private:
  std::unique_ptr<I2cDevIo> i2cDev_;
  BspTransceiverMapping tcvrMapping_;
  int tcvrID_;
};

} // namespace fboss
} // namespace facebook

// Copyright 2004-present Facebook. All Rights Reserved.

#include "fboss/agent/hw/sai/api/PortApi.h"

extern "C" {
#include <sai.h>

#include <experimental/sai_attr_ext.h>
}

namespace facebook::fboss {

std::optional<sai_attr_id_t>
SaiPortSerdesTraits::Attributes::AttributeRxCtleCodeIdWrapper::operator()() {
  return SAI_PORT_SERDES_ATTR_EXT_RX_CTLE_CODE;
}
std::optional<sai_attr_id_t>
SaiPortSerdesTraits::Attributes::AttributeRxDspModeIdWrapper::operator()() {
  return SAI_PORT_SERDES_ATTR_EXT_RX_DSP_MODE;
}
std::optional<sai_attr_id_t>
SaiPortSerdesTraits::Attributes::AttributeRxAfeTrimIdWrapper::operator()() {
  return SAI_PORT_SERDES_ATTR_EXT_RX_AFE_TRIM;
}
std::optional<sai_attr_id_t> SaiPortSerdesTraits::Attributes::
    AttributeRxAcCouplingBypassIdWrapper::operator()() {
  return SAI_PORT_SERDES_ATTR_EXT_RX_AC_COUPLING_BYPASS;
}
std::optional<sai_attr_id_t>
SaiPortTraits::Attributes::AttributeSystemPortId::operator()() {
#if !defined(TAJO_SDK_VERSION_1_38_0)
  return SAI_PORT_ATTR_EXT_SYSTEM_PORT_ID;
#else
  return std::nullopt;
#endif
}
} // namespace facebook::fboss

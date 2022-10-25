namespace cpp2 facebook.fboss.state
namespace go neteng.fboss.qsfp_state
namespace py neteng.fboss.qsfp_state
namespace py3 neteng.fboss
namespace py.asyncio neteng.fboss.asyncio.qsfp_state

include "fboss/lib/phy/phy.thrift"
include "fboss/qsfp_service/if/qsfp_service_config.thrift"
include "fboss/qsfp_service/if/transceiver.thrift"

struct QsfpState {
  1: map<string, phy.PhyState> phyStates;
  2: map<i32, transceiver.TcvrState> tcvrStates;
}

struct QsfpServiceData {
  1: qsfp_service_config.QsfpServiceConfig config;
  2: QsfpState state;
}

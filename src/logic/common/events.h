#ifndef RDP_EXT_LOGIC_EVENTS_H
#define RDP_EXT_LOGIC_EVENTS_H

#include "message.h"
#include "cpu_sample.h"

namespace rdp_ext::logic {

struct TransportConnectedEvent {};

struct TransportDisconnectedEvent {};

struct MessageReceivedEvent {
    protocol::Message message;
};

struct CpuSampleReadyEvent {
    telemetry::CpuSample sample;
};

struct PingRequestEvent {};

} // namespace rdp_ext::logic

#endif //RDP_EXT_LOGIC_EVENTS_H
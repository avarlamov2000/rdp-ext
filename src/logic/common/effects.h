#ifndef RDP_EXT_LOGIC_EFFECTS_H
#define RDP_EXT_LOGIC_EFFECTS_H

#include "message.h"

namespace rdp_ext::logic {

struct SendMessageEffect {
    protocol::Message message;
};

struct StartTelemetryTimerEffect {};
struct StopTelemetryTimerEffect {};

struct StartPingEffect {};
struct StopPingEffect {};

struct CloseTransportEffect {};

} // namespace rdp_ext::logic

#endif //RDP_EXT_LOGIC_EFFECTS_H
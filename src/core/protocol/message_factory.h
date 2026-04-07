#ifndef RDP_EXT_MESSAGE_FACTORY_H
#define RDP_EXT_MESSAGE_FACTORY_H

#include "message.h"

namespace rdp_ext::protocol {

class MessageFactory {
public:
    static Message makeHello(InstanceId instance_id);
    static Message makeCapabilitySet(bool is_ping_supported, bool is_telemetry_supported);
    static Message makeTelemetryStart();
    static Message makeCpuSample(float cpu_load);
    static Message makePing(uint32_t id);
    static Message makePong(uint32_t id);

    static Message makeInternalError(std::string_view message);
    static Message makeWrongProtocolVersionError(uint16_t version);
    static Message makeHandshakeRequiredError();
    static Message makeInvalidStateError(std::string_view message);
    static Message makeUnsupportedMessageTypeError(std::string_view message_type);

private:
    static Message makeError(ErrorCode code, std::string&& message);

};

} // namespace rdp_ext::protocol

#endif //RDP_EXT_MESSAGE_FACTORY_H
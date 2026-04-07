#ifndef RDP_EXT_PROTOCOL_MOCK_DATA_H
#define RDP_EXT_PROTOCOL_MOCK_DATA_H

#include "protocol/message.h"

namespace rdp_ext::protocol::test {

inline HelloMessage makeTestHelloMessage() {
    return HelloMessage{
        .protocol_version = PROTOCOL_VERSION,
        .instance_id = InstanceId{0x1234567890123456, 0x87654321},
    };
}

inline ErrorMessage makeTestErrorMessage() {
    return ErrorMessage{
        .code = ErrorCode::InternalError,
        .message = "test_error_message",
    };
}

inline CapabilitySetMessage makeTestCapabilitySetMessage() {
    return CapabilitySetMessage{
        .is_ping_supported = true,
        .is_telemetry_supported = false,
    };
}

inline TelemetryStartMessage makeTestTelemetryStartMessage() {
    return TelemetryStartMessage{};
}

inline CpuSampleMessage makeTestCpuSampleMessage() {
    return CpuSampleMessage{
        .cpu_load = 0.55,
    };
}

inline PingMessage makeTestPingMessage() {
    return PingMessage{
        .id = 0x12345678,
    };
}

inline PongMessage makeTestPongMessage() {
    return PongMessage{
        .id = 0x87654321,
    };
}

} // namespace rdp_ext::protocol::test

#endif //RDP_EXT_PROTOCOL_MOCK_DATA_H
#include "message_factory.h"

#include <format>

namespace rdp_ext::protocol {

Message MessageFactory::makeHello(InstanceId instance_id) {
    return HelloMessage {
        .protocol_version = PROTOCOL_VERSION,
        .instance_id = instance_id,
    };
}

Message MessageFactory::makeCapabilitySet(bool is_ping_supported, bool is_telemetry_supported) {
    return CapabilitySetMessage {
        .is_ping_supported = is_ping_supported,
        .is_telemetry_supported = is_telemetry_supported,
    };
}

Message MessageFactory::makeTelemetryStart() {
    return TelemetryStartMessage{};
}

Message MessageFactory::makeCpuSample(float cpu_load) {
    return CpuSampleMessage {
        .cpu_load = cpu_load,
    };
}

Message MessageFactory::makePing(uint32_t id) {
    return PingMessage {
        .id = id,
    };
}

Message MessageFactory::makePong(uint32_t id) {
    return PongMessage {
        .id = id,
    };
}

Message MessageFactory::makeInternalError(std::string_view message) {
    return makeError(ErrorCode::InternalError, std::move(std::format("Internal error: {}", message)));
}

Message MessageFactory::makeWrongProtocolVersionError(uint16_t version) {
    return makeError(ErrorCode::WrongProtocolVersion, std::move(std::format("Unsupported protocol version, required version 0x{:04X}", version)));
}

Message MessageFactory::makeHandshakeRequiredError() {
    return makeError(ErrorCode::HandshakeRequired, "No handshake was made");
}

Message MessageFactory::makeInvalidStateError(std::string_view message) {
    return makeError(ErrorCode::InternalError, std::move(std::format("Invalid state: {}", message)));
}

Message MessageFactory::makeUnsupportedMessageTypeError(std::string_view message_type) {
    return makeError(ErrorCode::UnsupportedMessageType, std::move(std::format("UnsupportedMessageType: {}", message_type)));
}

Message MessageFactory::makeError(ErrorCode code, std::string&& message) {
    return ErrorMessage {
        .code = code,
        .message = std::move(message),
    };
}

} // namespace rdp_ext::protocol

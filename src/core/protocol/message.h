#ifndef RDP_EXT_MESSAGE_H
#define RDP_EXT_MESSAGE_H

#include <cstdint>
#include <string>
#include <variant>

namespace rdp_ext::protocol {

constexpr uint16_t PROTOCOL_VERSION = 0x0103;

enum class ErrorCode: uint16_t {
    Unknown = 0,
    InternalError = 1,
    WrongProtocolVersion = 2,
    HandshakeRequired = 3,
    InvalidState = 4,
    UnsupportedMessageType = 5,
};

struct InstanceId {
    uint64_t timestamp{};
    uint32_t id{};

    bool operator==(const InstanceId& right) const = default;
};

struct HelloMessage {
    uint16_t protocol_version{PROTOCOL_VERSION};
    InstanceId instance_id{};

    bool operator==(const HelloMessage& right) const = default;
};

struct ErrorMessage {
    ErrorCode code{};
    std::string message;

    bool operator==(const ErrorMessage& right) const = default;
};

struct CapabilitySetMessage {
    bool is_ping_supported{};
    bool is_telemetry_supported{};

    bool operator==(const CapabilitySetMessage& right) const = default;
};

struct TelemetryStartMessage {
    bool operator==(const TelemetryStartMessage& right) const = default;
};

struct CpuSampleMessage {
    float cpu_load{};

    bool operator==(const CpuSampleMessage& right) const = default;
};

struct PingMessage {
    uint32_t id{};

    bool operator==(const PingMessage& right) const = default;
};

struct PongMessage {
    uint32_t id{};

    bool operator==(const PongMessage& right) const = default;
};

using Message = std::variant<
    ErrorMessage,
    HelloMessage,
    CapabilitySetMessage,
    TelemetryStartMessage,
    CpuSampleMessage,
    PingMessage,
    PongMessage
>;

struct Envelope {
    Message message;
};

} // namespace rdp_ext::protocol

#endif // RDP_EXT_MESSAGE_H
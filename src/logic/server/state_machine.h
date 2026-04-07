#ifndef RDP_EXT_SERVER_STATE_MACHINE_H
#define RDP_EXT_SERVER_STATE_MACHINE_H

#include "common/states.h"
#include "common/events.h"
#include "common/effects.h"
#include "logger.h"

#include <vector>

namespace rdp_ext::logic::server {

struct ServerState {
    ConnectionState connection_state{ConnectionState::Disconnected};
    HandshakeState handshake_state{};
    protocol::InstanceId instance_id{};
    bool telemetry_enabled{false};
};

using ServerEvent = std::variant<
    TransportConnectedEvent,
    TransportDisconnectedEvent,
    MessageReceivedEvent,
    CpuSampleReadyEvent
>;

using ServerEffect = std::variant<
    SendMessageEffect,
    StartTelemetryTimerEffect,
    StopTelemetryTimerEffect,
    CloseTransportEffect
>;

using ServerEffects = std::vector<ServerEffect>;

class ServerStateMachine {
public:
    explicit ServerStateMachine(logging::LogSink& log_sink);

    ServerEffects doStep(const ServerEvent& event);

private:
    ServerEffects handleEvent(const TransportConnectedEvent& event);
    ServerEffects handleEvent(const TransportDisconnectedEvent& event);
    ServerEffects handleEvent(const CpuSampleReadyEvent& event);
    ServerEffects handleEvent(const MessageReceivedEvent& event);

    ServerEffects handleMessage(const protocol::ErrorMessage& message);
    ServerEffects handleMessage(const protocol::HelloMessage& message);
    static ServerEffects handleMessage(const protocol::CapabilitySetMessage& message);
    ServerEffects handleMessage(const protocol::TelemetryStartMessage& message);
    static ServerEffects handleMessage(const protocol::CpuSampleMessage& message);
    static ServerEffects handleMessage(const protocol::PingMessage& message);
    static ServerEffects handleMessage(const protocol::PongMessage& message);

    static protocol::InstanceId generateRandomInstanceId();
    static ServerEffects makeSingleMessageEffect(protocol::Message&& message);

    ServerState state_{};
    logging::Logger log_;
};

} // namespace rdp_ext::logic::server

#endif //RDP_EXT_SERVER_STATE_MACHINE_H
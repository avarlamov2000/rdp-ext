#ifndef RDP_EXT_SERVER_STATE_MACHINE_H
#define RDP_EXT_SERVER_STATE_MACHINE_H

#include "common/states.h"
#include "common/events.h"
#include "common/effects.h"
#include "logger.h"

#include <vector>

namespace rdp_ext::logic::client {

struct ClientState {
    ConnectionState connection_state{ConnectionState::Disconnected};
    HandshakeState handshake_state{};
    protocol::InstanceId instance_id{};
    bool telemetry_requested{false};
};

using ClientEvent = std::variant<
    TransportConnectedEvent,
    TransportDisconnectedEvent,
    MessageReceivedEvent,
    PingRequestEvent
>;

using ClientEffect = std::variant<
    SendMessageEffect,
    StartPingEffect,
    StopPingEffect,
    CloseTransportEffect
>;

using ClientEffects = std::vector<ClientEffect>;

class ClientStateMachine {
public:
    explicit ClientStateMachine(logging::LogSink& log_sink);

    ClientEffects doStep(const ClientEvent& event);

private:
    ClientEffects handleEvent(const TransportConnectedEvent& event);
    ClientEffects handleEvent(const TransportDisconnectedEvent& event);
    ClientEffects handleEvent(const PingRequestEvent& event);
    ClientEffects handleEvent(const MessageReceivedEvent& event);

    ClientEffects handleMessage(const protocol::ErrorMessage& message);
    ClientEffects handleMessage(const protocol::HelloMessage& message);
    static ClientEffects handleMessage(const protocol::CapabilitySetMessage& message);
    static ClientEffects handleMessage(const protocol::TelemetryStartMessage& message);
    ClientEffects handleMessage(const protocol::CpuSampleMessage& message);
    static ClientEffects handleMessage(const protocol::PingMessage& message);
    static ClientEffects handleMessage(const protocol::PongMessage& message);

    static protocol::InstanceId generateRandomInstanceId();
    static ClientEffects makeSingleMessageEffect(protocol::Message&& message);

    ClientState state_{};
    logging::Logger log_;
};

} // namespace rdp_ext::logic::client

#endif //RDP_EXT_SERVER_STATE_MACHINE_H
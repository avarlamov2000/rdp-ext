#include "state_machine.h"
#include "message_factory.h"
#include "common/helpers.h"

#include <chrono>

namespace rdp_ext::logic::server {
ServerStateMachine::ServerStateMachine(logging::LogSink& log_sink)
    : state_{}, log_{log_sink, "server-state-machine"} {}

ServerEffects ServerStateMachine::doStep(const ServerEvent &event) {
    return std::visit([this](const auto& concrete_event) -> ServerEffects {
        return handleEvent(concrete_event);
    }, event);
}

ServerEffects ServerStateMachine::handleEvent(const TransportConnectedEvent& event) {
    log_.info("Transport connected. Sending Hello message");
    state_.connection_state = ConnectionState::AwaitingHandshake;
    state_.telemetry_enabled = false;
    state_.instance_id = generateRandomInstanceId();
    state_.handshake_state.reset();
    state_.handshake_state.localHelloSent();

    return ServerEffects{
        StopTelemetryTimerEffect{},
        SendMessageEffect{
            .message = protocol::MessageFactory::makeHello(state_.instance_id)
        }
    };
}

ServerEffects ServerStateMachine::handleEvent(const TransportDisconnectedEvent& event) {
    log_.info("Transport disconnected");
    state_.connection_state = ConnectionState::Disconnected;
    state_.telemetry_enabled = false;
    state_.handshake_state.reset();

    return ServerEffects{
        StopTelemetryTimerEffect{}
    };
}

ServerEffects ServerStateMachine::handleEvent(const CpuSampleReadyEvent& event) {
    if (!state_.telemetry_enabled || !state_.handshake_state.isCompleted())
        return {};

    return makeSingleMessageEffect(
        protocol::MessageFactory::makeCpuSample(event.sample.cpu_load));
}

    ServerEffects ServerStateMachine::handleEvent(const MessageReceivedEvent& event) {
    return std::visit([this](const auto& concrete_message) -> ServerEffects {
        return handleMessage(concrete_message);
    }, event.message);
}

ServerEffects ServerStateMachine::handleMessage(const protocol::ErrorMessage& message) {
    log_.error(std::format("error message received. Code: {}, message: {}", static_cast<uint16_t>(message.code), message.message));
    return {};
}

ServerEffects ServerStateMachine::handleMessage(const protocol::HelloMessage& message) {
    log_.info("Hello message arrived");
    if (message.protocol_version != protocol::PROTOCOL_VERSION) {
        state_.handshake_state.reset();
        log_.error("Handshake failed: unsupported protocol version. Client protocol version 0x{:04X}, required version 0x{:04X} ");
        return makeSingleMessageEffect(
            protocol::MessageFactory::makeWrongProtocolVersionError(protocol::PROTOCOL_VERSION));
    }

    ServerEffects result;
    state_.handshake_state.remoteHelloReceived(message.instance_id);
    if (state_.handshake_state.isLocalHelloNeeded()) {
        state_.handshake_state.localHelloSent();
        log_.info("Sending Hello message");
        result.emplace_back(SendMessageEffect{
            .message = protocol::MessageFactory::makeHello(state_.instance_id)
        });
    }
    if (state_.handshake_state.isCompleted()) {
        log_.info("Handshake completed");
    } else {
        state_.telemetry_enabled = false;
        result.emplace_back(StopTelemetryTimerEffect{});
    }
    return result;
}

ServerEffects ServerStateMachine::handleMessage(const protocol::CapabilitySetMessage& message) {
    return {};
}

ServerEffects ServerStateMachine::handleMessage(const protocol::TelemetryStartMessage& message) {
    log_.info("Start sending CPU load");
    state_.telemetry_enabled = true;
    return ServerEffects{
        StartTelemetryTimerEffect{}
    };
}

ServerEffects ServerStateMachine::handleMessage(const protocol::CpuSampleMessage& message) {
    return makeSingleMessageEffect(protocol::MessageFactory::makeUnsupportedMessageTypeError("CpuSampleMessage"));
}

ServerEffects ServerStateMachine::handleMessage(const protocol::PingMessage& message) {
    return makeSingleMessageEffect(protocol::MessageFactory::makePong(message.id));
}

ServerEffects ServerStateMachine::handleMessage(const protocol::PongMessage& message) {
    return makeSingleMessageEffect(protocol::MessageFactory::makeUnsupportedMessageTypeError("PongMessage"));
}

protocol::InstanceId ServerStateMachine::generateRandomInstanceId() {
    return protocol::InstanceId {
        .timestamp = static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count()),
        .id = generateRandomValue(),
    };
}

ServerEffects ServerStateMachine::makeSingleMessageEffect(protocol::Message&& message) {
    ServerEffects result;
    result.emplace_back(SendMessageEffect{
        .message = std::move(message)
    });
    return result;
}

} // namespace rdp_ext::logic::server

#include "state_machine.h"
#include "message_factory.h"
#include "common/helpers.h"

#include <chrono>
#include <random>

namespace rdp_ext::logic::client {
ClientStateMachine::ClientStateMachine(logging::LogSink& log_sink)
    : state_{}, log_{log_sink, "server-state-machine"} {}

ClientEffects ClientStateMachine::doStep(const ClientEvent &event) {
    return std::visit([this](const auto& concrete_event) -> ClientEffects {
        return handleEvent(concrete_event);
    }, event);
}

ClientEffects ClientStateMachine::handleEvent(const TransportConnectedEvent& event) {
    log_.info("Transport connected. Sending Hello message");
    state_.connection_state = ConnectionState::AwaitingHandshake;
    state_.telemetry_requested = false;
    state_.instance_id = generateRandomInstanceId();
    state_.handshake_state.reset();
    state_.handshake_state.localHelloSent();

    return ClientEffects{
        StopPingEffect{},
        SendMessageEffect{
            .message = protocol::MessageFactory::makeHello(state_.instance_id)
        }
    };
}

ClientEffects ClientStateMachine::handleEvent(const TransportDisconnectedEvent& event) {
    log_.info("Transport disconnected");
    state_.connection_state = ConnectionState::Disconnected;
    state_.telemetry_requested = false;
    state_.handshake_state.reset();

    return ClientEffects{
        StopPingEffect{}
    };
}

ClientEffects ClientStateMachine::handleEvent(const PingRequestEvent& event) {
    if (!state_.handshake_state.isCompleted())
        return {};

    return makeSingleMessageEffect(
        protocol::MessageFactory::makePing(generateRandomValue()));
}

ClientEffects ClientStateMachine::handleEvent(const MessageReceivedEvent& event) {
    return std::visit([this](const auto& concrete_message) -> ClientEffects {
        return handleMessage(concrete_message);
    }, event.message);
}

ClientEffects ClientStateMachine::handleMessage(const protocol::ErrorMessage& message) {
    log_.error(std::format("error message received. Code: {}, message: {}", static_cast<uint16_t>(message.code), message.message));
    return {};
}

ClientEffects ClientStateMachine::handleMessage(const protocol::HelloMessage& message) {
    log_.info("Hello message arrived");
    if (message.protocol_version != protocol::PROTOCOL_VERSION) {
        state_.handshake_state.reset();
        log_.error("Handshake failed: unsupported protocol version. Client protocol version 0x{:04X}, required version 0x{:04X} ");
        return makeSingleMessageEffect(
            protocol::MessageFactory::makeWrongProtocolVersionError(protocol::PROTOCOL_VERSION));
    }

    ClientEffects result;
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
        if (!state_.telemetry_requested) {
            log_.info("Requesting CPU load messages");
            state_.telemetry_requested = true;
            result.emplace_back(SendMessageEffect{
                .message = protocol::MessageFactory::makeTelemetryStart()
            });
        }
    } else {
        state_.telemetry_requested = false;
    }
    return result;
}

ClientEffects ClientStateMachine::handleMessage(const protocol::CapabilitySetMessage& message) {
    return makeSingleMessageEffect(protocol::MessageFactory::makeUnsupportedMessageTypeError("CapabilitySetMessage"));
}

ClientEffects ClientStateMachine::handleMessage(const protocol::TelemetryStartMessage& message) {
    return makeSingleMessageEffect(protocol::MessageFactory::makeUnsupportedMessageTypeError("TelemetryStartMessage"));
}

ClientEffects ClientStateMachine::handleMessage(const protocol::CpuSampleMessage& message) {
    log_.info(std::format("Server CPU load {:.1f}%", message.cpu_load * 100));
    return {};
}

ClientEffects ClientStateMachine::handleMessage(const protocol::PingMessage& message) {
    return makeSingleMessageEffect(protocol::MessageFactory::makeUnsupportedMessageTypeError("PingMessage"));
}

ClientEffects ClientStateMachine::handleMessage(const protocol::PongMessage& message) {
    return {};
}

protocol::InstanceId ClientStateMachine::generateRandomInstanceId() {
    return protocol::InstanceId {
        .timestamp = static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count()),
        .id = generateRandomValue(),
    };
}

ClientEffects ClientStateMachine::makeSingleMessageEffect(protocol::Message&& message) {
    ClientEffects result;
    result.emplace_back(SendMessageEffect{
        .message = std::move(message)
    });
    return result;
}

} // namespace rdp_ext::logic::client

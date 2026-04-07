#ifndef RDP_EXT_SERVER_CONTROLLER_H
#define RDP_EXT_SERVER_CONTROLLER_H

#include "common/effects.h"
#include "cpu_sample.h"
#include "state_machine.h"
#include "logger.h"
#include "serializer.h"

#include <functional>

namespace rdp_ext::logic::server {

template <typename T>
concept TransportType = requires(T transport, protocol::Buffer bytes,
    transport::ConnectedHandler&& connected_handler,
    transport::DisconnectedHandler&& disconnected_handler,
    transport::BytesHandler&& bytes_handler
) {
    { transport.close() } -> std::same_as<void>;
    { transport.send(std::move(bytes)) } -> std::same_as<void>;
    { transport.setConnectedHandler(connected_handler) } -> std::same_as<void>;
    { transport.setDisconnectedHandler(disconnected_handler) } -> std::same_as<void>;
    { transport.setBytesHandler(bytes_handler) } -> std::same_as<void>;
};

template <typename T>
concept TelemetryType = requires(T telemetry, std::function<void(telemetry::CpuSample)> handler) {
    { telemetry.start() } -> std::same_as<void>;
    { telemetry.stop() } -> std::same_as<void>;
    { telemetry.setSampleHandler(handler) } -> std::same_as<void>;
};

template <typename T>
concept ExecutorType = requires(T executor, std::function<void()> task) {
    { executor.post(std::move(task)) } -> std::same_as<void>;
};

template <TransportType Transport, TelemetryType Telemetry, ExecutorType Executor>
class Controller {
public:
    explicit Controller(Transport& transport, Telemetry& telemetry, Executor& executor, logging::LogSink& log_sink)
        : transport_{transport}, telemetry_{telemetry}, executor_{executor}, state_machine_{log_sink}, serializer_{}, log_{log_sink, "server-controller"}
    {
        linkHandlers();
    }

private:

    void linkHandlers() {
        telemetry_.setSampleHandler([this](telemetry::CpuSample sample) {
            executor_.post([this, sample]() {
                step(CpuSampleReadyEvent{sample});
            });
        });

        transport_.setConnectedHandler([this]() {
            executor_.post([this]() {
                step(TransportConnectedEvent{});
            });
        });

        transport_.setDisconnectedHandler([this]() {
            executor_.post([this]() {
                step(TransportDisconnectedEvent{});
            });
        });

        transport_.setBytesHandler([this](protocol::Buffer&& bytes) {
            executor_.post([this, bytes = std::move(bytes)]() {
                auto message = protocol::Serializer::deserialize(bytes);
                step(MessageReceivedEvent{message});
            });
        });
    }

    void step(ServerEvent&& event) {
        const auto effects = state_machine_.doStep(event);
        handleEffects(effects);
    }

    void handleEffects(const ServerEffects& effects) {
        for (const auto& effect: effects) {
            std::visit([this](const auto& concrete_effect) {
                handleEffect(concrete_effect);
            }, effect);
        }
    }

    void handleEffect(const SendMessageEffect& effect) {
        auto bytes = protocol::Serializer::serialize(effect.message);
        executor_.post([this, bytes = std::move(bytes)]() mutable {
            transport_.send(std::move(bytes));
        });
    }

    void handleEffect(const StartTelemetryTimerEffect& effect) {
        telemetry_.start();
    }

    void handleEffect(const StopTelemetryTimerEffect& effect) {
        telemetry_.stop();
    }

    void handleEffect(const CloseTransportEffect& effect) {
        executor_.post([this]() {
            transport_.close();
        });
    }

    Transport& transport_;
    Telemetry& telemetry_;
    Executor& executor_;
    ServerStateMachine state_machine_;
    protocol::Serializer serializer_;
    logging::Logger log_;
};

} // namespace rdp_ext::logic::server

#endif //RDP_EXT_SERVER_CONTROLLER_H
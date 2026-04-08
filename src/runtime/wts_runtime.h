#ifndef RDP_EXT_WTS_RUNTIME_H
#define RDP_EXT_WTS_RUNTIME_H

#include "defaults.h"
#include "logger.h"
#include "wts_transport.h"
#include "executor.h"
#include "server/controller.h"
#include "timer_control.h"

#include <string_view>

namespace rdp_ext::runtime {

class WtsRuntime {
public:
    explicit WtsRuntime(std::string_view channel_name, logging::LogSink& log_sink);

    ~WtsRuntime();
    WtsRuntime(const WtsRuntime&) = delete;
    WtsRuntime& operator=(const WtsRuntime&) = delete;
    WtsRuntime(WtsRuntime&&) = delete;
    WtsRuntime& operator=(WtsRuntime&&) noexcept = delete;

    void run();

private:
    void start();
    void stop();

    logging::LogSink& log_sink_;
    transport::WtsTransport transport_;
    Executor executor_;
    telemetry::TimerControl<Executor> telemetry_;
    logic::server::Controller<transport::WtsTransport, telemetry::TimerControl<Executor>, Executor> controller_;
};

} // namespace rdp_ext::runtime

#endif //RDP_EXT_WTS_RUNTIME_H
#include "wts_runtime.h"

namespace rdp_ext::runtime {

WtsRuntime::WtsRuntime(std::string_view channel_name, logging::LogSink& log_sink)
    : log_sink_{log_sink}
    , transport_{channel_name, log_sink_}
    , executor_{log_sink_}
    , telemetry_{executor_}
    , controller_{transport_, telemetry_, executor_, log_sink_}
{}

WtsRuntime::~WtsRuntime() {
    stop();
}

void WtsRuntime::run() {
    start();
    transport_.run();
    stop();
}

void WtsRuntime::start() {
    executor_.start();
}

void WtsRuntime::stop() {
    transport_.stop();
    executor_.stop();
}

} // namespace rdp_ext::runtime

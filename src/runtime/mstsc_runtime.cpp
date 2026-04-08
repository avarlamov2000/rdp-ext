#include "mstsc_runtime.h"

namespace rdp_ext::runtime {

MstscRuntime::MstscRuntime(std::string_view channel_name, logging::LogSink& log_sink)
    : log_sink_{log_sink}
    , transport_{channel_name, log_sink_}
    , executor_{log_sink_}
    , controller_{transport_, executor_, log_sink_}
{}

MstscRuntime::~MstscRuntime() {
    stop();
}

BOOL MstscRuntime::virtualChannelEntry(PCHANNEL_ENTRY_POINTS entryPoints) {
    start();
    return transport_.virtualChannelEntry(entryPoints);
}

void MstscRuntime::start() {
    executor_.start();
}

void MstscRuntime::stop() {
    executor_.stop();
}

#ifndef _WIN32
BOOL MstscRuntime::virtualChannelEntryEx(PCHANNEL_ENTRY_POINTS_FREERDP_EX entryPoints, PVOID initHandle) {
    start();
    return transport_.virtualChannelEntryEx(entryPoints, initHandle);
}
#endif

} // namespace rdp_ext::runtime

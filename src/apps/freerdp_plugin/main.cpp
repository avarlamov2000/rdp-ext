#include "mstsc_runtime.h"
#include "mstsc_transport.h"

using namespace rdp_ext::logging;
using namespace rdp_ext::runtime;

constexpr auto DEFAULT_LOG_LEVEL = LogLevel::Debug;

LogSink& getLogSinkInstance() {
    static LogSink sink{DEFAULT_LOG_LEVEL, LogOutput::Console};
    return sink;
}

MstscRuntime& getRuntimeInstance() {
    static MstscRuntime transport{DEFAULT_CHANNEL_NAME, getLogSinkInstance()};
    return transport;
}

extern "C" FREERDP_ENTRY_POINT(BOOL VCAPITYPE VirtualChannelEntryEx(PCHANNEL_ENTRY_POINTS_FREERDP_EX pEntryPoints, PVOID pInitHandle)) {
    return getRuntimeInstance().virtualChannelEntryEx(pEntryPoints, pInitHandle);
}
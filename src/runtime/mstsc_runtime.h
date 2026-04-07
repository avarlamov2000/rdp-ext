#ifndef RDP_EXT_MSTSC_RUNTIME_H
#define RDP_EXT_MSTSC_RUNTIME_H

#include "defaults.h"
#include "logger.h"
#include "mstsc_transport.h"
#include "executor.h"
#include "client/controller.h"

namespace rdp_ext::runtime {

class MstscRuntime {
public:
    explicit MstscRuntime(std::string_view channel_name, logging::LogSink& log_sink);

    BOOL virtualChannelEntry(PCHANNEL_ENTRY_POINTS entryPoints);
#ifndef _WIN32
    BOOL virtualChannelEntryEx(PCHANNEL_ENTRY_POINTS_FREERDP_EX entryPoints, PVOID initHandle);
#endif

    void run();

private:
    logging::LogSink& log_sink_;
    transport::MstscTransport transport_;
    Executor executor_;
    logic::client::Controller<transport::MstscTransport, Executor> controller_;
};

} // namespace rdp_ext::runtime

#endif //RDP_EXT_MSTSC_RUNTIME_H
#include "wts_runtime.h"

#include <string>

using namespace rdp_ext::logging;
using namespace rdp_ext::runtime;

constexpr auto DEFAULT_LOG_LEVEL = LogLevel::Debug;

// TODO: Разобраться с тем, что сообщения не доходят до RDP клиента
int main(int argc, char* argv[]) {
    const std::string channel_name = (argc > 1) ? argv[1] : DEFAULT_CHANNEL_NAME;
    LogSink log_sink{DEFAULT_LOG_LEVEL, LogOutput::Console};
    WtsRuntime runtime{channel_name, log_sink};
    runtime.run();
    return 0;
}

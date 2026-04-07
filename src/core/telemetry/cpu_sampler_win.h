#ifndef RDP_EXT_CPU_SAMPLER_WIN_H
#define RDP_EXT_CPU_SAMPLER_WIN_H

#include "cpu_sample.h"

#include <optional>
#include <windows.h>

namespace rdp_ext::telemetry {

class CpuSampler {
public:

    void reset();
    CpuSample readSample();

private:
    struct RawTimes {
        std::uint64_t idle = 0;
        std::uint64_t kernel = 0;
        std::uint64_t user = 0;
    };

    static RawTimes readRawTimes();
    static std::uint64_t fileTimeToUint64(const FILETIME& ft);

    std::optional<RawTimes> previous_;
};

} // namespace rdp_ext::telemetry


#endif //RDP_EXT_CPU_SAMPLER_WIN_H
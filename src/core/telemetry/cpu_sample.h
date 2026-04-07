#ifndef RDP_EXT_CPU_SAMPLER_H
#define RDP_EXT_CPU_SAMPLER_H

#include <cstdint>

namespace rdp_ext::telemetry {

struct CpuSample {
    float cpu_load;
};

} // namespace rdp_ext::telemetry

#endif //RDP_EXT_CPU_SAMPLER_H
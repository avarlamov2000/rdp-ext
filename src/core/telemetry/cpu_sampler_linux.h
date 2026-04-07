#ifndef RDP_EXT_CPU_SAMPLER_LINUX_H
#define RDP_EXT_CPU_SAMPLER_LINUX_H

#include "cpu_sample.h"

namespace rdp_ext::telemetry {

class CpuSampler {
public:
    explicit CpuSampler();

    void reset();
    CpuSample readSample();
};

} // namespace rdp_ext::telemetry

#endif //RDP_EXT_CPU_SAMPLER_LINUX_H
#include "cpu_sampler_linux.h"

#include <cpu_sample.h>
#include <stdexcept>

namespace rdp_ext::telemetry {

CpuSampler::CpuSampler()
{}

void CpuSampler::reset() {
    readSample();
}


CpuSample CpuSampler::readSample() {
    return CpuSample{};
}

} // namespace rdp_ext::telemetry

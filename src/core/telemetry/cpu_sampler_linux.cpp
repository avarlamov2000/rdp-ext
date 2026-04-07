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
    /*const RawTimes current = readRawTimes();

    if (!_previous.has_value()) {
        _previous = current;
    }

    const RawTimes previous = *_previous;
    _previous = current;

    const std::uint64_t deltaIdle = current.idle - previous.idle;
    const std::uint64_t deltaKernel = current.kernel - previous.kernel;
    const std::uint64_t deltaUser = current.user - previous.user;

    const std::uint64_t deltaTotal = deltaKernel + deltaUser;
    if (deltaTotal == 0) {
        return CpuSample{.cpu_load = 0.0};
    }

    const std::uint64_t deltaBusy = deltaTotal - deltaIdle;
    const float usage = static_cast<float>(deltaBusy) / static_cast<float>(deltaTotal);

    return CpuSample{.cpu_load = usage};*/
}

} // namespace rdp_ext::telemetry

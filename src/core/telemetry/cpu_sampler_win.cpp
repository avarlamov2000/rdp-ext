#include "cpu_sampler_win.h"

#include <cpu_sample.h>
#include <stdexcept>

namespace rdp_ext::telemetry {

void CpuSampler::reset() {
    readSample();
}


std::uint64_t CpuSampler::fileTimeToUint64(const FILETIME& ft) {
    ULARGE_INTEGER value{};
    value.LowPart = ft.dwLowDateTime;
    value.HighPart = ft.dwHighDateTime;
    return value.QuadPart;
}

CpuSampler::RawTimes CpuSampler::readRawTimes() {
    FILETIME idleTime{};
    FILETIME kernelTime{};
    FILETIME userTime{};

    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        throw std::runtime_error("GetSystemTimes failed");
    }

    return RawTimes{
        .idle = fileTimeToUint64(idleTime),
        .kernel = fileTimeToUint64(kernelTime),
        .user = fileTimeToUint64(userTime)
    };
}

CpuSample CpuSampler::readSample() {
    const RawTimes current = readRawTimes();

    if (!previous_.has_value()) {
        previous_ = current;
    }

    const RawTimes previous = *previous_;
    previous_ = current;

    const std::uint64_t deltaIdle = current.idle - previous.idle;
    const std::uint64_t deltaKernel = current.kernel - previous.kernel;
    const std::uint64_t deltaUser = current.user - previous.user;

    const std::uint64_t deltaTotal = deltaKernel + deltaUser;
    if (deltaTotal == 0) {
        return CpuSample{
            .cpu_load = 0.0
        };
    }

    const std::uint64_t deltaBusy = deltaTotal - deltaIdle;
    const float usage = static_cast<float>(deltaBusy) / static_cast<float>(deltaTotal);

    return CpuSample{
        .cpu_load = usage
    };
}

} // namespace rdp_ext::telemetry

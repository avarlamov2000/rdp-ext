#ifndef RDP_EXT_TIMER_CONTROL_H
#define RDP_EXT_TIMER_CONTROL_H

#ifdef _WIN32
#include "cpu_sampler_win.h"
#else
#include "cpu_sampler_linux.h"
#endif

namespace rdp_ext::telemetry {

constexpr size_t DEFAULT_SAMPLING_INTERVAL_MS = 1000;

using SampleHandler = std::function<void(CpuSample)>;

template <typename T>
concept ExecutorType = requires(T executor, std::function<void()> task, size_t delay_ms) {
    { executor.schedule(delay_ms, std::move(task)) } -> std::same_as<void>;
};

template <ExecutorType Executor>
class TimerControl {
public:
    explicit TimerControl(Executor& executor): executor_(executor), sampler_{} {}

    void start() {
        sampler_.reset();
        const bool was_running = running_.exchange(true, std::memory_order_acq_rel);
        if (was_running) {
            return;
        }
        scheduleSample();
    }

    void stop() {
        running_.store(false, std::memory_order_release);
    }

    void scheduleSample() {
        if (!running_.load(std::memory_order_acquire))
            return;

        // TODO: Использовать weak_ptr
        executor_.schedule(DEFAULT_SAMPLING_INTERVAL_MS, [this]() {
            postSample();
        });
    }

    void postSample() {
        if (!running_.load(std::memory_order_acquire))
            return;

        if (sample_handler_) {
            sample_handler_(sampler_.readSample());
        }
        scheduleSample();
    }

    void setSampleHandler(SampleHandler sample_handler) {
        sample_handler_ = std::move(sample_handler);
    }

private:
    Executor& executor_;
    CpuSampler sampler_;
    std::atomic_bool running_{false};
    SampleHandler sample_handler_;
};

} // namespace rdp_ext::telemetry

#endif //RDP_EXT_TIMER_CONTROL_H
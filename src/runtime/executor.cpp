#include "executor.h"

#include <boost/asio/bind_executor.hpp>
#include <format>

namespace rdp_ext::runtime {

Executor::Executor(logging::LogSink& log_sink)
    : context_{}
    , strand_{boost::asio::make_strand(context_)}
    , guard_{boost::asio::make_work_guard(context_)}
    , log_{log_sink, "runtime-executor"}
{}

Executor::~Executor() {
    stop();
}

void Executor::start() {
    if (bool expected = false; !started_.compare_exchange_strong(expected, true, std::memory_order_acq_rel, std::memory_order_acquire)) {
        return;
    }

    stopping_.store(false, std::memory_order_release);

    worker_ = std::jthread([this] {
        workerLoop();
    });
}

void Executor::stop() {
    if (!started_.load(std::memory_order_acquire)) {
        return;
    }

    // Обеспечиваем идемпотентность
    if (bool expected = false; !stopping_.compare_exchange_strong(expected, true, std::memory_order_acq_rel, std::memory_order_acquire)) {
        return;
    }

    cancelAllTimers();
    guard_.reset();
    context_.stop();

    if (worker_.joinable() && worker_.get_id() != std::this_thread::get_id()) {
        worker_.join();
    }
    started_.store(false, std::memory_order_release);
}

void Executor::post(Task&& task) {
    if (stopping_.load(std::memory_order_acquire)) {
        return;
    }

    boost::asio::post(strand_, [this, task = std::move(task)]() mutable {
        if (stopping_.load(std::memory_order_acquire)) {
            return;
        }
        executeTask(std::move(task));
    });
}

void Executor::schedule(Duration delay, Task&& task) {
    if (stopping_.load(std::memory_order_acquire)) {
        return;
    }

    auto timer = std::make_shared<Timer>(context_, delay);
    if (!registerTimer(timer)) {
        return;
    }

    timer->async_wait(boost::asio::bind_executor(
        strand_, [this, timer, task = std::move(task)](const boost::system::error_code& error) mutable {

        unregisterTimer(timer);
        if (error || stopping_.load(std::memory_order_acquire)) {
            return;
        }
        executeTask(std::move(task));
    }));
}

void Executor::schedule(size_t delay_ms, Task&& task) {
    schedule(std::chrono::duration_cast<Duration>(std::chrono::milliseconds{delay_ms}), std::move(task));
}

void Executor::workerLoop() {
    for (;;) {
        try {
            context_.run();
            break;
        } catch (const std::exception& ex) {
            log_.error(std::format("Unhandled exception in executor: {}", ex.what()));
        } catch (...) {
            log_.error("Unhandled unknown exception in executor");
        }
    }
}

void Executor::executeTask(Task&& task) {
    try {
        task();
    } catch (const std::exception& ex) {
        log_.error(std::format("Execution failed with exception, message: {}", ex.what()));
    }
}

bool Executor::registerTimer(const std::shared_ptr<Timer>& timer) {
    std::lock_guard lock(timers_mutex_);

    if (stopping_.load(std::memory_order_acquire)) {
        return false;
    }

    timers_.insert(timer);
    return true;
}

void Executor::unregisterTimer(const std::shared_ptr<Timer>& timer) {
    std::lock_guard lock(timers_mutex_);
    timers_.erase(timer);
}

void Executor::cancelAllTimers() {
    std::lock_guard lock(timers_mutex_);
    for (auto& timer : timers_) {
        timer->cancel();
    }
    timers_.clear();
}

} // namespace rdp_ext::runtime
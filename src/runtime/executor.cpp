#include "executor.h"

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/steady_timer.hpp>
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

void Executor::run() {
    context_.run();
}

void Executor::stop() {
    guard_.reset();
}

void Executor::post(Task&& task) {
    boost::asio::post(strand_, [this, task = std::move(task)]() mutable {
        executeTask(std::move(task));
    });
}

void Executor::schedule(Duration delay, Task&& task) {
    auto timer = std::make_shared<boost::asio::steady_timer>(context_, delay);
    timer->async_wait(boost::asio::bind_executor(
        strand_, [this, timer, task = std::move(task)](const boost::system::error_code& error) mutable {
        if (error) {
            return;
        }
        executeTask(std::move(task));
    }));
}

void Executor::schedule(size_t delay_ms, Task&& task) {
    schedule(std::chrono::duration_cast<Duration>(std::chrono::milliseconds{delay_ms}), std::move(task));
}

void Executor::executeTask(Task&& task) {
    try {
        task();
    } catch (const std::exception& ex) {
        log_.error(std::format("Execution failed with exception, message: {}", ex.what()));
    }
}


} // namespace rdp_ext::runtime
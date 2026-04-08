#ifndef RDP_EXT_EXECUTOR_H
#define RDP_EXT_EXECUTOR_H

#include "logger.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>
#include <condition_variable>
#include <functional>
#include <thread>
#include <mutex>
#include <unordered_set>

namespace rdp_ext::runtime {

class Executor {
public:
    using Task = std::function<void()>;
    using Clock = std::chrono::steady_clock;
    using Duration = Clock::duration;
    using Timer = boost::asio::steady_timer;

    Executor(logging::LogSink& log_sink);
    ~Executor();

    Executor(const Executor&) = delete;
    Executor& operator=(const Executor&) = delete;
    Executor(Executor&&) = delete;
    Executor& operator=(Executor&&) noexcept = delete;

    void start();
    void stop();

    void post(Task&& task);

    void schedule(Duration delay, Task&& task);
    void schedule(size_t delay_ms, Task&& task);

private:
    void workerLoop();
    void executeTask(Task&& task);
    bool registerTimer(const std::shared_ptr<Timer>& timer);
    void unregisterTimer(const std::shared_ptr<Timer>& timer);
    void cancelAllTimers();

    boost::asio::io_context context_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> guard_;

    std::jthread worker_;
    std::atomic_bool started_{false};
    std::atomic_bool stopping_{false};

    std::mutex timers_mutex_;
    std::unordered_set<std::shared_ptr<Timer>> timers_;

    logging::Logger log_;
};

} // namespace rdp_ext::runtime

#endif //RDP_EXT_EXECUTOR_H
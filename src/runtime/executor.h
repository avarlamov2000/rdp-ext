#ifndef RDP_EXT_EXECUTOR_H
#define RDP_EXT_EXECUTOR_H

#include "logger.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <condition_variable>
#include <functional>

namespace rdp_ext::runtime {

class Executor {
public:

    using Task = std::function<void()>;
    using Clock = std::chrono::steady_clock;
    using Duration = Clock::duration;

    Executor(logging::LogSink& log_sink);
    ~Executor();

    Executor(const Executor&) = delete;
    Executor& operator=(const Executor&) = delete;
    Executor(Executor&&) = delete;
    Executor& operator=(Executor&&) noexcept = delete;

    void run();
    void stop();

    void post(Task&& task);
    void schedule(Duration delay, Task&& task);
    void schedule(size_t delay_ms, Task&& task);

private:
    void executeTask(Task&& task);

    boost::asio::io_context context_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> guard_;

    logging::Logger log_;
};

} // namespace rdp_ext::runtime

#endif //RDP_EXT_EXECUTOR_H
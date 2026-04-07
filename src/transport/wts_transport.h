#ifndef RDP_EXT_WTS_TRANSPORT_H
#define RDP_EXT_WTS_TRANSPORT_H

#include "serializer.h"
#include "logger.h"
#include "transport_handlers.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

// TODO: Отрефакторить, добавить корректное обращение с ресурсами и убрать дублирующийся код
namespace rdp_ext::transport {

    class WtsTransport {
    public:
        explicit WtsTransport(std::string_view channel_name, logging::LogSink& log_sink);

        void send(protocol::Buffer&& bytes);
        void close();

        void setConnectedHandler(ConnectedHandler handler);
        void setDisconnectedHandler(DisconnectedHandler handler);
        void setBytesHandler(BytesHandler handler);

        void workerLoop();

    private:
        enum class WorkerState : std::uint8_t {
            Stopped = 0,
            Opening,
            Connected
        };

        bool tryOpenChannel();
        void closeChannelNoLock();
        bool readOnce();

        bool queryFileHandleNoLock();

        ConnectedHandler connected_handler_;
        DisconnectedHandler disconnected_handler_;
        BytesHandler bytes_handler_;

        std::string channel_name_;

        std::atomic_bool running_{false};
        std::thread worker_thread_;

        mutable std::mutex mutex_;
        void* channel_{nullptr};
        WorkerState state_{WorkerState::Stopped};
        void* file_handle_;

        std::mutex write_mutex_;

        logging::Logger logger_;
    };

} // namespace rdp_ext::transport

#endif //RDP_EXT_WTS_TRANSPORT_H
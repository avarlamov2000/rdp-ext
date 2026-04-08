#include "wts_transport.h"

#include <chrono>
#include <thread>
#include <iostream>

namespace rdp_ext::transport {

namespace {
constexpr int OPEN_FLAGS = 0x00000000;
constexpr int OPEN_RETRY_MS = 1000;
constexpr int READ_TIMEOUT_MS = 1000;
constexpr int READ_BUFFER_SIZE = 4096;
}

WtsTransport::WtsTransport(std::string_view channel_name, logging::LogSink& log_sink)
    : channel_name_{channel_name}, logger_{log_sink, "wts-transport"}
{}

void WtsTransport::send(protocol::Buffer&& bytes) {
    {
        std::lock_guard stateLock(mutex_);

         if (!running_.load(std::memory_order_acquire)) {
             return;
         }

         if (!channel_.valid()) {
             logger_.error("Send requested, but wts channel is not open");
             return;
         }
    }

    // TODO: Защитить этот блок
    ULONG bytesWritten = 0;
    const int rv = WTSVirtualChannelWrite(
        channel_.channel(),
        reinterpret_cast<char*>(bytes.data()),
        static_cast<int>(bytes.size()),
        &bytesWritten);

    if (rv == 0) {
        closeChannel();
    }
}

void WtsTransport::close() {
    closeChannel();
}

void WtsTransport::setConnectedHandler(ConnectedHandler handler) {
    connected_handler_ = std::move(handler);
}

void WtsTransport::setDisconnectedHandler(DisconnectedHandler handler) {
    disconnected_handler_ = std::move(handler);
}

void WtsTransport::setBytesHandler(BytesHandler handler) {
    bytes_handler_ = std::move(handler);
}

void WtsTransport::run() {
    running_.store(true, std::memory_order_release);
    while (running_.load(std::memory_order_acquire)) {
        if (!tryOpenChannel()) {
            if (!running_.load(std::memory_order_acquire)) {
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(OPEN_RETRY_MS));
            continue;
        }

        while (running_.load(std::memory_order_acquire)) {
            if (!readOnce()) {
                closeChannel();
                break;
            }
        }
    }
    closeChannel();
}

void WtsTransport::stop() {
    running_.store(false, std::memory_order_release);
}

bool WtsTransport::tryOpenChannel() {
    {
        std::lock_guard lock(mutex_);
        if (!running_.load(std::memory_order_acquire)) {
            return false;
        }

        if (channel_.valid()) {
            state_ = WorkerState::Connected;
            return true;
        }
        state_ = WorkerState::Opening;
    }

    auto channel_opt = WtsChannelHandle::open(channel_name_);
    if (!channel_opt.has_value()) {
        std::lock_guard lock(mutex_);
        state_ = WorkerState::Disconnected;
        return false;
    }

    {
        std::lock_guard lock(mutex_);
        if (!running_.load(std::memory_order_acquire)) {
            return false;
        }

        if (channel_.valid()) {
            state_ = WorkerState::Connected;
            return true;
        }

        channel_ = std::move(*channel_opt);
        state_ = WorkerState::Connected;
    }

    if (connected_handler_) {
        connected_handler_();
    }
    return true;
}

void WtsTransport::closeChannel() {
    bool was_connected = false;
    {
        std::lock_guard lock(mutex_);
        was_connected = (state_ == WorkerState::Connected);
        channel_.reset();
        state_ = WorkerState::Disconnected;
    }

    if (was_connected) {
        if (disconnected_handler_) {
            disconnected_handler_();
        }
    }
}

bool WtsTransport::readOnce() {
    {
        std::lock_guard lock(mutex_);

        if (!running_.load(std::memory_order_acquire)) {
            return false;
        }

        if (!channel_.valid()) {
            return false;
        }
    }

    // TODO: Защитить этот блок
    char buffer[READ_BUFFER_SIZE];
    ULONG bytesRead = 0;

#ifdef _WIN32
    OVERLAPPED ov{};
    ov.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (ov.hEvent == nullptr) {
        logger_.error(std::format("CreateEvent failed, GetLastError={}", GetLastError()));
        return false;
    }

    BOOL ok = ReadFile(
        channel_.file(),
        buffer,
        sizeof(buffer),
        nullptr,
        &ov);

    if (!ok) {
        const DWORD error = GetLastError();
        if (error != ERROR_IO_PENDING) {
            CloseHandle(ov.hEvent);
            logger_.error(std::format("ReadFile failed, GetLastError={}", error));
            return false;
        }

        if (!GetOverlappedResult(channel_.file(), &ov, &bytesRead, TRUE)) {
            const DWORD resultError = GetLastError();
            CloseHandle(ov.hEvent);
            logger_.error(std::format("GetOverlappedResult failed, GetLastError={}", resultError));
            return false;
        }
    } else {
        // Операция могла завершиться синхронно даже для overlapped handle
        if (!GetOverlappedResult(channel_.file(), &ov, &bytesRead, FALSE)) {
            const DWORD resultError = GetLastError();
            if (resultError != ERROR_IO_INCOMPLETE) {
                CloseHandle(ov.hEvent);
                logger_.error(std::format("GetOverlappedResult failed, GetLastError={}", resultError));
                return false;
            }
        }
    }

    CloseHandle(ov.hEvent);
#else
    const int rv = WTSVirtualChannelRead(
        channel,
        READ_TIMEOUT_MS,
        buffer,
        sizeof(buffer),
        &bytesRead);

    if (rv == 0) {
        logger_.error(std::format("ReadFile failed, GetLastError={}", GetLastError()));
        return false;
    }
#endif

    if (!running_.load(std::memory_order_acquire)) {
        return false;
    }

    if (bytesRead > 0) {
        protocol::Buffer bytes(
            reinterpret_cast<std::byte*>(buffer),
            reinterpret_cast<std::byte*>(buffer) + bytesRead);

        if (bytes_handler_) {
            bytes_handler_(std::move(bytes));
        }
    }

    return true;
}

} // namespace rdp_ext::transport
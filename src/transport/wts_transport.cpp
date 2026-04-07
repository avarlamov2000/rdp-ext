#include "wts_transport.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wtsapi32.h>
#else
#include <freerdp/api.h>
#include <xrdpapi.h>
#endif

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
    std::lock_guard writeLock(write_mutex_);
    std::lock_guard stateLock(mutex_);

    if (channel_ == nullptr) {
        logger_.error("Send requested, but wts channel is not open");
        return;
    }

    ULONG bytesWritten = 0;
    const int rv = WTSVirtualChannelWrite(
        channel_,
        reinterpret_cast<char*>(bytes.data()),
        static_cast<int>(bytes.size()),
        &bytesWritten);

    if (rv == 0) {
        close();
    }
}

void WtsTransport::close() {
    bool wasConnected = false;
    {
        std::lock_guard lock(mutex_);
        wasConnected = (state_ == WorkerState::Connected);
        closeChannelNoLock();
        state_ = WorkerState::Opening;
    }

    if (wasConnected) {
        if (disconnected_handler_) {
            disconnected_handler_();
        }
    }
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

void WtsTransport::workerLoop() {
    running_.store(true, std::memory_order_release);
    while (running_.load(std::memory_order_acquire)) {
        {
            std::lock_guard lock(mutex_);
            if (state_ != WorkerState::Connected) {
                state_ = WorkerState::Opening;
            }
        }

        if (!tryOpenChannel()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(OPEN_RETRY_MS));
            continue;
        }

        if (connected_handler_) {
            connected_handler_();
        }

        while (running_.load(std::memory_order_acquire)) {
            if (!readOnce()) {
                close();
                break;
            }
        }
    }
}

bool WtsTransport::tryOpenChannel() {
    std::lock_guard lock(mutex_);

    if (channel_ != nullptr) {
        return true;
    }

    channel_ = WTSVirtualChannelOpenEx(
        WTS_CURRENT_SESSION,
        const_cast<char*>(channel_name_.c_str()),
        OPEN_FLAGS);

    if (channel_ == nullptr) {
        return false;
    }

    if (!queryFileHandleNoLock()) {
        const DWORD error = GetLastError();
        WTSVirtualChannelClose(channel_);
        channel_ = nullptr;
        SetLastError(error);
        return false;
    }

    state_ = WorkerState::Connected;
    return true;
}

bool WtsTransport::readOnce() {
    char buffer[READ_BUFFER_SIZE];
    ULONG bytesRead = 0;

    void* channel = nullptr;
    {
        std::lock_guard lock(mutex_);
        channel = channel_;
    }

    if (channel == nullptr) {
        return false;
    }

#ifdef _WIN32
    OVERLAPPED ov{};
    ov.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (ov.hEvent == nullptr) {
        logger_.error(std::format("CreateEvent failed, GetLastError={}", GetLastError()));
        return false;
    }

    BOOL ok = ReadFile(
        file_handle_,
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

        if (!GetOverlappedResult(file_handle_, &ov, &bytesRead, TRUE)) {
            const DWORD resultError = GetLastError();
            CloseHandle(ov.hEvent);
            logger_.error(std::format("GetOverlappedResult failed, GetLastError={}", resultError));
            return false;
        }
    } else {
        // Операция могла завершиться синхронно даже для overlapped handle
        if (!GetOverlappedResult(file_handle_, &ov, &bytesRead, FALSE)) {
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

bool WtsTransport::queryFileHandleNoLock() {
    if (channel_ == nullptr) {
        return false;
    }

    PVOID buffer = nullptr;
    DWORD bytesReturned = 0;

    if (!WTSVirtualChannelQuery(
            channel_,
            WTSVirtualFileHandle,
            &buffer,
            &bytesReturned)) {
        return false;
            }

    file_handle_ = *reinterpret_cast<HANDLE*>(buffer);

    if (buffer != nullptr) {
        WTSFreeMemory(buffer);
    }

    return true;
}

void WtsTransport::closeChannelNoLock() {
    if (channel_ != nullptr) {
        WTSVirtualChannelClose(channel_);
        channel_ = nullptr;
    }
}

} // namespace rdp_ext::transport
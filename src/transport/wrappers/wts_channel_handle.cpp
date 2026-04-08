#include "wts_channel_handle.h"

#include <utility>

namespace rdp_ext::transport {

WtsChannelHandle::WtsChannelHandle(HANDLE channel_handle, HANDLE file_handle)
    : channel_handle_(channel_handle), file_handle_(file_handle)
{}

WtsChannelHandle::~WtsChannelHandle() {
    reset();
}

WtsChannelHandle::WtsChannelHandle(WtsChannelHandle&& other) noexcept
    : channel_handle_(std::exchange(other.channel_handle_, nullptr))
    , file_handle_(std::exchange(other.file_handle_, nullptr))
{}

WtsChannelHandle& WtsChannelHandle::operator=(WtsChannelHandle&& other) noexcept {
    if (this != &other) {
        reset();
        channel_handle_ = std::exchange(other.channel_handle_, nullptr);
        file_handle_ = std::exchange(other.file_handle_, nullptr);
    }
    return *this;
}

bool WtsChannelHandle::valid() const {
    return channel_handle_ != nullptr && file_handle_ != nullptr && file_handle_ != INVALID_HANDLE_VALUE;
}

HANDLE WtsChannelHandle::channel() const {
    return channel_handle_;
}

HANDLE WtsChannelHandle::file() const {
    return file_handle_;
}

void WtsChannelHandle::reset(HANDLE new_channel_handle, HANDLE new_file_handle) {
    if (channel_handle_ != nullptr) {
        WTSVirtualChannelClose(channel_handle_);
    }

    channel_handle_ = new_channel_handle;
    file_handle_ = new_file_handle;
}

std::optional<WtsChannelHandle> WtsChannelHandle::open(const std::string& channel_name) {
    HANDLE channel_handle = WTSVirtualChannelOpen(
        WTS_CURRENT_SERVER_HANDLE,
        WTS_CURRENT_SESSION,
        const_cast<LPSTR>(channel_name.c_str()));

    if (channel_handle == nullptr) {
        return std::nullopt;
    }

    HANDLE file_handle = queryFileHandle(channel_handle);
    if (file_handle == nullptr) {
        WTSVirtualChannelClose(channel_handle);
        return std::nullopt;
    }

    return WtsChannelHandle{channel_handle, file_handle};
}

HANDLE WtsChannelHandle::queryFileHandle(HANDLE channelHandle) {
    void* buffer = nullptr;
    DWORD bytes_returned = 0;

    const BOOL ok = WTSVirtualChannelQuery(
        channelHandle,
        WTSVirtualFileHandle,
        &buffer,
        &bytes_returned);

    if (!ok) {
        return nullptr;
    }

    if (buffer == nullptr || bytes_returned < sizeof(HANDLE)) {
        if (buffer != nullptr) {
            WTSFreeMemory(buffer);
        }
        return nullptr;
    }

    HANDLE file_handle = *reinterpret_cast<HANDLE*>(buffer);
    WTSFreeMemory(buffer);

    if (file_handle == nullptr || file_handle == INVALID_HANDLE_VALUE) {
        return nullptr;
    }

    return file_handle;
}

} // namespace rdp_ext::transport

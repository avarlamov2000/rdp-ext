#ifndef RDP_EXT_WTS_CHANNEL_HANDLE_H
#define RDP_EXT_WTS_CHANNEL_HANDLE_H

#include "wts_include.h"

#include <optional>
#include <string>

namespace rdp_ext::transport {

class WtsChannelHandle {
public:
    WtsChannelHandle() = default;
    ~WtsChannelHandle();

    WtsChannelHandle(const WtsChannelHandle&) = delete;
    WtsChannelHandle& operator=(const WtsChannelHandle&) = delete;

    WtsChannelHandle(WtsChannelHandle&& other) noexcept;
    WtsChannelHandle& operator=(WtsChannelHandle&& other) noexcept;

    bool valid() const;
    HANDLE channel() const;
    HANDLE file() const;

    void reset(HANDLE new_channel_handle = nullptr, HANDLE new_file_handle = nullptr);

    static std::optional<WtsChannelHandle> open(const std::string& channel_name);

private:
    explicit WtsChannelHandle(HANDLE channel_handle, HANDLE file_handle);

    static HANDLE queryFileHandle(HANDLE channelHandle);

    HANDLE channel_handle_{nullptr};
    HANDLE file_handle_{nullptr}; // Живёт внутри channel, не требует освобождения
};

} // namespace rdp_ext::transport

#endif //RDP_EXT_WTS_CHANNEL_HANDLE_H
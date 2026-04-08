#include "mstsc_transport.h"

#include <cstring>
#include <format>

namespace rdp_ext::transport {
namespace {
    std::mutex g_instances_mutex;
    std::unordered_map<LPVOID, MstscTransport*> g_init_instances;
    std::unordered_map<DWORD, MstscTransport*> g_open_instances;
}

MstscTransport::MstscTransport(std::string_view channel_name, logging::LogSink& log_sink)
    : channel_name_{channel_name}, logger_{log_sink, "mstsc-transport"}
{}


#ifdef _WIN32
void MstscTransport::send(protocol::Buffer&& bytes) {
    const UINT rv = entry_points_.pVirtualChannelWrite(
        open_handle_,
        bytes.data(),
        static_cast<UINT32>(bytes.size()),
        nullptr);

    if (rv != CHANNEL_RC_OK) {
        logger_.error(std::format("Send failed, GetLastError={}", GetLastError()));
    }
}
#else
void MstscTransport::send(protocol::Buffer&& bytes) {
    auto buffer = std::make_unique<protocol::Buffer>(std::move(bytes));
    auto* rawBuffer = buffer.get();
    {
        std::lock_guard lock(write_mutex_);
        pending_writes_.emplace(rawBuffer, std::move(buffer));
    }

    const UINT rv = entry_points_ex_.pVirtualChannelWriteEx(
        init_handle_,
        open_handle_,
        rawBuffer->data(),
        static_cast<UINT32>(rawBuffer->size()),
        nullptr);

    if (rv != CHANNEL_RC_OK) {
        std::lock_guard lock(write_mutex_);
        pending_writes_.erase(rawBuffer);
        logger_.error(std::format("Send failed, GetLastError={}", GetLastError()));
    }
}
#endif

void MstscTransport::close() {
}

void MstscTransport::setConnectedHandler(ConnectedHandler handler) {
    connected_handler_ = std::move(handler);
}

void MstscTransport::setDisconnectedHandler(DisconnectedHandler handler) {
    disconnected_handler_ = std::move(handler);
}

void MstscTransport::setBytesHandler(BytesHandler handler) {
    bytes_handler_ = std::move(handler);
}

void MstscTransport::registerInitHandle(LPVOID init_handle) {
    std::lock_guard lock(g_instances_mutex);
    g_init_instances[init_handle] = this;
}

void MstscTransport::registerOpenHandle(DWORD open_handle) {
    std::lock_guard lock(g_instances_mutex);
    g_open_instances[open_handle] = this;
}

MstscTransport* MstscTransport::findByInitHandle(LPVOID init_handle) {
    std::lock_guard lock(g_instances_mutex);
    const auto it = g_init_instances.find(init_handle);
    return it != g_init_instances.end() ? it->second : nullptr;
}

MstscTransport* MstscTransport::findByOpenHandle(DWORD open_handle) {
    std::lock_guard lock(g_instances_mutex);
    const auto it = g_open_instances.find(open_handle);
    return it != g_open_instances.end() ? it->second : nullptr;
}

BOOL MstscTransport::virtualChannelEntry(PCHANNEL_ENTRY_POINTS entry_points) {
    if (entry_points == nullptr) {
        return FALSE;
    }

    if (entry_points->cbSize < sizeof(CHANNEL_ENTRY_POINTS)) {
        return FALSE;
    }

    entry_points_ = *entry_points;

    CHANNEL_DEF channelDef{};
    std::strncpy(channelDef.name, channel_name_.c_str(), sizeof(channelDef.name) - 1);
    channelDef.options = 0;

    const UINT rc = entry_points_.pVirtualChannelInit(
        &init_handle_,
        &channelDef,
        1,
        VIRTUAL_CHANNEL_VERSION_WIN2000,
        &MstscTransport::InitEventThunk);

    registerInitHandle(init_handle_);

    return (rc == CHANNEL_RC_OK) ? TRUE : FALSE;
}

VOID VCAPITYPE MstscTransport::InitEventThunk(LPVOID init_handle, UINT event, LPVOID data, UINT data_length) {
    auto* self = findByInitHandle(init_handle);
    if (self == nullptr)
        return;
    self->onInitEvent(nullptr, init_handle, event, data, data_length);
}

VOID VCAPITYPE MstscTransport::OpenEventThunk(DWORD open_handle, UINT event, LPVOID data, UINT32 data_length, UINT32 total_length, UINT32 data_flags) {
    auto* self = findByOpenHandle(open_handle);
    if (self != nullptr) {
        self->onOpenEvent(open_handle, event, data, data_length, total_length, data_flags);
    }
}

void MstscTransport::onInitEvent(LPVOID user_param, LPVOID init_handle, UINT event, LPVOID data, UINT data_length) {
    switch (event) {
    case CHANNEL_EVENT_CONNECTED: {
        const UINT rc = entry_points_.pVirtualChannelOpen(
            init_handle,
            &open_handle_,
            const_cast<PCHAR>(channel_name_.c_str()),
            &MstscTransport::OpenEventThunk);

        if (rc != CHANNEL_RC_OK) {
            logger_.error(std::format("pVirtualChannelOpen failed, GetLastError={}", GetLastError()));
            return;
        }

        registerOpenHandle(open_handle_);

        if (connected_handler_) {
            connected_handler_();
        }
        break;
    }

    case CHANNEL_EVENT_DISCONNECTED:
        if (disconnected_handler_) {
            disconnected_handler_();
        }
        break;

    case CHANNEL_EVENT_TERMINATED:
        break;

    default:
        break;
    }
}

void MstscTransport::onOpenEvent(DWORD open_handle, UINT event, LPVOID data, UINT32 data_length, UINT32 total_length, UINT32 data_flags) {
    switch (event) {
        case CHANNEL_EVENT_DATA_RECEIVED: {
            std::lock_guard lock(receive_mutex_);

            const auto* input = static_cast<const std::byte*>(data);
            protocol::Buffer buffer;
            buffer.insert(buffer.end(), input, input + data_length);

            if ((data_flags & CHANNEL_FLAG_LAST) != 0U) {
                if (bytes_handler_) {
                    bytes_handler_(std::move(buffer));
                }
            }
            break;
        }

        case CHANNEL_EVENT_WRITE_COMPLETE:
            break;

        case CHANNEL_EVENT_WRITE_CANCELLED:
            logger_.error("Write to channel was cancelled");
            break;

        default:
            break;
    }
}

#ifndef _WIN32
BOOL MstscTransport::virtualChannelEntryEx(PCHANNEL_ENTRY_POINTS_FREERDP_EX entryPointsEx, PVOID initHandle) {
    if (entryPointsEx == nullptr) {
        return FALSE;
    }

    if (entryPointsEx->cbSize < sizeof(CHANNEL_ENTRY_POINTS_FREERDP_EX)) {
        return FALSE;
    }

    if (entryPointsEx->MagicNumber != FREERDP_CHANNEL_MAGIC_NUMBER) {
        return FALSE;
    }

    entry_points_ex_ = *entryPointsEx;
    init_handle_ = initHandle;

    CHANNEL_DEF channelDef{};
    std::strncpy(channelDef.name, channel_name_.c_str(), sizeof(channelDef.name) - 1);
    channelDef.options = 0;

    const UINT rc = entry_points_ex_.pVirtualChannelInitEx(
        this,
        nullptr,
        init_handle_,
        &channelDef,
        1,
        VIRTUAL_CHANNEL_VERSION_WIN2000,
        &MstscTransport::InitEventThunkEx);

    return (rc == CHANNEL_RC_OK) ? TRUE : FALSE;
}

void MstscTransport::onInitEventEx(LPVOID userParam, LPVOID initHandle, UINT event, LPVOID data, UINT dataLength) {
    switch (event) {
        case CHANNEL_EVENT_CONNECTED: {
            const UINT rc = entry_points_ex_.pVirtualChannelOpenEx(
                initHandle,
                &open_handle_,
                const_cast<PCHAR>(channel_name_.c_str()),
                &MstscTransport::OpenEventThunkEx);

            if (rc != CHANNEL_RC_OK) {
                logger_.error("pVirtualChannelOpen failed");
                return;
            }

            if (connected_handler_) {
                connected_handler_();
            }
            break;
        }

        case CHANNEL_EVENT_DISCONNECTED:
            if (disconnected_handler_) {
                disconnected_handler_();
            }
            break;

        case CHANNEL_EVENT_TERMINATED:
            break;

        default:
            break;
    }
}

void MstscTransport::onOpenEventEx(DWORD openHandle, UINT event, LPVOID data, UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags) {
    switch (event) {
        case CHANNEL_EVENT_DATA_RECEIVED: {
            std::lock_guard lock(receive_mutex_);

            const auto* input = static_cast<const std::byte*>(data);
            protocol::Buffer buffer;
            buffer.insert(buffer.end(), input, input + dataLength);

            if ((dataFlags & CHANNEL_FLAG_LAST) != 0U) {
                if (bytes_handler_) {
                    bytes_handler_(std::move(buffer));
                }
            }
            break;
        }

        case CHANNEL_EVENT_WRITE_COMPLETE:
        case CHANNEL_EVENT_WRITE_CANCELLED: {
            auto* rawBuffer = static_cast<protocol::Buffer*>(data);

            std::lock_guard lock(write_mutex_);
            pending_writes_.erase(rawBuffer);
            break;
        }

        default:
            break;
    }
}

VOID VCAPITYPE MstscTransport::InitEventThunkEx(LPVOID userParam, LPVOID initHandle, UINT event, LPVOID data, UINT dataLength) {
    auto* self = g_runtime;
    if (self != nullptr) {
        self->onInitEventEx(userParam, initHandle, event, data, dataLength);
    }
}

VOID VCAPITYPE MstscTransport::OpenEventThunkEx(LPVOID userParam, DWORD openHandle, UINT event, LPVOID data, UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags) {
    auto* self = g_runtime;
    if (self != nullptr) {
        self->onOpenEventEx(openHandle, event, data, dataLength, totalLength, dataFlags);
    }
}
#endif

} // namespace rdp_ext::transport

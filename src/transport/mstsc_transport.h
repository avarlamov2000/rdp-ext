#ifndef RDP_EXT_MSTSC_PLUGIN_TRANSPORT_ADAPTER_H
#define RDP_EXT_MSTSC_PLUGIN_TRANSPORT_ADAPTER_H

#include "serializer.h"
#include "logger.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cchannel.h>
#else
extern "C" {
#include <freerdp/svc.h>
}
#endif

#include "transport_handlers.h"

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

// TODO: Отрефакторить, добавить корректное обращение с ресурсами и убрать дублирующийся код
namespace rdp_ext::transport {

class MstscTransport {
public:
    explicit MstscTransport(std::string_view channel_name, logging::LogSink& log_sink);

    void send(protocol::Buffer&& bytes);
    void close();

    void setConnectedHandler(ConnectedHandler handler);
    void setDisconnectedHandler(DisconnectedHandler handler);
    void setBytesHandler(BytesHandler handler);

    BOOL virtualChannelEntry(PCHANNEL_ENTRY_POINTS entry_points);
#ifndef _WIN32
    BOOL virtualChannelEntryEx(PCHANNEL_ENTRY_POINTS_FREERDP_EX entryPoints, PVOID initHandle);
#endif

    void onInitEvent(LPVOID user_param, LPVOID init_handle, UINT event, LPVOID data, UINT data_length);
    void onOpenEvent(DWORD open_handle, UINT event, LPVOID data, UINT32 data_length, UINT32 total_length, UINT32 data_flags);

#ifndef _WIN32
    void onInitEventEx(LPVOID userParam, LPVOID initHandle, UINT event, LPVOID data, UINT dataLength);
    void onOpenEventEx(DWORD openHandle, UINT event, LPVOID data, UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags);
#endif

private:
    void registerInitHandle(LPVOID init_handle);
    void registerOpenHandle(DWORD open_handle);
    static MstscTransport *findByInitHandle(LPVOID init_handle);
    static MstscTransport *findByOpenHandle(DWORD open_handle);

    static VOID VCAPITYPE InitEventThunk(LPVOID init_handle, UINT event, LPVOID data, UINT data_length);
    static VOID VCAPITYPE OpenEventThunk(DWORD open_handle, UINT event, LPVOID data, UINT32 data_length, UINT32 total_length, UINT32 data_flags);

#ifndef _WIN32
    static VOID VCAPITYPE InitEventThunkEx(LPVOID userParam, LPVOID initHandle, UINT event, LPVOID data, UINT dataLength);
    static VOID VCAPITYPE OpenEventThunkEx(LPVOID userParam, DWORD openHandle, UINT event, LPVOID data, UINT32 dataLength, UINT32 totalLength, UINT32 dataFlags);
#endif

    CHANNEL_ENTRY_POINTS entry_points_{};
#ifndef _WIN32
    CHANNEL_ENTRY_POINTS_FREERDP_EX entry_points_ex_{};
#endif

    ConnectedHandler connected_handler_;
    DisconnectedHandler disconnected_handler_;
    BytesHandler bytes_handler_;

    std::string channel_name_;

    PVOID init_handle_{nullptr};
    DWORD open_handle_{0};
    bool started_{false};

    std::mutex write_mutex_;
    std::mutex receive_mutex_;
    std::unordered_map<protocol::Buffer*, std::unique_ptr<protocol::Buffer>> pending_writes_;

    logging::Logger logger_;
};

} // namespace rdp_ext::transport

#endif //RDP_EXT_MSTSC_PLUGIN_TRANSPORT_ADAPTER_H
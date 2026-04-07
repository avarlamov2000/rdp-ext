#ifndef RDP_EXT_LOGIC_STATES_H
#define RDP_EXT_LOGIC_STATES_H

#include "message.h"

#include <optional>

namespace rdp_ext::logic {

enum class ConnectionState {
    Disconnected,
    AwaitingHandshake,
    Active,
    Closed,
};

class HandshakeState {
public:
    bool isCompleted() const {
        return handshake_completed_;
    }

    bool isLocalHelloNeeded() const {
        return local_hello_needed_;
    }

    void reset() {
        handshake_completed_ = false;
        local_hello_needed_ = true;
        remote_instance_id_ = std::nullopt;
    }

    void localHelloSent() {
        local_hello_needed_ = false;
    }

    void remoteHelloReceived(protocol::InstanceId remote_instance_id) {
        if (remote_instance_id_ && remote_instance_id_ == remote_instance_id) {
            local_hello_needed_ = !handshake_completed_;
            handshake_completed_ = true;
        } else {
            local_hello_needed_ = true;
            handshake_completed_ = false;
        }
        remote_instance_id_ = remote_instance_id;
    }

private:
    std::optional<protocol::InstanceId> remote_instance_id_{};
    bool local_hello_needed_{true};
    bool handshake_completed_{false};
};

} // namespace rdp_ext::logic

#endif //RDP_EXT_LOGIC_STATES_H
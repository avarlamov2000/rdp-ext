#ifndef RDP_EXT_TRANSPORT_HANDLERS_H
#define RDP_EXT_TRANSPORT_HANDLERS_H

#include <functional>

#include "serializer.h"

namespace rdp_ext::transport {

using ConnectedHandler = std::function<void()>;
using DisconnectedHandler = std::function<void()>;
using BytesHandler = std::function<void(protocol::Buffer&&)>;
using ErrorHandler = std::function<void(std::string)>;

}

#endif //RDP_EXT_TRANSPORT_HANDLERS_H
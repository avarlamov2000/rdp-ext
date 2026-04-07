#ifndef RDP_EXT_SERIALIZER_H
#define RDP_EXT_SERIALIZER_H

#include "message.h"

#include <vector>
#include <span>
#include <stdexcept>

namespace rdp_ext::protocol {

class SerializationError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

using Buffer = std::vector<std::byte>;
using BufferSpan = std::span<const std::byte>;

class Serializer {
public:
    static Buffer serialize(const Message& message);
    static void serialize(const Message& message, Buffer& buffer);
    static Message deserialize(BufferSpan data);
};

} // namespace rdp_ext::protocol

#endif //RDP_EXT_SERIALIZER_H
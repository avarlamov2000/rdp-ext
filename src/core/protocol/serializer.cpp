#include "serializer.h"

#include <format>
#include <zpp_bits.h>

namespace rdp_ext::protocol {

Buffer Serializer::serialize(const Message &message) {
    auto [data, archive] = zpp::bits::data_out();
    auto& mutableMessage = const_cast<Message&>(message); // Костыль из-за бага библиотеки (issue #157)
    if (const auto result = archive(mutableMessage); zpp::bits::failure(result)) {
        throw SerializationError{"Serialization failed: " + std::make_error_code(result).message()};
    }
    return data;
}

void Serializer::serialize(const Message& message, Buffer& buffer) {
    buffer.clear();
    zpp::bits::out archive{buffer};
    auto& mutableMessage = const_cast<Message&>(message); // Костыль из-за бага библиотеки (issue #157)
    if (const auto result = archive(mutableMessage); zpp::bits::failure(result)) {
        throw SerializationError{"Serialization failed: " + std::make_error_code(result).message()};
    }
}

Message Serializer::deserialize(std::span<const std::byte> data) {
    zpp::bits::in archive(data);
    Message message{};
    if (const auto result = archive(message); zpp::bits::failure(result)) {
        throw SerializationError{std::format("Deserialization failed: {}", std::make_error_code(result).message())};
    }
    return message;
}

} // namespace rdp_ext::protocol

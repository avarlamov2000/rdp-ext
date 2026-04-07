#ifndef RDP_EXT_PROTOCOL_TEST_HELPERS_H
#define RDP_EXT_PROTOCOL_TEST_HELPERS_H

#include "protocol/serializer.h"

namespace rdp_ext::protocol::test {

template <typename MessageType>
void expectEqual(const MessageType& expected, const Message& message) {
    ASSERT_TRUE(std::holds_alternative<MessageType>(message));
    EXPECT_EQ(std::get<MessageType>(message), expected);
}

template <typename MessageType>
void expectPackUnpackWorks(const MessageType& expected) {
    const auto buffer = Serializer::serialize(expected);
    expectEqual(expected, Serializer::deserialize(buffer));
}

template <typename MessageType>
void expectPackUnpackWorks(const MessageType& expected, Buffer& buffer) {
    Serializer::serialize(expected, buffer);
    expectEqual(expected, Serializer::deserialize(buffer));
}

template <typename MessageType>
void expectPackUnpackTest(const MessageType& expected) {
    expectPackUnpackWorks(expected);
    Buffer buffer;
    expectPackUnpackWorks(expected, buffer);
}

} // namespace rdp_ext::protocol::test

#endif //RDP_EXT_PROTOCOL_TEST_HELPERS_H
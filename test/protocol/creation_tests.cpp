#include <gtest/gtest.h>

#include "protocol_mock_data.h"
#include "protocol_test_helpers.h"
#include "protocol/message_factory.h"

namespace rdp_ext::protocol::test {

using ExpectedMessage = std::variant<
    ErrorMessage,
    HelloMessage,
    CapabilitySetMessage,
    TelemetryStartMessage,
    CpuSampleMessage,
    PingMessage,
    PongMessage
>;

/// Тест реализован в compile time. Не даёт добавлять/удалять варианты без дополнения тестов
static_assert(std::is_same_v<Message, ExpectedMessage>,
              "Message was changed. Please update unit tests and verify protocol compatibility");

TEST(CreationTests, HelloCreate) {
    const auto expected = makeTestHelloMessage();
    expectEqual(expected, MessageFactory::makeHello(expected.instance_id));
}

TEST(CreationTests, CapabilitySetCreate) {
    const auto expected = makeTestCapabilitySetMessage();
    expectEqual(expected, MessageFactory::makeCapabilitySet(expected.is_ping_supported, expected.is_telemetry_supported));
}

TEST(CreationTests, TelemetryStartCreate) {
    const auto expected = makeTestTelemetryStartMessage();
    expectEqual(expected, MessageFactory::makeTelemetryStart());
}

TEST(CreationTests, CpuSampleCreate) {
    const auto expected = makeTestCpuSampleMessage();
    expectEqual(expected, MessageFactory::makeCpuSample(expected.cpu_load));
}

TEST(CreationTests, PingCreate) {
    const auto expected = makeTestPingMessage();
    expectEqual(expected, MessageFactory::makePing(expected.id));
}

TEST(CreationTests, PongCreate) {
    const auto expected = makeTestPongMessage();
    expectEqual(expected, MessageFactory::makePong(expected.id));
}

} // namespace rdp_ext::protocol::test
#include "protocol/message_factory.h"
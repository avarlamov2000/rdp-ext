#include <gtest/gtest.h>

#include "protocol_mock_data.h"
#include "protocol_test_helpers.h"

namespace rdp_ext::protocol::test {

TEST(SerializationTests, HelloPackUnpack) {
    expectPackUnpackTest(makeTestHelloMessage());
}

TEST(SerializationTests, ErrorPackUnpack) {
    expectPackUnpackTest(makeTestHelloMessage());
}

TEST(SerializationTests, CapabilitySetPackUnpack) {
    expectPackUnpackTest(makeTestCapabilitySetMessage());
}

TEST(SerializationTests, TelemetryStartPackUnpack) {
    expectPackUnpackTest(makeTestTelemetryStartMessage());
}

TEST(SerializationTests, CpuSamplePackUnpack) {
    expectPackUnpackTest(makeTestCpuSampleMessage());
}

TEST(SerializationTests, PingPackUnpack) {
    expectPackUnpackTest(makeTestPingMessage());
}

TEST(SerializationTests, PongPackUnpack) {
    expectPackUnpackTest(makeTestPongMessage());
}

TEST(SerializationTests, BufferReuse) {
    Buffer buffer;
    expectPackUnpackWorks(makeTestHelloMessage(), buffer);
    expectPackUnpackWorks(makeTestErrorMessage(), buffer);
    expectPackUnpackWorks(makeTestCapabilitySetMessage(), buffer);
    expectPackUnpackWorks(makeTestTelemetryStartMessage(), buffer);
    expectPackUnpackWorks(makeTestCpuSampleMessage(), buffer);
    expectPackUnpackWorks(makeTestPingMessage(), buffer);
    expectPackUnpackWorks(makeTestPongMessage(), buffer);
}

} // namespace rdp_ext::protocol::test
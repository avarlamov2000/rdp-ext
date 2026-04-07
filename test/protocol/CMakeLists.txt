add_executable(rdp_ext_protocol_tests
        creation_tests.cpp
        serialization_tests.cpp
)

target_link_libraries(rdp_ext_protocol_tests
    PRIVATE
        rdp_ext_core
        GTest::gtest_main
)

target_include_directories(rdp_ext_protocol_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src/core
)

include(GoogleTest)
gtest_discover_tests(rdp_ext_protocol_tests)
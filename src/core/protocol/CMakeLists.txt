target_sources(rdp_ext_core
    PRIVATE
        message_factory.cpp
        serializer.cpp
)

target_include_directories(rdp_ext_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

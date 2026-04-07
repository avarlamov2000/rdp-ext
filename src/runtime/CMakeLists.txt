add_library(rdp_ext_runtime STATIC
        executor.cpp
        wts_runtime.cpp
        mstsc_runtime.cpp
)

target_include_directories(rdp_ext_runtime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(rdp_ext_runtime
    PUBLIC
        rdp_ext_core
        rdp_ext_logic
        rdp_ext_transport
        rdp_ext_boost
)

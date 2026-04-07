add_library(rdp_ext_core STATIC)

add_subdirectory(logging)
add_subdirectory(protocol)
add_subdirectory(telemetry)

target_link_libraries(rdp_ext_core
    PUBLIC
        rdp_ext_zpp_bits
        rdp_ext_plog
)

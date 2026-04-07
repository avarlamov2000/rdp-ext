add_executable(rdp_ext_xrdp_server
        main.cpp
)

set_target_properties(rdp_ext_xrdp_server PROPERTIES
        OUTPUT_NAME xrdp_server
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_link_libraries(rdp_ext_xrdp_server
    PRIVATE
        rdp_ext_runtime
        #rdp_ext_xrdpapi
)

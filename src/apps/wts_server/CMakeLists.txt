add_executable(rdp_ext_wts_server
        main.cpp
)

set_target_properties(rdp_ext_wts_server PROPERTIES
        OUTPUT_NAME wts_server
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_link_libraries(rdp_ext_wts_server
    PRIVATE
        rdp_ext_runtime
        #Wtsapi32
)

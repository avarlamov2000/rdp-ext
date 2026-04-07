add_library(rdp_ext_mstsc_plugin SHARED
        main.cpp
)

set_target_properties(rdp_ext_mstsc_plugin PROPERTIES
        OUTPUT_NAME mstsc_plugin
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_link_libraries(rdp_ext_mstsc_plugin
    PRIVATE
        rdp_ext_runtime
)

add_library(rdp_ext_transport STATIC
        mstsc_transport.cpp
        wts_transport.cpp
)

add_subdirectory(wrappers)

target_include_directories(rdp_ext_transport
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(rdp_ext_transport
    PUBLIC
        rdp_ext_core
)

if(UNIX)
    target_link_libraries(rdp_ext_transport
        PUBLIC
            rdp_ext_core
            rdp_ext_xrdpapi
            rdp_ext_freerdp3
    )
elseif (WIN32)
    target_link_libraries(rdp_ext_transport
        PUBLIC
            rdp_ext_core
            Wtsapi32
    )
endif()


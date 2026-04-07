target_include_directories(rdp_ext_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if (WIN32)
    target_sources(rdp_ext_core
        PRIVATE
            cpu_sampler_win.cpp
    )
else()
    target_sources(rdp_ext_core
        PRIVATE
            cpu_sampler_linux.cpp
    )
endif()
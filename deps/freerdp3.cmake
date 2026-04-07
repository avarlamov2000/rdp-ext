find_package(PkgConfig REQUIRED)
pkg_check_modules(FREERDP3 REQUIRED freerdp3)
pkg_check_modules(WINPR3 REQUIRED winpr3)

add_library(rdp_ext_freerdp3 INTERFACE)

target_include_directories(rdp_ext_freerdp3
    INTERFACE
        ${FREERDP3_INCLUDE_DIRS}
        ${WINPR3_INCLUDE_DIRS}
)

target_link_libraries(rdp_ext_freerdp3
    INTERFACE
        ${FREERDP3_LIBRARIES}
        ${WINPR3_LIBRARIES}
)

target_compile_options(rdp_ext_freerdp3
    INTERFACE
        ${FREERDP3_CFLAGS_OTHER}
        ${WINPR3_CFLAGS_OTHER}
)
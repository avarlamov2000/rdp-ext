cmake_minimum_required(VERSION 3.28.3)
project(rdp_ext LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_definitions(_WIN32_WINNT=0x0601 WINVER=0x0601)

option(BUILD_TESTS "Build unit tests" ON)

add_subdirectory(deps)
add_subdirectory(src)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(UNIX)
    list(APPEND rdp_ext_all_deps
            rdp_ext_freerdp_plugin
            rdp_ext_xrdp_server
    )
elseif (WIN32)
    list(APPEND rdp_ext_all_deps
            rdp_ext_wts_server
            rdp_ext_mstsc_plugin
    )
endif()
add_custom_target(rdp_ext_all DEPENDS ${rdp_ext_all_deps})

find_program(XRDP_EXECUTABLE xrdp)
if (NOT XRDP_EXECUTABLE)
    message(FATAL_ERROR "xrdp executable not found in PATH")
endif()

execute_process(
    COMMAND ${XRDP_EXECUTABLE} --version
    OUTPUT_VARIABLE XRDP_VERSION_OUTPUT
    ERROR_VARIABLE XRDP_VERSION_ERROR
    RESULT_VARIABLE XRDP_VERSION_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (NOT XRDP_VERSION_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to execute 'xrdp --version': ${XRDP_VERSION_ERROR}")
endif()

string(REGEX MATCH "^[^\n]+" XRDP_VERSION_FIRST_LINE "${XRDP_VERSION_OUTPUT}")

if (NOT XRDP_VERSION_FIRST_LINE MATCHES "([0-9]+\\.[0-9]+\\.[0-9]+(\\.[0-9]+)?)")
    message(FATAL_ERROR
            "Unable to parse xrdp version from first line: '${XRDP_VERSION_FIRST_LINE}'")
endif()

set(XRDP_GIT_TAG "v${CMAKE_MATCH_1}")
message(STATUS "XRDP_GIT_TAG=${XRDP_GIT_TAG}")

FetchContent_Declare(
    xrdp_headers
    GIT_REPOSITORY https://github.com/neutrinolabs/xrdp.git
    GIT_TAG ${XRDP_GIT_TAG}
)

FetchContent_GetProperties(xrdp_headers)
if (NOT xrdp_headers_POPULATED)
    FetchContent_Populate(xrdp_headers)
endif()

find_path(XRDPAPI_INCLUDE_DIR
    NAMES xrdpapi.h
    PATHS
        /usr/include
        /usr/include/xrdp
        /usr/local/include
        ${xrdp_headers_SOURCE_DIR}/xrdpapi
)

find_library(XRDPAPI_LIBRARY
    NAMES xrdpapi libxrdpapi
    PATHS
        /usr/lib
        /usr/lib/x86_64-linux-gnu
        /usr/lib/x86_64-linux-gnu/xrdp
        /usr/local/lib
)

if (NOT XRDPAPI_INCLUDE_DIR)
    message(FATAL_ERROR "xrdpapi.h not found")
endif()

if (NOT XRDPAPI_LIBRARY)
    message(FATAL_ERROR "libxrdpapi not found")
endif()

add_library(rdp_ext_xrdpapi INTERFACE)
target_include_directories(rdp_ext_xrdpapi
    INTERFACE
        ${XRDPAPI_INCLUDE_DIR}
)

target_link_libraries(rdp_ext_xrdpapi
    INTERFACE
        ${XRDPAPI_LIBRARY}
)

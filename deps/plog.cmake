FetchContent_Declare(
    plog
    GIT_REPOSITORY https://github.com/SergiusTheBest/plog.git
    GIT_TAG 1.1.11
)

FetchContent_MakeAvailable(plog)

add_library(rdp_ext_plog INTERFACE)
target_include_directories(rdp_ext_plog
    INTERFACE
        ${plog_SOURCE_DIR}/include
)
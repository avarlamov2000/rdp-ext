FetchContent_Declare(
    zpp_bits
    GIT_REPOSITORY https://github.com/eyalz800/zpp_bits.git
    GIT_TAG v4.7
)

FetchContent_MakeAvailable(zpp_bits)

add_library(rdp_ext_zpp_bits INTERFACE)
target_include_directories(rdp_ext_zpp_bits
    INTERFACE
        ${zpp_bits_SOURCE_DIR}
)

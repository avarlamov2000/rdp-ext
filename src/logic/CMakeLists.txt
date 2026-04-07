add_library(rdp_ext_logic STATIC)

add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(server)

target_include_directories(rdp_ext_logic
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src/logic
)

target_link_libraries(rdp_ext_logic
    PUBLIC
        rdp_ext_core
)

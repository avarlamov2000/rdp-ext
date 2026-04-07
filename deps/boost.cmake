add_library(rdp_ext_boost INTERFACE)

set(RDP_EXT_BOOST_ROOT "" CACHE PATH "Path to Boost root")

if (RDP_EXT_BOOST_ROOT)
    target_include_directories(rdp_ext_boost
        INTERFACE
            ${RDP_EXT_BOOST_ROOT}
    )
else()
    find_package(Boost REQUIRED)

    target_include_directories(rdp_ext_boost
        INTERFACE
            ${Boost_INCLUDE_DIRS}
    )
endif()
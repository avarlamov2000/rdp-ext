include(FetchContent)

include(zpp_bits.cmake)
include(plog.cmake)
include(boost.cmake)
if (UNIX)
    include(xrdpapi.cmake)
    include(freerdp3.cmake)
endif()

if (BUILD_TESTS)
    include(googletest.cmake)
endif()

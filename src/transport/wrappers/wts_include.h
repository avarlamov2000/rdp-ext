#ifndef RDP_EXT_WTS_INCLUDE_H
#define RDP_EXT_WTS_INCLUDE_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wtsapi32.h>
#else
#include <freerdp/api.h>
#include <xrdpapi.h>
#endif

#endif //RDP_EXT_WTS_INCLUDE_H
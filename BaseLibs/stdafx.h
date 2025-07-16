// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Include Winsock2 before Windows.h to prevent redefinition conflicts
#ifdef WIN32
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_   // Prevent inclusion of winsock.h in windows.h
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#endif

// TODO: reference additional headers your program requires here

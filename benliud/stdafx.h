// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#pragma comment(linker, "/nodefaultlib:libc.lib")
#pragma comment(linker, "/nodefaultlib:libcd.lib")

// Target Windows 10 SDK
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Windows API headers instead of MFC
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>    // Windows Common Controls
#include <commdlg.h>     // Common Dialogs
#include <shellapi.h>    // Shell API
#include <wininet.h>     // Internet API
#include <ws2tcpip.h>    // Winsock2 for networking

// Standard C++ library includes
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

// Link required libraries
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "ws2_32.lib")

// Initialize Common Controls
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Custom message definitions for our application
#define WM_USER_UPDATE_SPEED    (WM_USER + 1)
#define WM_USER_UPDATE_PROGRESS (WM_USER + 2)
#define WM_USER_REMOVE_TASK     (WM_USER + 3)

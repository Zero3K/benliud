// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#pragma comment(linker, "/nodefaultlib:libc.lib")
#pragma comment(linker, "/nodefaultlib:libcd.lib")

// Target Windows 10 SDK
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

// Removed Windows CE configuration - targeting Windows 10 SDK
//#include <ceconfig.h>
//#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
//#define SHELL_AYGSHELL
//#endif

// Removed Windows CE-specific code - targeting Windows 10
//#ifdef _CE_DCOM
//#define _ATL_APARTMENT_THREADED
//#endif

#include <windows.h>
#include <commctrl.h>

// Removed Windows CE specific includes
//#include <aygshell.h>
//#pragma comment(lib, "aygshell.lib") 

//#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
//#ifndef _DEVICE_RESOLUTION_AWARE
//#define _DEVICE_RESOLUTION_AWARE
//#endif
//#endif

//#ifdef _DEVICE_RESOLUTION_AWARE
//#include "DeviceResolutionAware.h"
//#endif

//#if _WIN32_WCE < 0x500 && ( defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP) )
//	#pragma comment(lib, "ccrtrtti.lib")
//	#ifdef _X86_	
//		#if defined(_DEBUG)
//			#pragma comment(lib, "libcmtx86d.lib")
//		#else
//			#pragma comment(lib, "libcmtx86.lib")
//		#endif
//	#endif
//#endif

//#include <altcecrt.h>

// TODO: reference additional headers your program requires here

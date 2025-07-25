#pragma once

// The following macros define the minimum required platform.  The minimum required platform
// is the earliest version of Windows, Internet Explorer etc. that has the necessary features to run 
// your application.  The macros work by enabling all features available on platform versions up to and 
// including the version specified.

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifdef WIN32
#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows 10.
#define _WIN32_WINNT 0x0A00     // Target Windows 10 for consistency with main project
#endif
#ifndef WINVER
#define WINVER 0x0A00           // Target Windows 10 for consistency
#endif
#endif


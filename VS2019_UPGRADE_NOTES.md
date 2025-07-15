# Visual Studio 2019 Upgrade Notes

This project has been upgraded to compile with Visual Studio 2019.

## Changes Made

### Solution Files (.sln)
- Updated all solution files from VS2008 format (Format Version 10.00) to VS2019 format (Format Version 12.00)
- Changed header comments from "# Visual Studio 2008" to "# Visual Studio Version 16"
- Added VisualStudioVersion and MinimumVisualStudioVersion properties

### Project Files (.vcproj → .vcxproj)
- Converted all old .vcproj files to modern .vcxproj format
- Updated platform toolset to v142 (Visual Studio 2019)
- Set Windows Target Platform Version to 10.0 (Windows 10 SDK)
- Replaced Windows Mobile 6 platform with standard Win32 and x64 platforms

### Platform Configurations
- **Old**: Debug|Windows Mobile 6 Standard SDK (ARMV4I), Release|Windows Mobile 6 Standard SDK (ARMV4I)
- **New**: Debug|Win32, Debug|x64, Release|Win32, Release|x64

### Project Types
- **BaseLibs**: Static Library (.lib) - converted to modern format
- **benliud**: MFC Application (.exe) - converted to modern format with dynamic MFC linking
- **benliud_bt**: Dynamic Library (.dll) - BitTorrent functionality
- **benliud_btkad**: Dynamic Library (.dll) - DHT/Kademlia functionality  
- **benliud_upnp**: Dynamic Library (.dll) - UPnP functionality

### Dependencies
- Removed hardcoded OpenSSL paths (D:\Projects\openssl-0.9.8g)
- Removed legacy Source Code Control references
- Users will need to configure dependencies via:
  - Environment variables
  - vcpkg package manager
  - Manual library configuration

## Building

Open any .sln file in Visual Studio 2019 or later. The projects should now compile with the v142 toolset.

Note: You may need to install additional dependencies like OpenSSL, GMP, TinyXML, and SQLite as mentioned in the original readme.txt.
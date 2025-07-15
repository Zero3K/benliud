# Fix for Unresolved External Symbol Errors

## Problem
Users building the `benliud` project with Debug|Win32 configuration are experiencing unresolved external symbol errors for `HashLib::CSHA1` functions:

```
Error LNK2001 unresolved external symbol "public: virtual void __thiscall HashLib::CSHA1::Open(void)" 
Error LNK2001 unresolved external symbol "public: virtual void __thiscall HashLib::CSHA1::Close(void)"
Error LNK2001 unresolved external symbol "public: virtual void __thiscall HashLib::CSHA1::Update(char const *,int)" 
Error LNK2019 unresolved external symbol "public: __thiscall HashLib::CSHA1::CSHA1(void)" 
Error LNK2019 unresolved external symbol "public: virtual void __thiscall HashLib::CSHA1::Hash(char const *,int)"
```

## Root Cause
The issue was caused by several configuration problems:

1. **Missing libs directory**: The BaseLibs project outputs to `../libs/` but this directory didn't exist
2. **Incorrect solution configuration mapping**: Some solution configurations were mapped to wrong project configurations
3. **Missing x64 configurations**: BaseLibs project was missing DebugMD|x64 and ReleaseMD|x64 ItemDefinitionGroup sections
4. **Missing project configurations**: benliud_upnp project was missing DebugMD|Win32 and ReleaseMD|Win32 configurations

## Fix Applied

### 1. Created libs directory
```bash
mkdir -p libs
```

### 2. Fixed solution configuration mapping
Updated `benliud.sln` to properly map solution configurations to project configurations:
- DebugMD solution config → DebugMD project config (not Debug)
- ReleaseMD solution config → ReleaseMD project config (not Release)

### 3. Added missing BaseLibs x64 configurations
Added missing ItemDefinitionGroup sections for:
- DebugMD|x64 → outputs `baselibs_x64_md_debug.lib`
- ReleaseMD|x64 → outputs `baselibs_x64_md_release.lib`

### 4. Added missing benliud_upnp configurations
Added DebugMD|Win32 and ReleaseMD|Win32 configurations to benliud_upnp project.

## Runtime Library Configuration Matrix

| Project Configuration | Runtime Library | BaseLibs Library Output |
|----------------------|-----------------|-------------------------|
| Debug\|Win32 | MultiThreadedDebug | baselibs_x86_mt_debug.lib |
| DebugMD\|Win32 | MultiThreadedDebugDLL | baselibs_x86_md_debug.lib |
| Release\|Win32 | MultiThreaded | baselibs_x86_mt_release.lib |
| ReleaseMD\|Win32 | MultiThreadedDLL | baselibs_x86_md_release.lib |
| Debug\|x64 | MultiThreadedDebug | baselibs_x64_mt_debug.lib |
| Release\|x64 | MultiThreaded | baselibs_x64_mt_release.lib |
| DebugMD\|x64 | MultiThreadedDebugDLL | baselibs_x64_md_debug.lib |
| ReleaseMD\|x64 | MultiThreadedDLL | baselibs_x64_md_release.lib |

## Project Dependencies
All dependent projects (benliud_btkad, benliud_bt, benliud_upnp) are configured with:
- ProjectReference to BaseLibs (ensures build order)
- Correct library dependencies matching their runtime library settings
- Include paths to BaseLibs headers

## Result
After these fixes, the projects should build successfully in Visual Studio 2019+ without unresolved external symbol errors. The BaseLibs static library will be built first and the correct version will be linked based on the runtime library configuration.
# Linker Error Troubleshooting Guide

## Current Issue: Unresolved External Symbol Errors for HashLib::CSHA1

### Problem Description
When building benliud_btkad in Debug|Win32 configuration, you're getting linker errors:
```
LNK2019 unresolved external symbol "public: __thiscall HashLib::CSHA1::CSHA1(void)"
LNK2019 unresolved external symbol "public: virtual void __thiscall HashLib::CSHA1::Hash(char const *,int)"
LNK2001 unresolved external symbol "public: virtual void __thiscall HashLib::CSHA1::Open(void)"
LNK2001 unresolved external symbol "public: virtual void __thiscall HashLib::CSHA1::Close(void)"
LNK2001 unresolved external symbol "public: virtual void __thiscall HashLib::CSHA1::Update(char const *,int)"
```

### Root Cause Analysis
The issue occurs because benliud_btkad cannot link against the BaseLibs static library. This happens when:

1. **BaseLibs hasn't been built successfully** - The `baselibs_x86_mt_debug.lib` file doesn't exist
2. **Build order issue** - BaseLibs builds after benliud_btkad (wrong dependency order)
3. **Output path mismatch** - BaseLibs outputs to a different location than expected

### Project Configuration Verification

#### ✅ Confirmed Working:
- **Runtime Library Consistency**: Both BaseLibs and benliud_btkad use `MultiThreadedDebug` for Debug|Win32
- **Library References**: benliud_btkad correctly references `baselibs_x86_mt_debug.lib`
- **Include Paths**: benliud_btkad includes BaseLibs headers from `../BaseLibs/include`
- **Project Dependencies**: Solution file correctly defines BaseLibs as dependency
- **Source Files**: HashLib::CSHA1 implementation exists in `BaseLibs/src/SHA1.cpp`

#### 📁 Expected File Locations:
- **BaseLibs Output**: `libs/baselibs_x86_mt_debug.lib` (from solution root)
- **benliud_btkad Search Path**: `../libs/` (relative to benliud_btkad folder)

### Troubleshooting Steps

#### Step 1: Verify BaseLibs Build
1. Right-click on BaseLibs project in Solution Explorer
2. Select "Build" to build only BaseLibs
3. Check Output window for any build errors
4. Verify that `libs/baselibs_x86_mt_debug.lib` is created

#### Step 2: Check Build Output Location
Navigate to your solution directory and verify:
```
benliud/
├── libs/
│   ├── baselibs_x86_mt_debug.lib    ← This should exist after BaseLibs builds
│   └── baselibs_x86_mt_release.lib  ← This should exist for Release builds
├── BaseLibs/
├── benliud_btkad/
└── benliud.sln
```

#### Step 3: Manual Build Order
If automatic dependency resolution fails:
1. Build BaseLibs first (right-click → Build)
2. Then build benliud_btkad
3. Finally build the entire solution

#### Step 4: Clean and Rebuild
1. Build → Clean Solution
2. Build → Rebuild Solution

This ensures all projects are built in the correct order with fresh outputs.

### Alternative Solutions

#### Option 1: Verify Project Reference (Recommended)
The project already has proper ProjectReference, but if issues persist:
1. Right-click benliud_btkad → Properties
2. Go to Framework and References
3. Ensure BaseLibs is listed as a project reference

#### Option 2: Manual Library Path
If automatic linking fails, you can specify absolute paths:
1. Right-click benliud_btkad → Properties
2. Go to Configuration Properties → Linker → General
3. Add absolute path to libs directory in "Additional Library Directories"

### Configuration Summary

| Project | Configuration | Runtime Library | Output File |
|---------|---------------|-----------------|-------------|
| BaseLibs | Debug\|Win32 | MultiThreadedDebug | libs/baselibs_x86_mt_debug.lib |
| benliud_btkad | Debug\|Win32 | MultiThreadedDebug | Links: baselibs_x86_mt_debug.lib |

The configurations are correctly matched and should work once BaseLibs builds successfully.
# Fix for Unresolved External Symbol Errors

## Problem Diagnosis

The unresolved external symbol errors for `HashLib::CSHA1` were caused by a **filename collision** in the BaseLibs project:

### The Issue
1. `BaseLibs/src/SHA1.cpp` defines the `HashLib::CSHA1` wrapper class
2. `thirdparty/crypto/sha1.cpp` defines the underlying SHA1 implementation  
3. Both files were compiled to the same object filename: `sha1.obj`
4. MSVC linker warning LNK4042: "object specified more than once; extras ignored"
5. The linker kept only one of the object files, missing the `HashLib::CSHA1` symbols

### Evidence from Build Log
```
warning MSB8027: Two or more files with the name of SHA1.cpp will produce outputs to the same location.
warning LNK4042: object specified more than once; extras ignored
```

## Solution Applied

Modified `BaseLibs/BaseLibs.vcxproj` to specify unique object filenames:

```xml
<ClCompile Include="..\thirdparty\crypto\md5.cpp">
  <ObjectFileName>$(IntDir)md5_impl.obj</ObjectFileName>
</ClCompile>
<ClCompile Include="..\thirdparty\crypto\sha1.cpp">
  <ObjectFileName>$(IntDir)sha1_impl.obj</ObjectFileName>
</ClCompile>
```

This ensures:
- `BaseLibs/src/SHA1.cpp` compiles to `SHA1.obj` (HashLib wrapper)
- `thirdparty/crypto/sha1.cpp` compiles to `sha1_impl.obj` (implementation)
- Both object files are included in the final `baselibs_x86_mt_debug.lib`

## Build Instructions

1. **Clean** the solution first: Build → Clean Solution
2. **Rebuild** the solution: Build → Rebuild Solution
3. Both object files should now be included without conflicts

The unresolved external symbol errors should be completely resolved.
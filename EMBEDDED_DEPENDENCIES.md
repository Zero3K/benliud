# Embedded Dependencies Documentation

This document describes the replacement of external dependencies with user-specific libraries for easier building.

## Overview

The benliud project has been updated to use specific implementations from the project maintainer's repositories, providing better integration and reduced dependencies.

## Dependencies Replaced

### 1. OpenSSL → Zero3K Hash Library
**Location**: `thirdparty/crypto/`
**Replaced**: OpenSSL for MD5 and SHA1 hashing
**Source**: https://github.com/Zero3K/hash-library

**Files**:
- `thirdparty/crypto/md5.h` and `md5.cpp` - Stephan Brumme's MD5 implementation
- `thirdparty/crypto/sha1.h` and `sha1.cpp` - Stephan Brumme's SHA1 implementation

**Projects Updated**:
- BaseLibs project includes the hash library source files
- Updated `BaseLibs/include/MD5.h` and `SHA1.h` to use new headers
- Updated `BaseLibs/src/MD5.cpp` and `SHA1.cpp` to use new API

### 2. GMP → Zero3K BigInteger Library  
**Location**: `thirdparty/bigint/`
**Replaced**: GNU Multiple Precision Arithmetic Library (GMP)
**Source**: https://github.com/Zero3K/BigInteger

**Files**:
- `thirdparty/bigint/bigint.h` and `bigint.cpp` - Complete big integer implementation

**Projects Updated**:
- benliud_bt project includes the BigInteger source files
- Updated `benliud_bt/include/MSE_BigInt.h` to use new header

### 3. TinyXML → Zero3K XML Library
**Location**: `thirdparty/tinyxml/`
**Replaced**: External TinyXML library dependency
**Source**: https://github.com/Zero3K/xml

**Files**:
- `thirdparty/tinyxml/tinyxml.h` - Compatibility layer for xml3all.h

**Projects Updated**:
- benliud_upnp project includes the XML library
- Updated `benliud_upnp/src/UPnpNatParser.cpp` and `UPnpNatController.cpp` to use new header

### 4. SQLite → Zero3K Metakit Database
**Location**: `thirdparty/sqlite/`  
**Replaced**: External SQLite library dependency
**Source**: https://github.com/Zero3K/metakit

**Files**:
- `thirdparty/sqlite/mk4.h` and `mk4.cpp` - Metakit-based SQLite compatibility interface

**Projects Updated**:
- benliud main project includes the Metakit database interface
- Updated `benliud/include/DBOperator.h` to use new header

## Build Benefits

1. **Zero External Dependencies**: All required functionality is embedded from trusted sources
2. **Simplified Build Process**: No need to separately download, compile, and configure external libraries
3. **Self-Contained**: The entire project can be built with just Visual Studio 2019+
4. **Curated Libraries**: All dependencies come from the project maintainer's repositories

## Library Details

### Zero3K Hash Library
- **Features**: MD5, SHA1, SHA256, SHA3, CRC32, and more hash algorithms
- **Quality**: Professional implementation by Stephan Brumme
- **Size**: Compact and efficient C++ implementation
- **License**: Public domain / permissive

### Zero3K BigInteger
- **Features**: Full arbitrary precision arithmetic
- **Operations**: Add, subtract, multiply, divide, compare, increment/decrement
- **Size**: Lightweight string-based implementation  
- **License**: Compatible with project requirements

### Zero3K XML
- **Features**: Complete XML parsing and generation
- **Compatibility**: Drop-in replacement for TinyXML
- **Size**: Single header implementation available
- **License**: Compatible with project requirements

### Zero3K Metakit
- **Features**: Embedded database engine
- **Compatibility**: SQLite-compatible interface layer
- **Size**: Lightweight compared to full SQLite
- **License**: Open source database library

## Production Considerations

The embedded libraries provide:

1. **Cryptography**: Professional hash implementations suitable for file verification and checksums
2. **Big Integers**: Full arbitrary precision arithmetic for MSE encryption operations  
3. **Database**: Structured data storage with commit/rollback capabilities
4. **XML**: Complete XML parsing for configuration and UPnP operations

## Building

With embedded dependencies, building is now simplified:

1. Open any `.sln` file in Visual Studio 2019+
2. Build the solution
3. All dependencies are automatically included

No additional configuration or external library installation required.
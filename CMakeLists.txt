# RanOnline Asura — CMake bootstrap (PHASE 1, STEP 1)
# Non-invasive: does NOT replace RanOnline.sln / .vcxproj.
# Start with Lib_Helper only to prove C++17 flags before wider rollout.
cmake_minimum_required(VERSION 3.28)

project(RanOnline_Asura LANGUAGES CXX C)

# Global C++ standard — mirrors v143 default upgrade path
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Preserve legacy Win32/MBCS model for now (do NOT flip to Unicode yet)
# All targets below stay Win32-compatible; x64 added later after audit.
if(MSVC)
    add_compile_options(
        /W4            # warnings as diagnostics (not /WX yet)
        /permissive-   # conformance mode (was <ConformanceMode>false</ConformanceMode>)
        /utf-8         # source charset explicit; does NOT change MBCS CharacterSet
        /GS            # buffer security check (was <BufferSecurityCheck>false</BufferSecurityCheck>)
        /EHsc          # C++ exceptions (MFC-compatible)
        /Zc:__cplusplus # report correct __cplusplus for C++17 feature checks
        /MP            # multiprocessor compile (matches <MultiProcessorCompilation>true)
    )
    add_compile_definitions(
        WIN32
        _WINDOWS
        WINVER=0x0A00
        _WIN32_WINNT=0x0A00
    )
endif()

# Test target: smallest static lib, no Engine/Network deps
add_subdirectory(Lib_Helper)

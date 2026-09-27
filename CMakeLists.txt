cmake_minimum_required(VERSION 3.28)

project(RanOnline_Asura LANGUAGES CXX C)

set(CMAKE_SIZEOF_VOID_P 4)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

foreach(cfg DEBUG RELEASE)
    string(TOLOWER "${cfg}" cfg_low)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR}/${cfg})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR}/${cfg})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR}/${cfg})
endforeach()

# ---------------------------------------------------------------------------
# Repository layout
#
#   legacy/  original RAN implementation - reference only, never modernised
#   modern/  new authoritative RAN implementation
#
# See docs/MODERN_ARCHITECTURE.md for the dependency rules.
# ---------------------------------------------------------------------------

# The legacy tree is deliberately NOT added here.
#
# It used to be built only because modern/compatibility/legacy linked
# Lib_Client. That adapter is not built during CORE-001, so nothing in the
# CMake build needs legacy/ any more, and configuring it would only slow every
# build down and invite a dependency nobody intended.
#
# The one remaining modern -> legacy reference is an include in
# modern/tools/exptable_dump.cpp, which reaches its headers by relative path
# and links nothing.
#
# The legacy tree still builds on its own, with MSBuild and the Visual Studio
# solution: legacy/RanOnline.sln. See docs/MODERN_ARCHITECTURE.md.
#
# To re-enable the bridge, restore the add_subdirectory() calls below and
# re-add modern/compatibility/legacy to modern/CMakeLists.txt.
#
#   add_subdirectory(legacy/Lib_Helper)
#   add_subdirectory(legacy/Lib_ZLib)
#   add_subdirectory(legacy/BugTrap)
#   add_subdirectory(legacy/Lib_Engine)
#   add_subdirectory(legacy/Lib_Network)
#   add_subdirectory(legacy/Lib_ClientUI)
#   add_subdirectory(legacy/Lib_Client)

# Headless rule tests.
#
# enable_testing() must be called here, before the add_subdirectory() calls:
# CTest generates a CTestTestfile.cmake per directory, and a directory only
# gets one — along with its subdirs() list — if testing was already enabled
# when it was processed. Calling it afterwards silently registers nothing.
enable_testing()

# Modern source.
add_subdirectory(modern)

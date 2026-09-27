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

# Legacy libraries. These are built because modern/compatibility/legacy
# (the one sanctioned bridge) links Lib_Client. The modern core itself has
# no legacy dependency.
add_subdirectory(legacy/Lib_Helper)
add_subdirectory(legacy/Lib_ZLib)
add_subdirectory(legacy/BugTrap)
add_subdirectory(legacy/Lib_Engine)
add_subdirectory(legacy/Lib_Network)
add_subdirectory(legacy/Lib_ClientUI)
add_subdirectory(legacy/Lib_Client)

# Modern source.
add_subdirectory(modern)

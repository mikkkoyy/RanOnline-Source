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

add_subdirectory(Lib_Helper)
add_subdirectory(Lib_ZLib)
add_subdirectory(BugTrap)
add_subdirectory(Lib_Engine)
add_subdirectory(Lib_Network)
add_subdirectory(Lib_ClientUI)
add_subdirectory(Lib_Client)

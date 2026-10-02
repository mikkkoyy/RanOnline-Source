# VERTICAL-027: modern network boundary.
#
# Depends on Modern (core) and nothing else. It must never be depended on BY
# core: core stays free of sockets, transport and protocol concerns, which is
# the rule stated at the top of core/CMakeLists.txt.
#
#     core  <-  network / database / server / client
#     core  X   legacy
#
# This target links Modern for two reasons only: Result/Status/ErrorCode, which
# are ordinary domain vocabulary, and nothing else. No legacy RAN type appears
# here or is permitted to.

set(MODERN_NETWORK_SOURCES
    NetworkTypes.cpp
    NetworkCodec.h
    NetworkConnection.h
    NetworkTransport.h
    LoopbackTransport.h
    MessageRouter.h
    ServerSession.h
)

add_library(ModernNetwork STATIC ${MODERN_NETWORK_SOURCES})

target_include_directories(ModernNetwork PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ModernNetwork PUBLIC Modern)
target_compile_features(ModernNetwork PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(ModernNetwork PRIVATE /W4 /permissive- /utf-8)
endif()

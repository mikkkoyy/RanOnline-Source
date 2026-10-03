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
    # VERTICAL-030: server->client NET_COMPRESS + LZO1X layer. CompressionCodec
    # owns the only LZO dependency in the modern tree; NetCompressCodec owns the
    # 12-byte envelope; ServerBatchEncoder owns the 1000-byte flush trigger.
    CompressionCodec.h
    CompressionCodec.cpp
    NetCompressCodec.h
    NetCompressCodec.cpp
    ServerBatchEncoder.h
    ServerBatchEncoder.cpp
    # WORLD-001: the client -> server login protocol. MinTeaCodec reproduces the
    # legacy field cipher; LoginProtocol owns NET_MSG_LOGIN_2 framing, the garbage
    # token, and field encode/decode. Login is RAW and is never routed through the
    # V030 compression layer above - see LoginProtocol.h.
    MinTeaCodec.h
    MinTeaCodec.cpp
    LoginProtocol.h
    LoginProtocol.cpp
    # WORLD-002: NET_MSG_LOGIN_FB response codec (120 bytes, NOT minTea encrypted).
    LoginResponseProtocol.h
    LoginResponseProtocol.cpp
    # LOGIN-001: Login Server game-server list (REQ/SND_GAME_SVR, SND_GAME_SVR_END).
    GameServerListProtocol.h
    GameServerListProtocol.cpp
    # NETWORK-001: the real Winsock TCP layer. This is the only part of the modern
    # tree that includes <winsock2.h>, and it is why ws2_32 is linked below.
    #
    # WinSockSupport exists so that the transport and the listener share ONE mapping
    # from platform error to TransportFault. Two copies of that table would diverge,
    # and the divergence would show up as a fault one socket reports and the other
    # does not.
    WinSockSupport.h
    WinSockSupport.cpp
    WinSockRuntime.h
    WinSockRuntime.cpp
    SocketAddress.h
    SocketAddress.cpp
    TcpTransport.h
    TcpTransport.cpp
    TcpListener.h
    TcpListener.cpp
    # Vendored miniLZO 2.10, unmodified. Provenance and the open licensing
    # question are recorded in third_party/minilzo/VENDOR.md. Compiled as C
    # because that is what upstream ships; ModernNetwork itself stays C++17.
    third_party/minilzo/minilzo.c
    third_party/minilzo/minilzo.h
    third_party/minilzo/lzoconf.h
    third_party/minilzo/lzodefs.h
    third_party/minilzo/README.LZO
    third_party/minilzo/VENDOR.md
)

add_library(ModernNetwork STATIC ${MODERN_NETWORK_SOURCES})

target_include_directories(ModernNetwork PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ModernNetwork PUBLIC Modern)
target_compile_features(ModernNetwork PUBLIC cxx_std_17)

# NETWORK-001: Winsock.
#
# ws2_32 is a PRIVATE link. ModernNetwork's own headers deliberately name no
# Winsock type and no Winsock constant - handles cross as uintptr_t - so nothing
# that links ModernNetwork can possibly need Winsock itself. Making it PUBLIC would
# be a lie about the interface and would let a socket call leak into a consumer
# that never included a socket header.
#
# The loopback transport and every protocol codec above this line still build and
# run without a socket ever being opened, which is why ModernCoreTests links Modern
# alone and stays socket free.
target_link_libraries(ModernNetwork PRIVATE ws2_32)

if(MSVC)
    target_compile_options(ModernNetwork PRIVATE /W4 /permissive- /utf-8)
    # The vendored C is upstream code we must not "fix"; it is compiled at its
    # own warning level and excluded from ours so /W4 stays meaningful for code
    # this project owns.
    set_source_files_properties(third_party/minilzo/minilzo.c PROPERTIES
        LANGUAGE C
        COMPILE_OPTIONS "/W0")
endif()

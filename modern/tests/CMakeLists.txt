# Headless rule tests.
#
# These link Modern and nothing else. No renderer, no socket, no database, no
# legacy library. If a test here needs one of those, it is not a rule test and
# belongs with the adapter that owns the format.

add_executable(ModernCoreTests
    CoreFoundationTests.cpp
    StatCalculationTests.cpp
    EquipmentTests.cpp
    SkillTests.cpp
    ActiveSkillTests.cpp
    CodexTests.cpp
    CombatTests.cpp
    StatusEffectTests.cpp
    # WORLD-ENTRY-002d: RAN navigation kernel and WLD navigation asset reader.
    NavigationTests.cpp
    WldNavigationTests.cpp
    # WORLD-ENTRY-002e: map identity, mapslist.mst decoding and WLD binding.
    MapRegistryTests.cpp
    # WORLD-ENTRY-002f: the recovered 16-row class speed table, the base-term
    # speed formula, and the headless Actor's probes, arrival threshold, injected
    # elapsed-time stepping and blocked motion.
    MovementTests.cpp
SkillFactTests.cpp
)

target_link_libraries(ModernCoreTests PRIVATE Modern)

target_compile_features(ModernCoreTests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernCoreTests PRIVATE /W4 /permissive- /utf-8)
endif()

# VERTICAL-027: network boundary tests.
#
# A separate executable rather than more files in ModernCoreTests, because it
# links ModernNetwork. ModernCoreTests deliberately links Modern alone, and
# loosening that to reach one header would weaken a rule the whole project
# depends on: core is free of transport and protocol concerns.
add_executable(ModernNetworkTests
    NetworkTests.cpp
    # VERTICAL-030: server->client NET_COMPRESS + LZO1X layer.
    NetworkCompressTests.cpp
    # WORLD-001: client -> server login protocol (minTea, garbage token, framing).
    LoginProtocolTests.cpp
    # WORLD-002: NET_MSG_LOGIN_FB response codec (120 bytes, unencrypted).
    LoginResponseProtocolTests.cpp
    # LOGIN-001: Login Server game-server list (REQ/SND_GAME_SVR, SND_GAME_SVR_END).
    GameServerListProtocolTests.cpp
    # WORLD-ENTRY-001 Phase A: protocol layout tests.
    #
    # Pure codec tests. No socket, because these packets are framed and measured
    # independently of a transport - which is the point of Phase A being separate
    # from the later Agent/Field sessions.
    WorldEntryProtocolTests.cpp
    # WORLD-ENTRY-002f: the GOTO codec pair, 3034 and 3035.
    #
    # Pure layout and codec tests. The 60-unit rule, the vertical probe and
    # the arrival threshold are not here: they are decided AFTER a 3034 is
    # decoded, so they belong to the movement and GOTO rule tests.
    GotoProtocolTests.cpp
    # WORLD-ENTRY-002h: the UPDATE_STATE (3046) and UPDATE_STATE_BRD (3053)
    # codec pair.
    UpdateStateProtocolTests.cpp
)

target_link_libraries(ModernNetworkTests PRIVATE ModernNetwork)

target_compile_features(ModernNetworkTests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernNetworkTests PRIVATE /W4 /permissive- /utf-8)
endif()

add_test(NAME ModernNetworkTests COMMAND ModernNetworkTests)

# NETWORK-001: real-socket tests.
#
# A SEPARATE executable, and the reason is not tidiness. ModernNetworkTests links
# ModernNetwork and is a pure rule-test binary: it must stay runnable with no
# socket, no clock and no network, because that is what makes it deterministic and
# safe to run anywhere. These tests open real sockets on 127.0.0.1, so folding them
# into that binary would make the rule tests depend on the machine's network stack -
# and a test suite that can fail because of a firewall is a test suite nobody runs.
#
# Both binaries link the same library. The only difference is that this one
# exercises the Winsock backend behind INetworkTransport rather than the loopback
# pair. ws2_32 is already linked into ModernNetwork as a PRIVATE dependency, so
# nothing extra is needed here - which is itself the check that ModernNetwork's
# headers really do name no Winsock type.
add_executable(ModernTcpTransportTests
    TcpTransportTests.cpp
)

target_link_libraries(ModernTcpTransportTests PRIVATE ModernNetwork)

target_compile_features(ModernTcpTransportTests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernTcpTransportTests PRIVATE /W4 /permissive- /utf-8)
endif()

add_test(NAME ModernTcpTransportTests COMMAND ModernTcpTransportTests)

# enable_testing() is called by the top-level CMakeLists.txt, which is where
# CTest needs it in order to reach this directory.
add_test(NAME ModernCoreTests COMMAND ModernCoreTests)

# Modern server: the authoritative side of the gameplay model.
#
# VERTICAL-001 establishes only the authoritative character. It links Modern and
# nothing else: no socket, no database, no renderer, no legacy library, and no
# DirectX or Windows type. A character here is a headless value with rules.
#
# What is deliberately not here yet: a transport, a session, a login or agent
# process, persistence, maps, monsters, NPCs, quests, or combat. Each of those
# belongs to a later vertical slice, and each would consume what this target
# already owns rather than the other way round.

set(MODERN_SERVER_SOURCES
    character/ServerCharacter.cpp
    # WORLD-001: the login application boundary. LoginReceiver frames-to-verdict;
    # ILoginAuthenticator is the seam a real account store will implement.
    login/LoginReceiver.cpp
    login/LoginReceiver.h
    login/LoginResponder.cpp
    login/LoginResponder.h
    # LOGIN-001: Login Server game-server-list response. A sibling of LoginResponder,
    # not a stage after it - separate connection, separate protocol, separate phase.
    login/LoginServerResponder.cpp
    login/LoginServerResponder.h
    # LOGIN-002: the runnable Login Server. LoginServerConfig is the whole
    # configuration surface (bind address, port, grid) and LoginServerRuntime is the
    # bind / listen / accept / serve loop over ModernNetwork's TcpListener and
    # TcpTransport - still no threads and no IOCP, because this milestone serves one
    # connection at a time and a threaded server would only make the tests flaky.
    login/LoginServerConfig.cpp
    login/LoginServerConfig.h
    login/LoginServerRuntime.cpp
    login/LoginServerRuntime.h
    # WORLD-ENTRY-001 Phase B: the authoritative world-entry domain and the
    # Agent/Field roles' session state. NO TCP LISTENERS HERE ON PURPOSE - Phase C
    # binds these services to real sockets; keeping them socket-free is what lets
    # every rule below be tested without a network.
    #
    # CharacterRepository is the OWNERSHIP boundary: FindOwned is the modern
    # equivalent of legacy's `WHERE ChaNum=? AND UserNum=?` SQL predicate, written
    # as an interface method so it cannot be skipped.
    #
    # WorldEntryService holds a FieldEntryRegistry by reference rather than
    # containing it, because in legacy the Agent and the Field are different
    # processes with different owners - an Agent that could mint its own
    # authorization would be an Agent that could forge its own proof of entry.
    world/CharacterClassMovementSpeed.cpp
    world/GotoService.cpp
    # WORLD-ENTRY-002i: the ATTACK validation rule (3036). Pure; no damage.
    world/AttackService.cpp
    # WORLD-ENTRY-002k: the damage resolution boundary. Pure; no I/O, no clock.
    world/DamageResolution.cpp
    world/WorldCharacter.cpp
    world/WorldMovementRuntime.cpp
    world/WorldCharacter.h
    world/CharacterRepository.cpp
    world/CharacterRepository.h
    world/AgentSession.cpp
    world/AgentSession.h
    world/CharacterSelectService.cpp
    world/CharacterSelectService.h
    world/WorldEntryService.cpp
    world/WorldEntryService.h
    world/FieldSession.cpp
    world/FieldSession.h
    # WORLD-ENTRY-001 Phase C: the two TCP listeners and the process that owns them.
    #
    # Phase C adds NO new rule. Every decision about ownership, selection,
    # authorization, gaeaId allocation and spawn content is still Phase B's, reached
    # through the same objects; these files translate packets into service calls and
    # nothing else. AgentRoleRuntime and FieldRoleRuntime own a TcpListener each -
    # TWO listeners, because the brief's whole claim is that the client-visible
    # topology is two real connections, and one socket wearing two hats would prove
    # nothing.
world/WorldServerConfig.cpp
    world/WorldServerConfig.h
    world/AgentRoleRuntime.cpp
    world/AgentRoleRuntime.h
    world/FieldRoleRuntime.cpp
    world/FieldRoleRuntime.h
    world/WorldServerRuntime.cpp
    world/WorldServerRuntime.h
    # WORLD-ENTRY-002a: the authoritative movement-state rule. A pure function over
    # WorldCharacter - no socket, no packet, no actor - plus the named speed seam that
    # WORLD-ENTRY-002a deliberately leaves unimplemented.
    world/MovementStateService.cpp
    world/MovementStateService.h
    # WORLD-ENTRY-002h: the authoritative resource synchronisation service.
    world/ResourceSyncService.cpp
    world/ResourceSyncService.h
)

add_library(ModernServer STATIC ${MODERN_SERVER_SOURCES})

target_include_directories(ModernServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# The one direction of dependency: server -> core. Never core -> server.
#
# WORLD-001 adds server -> ModernNetwork. LoginReceiver consumes a decoded
# LoginRequest, so it needs the type; the dependency is deliberate and one-way,
# and ModernNetwork in turn links only Modern. A server that receives bytes from
# the network depending on the network codec is the intended direction, not a
# layering inversion. What it does NOT do is reach a socket: LoginReceiver takes
# an already-framed buffer.
target_link_libraries(ModernServer PUBLIC Modern ModernNetwork)

target_compile_features(ModernServer PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(ModernServer PRIVATE /W4 /permissive- /utf-8)
endif()

# Headless server rule tests. They link ModernServer and therefore Modern and
# ModernNetwork, and nothing else: no socket, no database, no legacy library.
add_executable(ModernServerTests
    ServerCharacterTests.cpp
    LoginReceiverTests.cpp
    LoginResponderTests.cpp
    # LOGIN-001: Login Server responder + client end-to-end and fragmentation tests.
    LoginServerTests.cpp
    # WORLD-ENTRY-001 Phase B: repository ownership, character selection, world-entry
    # authorization, Field 2359 validation, spawn generation, and the illegal state
    # transitions. Headless, like the rest of this binary.
    WorldEntryServiceTests.cpp
    # WORLD-ENTRY-002a: the authoritative movement-state rule. Headless like the
    # rest - 3032's meaning is decided before any byte is framed, so testing it
    # needs no socket. WorldEntryTcpTests then proves the same rule survives a real
    # TCP connection.
    MovementStateServiceTests.cpp
    # WORLD-ENTRY-002f: the GOTO rule - the 60-unit desync check, the legacy
    # validation ORDER, the silence of a refusal, and the class speed seam.
    GotoServiceTests.cpp
    # WORLD-ENTRY-002i: the ATTACK validation rule - deterministic, no sockets.
    AttackServiceTests.cpp
    # WORLD-ENTRY-002k: the damage resolution boundary - deterministic, no sockets.
    DamageResolutionTests.cpp
    # WORLD-ENTRY-002f: the Field role's movement runtime - the actor registry,
    # injected-time Tick progression, snapshots and multi-actor isolation.
    WorldMovementRuntimeTests.cpp
    # WORLD-ENTRY-002h: the authoritative resource synchronisation service.
    ResourceSyncServiceTests.cpp
)

target_include_directories(ModernServerTests PRIVATE
    ${CMAKE_SOURCE_DIR}/modern/tests
)

target_link_libraries(ModernServerTests PRIVATE ModernServer ModernClientLogin)

target_compile_features(ModernServerTests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernServerTests PRIVATE /W4 /permissive- /utf-8)
endif()

# enable_testing() is called by the top-level CMakeLists.txt.
add_test(NAME ModernServerTests COMMAND ModernServerTests)

# LOGIN-002: real Winsock TCP integration for the Login Server.
#
# A SEPARATE executable from ModernServerTests, and the reason is the same one that
# split ModernTcpTransportTests out of ModernNetworkTests: ModernServerTests is a
# headless rule-test binary that must keep running with no socket and no network,
# because that is what makes it deterministic and safe to run anywhere. These open
# real sockets on 127.0.0.1:0.
#
# It links ModernClientLogin as well as ModernServer, because the milestone is the
# EXCHANGE: a real client and a real server talking to each other, which needs both
# halves. LOGIN-001's own end-to-end test already links both through
# ModernServerTests; this one just puts a socket between them.
add_executable(ModernLoginServerTcpTests
    LoginServerTcpTests.cpp
)

target_include_directories(ModernLoginServerTcpTests PRIVATE
    ${CMAKE_SOURCE_DIR}/modern/tests
)

target_link_libraries(ModernLoginServerTcpTests PRIVATE ModernServer ModernClientLogin)

target_compile_features(ModernLoginServerTcpTests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernLoginServerTcpTests PRIVATE /W4 /permissive- /utf-8)
endif()

add_test(NAME ModernLoginServerTcpTests COMMAND ModernLoginServerTcpTests)

# WORLD-ENTRY-001 Phase C: real Winsock TCP integration for the two-connection
# world-entry flow.
#
# A SEPARATE executable from ModernServerTests, for the same reason
# ModernLoginServerTcpTests is separate: ModernServerTests is a headless rule-test
# binary that must keep running with no socket and no network, because that is what
# makes it deterministic and safe to run anywhere. These bind 127.0.0.1:0 and prove
# the thing a byte queue cannot - that a client really opens a SECOND connection to
# the endpoint a 2358 named.
#
# It links ModernClientLogin because the milestone is the EXCHANGE: a real client
# and a real server, which needs both halves.
add_executable(ModernWorldEntryTcpTests
    WorldEntryTcpTests.cpp
)

target_include_directories(ModernWorldEntryTcpTests PRIVATE
    ${CMAKE_SOURCE_DIR}/modern/tests
)

target_link_libraries(ModernWorldEntryTcpTests PRIVATE ModernServer ModernClientLogin)

target_compile_features(ModernWorldEntryTcpTests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernWorldEntryTcpTests PRIVATE /W4 /permissive- /utf-8)
endif()

add_test(NAME ModernWorldEntryTcpTests COMMAND ModernWorldEntryTcpTests)
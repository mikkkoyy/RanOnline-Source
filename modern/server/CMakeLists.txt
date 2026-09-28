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
)

add_library(ModernServer STATIC ${MODERN_SERVER_SOURCES})

target_include_directories(ModernServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# The one direction of dependency: server -> core. Never core -> server.
target_link_libraries(ModernServer PUBLIC Modern)

target_compile_features(ModernServer PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(ModernServer PRIVATE /W4 /permissive- /utf-8)
endif()

# Headless server rule tests. They link ModernServer and therefore Modern, and
# nothing else.
add_executable(ModernServerTests
    ServerCharacterTests.cpp
)

target_include_directories(ModernServerTests PRIVATE
    ${CMAKE_SOURCE_DIR}/modern/tests
)

target_link_libraries(ModernServerTests PRIVATE ModernServer)

target_compile_features(ModernServerTests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernServerTests PRIVATE /W4 /permissive- /utf-8)
endif()

# enable_testing() is called by the top-level CMakeLists.txt.
add_test(NAME ModernServerTests COMMAND ModernServerTests)

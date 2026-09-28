# Modern client gameplay presentation: the non-authoritative character state.
#
# VERTICAL-001. ModernClientGameplay depends on Modern (the shared snapshot, the
# identity and stat value types) and nothing else -- no legacy, no DirectX, no
# <Windows.h>, no socket, no database, and no renderer.
#
# The dependency direction is the whole point of this target: it links core and
# can therefore be handed a snapshot, but it has no stat calculator, no class
# table and no way to produce a derived value. A future HUD binds to what the
# server published.

add_library(ModernClientGameplay STATIC
    ClientCharacterState.cpp
)

target_include_directories(ModernClientGameplay PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(ModernClientGameplay PUBLIC Modern)

target_compile_features(ModernClientGameplay PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(ModernClientGameplay PRIVATE /W4 /permissive- /utf-8)
endif()

# Headless gameplay tests. They also link ModernServer, because the interesting
# assertions are about the *relationship* between the two sides: that the client
# presents what the server computed and invents nothing. That is a test
# dependency only; ModernClientGameplay itself has no knowledge of the server.
add_executable(ModernClientGameplayTests
    ClientGameplayTests.cpp
)

target_include_directories(ModernClientGameplayTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/modern/tests
)

target_link_libraries(ModernClientGameplayTests PRIVATE ModernClientGameplay ModernServer)

target_compile_features(ModernClientGameplayTests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernClientGameplayTests PRIVATE /W4 /permissive- /utf-8)
endif()

add_test(NAME ModernClientGameplayTests COMMAND ModernClientGameplayTests)

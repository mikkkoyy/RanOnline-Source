# Modern client input layer: platform-independent events, per-frame queue,
# current state, and the Win32 translation boundary.
#
# Depends on Modern (core Result/Status conventions) and nothing else. No
# legacy, no <Windows.h> in public headers, no DirectX, no sockets.
# modern/core must never depend on this target.

add_library(ModernClientInput STATIC
    FakeInputSource.cpp
    InputEvents.cpp
    InputSystem.cpp
    platform/Win32InputAdapter.cpp
)

target_include_directories(ModernClientInput PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(ModernClientInput PUBLIC Modern)

target_compile_features(ModernClientInput PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(ModernClientInput PRIVATE /W4 /permissive- /utf-8)
endif()

# Headless input tests. Link ModernClientApplication (which links
# ModernClientInput and Modern) and nothing else: no window, no DirectX,
# no legacy library.
add_executable(ModernClientInputTests
    ClientInputTests.cpp
)

target_include_directories(ModernClientInputTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/modern/tests
)

target_link_libraries(ModernClientInputTests PRIVATE ModernClientApplication)

target_compile_features(ModernClientInputTests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernClientInputTests PRIVATE /W4 /permissive- /utf-8)
endif()

# enable_testing() is called by the top-level CMakeLists.txt, which is where
# CTest needs it in order to reach this directory.
add_test(NAME ModernClientInputTests COMMAND ModernClientInputTests)

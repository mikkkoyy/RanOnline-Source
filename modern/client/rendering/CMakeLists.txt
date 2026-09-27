# Modern client rendering layer: platform-independent renderer abstraction,
# configuration types, and headless null renderer.
#
# Depends on Modern (core Result/Status conventions) and nothing else.
# No legacy, no DirectX/Vulkan/OpenGL headers, no <Windows.h> in public headers.

add_library(ModernClientRendering STATIC
    Renderer.cpp
    RenderingTypes.cpp
    NullRenderer.cpp
)

target_include_directories(ModernClientRendering PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(ModernClientRendering PUBLIC Modern)

target_compile_features(ModernClientRendering PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(ModernClientRendering PRIVATE /W4 /permissive- /utf-8)
endif()

# Headless rendering tests. Link ModernClientApplication (which links
# ModernClientRendering, ModernClientInput, and Modern) and test harness.
add_executable(ModernClientRenderingTests
    ClientRenderingTests.cpp
)

target_include_directories(ModernClientRenderingTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/modern/tests
)

target_link_libraries(ModernClientRenderingTests PRIVATE ModernClientApplication ModernClientRendering)

target_compile_features(ModernClientRenderingTests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernClientRenderingTests PRIVATE /W4 /permissive- /utf-8)
endif()

add_test(NAME ModernClientRenderingTests COMMAND ModernClientRenderingTests)

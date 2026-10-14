# Modern client application layer: lifecycle, update loop, platform boundary.
#
# Depends on Modern (core Result/Status conventions) and ModernClientInput.
# No legacy, no Win32, no DirectX, no sockets. modern/core must never depend
# on this target: the dependency flows one way, client -> core.

add_library(ModernClientApplication STATIC
    Application.cpp
)

target_include_directories(ModernClientApplication PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(ModernClientApplication PUBLIC Modern ModernClientInput ModernClientRendering)

target_compile_features(ModernClientApplication PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(ModernClientApplication PRIVATE /W4 /permissive- /utf-8)
endif()

# Headless application tests. Link ModernClientApplication (which links
# Modern) and nothing else: no renderer, no socket, no legacy library.
add_executable(ModernClientAppTests
    ClientApplicationTests.cpp
)

target_include_directories(ModernClientAppTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/modern/tests
)

target_link_libraries(ModernClientAppTests PRIVATE ModernClientApplication)

target_compile_features(ModernClientAppTests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernClientAppTests PRIVATE /W4 /permissive- /utf-8)
endif()

# enable_testing() is called by the top-level CMakeLists.txt, which is where
# CTest needs it in order to reach this directory.
add_test(NAME ModernClientAppTests COMMAND ModernClientAppTests)

# CLIENT-RENDER-001B: the production executable. Not a test: it opens a
# native window, creates a real D3D11 device, uploads the ASURA character
# mesh and draws it every frame until the window closes.
#
# It deliberately uses the real backends only -- D3D11Renderer and
# D3D11AssetUploader, never the Null implementations -- and pulls in the
# Win32 platform boundary, the asset pipeline (MXF transform + X decoder)
# and the storage layer (file provider + resource manager) through the
# same targets the headless tests use.
#
# The graphics libraries come transitively through ModernClientRendering /
# ModernClientAssetUpload (PUBLIC), but naming them here too keeps the
# executable's own dependency on the D3D11 runtime explicit.
add_executable(ModernClientApp
    ModernClientApp.cpp
)

target_include_directories(ModernClientApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(ModernClientApp PRIVATE
    ModernClientApplication
    ModernClientAssetUpload
    ModernClientAssets
    ModernClientResources
    d3d11
    dxgi
    d3dcompiler
)

target_compile_features(ModernClientApp PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernClientApp PRIVATE /W4 /permissive- /utf-8)
endif()

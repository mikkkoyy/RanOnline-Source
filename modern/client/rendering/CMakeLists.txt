# Modern client rendering layer: platform-independent renderer abstraction,
# configuration types, headless null renderer (CLIENT-004), and the renderer
# asset upload boundary (CLIENT-009).
#
# ModernClientRendering depends on Modern (core Result/Status conventions) and
# nothing else -- no legacy, no DirectX/Vulkan/OpenGL headers, no <Windows.h>
# in public headers. ModernClientAssetUpload is the *separate* adapter target
# below: it links ModernClientAssets (CPU values) and ModernClientRendering
# (IRenderer / RendererState) on its behalf, so neither of those two learns
# about the other and no dependency cycle is possible.

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

# CLIENT-009: the renderer asset upload boundary. Deliberately its own target
# rather than part of ModernClientRendering:
#
#   ModernClientAssets ----\
#                           >-- ModernClientAssetUpload
#   ModernClientRendering --/
#
# The adapter links both sides; neither side links the adapter or the other.
# ModernClientAssets still compiles without any rendering target (its CMake
# block is untouched), and ModernClientRendering still links Modern only.
# This target names no graphics API, no Windows/MFC header and no legacy
# library, and -- although ModernClientResources arrives transitively through
# the asset layer for the decoders -- no storage type (ResourceId,
# ResourceData, ResourceManager, IResourceProvider) appears in its sources or
# public headers: the uploader receives already-decoded CPU assets.
add_library(ModernClientAssetUpload STATIC
    NullAssetUploader.cpp
)

target_include_directories(ModernClientAssetUpload PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(ModernClientAssetUpload PUBLIC ModernClientAssets ModernClientRendering)

target_compile_features(ModernClientAssetUpload PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(ModernClientAssetUpload PRIVATE /W4 /permissive- /utf-8)
endif()

# Headless upload-boundary tests. The two pipeline cases walk the storage
# layer end to end (provider -> ResourceManager -> decoder -> asset ->
# uploader), so ModernClientResources is linked explicitly beside the adapter
# and the asset/rendering layers. No GPU, no window, no legacy library.
add_executable(ModernClientAssetUploadTests
    ClientAssetUploadTests.cpp
)

target_include_directories(ModernClientAssetUploadTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/modern/tests
)

target_link_libraries(ModernClientAssetUploadTests PRIVATE
    ModernClientAssetUpload
    ModernClientAssets
    ModernClientRendering
    ModernClientResources
)

target_compile_features(ModernClientAssetUploadTests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernClientAssetUploadTests PRIVATE /W4 /permissive- /utf-8)
endif()

add_test(NAME ModernClientAssetUploadTests COMMAND ModernClientAssetUploadTests)

# TOOLS-002: headless message-processing boundary.
#
# ModernMessageQueue is a small, renderer-independent delayed-message queue --
# a modern equivalent of the legacy emulator's CEMULMSG / g_vecMsg buffer in
# legacy/Lib_Client/DxGlobalStage.cpp -- that can be unit-tested without MFC,
# D3D9, GLGaeaServer, or any compiled legacy library. It links Modern only.

add_library(ModernMessageQueue STATIC
    MessageQueue.cpp
)

target_include_directories(ModernMessageQueue PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ModernMessageQueue PUBLIC Modern)

target_compile_features(ModernMessageQueue PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(ModernMessageQueue PRIVATE /W4 /permissive- /utf-8)
endif()

add_executable(ModernEmulator main.cpp)
# Links every modern slice the demonstration exercises: the asset layer
# (CLIENT-007) sits above the resource layer, which sits above core, and the
# asset upload boundary (CLIENT-009) consumes decoded assets from the
# rendering side.
target_link_libraries(ModernEmulator PRIVATE
    Modern
    ModernClientApplication
    ModernClientInput
    ModernClientRendering
    ModernClientResources
    ModernClientAssets
    ModernClientAssetUpload
    ModernMessageQueue
)

target_include_directories(ModernEmulator PRIVATE
    ${CMAKE_SOURCE_DIR}/modern/client
)

target_compile_features(ModernEmulator PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernEmulator PRIVATE /W4 /permissive- /utf-8)
endif()

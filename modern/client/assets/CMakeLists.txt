# Modern client asset layer: typed CPU-side assets, and the decoder boundary
# that turns ResourceData into them (CLIENT-007 images, CLIENT-008 meshes).
#
# Depends on Modern (core Result/Status conventions) and on
# ModernClientResources, which supplies ResourceData -- the byte payload a
# decoder consumes. The edge is one-way and stated here rather than hidden in
# an include path: assets sit above resources.
#
# Deliberately does NOT depend on ModernClientRendering. Assets are CPU-side
# values and a future renderer adapter consumes them; rendering never becomes
# a dependency of decoding.

add_library(ModernClientAssets STATIC
    AssetTypes.cpp
    ImageAsset.cpp
    TestImageDecoder.cpp
    MeshAsset.cpp
    TestMeshDecoder.cpp
)

target_include_directories(ModernClientAssets PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(ModernClientAssets PUBLIC Modern ModernClientResources)

target_compile_features(ModernClientAssets PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(ModernClientAssets PRIVATE /W4 /permissive- /utf-8)
endif()

# Headless asset tests. These link the asset layer *and* the resource layer, so
# the provider -> ResourceManager -> ResourceData -> decoder -> asset path can
# be walked end to end over temporary data. No renderer, no graphics API, no
# legacy library.
add_executable(ModernClientAssetTests
    ClientAssetTests.cpp
)

target_include_directories(ModernClientAssetTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/modern/tests
)

target_link_libraries(ModernClientAssetTests PRIVATE ModernClientAssets ModernClientResources)

target_compile_features(ModernClientAssetTests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernClientAssetTests PRIVATE /W4 /permissive- /utf-8)
endif()

# Mesh boundary tests (CLIENT-008). A second executable, because the image
# suite owns its own main(): the two suites stay independent translation
# units and both register with CTest. Same linkage rules as above -- no
# renderer, no graphics API, no legacy library.
add_executable(ModernClientMeshAssetTests
    ClientMeshTests.cpp
)

target_include_directories(ModernClientMeshAssetTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/modern/tests
)

target_link_libraries(ModernClientMeshAssetTests PRIVATE ModernClientAssets ModernClientResources)

target_compile_features(ModernClientMeshAssetTests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernClientMeshAssetTests PRIVATE /W4 /permissive- /utf-8)
endif()

# enable_testing() is called by the top-level CMakeLists.txt, which is where
# CTest needs it in order to reach this directory.
add_test(NAME ModernClientAssetTests COMMAND ModernClientAssetTests)
add_test(NAME ModernClientMeshAssetTests COMMAND ModernClientMeshAssetTests)

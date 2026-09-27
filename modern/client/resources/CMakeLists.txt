# Modern client resource layer: platform-independent resource abstraction,
# identifiers, provider interfaces, memory test provider, and deterministic cache manager.
#
# Depends on Modern (core Result/Status conventions) and nothing else.
# No legacy, no DirectX/Vulkan/OpenGL headers, no Windows headers in public headers.

add_library(ModernClientResources STATIC
    ResourceId.cpp
    MemoryResourceProvider.cpp
    ResourceManager.cpp
)

target_include_directories(ModernClientResources PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(ModernClientResources PUBLIC Modern)

target_compile_features(ModernClientResources PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(ModernClientResources PRIVATE /W4 /permissive- /utf-8)
endif()

# Headless resource tests. Link ModernClientResources and Modern.
add_executable(ModernClientResourceTests
    ClientResourceTests.cpp
)

target_include_directories(ModernClientResourceTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/modern/tests
)

target_link_libraries(ModernClientResourceTests PRIVATE ModernClientResources Modern)

target_compile_features(ModernClientResourceTests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernClientResourceTests PRIVATE /W4 /permissive- /utf-8)
endif()

add_test(NAME ModernClientResourceTests COMMAND ModernClientResourceTests)

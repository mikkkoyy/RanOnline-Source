add_executable(ModernEmulator main.cpp)

target_link_libraries(ModernEmulator PRIVATE Modern ModernClientApplication ModernClientInput)

target_include_directories(ModernEmulator PRIVATE
    ${CMAKE_SOURCE_DIR}/modern/client
)

target_compile_features(ModernEmulator PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernEmulator PRIVATE /W4 /permissive- /utf-8)
endif()

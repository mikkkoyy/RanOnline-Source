add_library(ModernLegacyAdapter STATIC
    CharacterAdapter.cpp
)

target_link_libraries(ModernLegacyAdapter PRIVATE
    Modern
    Lib_Client
)

target_include_directories(ModernLegacyAdapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Lib_Client/G-Logic
)

target_compile_features(ModernLegacyAdapter PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(ModernLegacyAdapter PRIVATE /W4 /permissive-)
endif()

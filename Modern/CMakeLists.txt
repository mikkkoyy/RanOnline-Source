set(MODERN_SOURCES
    Character/Character.cpp
    Character/CharacterBaseData.cpp
    Progression/ProgressionData.cpp
)

add_library(Modern STATIC ${MODERN_SOURCES})

target_include_directories(Modern PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(Modern PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(Modern PRIVATE /W4 /permissive- /utf-8)
endif()

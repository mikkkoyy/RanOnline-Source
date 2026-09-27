# Modern core: game rules and domain models only.
#
# This target must stay free of Windows UI, DirectX, sockets, PostgreSQL and
# any legacy RAN type (GLChar, GLItemMan, SITEM, ...). If a change needs one
# of those, it belongs in modern/compatibility/legacy instead.

set(MODERN_SOURCES
    character/Character.cpp
    character/CharacterBaseData.cpp
    item/ItemData.cpp
    progression/ProgressionData.cpp
)

add_library(Modern STATIC ${MODERN_SOURCES})

target_include_directories(Modern PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(Modern PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(Modern PRIVATE /W4 /permissive- /utf-8)
endif()

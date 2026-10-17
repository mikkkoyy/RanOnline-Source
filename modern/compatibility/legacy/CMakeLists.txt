# The one sanctioned modern -> legacy bridge.
#
# Adapters here are allowed to understand legacy RAN formats and legacy RAN
# types. Nothing in modern/core may include anything from this directory, and
# this directory must never be linked into the core.
#
# Planned adapters: LegacyCharacterAdapter, LegacyItemAdapter,
#                   LegacyPacketAdapter, LegacyDataImporter
#
# WORLD-PLAYABLE-001A note: this directory is only reached now because the
# terrain reader below needs a home, and `modern/CMakeLists.txt` adds
# `compatibility` as a subdirectory for it. That makes every target here part of
# the build for the first time -- until now nothing added this directory, so
# `ModernLegacyAdapter` had never been compiled. It does not compile today:
# `CharacterAdapter.cpp` includes `types/Types.h`, which is not on any include
# path in this build (the other three headers it uses resolve). Rather than
# delete a target that is separate, pre-existing work, it is kept here and
# switched off by default. Turning MODERN_BUILD_LEGACY_ADAPTER on once those
# include paths are sorted out brings it back with no other change.

option(MODERN_BUILD_LEGACY_ADAPTER "Build the legacy -> modern adapter bridge" OFF)

if(MODERN_BUILD_LEGACY_ADAPTER)
    add_library(ModernLegacyAdapter STATIC
        CharacterAdapter.cpp
    )

    target_link_libraries(ModernLegacyAdapter PRIVATE
        Modern
        Lib_Client
    )

    target_include_directories(ModernLegacyAdapter PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/legacy/Lib_Client/G-Logic
    )

    target_compile_features(ModernLegacyAdapter PUBLIC cxx_std_17)

    if(MSVC)
        target_compile_options(ModernLegacyAdapter PRIVATE /W4 /permissive-)
    endif()
endif()

# WORLD-PLAYABLE-001A: the ASURA `.wld0` static-mesh terrain reader.
#
# Its own static library, rather than a member of ModernLegacyAdapter, because
# the terrain reader has no legacy dependency at all -- it reimplements the
# layout read out of the legacy sources -- so linking it against Lib_Client
# would drag the legacy G-Logic headers into a parser that needs none of them.
# That also keeps it independently testable: it links Modern and nothing else.
#
# It deliberately builds neither the legacy sources nor any Direct3D headers:
# the reader is CPU-side only.
add_library(ModernWorldStaticMesh STATIC
    WorldStaticMesh.cpp
)

# The reader is a C++ translation unit; without this the compiler can treat the
# `.cpp` as C, which breaks on the `Result<void>` specialisation.
set_source_files_properties(WorldStaticMesh.cpp PROPERTIES LANGUAGE CXX)

# Fall back to an explicit /TP as well, so the language is right even if a
# consuming toolchain ignores the property above.
if(MSVC)
    set_source_files_properties(WorldStaticMesh.cpp PROPERTIES COMPILE_OPTIONS "/TP")
endif()

target_include_directories(ModernWorldStaticMesh PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/modern/core
)

target_link_libraries(ModernWorldStaticMesh PUBLIC Modern)

target_compile_features(ModernWorldStaticMesh PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(ModernWorldStaticMesh PRIVATE /W4 /permissive- /utf-8)
endif()
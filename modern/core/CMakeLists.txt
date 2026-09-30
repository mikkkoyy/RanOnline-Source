# Modern core: game rules and domain models only.
#
# This target must stay free of Windows UI, DirectX, sockets, PostgreSQL and
# any legacy RAN type (GLChar, GLItemMan, SITEM, ...). If a change needs one
# of those, it belongs in modern/compatibility/legacy instead.
#
# CORE-001 deliberately keeps this list short. Calculated stats, equipment,
# passive skills, codex effects and the experience curve are not here: they are
# derived state, they belong to their own systems, and until those systems
# exist there is nothing for them to compute. See
# reference/legacy-calculation-port/README.md for what was moved out and why.

set(MODERN_SOURCES
    character/Character.cpp
    character/CharacterClassTable.cpp
    entity/Entity.cpp
    equipment/EquipmentState.cpp
    equipment/ItemContributionAggregator.cpp
    equipment/ItemDefinitionProvider.cpp
    gameplay/CharacterSnapshot.cpp
    item/ItemDefinition.cpp
    progression/CodexDefinition.cpp
    progression/CodexDefinitionProvider.cpp
    progression/CodexState.cpp
    progression/CodexContributionAggregator.cpp
    skills/SkillDefinition.cpp
    skills/SkillState.cpp
    skills/SkillDefinitionProvider.cpp
    skills/PassiveContributionAggregator.cpp
    stats/StatCalculator.cpp
    types/Result.cpp
    resources/ResourceState.cpp
    combat/CombatCalculator.h
    engine/GameCharacterCalculations.cpp
)

add_library(Modern STATIC ${MODERN_SOURCES})

target_include_directories(Modern PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(Modern PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(Modern PRIVATE /W4 /permissive- /utf-8)
endif()

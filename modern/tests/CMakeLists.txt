# Headless rule tests.
#
# These link Modern and nothing else. No renderer, no socket, no database, no
# legacy library. If a test here needs one of those, it is not a rule test and
# belongs with the adapter that owns the format.

add_executable(ModernCoreTests
    CoreFoundationTests.cpp
    StatCalculationTests.cpp
    EquipmentTests.cpp
    SkillTests.cpp
    ActiveSkillTests.cpp
    CodexTests.cpp
    CombatTests.cpp
  StatusEffectTests.cpp
SkillFactTests.cpp
)

target_link_libraries(ModernCoreTests PRIVATE Modern)

target_compile_features(ModernCoreTests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernCoreTests PRIVATE /W4 /permissive- /utf-8)
endif()

# enable_testing() is called by the top-level CMakeLists.txt, which is where
# CTest needs it in order to reach this directory.
add_test(NAME ModernCoreTests COMMAND ModernCoreTests)

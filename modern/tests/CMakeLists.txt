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

# VERTICAL-027: network boundary tests.
#
# A separate executable rather than more files in ModernCoreTests, because it
# links ModernNetwork. ModernCoreTests deliberately links Modern alone, and
# loosening that to reach one header would weaken a rule the whole project
# depends on: core is free of transport and protocol concerns.
add_executable(ModernNetworkTests
    NetworkTests.cpp
    # VERTICAL-030: server->client NET_COMPRESS + LZO1X layer.
    NetworkCompressTests.cpp
)

target_link_libraries(ModernNetworkTests PRIVATE ModernNetwork)

target_compile_features(ModernNetworkTests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernNetworkTests PRIVATE /W4 /permissive- /utf-8)
endif()

add_test(NAME ModernNetworkTests COMMAND ModernNetworkTests)

# enable_testing() is called by the top-level CMakeLists.txt, which is where
# CTest needs it in order to reach this directory.
add_test(NAME ModernCoreTests COMMAND ModernCoreTests)

cmake_minimum_required(VERSION 3.28)

# exptable_dump reads the packed/encrypted legacy RAN EXP table and writes a
# portable text export. It is a *research* tool: it includes legacy headers
# (legacy/Lib_Engine/Common/ByteCryptDef*.h) on purpose and is therefore the
# one known exception to the "modern never depends on legacy" rule.
# See reference/notes/modern-legacy-dependencies.md.
add_executable(exptable_dump exptable_dump.cpp)
target_compile_features(exptable_dump PRIVATE cxx_std_17)
if(MSVC)
    target_compile_options(exptable_dump PRIVATE /W3 /permissive- /utf-8)
endif()

# Runtime testing target. Per the project rule only Emulator.exe is run;
# MiniA.exe is never launched.
add_subdirectory(emulator)

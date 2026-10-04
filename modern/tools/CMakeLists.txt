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

# Demonstration target: prints the core foundation in use. The authority is
# the headless suite in modern/tests/, which is what gets run and what gates
# changes. No client or server executable is ever launched.
add_subdirectory(emulator)

# LOGIN-002: the runnable Login Server.
#
# The first modern executable that opens a socket. ModernEmulator prints the core
# foundation and deliberately never starts a client or server, because the
# authority is the headless suite. This one exists to be started BY HAND and
# watched, which is a different purpose and needs a different dependency set: it
# links the server and, through it, the real Winsock transport.
add_subdirectory(login_server)

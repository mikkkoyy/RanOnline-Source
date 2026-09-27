# Modern RAN source root.
#
# Dependency direction (enforced, see docs/MODERN_ARCHITECTURE.md):
#
#     core  <-  network / database / server / client
#     core  X   legacy
#
# The only permitted modern -> legacy edge lives in compatibility/legacy.

add_subdirectory(core)
add_subdirectory(tools)
add_subdirectory(compatibility/legacy)

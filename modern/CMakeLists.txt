# Modern RAN source root.
#
# Dependency direction (enforced, see docs/MODERN_ARCHITECTURE.md):
#
#     core  <-  network / database / server / client
#     core  X   legacy
#
# The only permitted modern -> legacy edge lives in compatibility/legacy.

add_subdirectory(core)
add_subdirectory(network)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(tools)
add_subdirectory(tests)

# compatibility/legacy is the sanctioned modern -> legacy bridge, and it stays
# on disk with its sources intact. It is not built during CORE-001:
#
#   - CORE-001 is scoped to modern/core. Legacy compatibility is a later phase.
#   - CharacterAdapter maps GLChar fields (HP/MP/SP pools, the ActState
#     bitfield, die/revive) that the clean Character deliberately does not
#     have, so it has to be rewritten against the new type rather than
#     patched.
#
# Enabling it again is a one-line change plus that rewrite.

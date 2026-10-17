# ASURA-CHARACTER-001A: the compatibility root.
#
# Dependency direction (enforced, see docs/MODERN_ARCHITECTURE.md):
#
#     core  <-  network / server / client
#     core  X   legacy
#
# `legacy/` is the one sanctioned modern -> legacy edge. Adapters there may
# understand legacy RAN *formats* and legacy RAN *types*; they must never be
# linked into core, and nothing in core may include anything from them.
#
# They are built on demand rather than as a hard dependency of the client, so a
# client target that needs one asks for ModernLegacyAdapter directly. That keeps
# a client target with no legacy need from pulling the bridge in.

add_subdirectory(legacy)
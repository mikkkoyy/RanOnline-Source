# Modern client: future owner of the new RAN client.
#
# CLIENT-002 establishes only modern/client/application. The sibling
# directories named in reference/client/MODERN_CLIENT_MAPPING.md
# (input, rendering, resources, assets, ui, character, world, audio) are created
# when there is code to put in them, per docs/MODERN_ARCHITECTURE.md: a directory
# that exists only to hold a placeholder is a directory whose dependency rules
# nobody has checked yet.
#
# Slices built so far: application (CLIENT-002), input (CLIENT-003),
# rendering (CLIENT-004 renderer, CLIENT-009 asset upload boundary),
# resources (CLIENT-005, CLIENT-006), assets (CLIENT-007 images, CLIENT-008
# meshes, CLIENT-011 real DDS decoder).

add_subdirectory(input)
add_subdirectory(resources)
add_subdirectory(assets)
add_subdirectory(rendering)
add_subdirectory(gameplay)
add_subdirectory(application)


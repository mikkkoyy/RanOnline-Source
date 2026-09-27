# Modern client: future owner of the new RAN client.
#
# CLIENT-002 establishes only modern/client/application. The sibling
# directories named in reference/client/MODERN_CLIENT_MAPPING.md
# (input, rendering, resources, ui, character, world, audio) are created when
# there is code to put in them, per docs/MODERN_ARCHITECTURE.md: a directory
# that exists only to hold a placeholder is a directory whose dependency rules
# nobody has checked yet.

add_subdirectory(application)
add_subdirectory(input)

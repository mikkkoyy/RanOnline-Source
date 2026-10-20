# MODERN-FIELD-SERVER-001: the runnable Field/World Server executable.
#
# Links ModernServer (which brings Modern and ModernNetwork). Every rule the
# process executes - the login exchange, the world-entry handoff, the Field
# session and its movement - already lives in that library; this target adds
# only a world to seed it with and a loop to serve the Agent role.
#
# It deliberately does NOT link ModernGameSession or the client halves: the
# executable is the server, and pulling in a client it never uses would make its
# dependencies a worse description of what it does.
add_executable(ModernFieldServer main.cpp)

target_link_libraries(ModernFieldServer PRIVATE
    ModernServer
)

target_include_directories(ModernFieldServer PRIVATE
    ${CMAKE_SOURCE_DIR}/modern/server
)

target_compile_features(ModernFieldServer PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernFieldServer PRIVATE /W4 /permissive- /utf-8)
endif()

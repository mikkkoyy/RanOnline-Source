# LOGIN-002: the runnable Login Server executable.
#
# Links ModernServer (which brings Modern and ModernNetwork) and ModernClientLogin,
# because it is the executable that demonstrates BOTH ends of the exchange are
# speakable. It does not link the client session: the executable is the server, and
# adding a client it never uses would make its dependencies a worse description of
# what it does.
add_executable(ModernLoginServer main.cpp)

target_link_libraries(ModernLoginServer PRIVATE
    Modern
    ModernNetwork
    ModernServer
)

target_include_directories(ModernLoginServer PRIVATE
    ${CMAKE_SOURCE_DIR}/modern/server
)

target_compile_features(ModernLoginServer PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(ModernLoginServer PRIVATE /W4 /permissive- /utf-8)
endif()
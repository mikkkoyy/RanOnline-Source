# Modern client login slice: builds the NET_MSG_LOGIN_2 request.
#
# WORLD-001. Header-only, and that is deliberate: the client-side login operation
# has no state beyond the cipher, and a translation unit would exist only to hold
# one class.
#
# Depends on ModernNetwork for the login protocol codec. This does NOT make the
# client depend on any server, legacy tree, renderer or socket: LoginProtocol is
# pure wire encoding, and World001LoginClient hands finished bytes to its caller
# rather than opening a connection.
#
# The server -> client compression layer (NetCompress, Lzo1xCodec,
# ServerBatchEncoder) lives in the same library but is deliberately unused here.
# Login is client -> server and travels raw; see LoginProtocol.h.

# INTERFACE, not STATIC: World001LoginClient is header-only, and a STATIC target
# with no compilable source emits no archive, which then fails to link.
add_library(ModernClientLogin INTERFACE)

target_include_directories(ModernClientLogin INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(ModernClientLogin INTERFACE Modern ModernNetwork)

target_compile_features(ModernClientLogin INTERFACE cxx_std_17)

if(MSVC)
    target_compile_options(ModernClientLogin INTERFACE /W4 /permissive- /utf-8)
endif()
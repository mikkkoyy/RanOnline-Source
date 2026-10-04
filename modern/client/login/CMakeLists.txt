# Modern client login slice: builds the NET_MSG_LOGIN_2 request.
#
# WORLD-001. Header-only, and that is deliberate: the client-side login operation
# has no state beyond the cipher, and a translation unit would exist only to hold
# one class.
#
# Depends on ModernNetwork for the login protocol codecs. This does NOT make the
# client depend on any server, legacy tree, renderer or socket: the codecs are pure
# wire handling, and the client hands finished bytes to its caller rather than
# opening a connection.
#
# STATIC, not INTERFACE: WORLD-001 added a header-only client, but WORLD-002's
# LoginResponseClient has an implementation, so this target now has a .cpp to
# compile.
#
# The server -> client compression layer (NetCompress, Lzo1xCodec,
# ServerBatchEncoder) lives in the same library but is deliberately unused by the
# request builder. Login requests are client -> server and travel raw; see
# LoginProtocol.h.

add_library(ModernClientLogin STATIC
    World001LoginClient.h
    LoginResponseClient.h
    LoginResponseClient.cpp
    # LOGIN-001: Login Server client (pre-login phase, separate from Agent login).
    LoginServerClient.h
    LoginServerClient.cpp
    # LOGIN-002: the socket half of the Login Server conversation.
    #
    # LoginServerClient owns the protocol and owns no socket; LoginServerSession owns
    # the TcpTransport and drives the protocol through it. Keeping them apart is what
    # lets LOGIN-001's protocol tests run with no network at all.
    LoginServerSession.cpp
    LoginServerSession.h
)

target_include_directories(ModernClientLogin PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(ModernClientLogin PUBLIC Modern ModernNetwork)

target_compile_features(ModernClientLogin PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(ModernClientLogin PRIVATE /W4 /permissive- /utf-8)
endif()

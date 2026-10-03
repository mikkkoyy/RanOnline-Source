#pragma once

// NETWORK-001: turning an endpoint the application names into an address the
// socket layer can use.
//
// WHY THIS EXISTS RATHER THAN AN INET_ADDR CALL INLINE.
//
// LOGIN-001 proved that legacy cannot resolve a hostname: CNetClient::ConnectLoginServer
// calls ::inet_addr directly with its gethostbyname branch commented out
// (s_NetGlobalClient reference s_NetClient.cpp:436-474), so a name becomes
// INADDR_NONE and the connect fails SILENTLY.
// Modern::Network::EndpointAddress::IsNumericIPv4 turns that silent
// misconnection into a reported one, and that rule is correct for what it models
// - the RAN client configuration, which is a numeric dotted-quad.
//
// But that is a limitation of legacy's CLIENT CONFIG, not of TCP. The brief asks
// for the transport to accept a hostname without touching the RAN protocol, and
// those are compatible: this resolver lives at the transport boundary, so
//
//     LoginServerClient  still validates a numeric address   (LOGIN-001's rule,
//                                                               about configuration)
//     TcpTransport        accepts a host OR a name          (transport capability)
//
// and the session layer feeds the client the numeric address the socket actually
// reached. Nothing on the wire changes, and no LOGIN-001 test changes meaning.
//
// getaddrinfo rather than gethostbyname: it is the only Winsock resolver that is
// safe to call from several threads, it takes a port rather than requiring the
// caller to htons() first, and it is IPv4/IPv6 aware so the AF_INET-only filter
// below is a deliberate choice rather than an accident of the API.
//
// NO WINSOCK TYPE APPEARS IN THIS HEADER, and that is the whole reason the
// resolved address is a small struct of our own rather than a `sockaddr_in`.
// A public header that named a Winsock type would force every consumer - the
// headless rule tests included - to inherit the platform's declaration order, its
// macro set, and its requirement that nothing included <windows.h> first. The
// socket code is confined to the .cpp for exactly the reason core is socket free.

#include "NetworkTransport.h"
#include "types/Result.h"

#include <cstdint>
#include <string>

namespace Modern::Network::SocketAddress
{
	// A resolved IPv4 endpoint, in network byte order.
	//
	// Both fields are already in the order the wire and Winsock use, so moving
	// them in or out of a `sockaddr_in` is a copy of the same bytes rather than a
	// conversion. Recording that here is deliberate: the failure mode of getting
	// it wrong is a connection to the wrong port, which looks like a hang.
	struct Ipv4Endpoint
	{
		std::uint32_t address = 0;  // network byte order, as sin_addr.s_addr
		std::uint16_t port    = 0;  // network byte order, as sin_port
	};

	// Resolves `endpoint` to a single IPv4 address and, optionally, its
	// dotted-quad text.
	//
	// AF_INET only, deliberately. RAN's wire carries a numeric IPv4 in a 21-byte
	// field (GameServerListProtocol.h, MAX_IP_LENGTH), so a dual-stack socket
	// could reach a peer the RAN protocol has no way to name afterwards. Refusing
	// to produce an IPv6 result keeps that impossibility from being discovered at
	// the point of use.
	//
	// `outText`, when asked for, is what the application should display and hand
	// to the protocol layer: the resolved NUMERIC address, never the name it was
	// asked for. A caller that recorded "localhost" as the peer address would be
	// recording something the wire cannot carry.
	//
	// Returns InvalidArgument for an empty host, a host that does not resolve,
	// and a host that resolves but has no IPv4 address. Those last two are
	// deliberately the SAME error: from the transport's point of view there is
	// nowhere to connect, and distinguishing them would leak resolver internals
	// into application code.
	Status Resolve(const Endpoint& endpoint, Ipv4Endpoint& out) noexcept;

	// As above, and additionally renders the dotted-quad form.
	//
	// Separated rather than defaulted because resolving is comparatively
	// expensive - it can hit DNS - and a caller that only needs the sockaddr
	// should not pay for the text.
	Status Resolve(const Endpoint& endpoint, Ipv4Endpoint& out, std::string& outText) noexcept;

	// Renders an address as a dotted quad. Returns an empty string only if the
	// caller's value is not a usable IPv4 address, which a caller must treat as
	// "unknown" rather than as a real address.
	std::string ToText(const Ipv4Endpoint& address) noexcept;
}

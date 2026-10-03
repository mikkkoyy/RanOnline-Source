#include "SocketAddress.h"

// NOMINMAX, before <winsock2.h>.
//
// WinSock2.h includes <windows.h> itself unless _INC_WINDOWS is already defined,
// so avoiding windows.h is not achievable in this file - which means the file's own
// claim that it must not pull in windows.h cannot be kept literally. windows.h
// reaches minwindef.h, which defines function-like min and max macros; those
// swallow the opening parenthesis of std::min(a, b) and std::max(a, b), and MSVC
// then reports C2589, an error whose text mentions neither macros nor windows.h.
//
// NOMINMAX removes only those two macros. It is defined here rather than as a
// project-wide setting so that nothing else in the build changes behaviour.
#ifndef NOMINMAX
#define NOMINMAX
#endif#include <winsock2.h>

#include <ws2tcpip.h>

#include <cstdio>

namespace Modern::Network::SocketAddress
{
	Status Resolve(const Endpoint& endpoint, Ipv4Endpoint& out) noexcept
	{
		// A port is a port. Zero is legal here - a listener binds it to mean
		// "any" and then reads back what it got - so it is not rejected, and the
		// decision belongs to whoever is binding.
		if (endpoint.host.empty())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		addrinfo hints{};
		// AF_INET, and the reason is stated in the header: RAN's wire can only
		// name an IPv4 address, so resolving to IPv6 would produce a peer the
		// protocol layer could not describe.
		hints.ai_family   = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		// AI_ADDRCONFIG keeps a name from resolving to an address family this
		// machine has no interface for, which turns a would-be connect timeout
		// into an immediate, reportable failure.
		hints.ai_flags = AI_ADDRCONFIG;

		addrinfo* results = nullptr;

		// The port is passed as TEXT, not as a network-order 16-bit value, so
		// getaddrinfo performs the conversion. Handing it an already-converted
		// value is the classic way to connect to a port nobody is listening on.
		char service[16];
		std::snprintf(service, sizeof(service), "%u",
		              static_cast<unsigned>(endpoint.port));

		const int status = ::getaddrinfo(endpoint.host.c_str(), service, &hints, &results);
		if (status != 0 || results == nullptr)
		{
			// Every failure - WSAHOST_NOT_FOUND, WSA_SERVICE_NOT_FOUND, a
			// malformed name, a name with no IPv4 - collapses to one error, as
			// the header promises. The specific platform value is deliberately
			// not surfaced: it is resolver trivia, not a transport decision.
			if (results != nullptr)
			{
				::freeaddrinfo(results);
			}
			return Status(ErrorCode::InvalidArgument);
		}

		// The first result, always. The AF_INET + SOCK_STREAM hints make every
		// entry in the list usable, and picking the first is what "connect to
		// this host" means; walking the list in preference order belongs in an
		// IPv6-capable future, not in a transport whose protocol is IPv4-only.
		const sockaddr_in* resolved = nullptr;
		for (const addrinfo* entry = results; entry != nullptr; entry = entry->ai_next)
		{
			if (entry->ai_family == AF_INET && entry->ai_addr != nullptr)
			{
				resolved = reinterpret_cast<const sockaddr_in*>(entry->ai_addr);
				break;
			}
		}

		if (resolved == nullptr)
		{
			::freeaddrinfo(results);
			return Status(ErrorCode::InvalidArgument);
		}

		// Straight copy, not conversion: s_addr and sin_port are already the
		// network-order values the struct promises to hold in exactly this
		// width and order.
		out.address = resolved->sin_addr.s_addr;
		out.port    = resolved->sin_port;

		::freeaddrinfo(results);
		return Ok();
	}

	Status Resolve(const Endpoint& endpoint, Ipv4Endpoint& out, std::string& outText) noexcept
	{
		if (const Status status = Resolve(endpoint, out); status.IsError())
		{
			return status;
		}

		outText = ToText(out);
		// A resolution that produced no renderable text would hand the protocol
		// layer an empty address field, which is exactly the silent
		// misconnection IsNumericIPv4 exists to prevent. Treat it as a failure of
		// resolution rather than passing an empty string on.
		if (outText.empty())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		return Ok();
	}

	std::string ToText(const Ipv4Endpoint& address) noexcept
	{
		// in_addr.S_un.S_addr is the portable spelling and does not depend on
		// which of the historical union members the toolchain happens to prefer.
		in_addr in{};
		in.S_un.S_addr = address.address;

		// InetNtop rather than inet_ntoa. inet_ntoa is deprecated at /W4 and the
		// warning is not something to suppress with a project-wide define: it
		// would also hide the deprecation everywhere else it appears. InetNtop
		// additionally reports failure instead of silently returning a static
		// buffer, so the empty-string case below is a real outcome rather than a
		// shape the compiler inferred.
		//
		// INET_ADDRSTRLEN is the documented buffer size for a dotted quad, and the
		// buffer is sized for the longest form InetNtop can produce (an IPv6
		// literal) so the size is never the reason the call fails.
		char text[INET6_ADDRSTRLEN] = {};
		if (::InetNtop(AF_INET, const_cast<in_addr*>(&in), text, sizeof(text)) == nullptr)
		{
			return std::string();
		}

		return std::string(text);
	}
}

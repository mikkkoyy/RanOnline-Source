#include "WinSockSupport.h"

// Winsock first, and before any project header. The rule is not stylistic: this
// file must not pull in <windows.h>, and winsock2.h is sensitive to what came
// before it. The project header is included afterwards by the including .cpp, but
// this one is included here too so that nothing can reach it first.
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

// WSAEHOSTNOTFOUND, WSAENAMENOTRESOLVED and WSA_SERVICE_NOT_FOUND are declared
// here rather than in winsock2.h. A file that included only the latter would
// compile with three of the mapping's cases silently missing, which is the kind
// of gap that becomes a fallthrough nobody notices.
#include <ws2tcpip.h>

namespace Modern::Network::WinSockDetail
{
	int LastSocketError() noexcept
	{
		return ::WSAGetLastError();
	}

	TransportFault FromSocketError(int error) noexcept
	{
		switch (error)
		{
		case 0:
			return TransportFault::None;

		case WSAEINPROGRESS:      // connect still running when we asked
		case WSAEALREADY:         // a blocking connect is already in flight
		case WSAEWOULDBLOCK:
			return TransportFault::WouldBlock;

		case WSAETIMEDOUT:
			return TransportFault::Timeout;

		case WSAECONNREFUSED:     // RST, or nothing listening
			return TransportFault::ConnectionRefused;

		case WSAENETUNREACH:
		case WSAEHOSTUNREACH:
		case WSAEHOSTDOWN:
			return TransportFault::HostUnreachable;

		case WSAECONNRESET:       // the peer aborted mid-stream
		case WSAECONNABORTED:
			return TransportFault::ConnectionReset;

		case WSAESHUTDOWN:        // our own close raced a blocking call
		case WSAENOTCONN:
			return TransportFault::NotConnected;

		case WSAEINVAL:
		case WSAEFAULT:
		case WSAENOTSOCK:
		case WSAEAFNOSUPPORT:
		case WSAEPROTONOSUPPORT:
		case WSAEOPNOTSUPP:
		case WSAESOCKTNOSUPPORT:
		case WSAEADDRNOTAVAIL:
		case WSAEADDRINUSE:
		case WSAHOST_NOT_FOUND:
		case WSASERVICE_NOT_FOUND:
			return TransportFault::AddressInvalid;

		case WSAENETDOWN:
		case WSAENETRESET:
			return TransportFault::NetworkUnavailable;

		case WSANOTINITIALISED:   // WinSock::Acquire was never called
		case WSASYSNOTREADY:
		case WSAVERNOTSUPPORTED:
			return TransportFault::NotSupported;

		default:
			// Distinct from None on purpose. An unmapped platform error must be
			// visible; mapping it to None would report a failure as success.
			return TransportFault::Unexpected;
		}
	}

	ErrorCode ErrorCodeFor(TransportFault fault) noexcept
	{
		switch (fault)
		{
		case TransportFault::None:
		case TransportFault::WouldBlock:
		case TransportFault::PeerClosed:
			return ErrorCode::None;

		// "Nowhere to connect" and "the connection is gone": in both cases the
		// endpoint could not be found, which is what NotFound means to a caller
		// that has one.
		case TransportFault::Timeout:
		case TransportFault::ConnectionRefused:
		case TransportFault::HostUnreachable:
		case TransportFault::ConnectionReset:
			return ErrorCode::NotFound;

		// A name that does not parse, resolve or bind is a bad argument, and
		// saying so is what turns legacy's silent INADDR_NONE into a report.
		case TransportFault::AddressInvalid:
		case TransportFault::NotSupported:
			return ErrorCode::InvalidArgument;

		// The object is in a state that forbids the operation: not connected, no
		// network, or a send that cannot complete and has therefore desynchronised
		// the stream. The only correct response to any of them is to drop the
		// connection.
		case TransportFault::NotConnected:
		case TransportFault::NetworkUnavailable:
		case TransportFault::PartialSend:
			return ErrorCode::InvalidState;

		case TransportFault::Unexpected:
		default:
			return ErrorCode::NotAllowed;
		}
	}

	Status StatusFor(int socketError) noexcept
	{
		return Status(ErrorCodeFor(FromSocketError(socketError)));
	}

	TransportFault WaitReady(std::uintptr_t handle, bool forWrite, int timeoutMilliseconds) noexcept
	{
		fd_set set;
		FD_ZERO(&set);
		FD_SET(static_cast<SOCKET>(handle), &set);

		timeval timeout{};
		timeout.tv_sec  = timeoutMilliseconds / 1000;
		timeout.tv_usec = (timeoutMilliseconds % 1000) * 1000;

		// nfds is ignored on Windows and must be 0, which the platform headers say
		// explicitly. Passing a real value is the classic way to get a WSAEINVAL
		// that looks like a timeout.
		const int result = ::select(0, forWrite ? nullptr : &set,
		                            forWrite ? &set : nullptr,
		                            nullptr, &timeout);

		if (result == 0)
		{
			return TransportFault::WouldBlock;   // deadline passed
		}
		if (result == SOCKET_ERROR)
		{
			return FromSocketError(LastSocketError());
		}
		return TransportFault::None;
	}

	bool SetBlocking(std::uintptr_t handle, bool blocking) noexcept
	{
		u_long mode = blocking ? 0u : 1u;
		return ::ioctlsocket(static_cast<SOCKET>(handle), FIONBIO, &mode) != SOCKET_ERROR;
	}
}

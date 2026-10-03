#include "TcpListener.h"

#include "SocketAddress.h"
#include "WinSockRuntime.h"
#include "WinSockSupport.h"

// Winsock first, and before any project header. The rule is not stylistic: this
// file must not pull in <windows.h>, and winsock2.h is sensitive to what came
// before it.
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

#include <algorithm>

namespace Modern::Network
{
	namespace
	{
		using namespace WinSockDetail;

		// The largest backlog the platform will accept.
		//
		// Read from the header rather than hard-coded, because it is a property of
		// this Windows build. SOMAXCONN is the documented "as deep as the system
		// allows" value, and asking for more than it means being silently rounded -
		// see TcpListenerOptions::backlog.
		int PlatformMaximumBacklog() noexcept
		{
			return SOMAXCONN;
		}

		void RecordFault(std::atomic<int>& fault, std::atomic<int>& native, int socketError) noexcept
		{
			native.store(socketError, std::memory_order_relaxed);
			fault.store(static_cast<int>(FromSocketError(socketError)), std::memory_order_relaxed);
		}

		void ClearFault(std::atomic<int>& fault, std::atomic<int>& native) noexcept
		{
			native.store(0, std::memory_order_relaxed);
			fault.store(static_cast<int>(TransportFault::None), std::memory_order_relaxed);
		}
	}

	TcpListener::TcpListener(const TcpListenerOptions& options)
		: m_options(options)
	{
		// Winsock must be up before socket(), which is itself a Winsock call.
		if (WinSock::Acquire().IsError())
		{
			// No reference taken, so the destructor releases nothing. Every operation
			// will fail, which is the truth: this process cannot open sockets.
			return;
		}
		m_holdsRuntime = true;

		// Failure is recorded rather than reported: a constructor cannot return a
		// Status, and a listener that cannot bind is still a well-formed object that
		// a caller must be able to ask about and destroy. See CreateSocket.
		(void)CreateSocket();
	}

	Status TcpListener::CreateSocket() noexcept
	{
		// WSA_FLAG_OVERLAPPED for the same reason TcpTransport asks for it: the
		// accepted socket is what a later event-driven reader would use, and a
		// listening socket created without the flag cannot produce one that has it.
		// It costs nothing while everything here is blocking.
		const SOCKET handle = ::WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP,
		                                   nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (handle == INVALID_SOCKET)
		{
			// Read once. WSAGetLastError() is per-thread and only meaningful until
			// the next Winsock call, so capturing it twice risks reporting a
			// different failure than the one that happened.
			RecordFault(m_fault, m_nativeError, LastSocketError());
			return StatusFor(m_nativeError.load(std::memory_order_relaxed));
		}

		m_socket.store(static_cast<std::uintptr_t>(handle), std::memory_order_release);
		ClearFault(m_fault, m_nativeError);
		return Ok();
	}

	TcpListener::~TcpListener()
	{
		Close();
		if (m_holdsRuntime)
		{
			// After the socket, never before: WSACleanup while a handle is still open
			// is the ordering bug WinSockRuntime exists to prevent.
			WinSock::Release();
			m_holdsRuntime = false;
		}
	}

	TcpListener::TcpListener(TcpListener&& other) noexcept
		: m_options(other.m_options),
		  m_holdsRuntime(other.m_holdsRuntime),
		  m_bound(other.m_bound)
	{
		// The handle is taken, never copied: the source is left owning nothing, so
		// its destructor cannot close a socket this object now uses.
		m_socket.store(other.m_socket.exchange(kNoSocket, std::memory_order_acq_rel),
		               std::memory_order_release);

		m_listening.store(other.m_listening.exchange(false, std::memory_order_acq_rel), std::memory_order_release);
		m_fault.store(other.m_fault.load(std::memory_order_acquire), std::memory_order_release);
		m_nativeError.store(other.m_nativeError.load(std::memory_order_acquire),
		                    std::memory_order_release);

		other.m_holdsRuntime = false;
		other.m_bound        = Endpoint{};
	}

	TcpListener& TcpListener::operator=(TcpListener&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}

		// Release what THIS object holds first, or its socket and Winsock reference
		// would leak.
		Close();
		if (m_holdsRuntime)
		{
			WinSock::Release();
			m_holdsRuntime = false;
		}

		m_options = other.m_options;
		m_listening.store(other.m_listening.exchange(false, std::memory_order_acq_rel), std::memory_order_release);

		m_socket.store(other.m_socket.exchange(kNoSocket, std::memory_order_acq_rel),
		               std::memory_order_release);

		m_fault.store(other.m_fault.load(std::memory_order_acquire), std::memory_order_release);
		m_nativeError.store(other.m_nativeError.load(std::memory_order_acquire),
		                    std::memory_order_release);

		other.m_holdsRuntime = false;

		{
			const std::lock_guard<std::mutex> lock(other.m_mutex);
			m_bound = other.m_bound;
			other.m_bound = Endpoint{};
			// observe a listener as listening against a socket on its way out.
			m_listening.store(false, std::memory_order_release);
			m_listening.store(false, std::memory_order_release);
			// Cleared before the handle is taken, so a concurrent IsListening() can never
		}
		return *this;
	}

	std::uintptr_t TcpListener::CurrentSocket() const noexcept
	{
		// uintptr_t, not SOCKET: this header must not name a Winsock type. The casts
		// at the call sites are exact, because SOCKET IS UINT_PTR on Windows.
		return m_socket.load(std::memory_order_acquire);
	}

	bool TcpListener::HasSocket() const noexcept
	{
		return CurrentSocket() != kNoSocket;
	}

	Status TcpListener::Listen(const Endpoint& bindEndpoint)
	{
		// Validated BEFORE any state is touched. The previous version of this method
		// closed the existing socket first and only then discovered the host was
		// unusable - which meant a typo in an address silently destroyed a working
		// server. An argument this function will refuse must not cost the caller
		// anything.
		if (bindEndpoint.host.empty())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// A socket that has been BOUND cannot be bound again, so re-listening on a
		// different address needs a different socket. Only the already-listening case
		// is replaced; a merely-constructed listener keeps the socket it already has.
		//
		// (The earlier version called Close() unconditionally and then tested for a
		// socket, which it had just destroyed - so Listen always failed with
		// InvalidState and fault=None. A bug worth recording because the symptom -
		// an error with no platform code behind it - looked like a resource problem
		// rather than an ordering mistake.)
		if (IsListening())
		{
			Close();
			if (const Status created = CreateSocket(); created.IsError())
			{
				return created;
			}
		}

		if (!HasSocket())
		{
			// Winsock refused to start, or the socket could not be created. The fault
			// was recorded by whichever of those happened; reporting InvalidState here
			// is the honest summary, and Fault() carries the detail.
			return Status(ErrorCode::InvalidState);
		}

		// Resolved through the same resolver the transport uses, so "127.0.0.1",
		// "localhost" and a numeric address behave identically here and there, and a
		// name that cannot resolve is reported once, in one place, in one vocabulary.
		SocketAddress::Ipv4Endpoint resolved;
		if (const Status status = SocketAddress::Resolve(bindEndpoint, resolved); status.IsError())
		{
			RecordFault(m_fault, m_nativeError, WSAHOST_NOT_FOUND);
			return status;
		}

		sockaddr_in local{};
		local.sin_family      = AF_INET;
		local.sin_addr.s_addr = resolved.address;
		// Port 0 is passed through deliberately: it is how a caller asks the OS for a
		// free port, and the real number is read back with getsockname below. A port
		// of 0 is therefore NOT an error here, unlike almost everywhere else in this
		// layer.
		local.sin_port        = resolved.port;

		const std::uintptr_t handle = CurrentSocket();

		if (::bind(static_cast<SOCKET>(handle),
		           reinterpret_cast<const sockaddr*>(&local), sizeof(local)) == SOCKET_ERROR)
		{
			// WSAEADDRINUSE is the common case and maps to AddressInvalid, reported as
			// InvalidArgument. That is a blunt word for "the port is taken", but the
			// platform code is available through NativeError() for anyone who needs to
			// say so more precisely, and inventing a TransportFault for it would put a
			// socket-specific fact into a set meant to describe transport-level
			// outcomes.
			//
			// The socket is closed rather than kept: a bound socket cannot be bound
			// again, so leaving it would make a retry impossible without a
			// reconstruction, and a caller that retries will get a fresh socket from
			// Listen's replace path only if it is still listening - which it is not.
			// Closing here makes the failure total and the recovery obvious.
			const int error = LastSocketError();
			RecordFault(m_fault, m_nativeError, error);
			Close();
			return StatusFor(error);
		}

		// Clamped rather than trusted. See TcpListenerOptions::backlog.
		const int requested = (std::max)(1, m_options.backlog);
		const int backlog   = (std::min)(requested, PlatformMaximumBacklog());

		if (::listen(static_cast<SOCKET>(handle), backlog) == SOCKET_ERROR)
		{
			const int error = LastSocketError();
			RecordFault(m_fault, m_nativeError, error);
			Close();
			return StatusFor(error);
		}

		// Read the bound address back rather than echoing what was asked for. With
		// port 0 the two differ, and a server that logged the port it requested
		// instead of the port it got would print "listening on 0".
		sockaddr_in actual{};
		int         length = sizeof(actual);
		if (::getsockname(static_cast<SOCKET>(handle),
		                   reinterpret_cast<sockaddr*>(&actual), &length) == SOCKET_ERROR)
		{
			// Bound and listening, but unreadable. Failing here is right: a caller
			// that cannot learn its port cannot connect to it, and for a port-0 bind
			// that makes the listener useless.
			const int error = LastSocketError();
			RecordFault(m_fault, m_nativeError, error);
			Close();
			return StatusFor(error);
		}

		{
			const std::lock_guard<std::mutex> lock(m_mutex);
			m_bound.host = SocketAddress::ToText(
			    SocketAddress::Ipv4Endpoint{actual.sin_addr.s_addr, 0});
			m_bound.port = ntohs(actual.sin_port);
		}

		// Only now. Set after everything that could fail, so the flag never describes
		// a listener that is not actually accepting.
		m_listening.store(true, std::memory_order_release);
		ClearFault(m_fault, m_nativeError);
		return Ok();
	}

	bool TcpListener::IsListening() const noexcept
	{
		// Both, not either. The flag alone could outlive the socket if a close failed;
		// the socket alone is true for a constructed listener that has bound nothing.
		// Only the conjunction describes a listener that can accept a connection, which
		// is the question a caller is actually asking.
		return m_listening.load(std::memory_order_acquire) && HasSocket();
	}

	Endpoint TcpListener::BoundEndpoint() const noexcept
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		return m_bound;
	}

	Status TcpListener::Accept(std::uintptr_t& outHandle, int timeoutMilliseconds)
	{
		// Set on EVERY path, including the failures. A caller that reads
		// outHandle after an error must get the sentinel rather than whatever was on
		// its stack - a plausible garbage handle is far worse than an obvious one,
		// because garbage handed to TcpTransport::Adopt would be adopted.
		outHandle = kNoAcceptedSocket;

		if (!IsListening())
		{
			m_fault.store(static_cast<int>(TransportFault::NotConnected), std::memory_order_relaxed);
			return Status(ErrorCode::InvalidState);
		}

		const int timeout = timeoutMilliseconds > 0
		                        ? timeoutMilliseconds
		                        : m_options.acceptTimeoutMilliseconds;

		// Copied out under the lock, then used without it, so Close() from another
		// thread can shut the socket down underneath a blocked accept and have that
		// accept return. That is the ordinary shutdown path: a server told to stop
		// must not wait for a client that may never connect.
		const std::uintptr_t handle = CurrentSocket();

		const TransportFault ready = WaitReady(handle, /*forWrite=*/false, timeout);

		if (ready == TransportFault::WouldBlock)
		{
			// The deadline passed with nobody knocking.
			//
			// SUCCESS with the sentinel handle, not an error, and deliberately the
			// same shape as a TcpTransport::Receive that found nothing: "nothing
			// happened yet" is not a failure, and an accept loop that had to invent an
			// error for every idle pass would be an accept loop nobody writes
			// correctly. Fault() still records WouldBlock for a caller that wants to
			// know the deadline elapsed.
			m_fault.store(static_cast<int>(TransportFault::WouldBlock), std::memory_order_relaxed);
			m_nativeError.store(WSAETIMEDOUT, std::memory_order_relaxed);
			return Ok();
		}
		if (ready != TransportFault::None)
		{
			// A real select() failure. LastSocketError is meaningful here because
			// WaitReady has not touched Winsock since it failed.
			const int error = LastSocketError();
			RecordFault(m_fault, m_nativeError, error);
			return StatusFor(error);
		}

		// select() said readable, which for a listening socket means a connection is
		// waiting or has just arrived.
		//
		// The address is deliberately NOT requested. A pre-sized sockaddr_in means
		// accept() fails when the real peer has a longer address than the buffer, and
		// passing nullptr with a null length asks the platform to allocate whatever
		// the peer needs - which is the only version that cannot fail for that reason.
		const SOCKET accepted = ::accept(static_cast<SOCKET>(handle), nullptr, nullptr);

		if (accepted == INVALID_SOCKET)
		{
			const int error = LastSocketError();

			// Our own Close() raced this accept. That is the shutdown path completing,
			// not a fault: the local side asked for it.
			if (error == WSAEINVAL || error == WSAENOTSOCK || error == WSAESHUTDOWN)
			{
				m_fault.store(static_cast<int>(TransportFault::NotConnected), std::memory_order_relaxed);
				m_nativeError.store(error, std::memory_order_relaxed);
				return Status(ErrorCode::InvalidState);
			}

			// WSAEWOULDBLOCK from a non-blocking listening socket: another thread got
			// there first, which is a legitimate outcome of concurrent accepts and not
			// a failure of this one.
			if (error == WSAEWOULDBLOCK)
			{
				m_fault.store(static_cast<int>(TransportFault::WouldBlock), std::memory_order_relaxed);
				m_nativeError.store(WSAETIMEDOUT, std::memory_order_relaxed);
				return Ok();
			}

			// WSAECONNABORTED: the client connected and vanished before the handshake
			// finished. Common enough under load that treating it as a connection to
			// serve would be wrong, and treating it as a server fault would make a
			// scan look like a crash.
			if (error == WSAECONNABORTED)
			{
				m_fault.store(static_cast<int>(TransportFault::ConnectionReset), std::memory_order_relaxed);
				m_nativeError.store(error, std::memory_order_relaxed);
				return Ok();
			}

			RecordFault(m_fault, m_nativeError, error);
			return StatusFor(error);
		}

		outHandle = static_cast<std::uintptr_t>(accepted);
		ClearFault(m_fault, m_nativeError);
		return Ok();
	}

	void TcpListener::Close() noexcept
	{
		const std::uintptr_t raw = m_socket.load(std::memory_order_acquire);
		if (raw == kNoSocket)
		{
			// Already closed. Idempotent by design: a second close is a no-op rather
			// than an error, and a double closesocket would free a handle the OS may
			// already have handed to something else.
			return;
		}

		// Taken under the lock and cleared immediately, so a concurrent Close sees
		// the sentinel and does nothing, and a concurrent Accept that copied the old
		// handle has exactly one socket to act on.
		{
			const std::lock_guard<std::mutex> lock(m_mutex);
			if (m_socket.load(std::memory_order_acquire) == kNoSocket)
			{
				return;
			}
			m_socket.store(kNoSocket, std::memory_order_release);
			m_bound = Endpoint{};
		}

		const SOCKET handle = static_cast<SOCKET>(raw);

		// shutdown() before closesocket(), and the ordering is the reason both are
		// called: closesocket() alone does not reliably wake a thread already blocked
		// in accept() on the same socket, so Close() would not return until a client
		// eventually arrived. That would make shutting a server down depend on a
		// client connecting, which is exactly backwards.
		//
		// The return value is ignored on purpose: a socket the peer already reset
		// cannot be shut down cleanly, and that is no reason to skip the close.
		::shutdown(handle, SD_BOTH);
		::closesocket(handle);
	}

	Status TcpListener::AcquireRuntime() noexcept
	{
		return WinSock::Acquire();
	}

	void TcpListener::ReleaseRuntime() noexcept
	{
		WinSock::Release();
	}
}

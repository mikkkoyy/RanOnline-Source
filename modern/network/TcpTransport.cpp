#include "TcpTransport.h"

#include "SocketAddress.h"
#include "WinSockRuntime.h"
#include "WinSockSupport.h"

// Winsock first, and before any project header. The rule is not stylistic: this
// file must not pull in <windows.h>, and winsock2.h is sensitive to what came
// before it. Project headers are included afterwards so nothing they declare can
// precede it.
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
#include <atomic>

namespace Modern::Network
{
	namespace
	{
		// NETWORK-001: the per-thread error code, the single sanctioned mapping from a
		// Winsock code to a TransportFault, the bounded readiness wait and the
		// blocking-mode switch all live in WinSockSupport, because TcpListener needs
		// exactly the same six things, and a second mapping table in this file would
		// eventually disagree with the one over there.
		using namespace WinSockDetail;

		// SOCKET and int, without a Winsock type in the signature.
		//
		// Every socket call below takes the handle through this alias, so the .cpp
		// reads the same as the platform documentation, which is the point of
		// writing the free functions at all: the platform calls are visible, and
		// the error interpretation is visible, with nothing hidden in between.
		std::uintptr_t ToHandle(std::uintptr_t value) noexcept { return value; }

		// Records a fault, keeping the platform code for diagnostics only.
		//
		// Free functions rather than methods because they take the atomics as
		// parameters: the fault register is three separate members here and one in
		// TcpListener, and a shared helper that wrote "the" fault would have to be
		// told which transport it was talking about at every call site.
		void SetFault(std::atomic<int>& fault, std::atomic<int>& native, int socketError) noexcept
		{
			native.store(socketError, std::memory_order_relaxed);
			fault.store(static_cast<int>(FromSocketError(socketError)), std::memory_order_relaxed);
		}

		// The success counterpart. Called after an operation that did what was
		// asked, because the fault register is "what just happened" rather than
		// sticky: leaving a stale ConnectionRefused on a transport that has since
		// connected successfully would make Fault() a lie about the present.
		void ClearFault(std::atomic<int>& fault, std::atomic<int>& native) noexcept
		{
			native.store(0, std::memory_order_relaxed);
			fault.store(static_cast<int>(TransportFault::None), std::memory_order_relaxed);
		}

	}

	const char* ToString(TcpTransportPhase phase) noexcept
	{
		switch (phase)
		{
		case TcpTransportPhase::Disconnected: return "Disconnected";
		case TcpTransportPhase::Initialized:  return "Initialized";
		case TcpTransportPhase::Connecting:   return "Connecting";
		case TcpTransportPhase::Connected:    return "Connected";
		case TcpTransportPhase::Closing:      return "Closing";
		}
		return "Unrecognised";
	}

	TcpTransport::TcpTransport(const TcpTransportOptions& options)
		: m_options(options)
	{
		// The runtime reference is taken BEFORE socket(), because socket() is itself a
		// Winsock call and would fail with WSANOTINITIALISED otherwise.
		if (WinSock::Acquire().IsError())
		{
			// No reference, so there is nothing to release in the destructor. The
			// transport stays invalid and every operation fails, which is the truth:
			// this process cannot open sockets.
			return;
		}
		m_holdsRuntime = true;

		// Failure is recorded rather than reported: a constructor cannot return a
		// Status, and an unusable transport is still a well-formed object a caller
		// must be able to ask about and destroy. IsValid() is how it is asked about.
		if (CreateSocket().IsError())
		{
			return;
		}

		m_phase.store(static_cast<int>(TcpTransportPhase::Initialized), std::memory_order_release);
	}

	Status TcpTransport::CreateSocket() noexcept
	{
		// WSA_FLAG_OVERLAPPED even though every call below is a blocking one.
		//
		// This is what legacy does - WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
		// WSA_FLAG_OVERLAPPED) in the public backread of CClientSocket::CreateSocket -
		// and it costs nothing here. It is kept so the handle this transport owns is
		// usable by an overlapped or event-based reader later without having to be
		// recreated, and so the modern socket matches the shape of the legacy one
		// rather than quietly diverging from it.
		const SOCKET handle = ::WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP,
		                                   nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (handle == INVALID_SOCKET)
		{
			// Read once. WSAGetLastError() is per-thread and only meaningful until the
			// next Winsock call, so capturing it twice risks reporting a different
			// failure than the one that happened.
			SetFault(m_fault, m_nativeError, LastSocketError());
			return StatusFor(m_nativeError.load(std::memory_order_relaxed));
		}

		// The handle is stored in the platform's own width so that no later comparison
		// has to guess at INVALID_SOCKET: kNoSocket is that same all-ones value.
		m_socket.store(ToHandle(static_cast<std::uintptr_t>(handle)), std::memory_order_release);
		ApplyOptions();
		return Ok();
	}

	void TcpTransport::CloseSocket() noexcept
	{
		const std::uintptr_t raw = m_socket.load(std::memory_order_acquire);
		if (raw == kNoSocket)
		{
			// Already closed. Idempotent by design: a second close is a no-op rather
			// than an error, and a double closesocket would free a handle the OS may
			// already have handed to something else - which fails in a way that is very
			// hard to attribute.
			return;
		}

		// Taken under the lock and cleared immediately, so a concurrent CloseSocket
		// sees kNoSocket and does nothing, and a concurrent Receive that copied the
		// old handle has exactly one socket to act on.
		{
			const std::lock_guard<std::mutex> lock(m_mutex);
			if (m_socket.load(std::memory_order_acquire) == kNoSocket)
			{
				return;
			}
			m_socket.store(kNoSocket, std::memory_order_release);
		}

		const SOCKET handle = static_cast<SOCKET>(raw);

		// shutdown() BEFORE closesocket(), and this ordering is the whole reason both
		// are called.
		//
		// closesocket() alone does not reliably wake a thread already blocked in recv()
		// on the same socket, so a server thread waiting for a request would sit there
		// until the peer happened to time out. shutdown(SD_BOTH) makes a blocked
		// receive return immediately, which is what makes "the server closes first"
		// observable at all.
		//
		// The return value is deliberately ignored: a socket already reset by the peer
		// cannot be shut down cleanly, and that is no reason to skip the close.
		::shutdown(handle, SD_BOTH);
		::closesocket(handle);
	}

	TcpTransport::TcpTransport(Adopted, std::uintptr_t handle, const TcpTransportOptions& options)
		: m_options(options)
	{
		// The reference is taken here for the same reason the public constructor takes
		// one before its socket() call: Winsock must be up before any socket exists.
		// The accept that produced this socket happened on a listener holding its own
		// reference, but this transport may outlive that listener, so it cannot rely
		// on the listener's reference still being held.
		if (WinSock::Acquire().IsError())
		{
			return;
		}
		m_holdsRuntime = true;

		m_socket.store(handle, std::memory_order_release);
		ApplyOptions();

		// Connected, not Initialized: an adopted socket came out of accept(), which
		// only returns for a connection that is already established. Reporting
		// Initialized would make State() report Closed, and Send would refuse with
		// NotConnected on a perfectly good connection.
		m_phase.store(static_cast<int>(TcpTransportPhase::Connected), std::memory_order_release);

		// The peer, read from the socket rather than recorded from configuration, so
		// RemoteEndpoint() names the peer that actually arrived.
		sockaddr_in peer{};
		int         length = sizeof(peer);
		if (::getpeername(static_cast<SOCKET>(handle), reinterpret_cast<sockaddr*>(&peer), &length) != SOCKET_ERROR)
		{
		in_addr address{};
		address.S_un.S_addr = peer.sin_addr.s_addr;

		const std::lock_guard<std::mutex> lock(m_mutex);
		m_remote.host = SocketAddress::ToText(SocketAddress::Ipv4Endpoint{address.S_un.S_addr, 0});
		m_remote.port = ntohs(peer.sin_port);

		SocketAddress::Ipv4Endpoint local;
		ResolveLocal(handle, local);
		m_local.host = SocketAddress::ToText(local);
		m_local.port = static_cast<WireU16>(ntohs(local.port));
		}
	}

	Result<TcpTransport> TcpTransport::Adopt(std::uintptr_t handle, const TcpTransportOptions& options)
	{
		// The all-ones value is INVALID_SOCKET in the platform's own width, which
		// is exactly what a caller passes when it has no socket to give. Rejecting it
		// here is what stops that becoming a transport that reports Connected and
		// fails every operation.
		if (handle == kNoSocket)
		{
			return Result<TcpTransport>(Status(ErrorCode::InvalidArgument));
		}

		return Result<TcpTransport>(TcpTransport(Adopted{}, handle, options));
	}

	void TcpTransport::ApplyOptions() noexcept
	{
		const SOCKET handle = static_cast<SOCKET>(CurrentSocket());
		if (CurrentSocket() == kNoSocket)
		{
			return;
		}

		// TCP_NODELAY.
		//
		// RAN sends small messages - REQ_GAME_SVR is EIGHT bytes - and expects
		// the response to follow promptly. Nagle holds a small write back until
		// the previous segment is acknowledged, which on a healthy connection is
		// invisible and on a slow one turns an 8-byte request into a stall. Legacy
		// did not set it, but legacy also ran over links where that did not bite
		// in a way it could observe; for a request/response protocol whose whole
		// latency budget is one round trip, disabling it is the correct default
		// and is a documented, ordinary option rather than a trick.
		const BOOL noDelay = TRUE;
		::setsockopt(handle, IPPROTO_TCP, TCP_NODELAY,
		             reinterpret_cast<const char*>(&noDelay), sizeof(noDelay));

		// SO_SNDTIMEO / SO_RCVTIMEO as a backstop for the window between select()
		// reporting a socket ready and the blocking call starting. On Windows
		// these are DWORD milliseconds, unlike the timeval used by select().
		const DWORD sendTimeout = static_cast<DWORD>(std::max(0, m_options.sendTimeoutMilliseconds));
		::setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO,
		             reinterpret_cast<const char*>(&sendTimeout), sizeof(sendTimeout));

		const DWORD receiveTimeout = static_cast<DWORD>(std::max(0, m_options.receiveTimeoutMilliseconds));
		::setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO,
		             reinterpret_cast<const char*>(&receiveTimeout), sizeof(receiveTimeout));
	}

	TcpTransport::~TcpTransport()
	{
		Disconnect();
		if (m_holdsRuntime)
		{
			// After the socket, never before: WSACleanup while a handle is still
			// open is exactly the ordering bug this type is here to prevent.
			WinSock::Release();
			m_holdsRuntime = false;
		}
	}

	TcpTransport::TcpTransport(TcpTransport&& other) noexcept
		: m_options(other.m_options),
		  m_holdsRuntime(other.m_holdsRuntime),
		  m_local(other.m_local),
		  m_remote(other.m_remote)
	{
		// The handle is taken, never copied. The source is left owning nothing, so
		// its destructor cannot close a socket this object is now using - the
		// double-close that makes a moved handle one of the two worst outcomes
		// possible here (the other being a leak).
		const std::uintptr_t handle = kNoSocket;
		{
			const std::lock_guard<std::mutex> lock(other.m_mutex);
			m_socket.store(other.m_socket.load(std::memory_order_acquire), std::memory_order_release);
			other.m_socket.store(handle, std::memory_order_release);
		}

		m_phase.store(other.m_phase.load(std::memory_order_acquire), std::memory_order_release);
		other.m_phase.store(static_cast<int>(TcpTransportPhase::Disconnected), std::memory_order_release);

		m_fault.store(other.m_fault.load(std::memory_order_acquire), std::memory_order_release);
		m_nativeError.store(other.m_nativeError.load(std::memory_order_acquire), std::memory_order_release);
		m_lastSend.store(other.m_lastSend.load(std::memory_order_acquire), std::memory_order_release);
		m_lastReceive.store(other.m_lastReceive.load(std::memory_order_acquire), std::memory_order_release);
		m_totalSendCalls.store(other.m_totalSendCalls.load(std::memory_order_acquire), std::memory_order_release);

		other.m_holdsRuntime = false;
		other.m_local        = Endpoint{};
		other.m_remote       = Endpoint{};
	}

	TcpTransport& TcpTransport::operator=(TcpTransport&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}

		// Release what THIS object holds before adopting the other's, or the
		// handle and the runtime reference of the old connection would leak.
		Disconnect();
		if (m_holdsRuntime)
		{
			WinSock::Release();
			m_holdsRuntime = false;
		}

		m_options = other.m_options;

		const std::lock_guard<std::mutex> lock(other.m_mutex);
		m_socket.store(other.m_socket.load(std::memory_order_acquire), std::memory_order_release);
		other.m_socket.store(kNoSocket, std::memory_order_release);

		m_phase.store(other.m_phase.load(std::memory_order_acquire), std::memory_order_release);
		other.m_phase.store(static_cast<int>(TcpTransportPhase::Disconnected), std::memory_order_release);

		m_fault.store(other.m_fault.load(std::memory_order_acquire), std::memory_order_release);
		m_nativeError.store(other.m_nativeError.load(std::memory_order_acquire), std::memory_order_release);
		m_lastSend.store(other.m_lastSend.load(std::memory_order_acquire), std::memory_order_release);
		m_lastReceive.store(other.m_lastReceive.load(std::memory_order_acquire), std::memory_order_release);
		m_totalSendCalls.store(other.m_totalSendCalls.load(std::memory_order_acquire), std::memory_order_release);

		other.m_holdsRuntime = false;

		m_local  = other.m_local;
		m_remote = other.m_remote;
		return *this;
	}

	std::uintptr_t TcpTransport::CurrentSocket() const noexcept
	{
		// Deliberately uintptr_t, not SOCKET: the header declares it that way so
		// that nothing which includes this header inherits a Winsock type. The
		// casts at the call sites below are the only places the stored width
		// becomes the platform's, and SOCKET IS UINT_PTR on Windows, so neither is
		// lossy.
		return m_socket.load(std::memory_order_acquire);
	}

	bool TcpTransport::IsValid() const noexcept
	{
		return CurrentSocket() != kNoSocket;
	}

	TcpTransportPhase TcpTransport::Phase() const noexcept
	{
		return static_cast<TcpTransportPhase>(m_phase.load(std::memory_order_acquire));
	}

	TransportState TcpTransport::State() const noexcept
	{
		// Exactly one of the two phases, and nothing inferred: a transport that
		// believes it is Connected while its handle is gone would let a caller
		// send into a closed socket and call the result a protocol error.
		return (Phase() == TcpTransportPhase::Connected && IsValid())
		           ? TransportState::Open
		           : TransportState::Closed;
	}

	std::size_t TcpTransport::PendingBytes() const noexcept
	{
		// Always zero, and the reason is in the header: this transport does not
		// queue. Send does not return until every byte has been handed to the
		// kernel, so there is no interval in which the application holds bytes the
		// peer has not been offered.
		return 0;
	}

	Status TcpTransport::Connect(const Endpoint& endpoint, int timeoutMilliseconds)
	{
		// Winsock itself must be available. Checked before anything else because a
		// transport whose socket could not be created can never succeed, and the
		// failure to report is "there are no sockets here", not "that endpoint is
		// unreachable" - two very different diagnoses.
		if (!m_holdsRuntime)
		{
			m_fault.store(static_cast<int>(TransportFault::NotSupported), std::memory_order_relaxed);
			return Status(ErrorCode::InvalidState);
		}
		if (endpoint.host.empty())
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// Any previous connection is ended, and any previous socket destroyed, before
		// a new one is created.
		//
		// Destroying rather than reusing is not tidiness, it is a platform rule: a
		// connected TCP socket cannot be re-pointed at another peer, and a socket
		// shutdown() has had can never be reconnected. (The earlier version of this
		// method called Disconnect() and then immediately asked IsValid() - which
		// could never be true, because Disconnect is what destroys the socket. Every
		// connect therefore failed with InvalidState and fault=None.)
		//
		// Legacy closes before it connects too - CNetClient::ConnectLoginServer calls
		// CloseConnect first (s_NetClient.cpp:367-372) - so this matches the behaviour
		// being replaced, not just a modern convenience.
		CloseSocket();
		if (const Status created = CreateSocket(); created.IsError())
		{
			m_phase.store(static_cast<int>(TcpTransportPhase::Disconnected), std::memory_order_release);
			return created;
		}

		const int timeout = timeoutMilliseconds > 0
		                        ? timeoutMilliseconds
		                        : m_options.connectTimeoutMilliseconds;

		SocketAddress::Ipv4Endpoint resolved;
		std::string                resolvedText;
		if (const Status status = SocketAddress::Resolve(endpoint, resolved, resolvedText);
		    status.IsError())
		{
			// Resolution failed, so there is nowhere to connect. Reported as a bad
			// argument because that is the honest diagnosis: legacy's equivalent
			// failure was a name silently becoming INADDR_NONE, and this is the
			// difference - the caller is told, in those words.
			//
			// The socket stays. Nothing has been sent on it, so it is not
			// desynchronised, and a caller that wants to try a different endpoint
			// should not have to construct a new transport to do it.
			SetFault(m_fault, m_nativeError, WSAHOST_NOT_FOUND);
			m_phase.store(static_cast<int>(TcpTransportPhase::Initialized), std::memory_order_release);
			return status;
		}

		sockaddr_in target{};
		target.sin_family      = AF_INET;
		target.sin_addr.s_addr = resolved.address;
		target.sin_port        = resolved.port;

		const SOCKET handle = static_cast<SOCKET>(CurrentSocket());

		m_phase.store(static_cast<int>(TcpTransportPhase::Connecting), std::memory_order_release);

		// Non-blocking for the duration of the connect, because a blocking connect has
		// no deadline at all: it waits out the kernel's own retry schedule, which is
		// minutes. A transport that can hang for minutes on an unreachable host is
		// neither testable nor shippable.
		if (!SetBlocking(handle, /*blocking=*/false))
		{
			const int error = LastSocketError();
			SetFault(m_fault, m_nativeError, error);
			m_phase.store(static_cast<int>(TcpTransportPhase::Initialized), std::memory_order_release);
			return StatusFor(error);
		}

		Status outcome = Ok();
		int    failure = 0;

		if (::connect(handle, reinterpret_cast<const sockaddr*>(&target), sizeof(target)) == SOCKET_ERROR)
		{
			const int error = LastSocketError();

			if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS)
			{
				// Refused, unreachable, or a bad address: the connect never started,
				// so there is nothing to wait for.
				failure = error;
			}
			else
			{
				const TransportFault ready = WaitReady(handle, /*forWrite=*/true, timeout);
				if (ready == TransportFault::WouldBlock)
				{
					// The deadline passed with the connection still in progress.
					// Distinct from a refusal, and reported as such, because the two
					// have completely different diagnoses: one is "nothing is
					// listening", the other is "nothing answered".
					m_nativeError.store(WSAETIMEDOUT, std::memory_order_relaxed);
					m_fault.store(static_cast<int>(TransportFault::Timeout), std::memory_order_relaxed);
					outcome = Status(ErrorCodeFor(TransportFault::Timeout));
				}
				else if (ready != TransportFault::None)
				{
					failure = LastSocketError();
				}
				else
				{
					// select() says writable, which for a connect means either success
					// or failure - and the only way to tell is SO_ERROR. Reading
					// select()'s return as success is the classic bug here.
					int socketError = 0;
					int length      = sizeof(socketError);
					if (::getsockopt(handle, SOL_SOCKET, SO_ERROR,
					                 reinterpret_cast<char*>(&socketError), &length) == SOCKET_ERROR)
					{
						failure = LastSocketError();
					}
					else if (socketError != 0)
					{
						failure = socketError;
					}
				}
			}
		}

		// Back to blocking for send and receive, whatever happened. Restoring the mode
		// is not optional: a socket left non-blocking makes every recv() report
		// WSAEWOULDBLOCK immediately, which a caller would have to distinguish from
		// "no data yet" on every single call, forever.
		const bool restored = SetBlocking(handle, /*blocking=*/true);

		if (failure != 0)
		{
			// The socket survives a failed connect. Returning to Initialized rather
			// than Disconnected is deliberate: the handle is still good, and a caller
			// that wants to try a different endpoint should be able to.
			SetFault(m_fault, m_nativeError, failure);
			m_phase.store(static_cast<int>(TcpTransportPhase::Initialized), std::memory_order_release);
			return StatusFor(failure);
		}

		if (!restored)
		{
			const int error = LastSocketError();
			SetFault(m_fault, m_nativeError, error);
			CloseSocket();
			m_phase.store(static_cast<int>(TcpTransportPhase::Disconnected), std::memory_order_release);
			return StatusFor(error);
		}

		if (outcome.IsError())
		{
			// A connect that timed out leaves the socket mid-handshake, and a
			// half-open socket is worse than a closed one: it accepts a send that
			// vanishes. So this one is destroyed and replaced, leaving the transport
			// Initialized with a socket that has never been used - which is the only
			// state from which a retry is trustworthy.
			CloseSocket();
			if (CreateSocket().IsError())
			{
				m_phase.store(static_cast<int>(TcpTransportPhase::Disconnected), std::memory_order_release);
			}
			else
			{
				m_phase.store(static_cast<int>(TcpTransportPhase::Initialized), std::memory_order_release);
			}
			return outcome;
		}

		// Record the peer as a RESOLVED NUMERIC address, not as the name that was
		// asked for. RAN's wire carries a 21-byte numeric IPv4, so "localhost" is not
		// something the protocol layer could record even if it wanted to.
		{
			const std::lock_guard<std::mutex> lock(m_mutex);
			m_remote.host = resolvedText;
			m_remote.port = endpoint.port;

			SocketAddress::Ipv4Endpoint local;
			ResolveLocal(handle, local);
			m_local.host = SocketAddress::ToText(local);
			m_local.port = static_cast<WireU16>(ntohs(local.port));
		}

		m_phase.store(static_cast<int>(TcpTransportPhase::Connected), std::memory_order_release);
		ClearFault(m_fault, m_nativeError);
		return Ok();
	}

	void TcpTransport::ResolveLocal(std::uintptr_t raw, SocketAddress::Ipv4Endpoint& out) const noexcept
	{
		// uintptr_t rather than SOCKET for the same reason CurrentSocket() is: the
		// header must not name a Winsock type. The cast is exact, because SOCKET IS
		// UINT_PTR on Windows.
		const SOCKET handle = static_cast<SOCKET>(raw);
		sockaddr_in address{};
		int         length = sizeof(address);
		if (::getsockname(handle, reinterpret_cast<sockaddr*>(&address), &length) == SOCKET_ERROR)
		{
			// Nothing to record. An endpoint reported as unknown is honest; a zeroed
			// 0.0.0.0:0 would read as a real address and invite a bug report about a
			// connection to nowhere.
			out = SocketAddress::Ipv4Endpoint{};
			return;
		}

		out.address = address.sin_addr.s_addr;
		out.port    = address.sin_port;
	}

	Status TcpTransport::Send(const WireU8* data, std::size_t size)
	{
		return Send(data, size, m_options.sendTimeoutMilliseconds);
	}

	Status TcpTransport::Send(const WireU8* data, std::size_t size, int timeoutMilliseconds)
	{
		m_lastSend.store(0, std::memory_order_relaxed);

		if (data == nullptr && size != 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		// A zero-length send is a no-op, not a failure. Callers build messages by
		// appending, and an empty one is a legitimate thing to hand over.
		if (size == 0)
		{
			return Ok();
		}
		if (State() != TransportState::Open)
		{
			m_fault.store(static_cast<int>(TransportFault::NotConnected), std::memory_order_relaxed);
			return Status(ErrorCode::InvalidState);
		}

		const int timeout = timeoutMilliseconds > 0
		                        ? timeoutMilliseconds
		                        : m_options.sendTimeoutMilliseconds;

		// Copied out under the lock, then used without it. A blocking send must NOT
		// be made while holding m_mutex, or Disconnect() from another thread - the
		// "server closed first" case - would deadlock behind it. The race this
		// leaves is benign: the send then fails on a closed handle, which is the
		// truthful outcome of having it closed underneath us.
		const SOCKET handle = static_cast<SOCKET>(CurrentSocket());

		std::size_t offset = 0;
		int          timeoutError = 0;

		while (offset < size)
		{
			const TransportFault ready = WaitReady(m_socket.load(std::memory_order_acquire),
			                                       /*forWrite=*/true, timeout);
			if (ready == TransportFault::WouldBlock)
			{
				// Deadline passed with bytes still unsent. Whether or not the socket
				// is still healthy, the PEER has received a prefix of a message and
				// will never receive the rest, so the stream is desynchronised and
				// the connection is finished. Reported distinctly from every other
				// failure because it is the one that must never be retried on the
				// same socket.
				timeoutError = WSAETIMEDOUT;
				break;
			}
			if (ready != TransportFault::None)
			{
				const int error = LastSocketError();
				SetFault(m_fault, m_nativeError, error);
				return StatusFor(error);
			}

			const int chunk = static_cast<int>(
				std::min<std::size_t>(size - offset, static_cast<std::size_t>(64 * 1024)));

			m_totalSendCalls.fetch_add(1, std::memory_order_relaxed);

			const int written = ::send(handle, reinterpret_cast<const char*>(data + offset),
			                           chunk, 0);

			if (written == SOCKET_ERROR)
			{
				const int error = LastSocketError();

				// A blocking socket with SO_SNDTIMEO reports WSAEWOULDBLOCK when its
				// own timeout expires. That is this transport's send deadline, and it
				// is the same situation as select() timing out, so it must not be
				// mistaken for a benign "try again later".
				if (error == WSAEWOULDBLOCK)
				{
					timeoutError = WSAETIMEDOUT;
					break;
				}

				// WSAEINTR is the one recoverable send error: the call was aborted
				// before transferring anything, so retrying is correct and does not
				// risk duplication.
				if (error == WSAEINTR)
				{
					continue;
				}

				SetFault(m_fault, m_nativeError, error);
				return StatusFor(error);
			}

			if (written == 0)
			{
				// A zero-byte send on a connected socket means the peer is gone.
				// Treated as a reset rather than a no-op, because the alternative -
				// spinning on a dead socket - is a hang.
				m_fault.store(static_cast<int>(TransportFault::ConnectionReset), std::memory_order_relaxed);
				m_nativeError.store(WSAECONNRESET, std::memory_order_relaxed);
				return Status(ErrorCodeFor(TransportFault::ConnectionReset));
			}

			// The whole reason this method is a loop. send() is documented to accept
			// FEWER bytes than requested, and does so whenever the send buffer is
			// fuller than the remaining space. One send() call is NOT one message,
			// and a transport that assumed it was would truncate the RAN stream
			// silently at exactly the moment a server started flooding a client
			// with the 200-entry game-server list.
			offset += static_cast<std::size_t>(written);
			m_lastSend.fetch_add(static_cast<std::size_t>(written), std::memory_order_relaxed);
		}

		if (timeoutError != 0)
		{
			// Reported as PartialSend even when every byte happened to go, because
			// the distinction that matters is whether the CALLER was told the whole
			// buffer left. A caller cannot know that unless this says so.
			m_fault.store(static_cast<int>(TransportFault::PartialSend), std::memory_order_relaxed);
			m_nativeError.store(timeoutError, std::memory_order_relaxed);
			return Status(ErrorCodeFor(TransportFault::PartialSend));
		}

		ClearFault(m_fault, m_nativeError);
		return Ok();
	}

	Status TcpTransport::Receive(WireU8* out, std::size_t maxBytes, std::size_t& received)
	{
		return Receive(out, maxBytes, received, m_options.receiveTimeoutMilliseconds);
	}

	Status TcpTransport::Receive(WireU8* out, std::size_t maxBytes, std::size_t& received,
	                             int timeoutMilliseconds)
	{
		// `received` is set on EVERY path, including the error paths. A caller that
		// reads it after a failed receive must get a real number rather than
		// whatever was on its stack, because a plausible garbage length is far
		// worse than an obvious zero.
		received = 0;
		m_lastReceive.store(0, std::memory_order_relaxed);

		if (out == nullptr && maxBytes != 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}
		// Asking for no bytes is a no-op, and asking for no bytes AND having no
		// destination is the same no-op. Neither is a socket operation.
		if (maxBytes == 0)
		{
			return Ok();
		}
		if (State() != TransportState::Open)
		{
			m_fault.store(static_cast<int>(TransportFault::NotConnected), std::memory_order_relaxed);
			return Status(ErrorCode::InvalidState);
		}

		const int timeout = timeoutMilliseconds > 0
		                        ? timeoutMilliseconds
		                        : m_options.receiveTimeoutMilliseconds;

		// See Send: the handle is copied out under the lock and used without it, so
		// a Disconnect() on another thread can shut the socket down underneath a
		// blocked receive and have that receive return.
		const SOCKET handle = static_cast<SOCKET>(CurrentSocket());

		const TransportFault ready =
		    WaitReady(m_socket.load(std::memory_order_acquire), /*forWrite=*/false, timeout);

		if (ready == TransportFault::WouldBlock)
		{
			// The deadline passed with nothing to read.
			//
			// SUCCESS with zero bytes, not an error. The interface says so -
			// "0 means nothing available yet, which is NOT an error" - and it is
			// what lets a caller run a poll loop without inventing an error for
			// every idle frame. Fault() still records WouldBlock so a caller that
			// wants to know a deadline elapsed can ask.
			m_fault.store(static_cast<int>(TransportFault::WouldBlock), std::memory_order_relaxed);
			m_nativeError.store(WSAETIMEDOUT, std::memory_order_relaxed);
			return Ok();
		}
		if (ready != TransportFault::None)
		{
			const int error = LastSocketError();
			SetFault(m_fault, m_nativeError, error);
			return StatusFor(error);
		}

		const int chunk = static_cast<int>(
			std::min<std::size_t>(maxBytes, static_cast<std::size_t>(64 * 1024)));

		const int read = ::recv(handle, reinterpret_cast<char*>(out), chunk, 0);

		if (read == SOCKET_ERROR)
		{
			const int error = LastSocketError();

			// Our own shutdown, seen by a receive that was already blocked. That is
			// the peer-observes-our-close path completing, not a fault: the local
			// side asked for it.
			if (error == WSAESHUTDOWN || error == WSAENOTCONN)
			{
				m_fault.store(static_cast<int>(TransportFault::NotConnected), std::memory_order_relaxed);
				m_nativeError.store(error, std::memory_order_relaxed);
				return Status(ErrorCodeFor(TransportFault::NotConnected));
			}

			// WSAEWOULDBLOCK from the socket's own SO_RCVTIMEO: same situation as
			// select() timing out, and equally not an error.
			if (error == WSAEWOULDBLOCK)
			{
				m_fault.store(static_cast<int>(TransportFault::WouldBlock), std::memory_order_relaxed);
				m_nativeError.store(WSAETIMEDOUT, std::memory_order_relaxed);
				return Ok();
			}

			if (error == WSAECONNRESET || error == WSAENETRESET)
			{
				m_fault.store(static_cast<int>(TransportFault::ConnectionReset), std::memory_order_relaxed);
				m_nativeError.store(error, std::memory_order_relaxed);
				return Status(ErrorCodeFor(TransportFault::ConnectionReset));
			}

			SetFault(m_fault, m_nativeError, error);
			return StatusFor(error);
		}

		if (read == 0)
		{
			// recv() returning 0 on a connected socket is an orderly FIN: the peer
			// closed its half. NOT an error - the interface says zero bytes is not
			// an error, and this is zero bytes. PeerClosed records WHY, because a
			// caller genuinely does need to distinguish "the connection ended" from
			// "nothing arrived yet", and State() is about our own side.
			m_fault.store(static_cast<int>(TransportFault::PeerClosed), std::memory_order_relaxed);
			m_nativeError.store(0, std::memory_order_relaxed);

			// The peer is gone, so this transport is not connected any more, no
			// matter what our own handle still says. Reporting Connected here would
			// invite a send that fails with a reset instead of a clean refusal.
			m_phase.store(static_cast<int>(TcpTransportPhase::Disconnected), std::memory_order_release);
			return Ok();
		}

		received        = static_cast<std::size_t>(read);
		m_lastReceive.store(received, std::memory_order_relaxed);
		ClearFault(m_fault, m_nativeError);
		return Ok();
	}

	void TcpTransport::Disconnect() noexcept
	{
		CloseSocket();
		m_phase.store(static_cast<int>(TcpTransportPhase::Disconnected), std::memory_order_release);
	}

	Endpoint TcpTransport::LocalEndpoint() const noexcept
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		return m_local;
	}

	Endpoint TcpTransport::RemoteEndpoint() const noexcept
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		return m_remote;
	}

	TransportFault TcpTransport::Fault() const noexcept
	{
		return static_cast<TransportFault>(m_fault.load(std::memory_order_acquire));
	}

	int TcpTransport::NativeError() const noexcept
	{
		return m_nativeError.load(std::memory_order_acquire);
	}

	std::size_t TcpTransport::LastSendCount() const noexcept
	{
		return m_lastSend.load(std::memory_order_acquire);
	}

	std::size_t TcpTransport::LastReceiveCount() const noexcept
	{
		return m_lastReceive.load(std::memory_order_acquire);
	}

	std::size_t TcpTransport::TotalSendCalls() const noexcept
	{
		return m_totalSendCalls.load(std::memory_order_acquire);
	}

	Status TcpTransport::AcquireRuntime() noexcept
	{
		return WinSock::Acquire();
	}

	void TcpTransport::ReleaseRuntime() noexcept
	{
		WinSock::Release();
	}

	Status EnsureWinsockRuntime() noexcept
	{
		return WinSock::Acquire();
	}
}

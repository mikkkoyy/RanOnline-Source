#pragma once

// NETWORK-001: the real TCP transport.
//
// This is the type LOGIN-001 was blocked on. modern/ contained no socket code at
// all - no <winsock2.h>, no socket() call - so the Login Server exchange could be
// proven at the protocol level and at the INetworkTransport abstraction level, but
// never across a real connection. This is that connection.
//
// WHAT IT IS DELIBERATELY NOT:
//
//   * Not an asynchronous framework. The brief asks for correctness, deterministic
//     behaviour, clean ownership and simple integration, and explicitly not a
//     giant async networking stack. Legacy runs an IOCP accept thread, S_HEURISTIC_NUM
//     worker threads per CPU and an update thread (s_CServer.h); none of that is
//     reproduced, because none of it is needed to prove the transport correct and
//     all of it is needed to make a test non-deterministic.
//
//   * Not a message boundary. A TCP read has nothing to do with a RAN message
//     boundary. This type hands over whatever bytes arrived, in whatever
//     quantities, and ConnectionFramer decides where a message starts and stops.
//     That separation is the WORLD-002 fix preserved: the bug that cost the most
//     was a parser that treated one recv() as one packet.
//
//   * Not aware of any protocol. No message id appears here. 1542, 1552 and 1562
//     are as invisible to this file as they are to a socket in legacy.
//
// SOCKET OWNERSHIP. The transport owns its socket for its whole life, acquired in
// the constructor's dependency and released in the destructor. A SOCKET is a
// `UINT_PTR` on Windows, so a plain member that reaches 0 through a bug would be
// indistinguishable from a real socket, and `closesocket(0)` fails quietly. The
// handle is therefore initialised to INVALID_SOCKET, tested against that before
// every use, and cleared the moment it is closed.
//
// THE LOCK. `m_mutex` protects `m_socket` and nothing else, and is deliberately
// NOT held across a blocking send or receive. Holding it would mean Disconnect()
// from another thread deadlocks against a Receive() that is waiting for data -
// and that is precisely the "server closes first" case that has to work. So a
// blocking call copies the handle out under the lock and then works on its own,
// and Disconnect() takes the lock only to take the handle away. The race that
// leaves is benign and correct: an in-flight send then fails, which is the
// truthful outcome of having the socket closed underneath it.
//
// TIME-OUTS ARE NOT OPTIONAL. Every blocking operation takes a deadline, and
// every one of them returns before it. A test that can hang is a test CI cannot
// trust, and a client that can hang on a dead peer is a frozen window.

#include "NetworkTransport.h"
#include "SocketAddress.h"
#include "types/Result.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>

namespace Modern::Network
{
	// Where a transport is in its own lifecycle.
	//
	// Modelled rather than inferred from State() because the interesting cases
	// are the ones a boolean cannot express: a socket created but not yet
	// connected, a close in progress, and a connect that failed partway. Each of
	// those has a distinct correct next step, and collapsing them into Closed /
	// Open is how a failed connect ends up looking like a clean disconnect.
	enum class TcpTransportPhase : uint8_t
	{
		// No socket, or the socket is closed. The resting state, and the state a
		// failed connect or a peer disconnect returns to.
		Disconnected = 0,

		// Winsock is up and a socket exists, but nothing is connected yet.
		Initialized,

		// Connect is in flight. Observable only from another thread, which is why
		// it exists as a value rather than as an internal step.
		Connecting,

		// Connected. The only phase in which Send and Receive are permitted.
		Connected,

		// A close has been requested and the handle taken, but the peer has not
		// necessarily observed it yet.
		Closing,
	};

	const char* ToString(TcpTransportPhase phase) noexcept;

	// Per-transport time-outs, all in milliseconds.
	//
	// There is no "infinite" option and that is deliberate. Legacy sets
	// SO_SNDTIMEO and SO_RCVTIMEO to 10 seconds on the client socket (public
	// backread of CClientSocket::CreateSocket) and gives connect a 4-second
	// select() deadline, so this is also what RAN actually did; a modern
	// transport with no deadline at all would be a regression against the
	// legacy behaviour it is replacing.
	struct TcpTransportOptions
	{
		// Applied to the non-blocking connect and its select() wait.
		int connectTimeoutMilliseconds = 5000;

		// Applied to the blocking send loop.
		int sendTimeoutMilliseconds = 5000;

		// Applied to a single receive. A receive that elapses this returns zero
		// bytes and no error, exactly as INetworkTransport::Receive specifies for
		// "nothing available yet"; the caller owns the overall deadline.
		int receiveTimeoutMilliseconds = 1000;

		// Applied to TcpListener::Accept, so it does not live here; a listener
		// has no send or receive of its own.
	};

	// A connected TCP socket, over Winsock.
	//
	// Default constructed, it is an Initialized transport with a live socket and
	// nothing connected. Destroying a connected transport closes the socket; that
	// is not a fallback, it is the ownership rule, and it is why there is no
	// separate Adopt() for a handle somebody else owns.
	class TcpTransport final : public INetworkTransport
	{
	public:
		// Creates a transport and takes a Winsock reference.
		//
		// Constructing a socket is not guaranteed to succeed - the process can be
		// out of descriptors - so a caller must check IsValid() before assuming
		// it has a transport at all. No Winsock error is thrown or reported
		// through an exception; the modern tree reports failure by value.
		explicit TcpTransport(const TcpTransportOptions& options = TcpTransportOptions{});

		// Takes ownership of an ALREADY-CONNECTED socket, and takes a Winsock
		// reference for it.
		//
		// This exists for the server side, and only for the server side: a socket
		// returned by accept() is already connected, so there is nothing for
		// Connect() to do and nothing for it to decide. Handing that socket to a
		// constructor that insists on making its own would mean either leaking the
		// accepted one or inventing a wrapper type that owns two sockets' worth of
		// state. So ownership TRANSFERS here, and the caller must not close the
		// handle afterwards.
		//
		// `handle` is the platform's socket value in its own width (uintptr_t, which
		// is exactly SOCKET on Windows) so that this header still names no Winsock
		// type. Passing kNoSocket-equivalent - the all-ones value, which is
		// INVALID_SOCKET - returns InvalidArgument and adopts nothing.
		//
		// The socket's own options (TCP_NODELAY, SO_*TIMEO) are applied, so an
		// adopted connection behaves like one this class opened. That is not
		// cosmetic: TCP_NODELAY in particular is what keeps an 8-byte REQ_GAME_SVR
		// from sitting behind Nagle waiting for an acknowledgement.
		//
		// A failure here means the transport could not be built; the handle is NOT
		// closed on failure, because a caller that gets an error still owns what it
		// passed in and is better able to decide what to do with it than this
		// factory is.
		static Result<TcpTransport> Adopt(std::uintptr_t               handle,
		                                  const TcpTransportOptions& options = TcpTransportOptions{});

		// Closes the socket and drops the Winsock reference.
		//
		// Closing here rather than hoping the caller remembered is the point of
		// owning the socket. Safe to destroy while connected.
		~TcpTransport() override;

		// TcpTransport owns a socket, so copying one would either double-close it
		// or silently share a handle. Moves are therefore explicit and unique,
		// and the moved-from transport is left Disconnected rather than Valid.
		TcpTransport(const TcpTransport&)            = delete;
		TcpTransport& operator=(const TcpTransport&) = delete;
		TcpTransport(TcpTransport&& other) noexcept;
		TcpTransport& operator=(TcpTransport&& other) noexcept;

		// ---- lifecycle ----------------------------------------------------

		// True when a socket exists, connected or not. False means socket()
		// failed or Winsock was unavailable, and every operation will fail.
		//
		// Distinct from IsConnected(): a transport can be perfectly usable in one
		// moment and not the other.
		bool IsValid() const noexcept;

		TcpTransportPhase Phase() const noexcept;

		// ---- INetworkTransport --------------------------------------------

		TransportState State() const noexcept override;

		// Always 0.
		//
		// Not because there is nothing to report, but because this transport does
		// not queue: Send writes to the socket before it returns, so there is no
		// interval in which bytes are accepted by the application and not yet
		// handed to the peer. A queue here would be a lie about where the bytes
		// are. (The loopback transport, which DOES queue, is what the interface
		// comment means by "the loopback transport lets a test assert that a
		// partial flush is possible".)
		std::size_t PendingBytes() const noexcept override;

		// Writes the whole buffer, looping over partial sends.
		//
		// send() is permitted to accept fewer bytes than it was given, and does
		// so routinely once the socket buffer fills. Treating one send() as one
		// message is the single most common way to corrupt a stream protocol, and
		// legacy's own client loops for exactly this reason (CClientSocket::SendDataS
		// in the public backread: `while(left>0) { ... left-=thisret; idx+=thisret; }`).
		//
		// A zero-length send succeeds and does nothing. A send that cannot
		// complete inside the timeout is reported as an error AND leaves Fault()
		// set to PartialSend, because the stream is now desynchronised: the peer
		// has seen a prefix of a message and will never see the rest. The
		// connection must be dropped rather than reused.
		Status Send(const WireU8* data, std::size_t size) override;

		// Reads whatever has arrived, up to `maxBytes`.
		//
		// Per the interface contract, zero bytes is not an error - it means the
		// peer has not sent yet. `received` is always set, including on the error
		// paths that still copied something, so a caller can never be left
		// reading an uninitialised length.
		//
		// The return of zero bytes with no error is what lets a caller drive this
		// from a frame loop: feed whatever arrived, wait, try again. A transport
		// that blocked until `maxBytes` arrived would be a transport that had
		// invented message boundaries.
		Status Receive(WireU8* out, std::size_t maxBytes, std::size_t& received) override;

		// Shuts the socket down and closes it, if it is open.
		//
		// Idempotent: a second call does nothing and is not an error. shutdown()
		// before closesocket() is not tidiness - it is what makes a peer blocked
		// in recv() observe the close, and what makes a Receive() blocked on
		// THIS socket return rather than hang when another thread closes it.
		//
		// Safe to call concurrently with a blocked Send or Receive on the same
		// transport; that is the "server closes first" case.
		void Disconnect() noexcept override;

		Endpoint LocalEndpoint() const noexcept override;

		// The peer, as a resolved NUMERIC address once connected, and as the
		// configured host before that.
		//
		// Numeric after a successful connect even when Connect was handed a
		// hostname, because that is the address the RAN protocol layer can
		// actually name.
		Endpoint RemoteEndpoint() const noexcept override;

		TransportFault Fault() const noexcept override;
		int           NativeError() const noexcept override;

		// ---- connect -------------------------------------------------------

		// Connects to `endpoint`, resolving a hostname if one was given.
		//
		// `timeoutMilliseconds` of 0 or less means "use the option", so a caller
		// that wants a one-off long wait for a slow link can ask without
		// reconfiguring the transport.
		//
		// Non-blocking connect plus a bounded wait, which is the only way to put
		// a deadline on a connect at all: a blocking connect waits for the
		// kernel's own, much longer, retry schedule. The socket is put back into
		// blocking mode afterwards, because a half-non-blocking connect is a
		// subtle source of send/recv surprises.
		//
		// On failure the transport returns to Initialized, not Disconnected: the
		// socket exists and is still usable, and a caller that wants to try a
		// different endpoint should be able to. Fault() explains the failure -
		// ConnectionRefused, HostUnreachable, Timeout and AddressInvalid are four
		// different diagnoses and the brief asks for all of them to be
		// distinguishable.
		Status Connect(const Endpoint& endpoint, int timeoutMilliseconds = 0);

		// ---- explicit per-call time-outs ------------------------------------

		// As Send, with a one-off deadline. <= 0 means "use the option".
		//
		// A separate overload rather than only a setter because the natural
		// pattern - a short receive deadline in a poll loop, a long one when
		// actually waiting for an answer - would otherwise mean reconfiguring the
		// socket's SO_RCVTIMEO around every call.
		Status Send(const WireU8* data, std::size_t size, int timeoutMilliseconds);

		// As Receive, with a one-off deadline. <= 0 means "use the option".
		Status Receive(WireU8* out, std::size_t maxBytes, std::size_t& received,
		              int timeoutMilliseconds);

		// ---- diagnostics ---------------------------------------------------

		// How many bytes the last successful Receive returned, and how many the
		// last successful Send handed to the kernel in total.
		//
		// A partial-send counter is genuinely useful: it is the only place a test
		// can see that the loop ran, since a socket that happens to accept a whole
		// buffer in one call and one that needs three look identical from the
		// outside.
		std::size_t LastSendCount() const noexcept;
		std::size_t LastReceiveCount() const noexcept;
		std::size_t TotalSendCalls() const noexcept;

		// Takes a Winsock reference for a subsystem that outlives any single
		// transport.
		//
		// The constructor already does this per socket, so a caller normally
		// needs nothing. This exists for the application-level case the brief
		// names - keeping Winsock up across a teardown and rebuild of a
		// connection - and it is the same counter, so it cannot double-start
		// Winsock.
		static Status AcquireRuntime() noexcept;
		static void   ReleaseRuntime() noexcept;

	private:
		// ---- internals ------------------------------------------------------


		// Construction path for Adopt only.
		//
		// A tag type rather than a bool, a sentinel handle, or an overload taking
		// uintptr_t: every one of those could be reached by accident from outside,
		// and building a transport around a handle somebody else owns is exactly the
		// ownership question this class is careful about. Private means only Adopt can
		// ask for it.
		struct Adopted
		{
		};

		// Builds a transport around an already-connected socket.
		//
		// Takes the Winsock reference and applies the socket options. Deliberately
		// does NOT close the handle on failure: Adopt has to report that it could not
		// build a transport, and a factory that also closed the caller's socket on the
		// way out would be making an ownership decision the caller is better placed to
		// make.
		TcpTransport(Adopted, std::uintptr_t handle, const TcpTransportOptions& options);
		// The live handle, or kNoSocket.
		//
		// uintptr_t rather than SOCKET, because SOCKET is a Winsock type and this
		// header must not name one. SOCKET IS UINT_PTR on Windows, so the width is
		// exact on x86 and x64 rather than approximately right, and the value is a
		// faithful stand-in for every call that wants the real thing.
		static constexpr std::uintptr_t kNoSocket = ~static_cast<std::uintptr_t>(0);

		// Reads the handle. Every socket operation starts here, because the handle
		// can be taken away by another thread at any moment.
		std::uintptr_t CurrentSocket() const noexcept;

		// ---- socket lifetime ------------------------------------------------
		//
		// CreateSocket and CloseSocket are separate from the public Disconnect
		// because Connect needs both halves and cannot use Disconnect for either.
		//
		// Disconnect destroys the socket, and it must: shutdown() before
		// closesocket() is what makes a peer blocked in recv() observe the close, and
		// that is the whole reason this class closes rather than merely marking
		// itself disconnected. But a TCP socket that has been connected cannot be
		// re-pointed at a different peer, and neither can one that has been closed -
		// so Connect has to destroy any previous socket AND create a replacement
		// before it can dial again. Expressing that as two steps here is what keeps
		// Connect from looking like a sequence of operations that quietly cannot work.

		// Creates a fresh socket and stores it. Returns the socket error on failure,
		// having recorded it in Fault()/NativeError().
		//
		// Does NOT touch the phase: the caller decides whether the result means
		// Initialized or Connected, because those are different conclusions about the
		// same successful socket creation.
		Status CreateSocket() noexcept;

		// shutdown()s and closes the socket if there is one, then forgets it.
		//
		// Idempotent. Does NOT touch the phase, for the same reason as above: Close
		// is called both by Disconnect (which must end Disconnected) and by Connect
		// (which must go on to Initialized).
		void CloseSocket() noexcept;

		// Applies the options that are fixed for the transport's lifetime:
		// TCP_NODELAY and the SO_*TIMEO backstops. Called once, after the socket
		// exists.
		void ApplyOptions() noexcept;

		// Fills in `out` from getsockname. Leaves `out` zeroed if the local
		// address cannot be read, which a caller renders as "unknown" rather than
		// as 0.0.0.0.
		void ResolveLocal(std::uintptr_t handle, SocketAddress::Ipv4Endpoint& out) const noexcept;

		TcpTransportOptions m_options;

		// True when this object holds a Winsock reference. False for a transport
		// whose socket could not be created, so its destructor releases nothing.
		//
		// Plain bool, not atomic: it is written only during construction and
		// destruction, and a move transfers it between objects that are not yet
		// shared by any other thread.
		bool m_holdsRuntime = false;

		// The handle, and every diagnostic, behind the lock.
		//
		// Everything here is atomic or short, and none of it is held across a
		// blocking call - see the class comment for why that is the whole design.
		mutable std::mutex m_mutex;
		std::atomic<std::uintptr_t> m_socket{kNoSocket};

		// Phase, fault and counters are atomic because a caller legitimately
		// inspects them from another thread - the accept loop checking whether the
		// peer vanished, a test checking what a send did - and a torn read of a
		// phase would report a connection state that never existed.
		std::atomic<int> m_phase{static_cast<int>(TcpTransportPhase::Disconnected)};
		std::atomic<int> m_fault{static_cast<int>(TransportFault::None)};
		std::atomic<int> m_nativeError{0};

		std::atomic<std::size_t> m_lastSend{0};
		std::atomic<std::size_t> m_lastReceive{0};
		std::atomic<std::size_t> m_totalSendCalls{0};

		// Recorded endpoints. Protected by m_mutex, and only ever written by
		// Connect and the destructor.
		Endpoint m_local;
		Endpoint m_remote;
	};

	// Makes Winsock available.
	//
	// The named function rather than a bare WinSock::Acquire() at every call site,
	// because "prepare the platform for sockets" reads better at a subsystem
	// boundary and it keeps WinSockRuntime's name out of application headers.
	Status EnsureWinsockRuntime() noexcept;
}

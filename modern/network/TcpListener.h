#pragma once

// NETWORK-001: the server side of a TCP endpoint. bind, listen, accept.
//
// WHY THIS IS A SEPARATE TYPE FROM TcpTransport.
//
// A listening socket and a connected socket are not the same thing with a flag
// between them. The listening socket has no peer - LocalEndpoint is meaningful
// and RemoteEndpoint is meaningless - it has a backlog rather than a
// conversation, accept() hands back a NEW socket rather than returning the one it
// was called on, and the thing accept() returns is already connected. Modelling
// that with a bool on TcpTransport would mean every method having to ask which
// mode it is in before doing anything, and the answer would be wrong in at least
// one place.
//
// A listener also outlives the connections it produces. The Login Server binds
// once and accepts many, and the Winsock reference a listener holds is what keeps
// Winsock up between them - which is why Close() and the transports' destructors
// have to be ordered rather than merely both happening.
//
// PORT 0 IS THE POINT OF A LARGE PART OF THIS FILE.
//
// Binding 127.0.0.1:0 asks the OS for a free port, and getsockname then reports
// which one it chose. That is what makes a test that needs a real listening
// socket possible without the test and the machine agreeing in advance on a port
// number: two runs of the test suite, or a developer's server already on 5001,
// cannot collide. BoundEndpoint() exists solely to hand that number back, and a
// caller that binds port 0 and then ignores BoundEndpoint() has built something
// untestable.
//
// NOT AN ACCEPT LOOP, AND NOT THREADED. Accept() hands back one connection and
// the caller decides what to do with it. There is no thread here, no pool, and no
// callback: the modern runtime is synchronous, and the caller that needs several
// connections concurrently should say so by spawning the thread, where the
// lifetime is visible, rather than finding a hidden one inside a socket class.
//
// TIME-OUTS ARE NOT OPTIONAL, SAME AS THE TRANSPORT. An Accept that can block
// forever is a server that cannot be shut down, and a test that hangs on one is a
// test CI cannot trust. Every Accept takes a deadline.

// NO WINSOCK TYPE APPEARS IN THIS HEADER, for the reason given in
// SocketAddress.h: a public header that named one would force every consumer to
// inherit the platform's declaration order, its macro set, and its requirement
// that nothing included <windows.h> first. Handles cross the boundary as
// uintptr_t, which is exactly SOCKET's width on Windows.

#include "NetworkTransport.h"
#include "types/Result.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>

namespace Modern::Network
{
	// Per-listener settings.
	struct TcpListenerOptions
	{
		// Pending-connection queue depth. Clamped to the platform's maximum inside
		// Listen, because passing a backlog the OS cannot honour is not an error
		// the platform reports - it is silently rounded, and a server that believes
		// it has a deep queue and does not is a server whose connections are refused
		// for no visible reason.
		//
		// 16 rather than legacy's much larger figures: this milestone serves one
		// client at a time, and a backlog that is honest about that is better than
		// one that advertises a capacity nothing here can use.
		int backlog = 16;

		// Applied to a call to Accept that does not pass its own deadline.
		int acceptTimeoutMilliseconds = 5000;
	};

	// A bound, listening TCP socket.
	//
	// Default constructed it owns a socket but has not bound it; call Listen. A
	// caller must therefore check IsListening() rather than assuming a constructed
	// listener is usable - the same rule TcpTransport states for IsValid().
	class TcpListener
	{
	public:
		explicit TcpListener(const TcpListenerOptions& options = TcpListenerOptions{});

		// Closes the listening socket and drops the Winsock reference.
		~TcpListener();

		// A listener owns a socket, so copying one would either double-close it or
		// silently share a handle. Moves are explicit and unique, and the moved-from
		// listener is left not-listening rather than valid.
		TcpListener(const TcpListener&)            = delete;
		TcpListener& operator=(const TcpListener&) = delete;
		TcpListener(TcpListener&& other) noexcept;
		TcpListener& operator=(TcpListener&& other) noexcept;

		// ---- lifecycle ----------------------------------------------------

		// Binds to `bindEndpoint` and starts listening.
		//
		// `bindEndpoint.port` may be 0, meaning "any free port"; read the result from
		// BoundEndpoint(). A host of "0.0.0.0" binds every interface, and
		// "127.0.0.1" binds loopback only - the distinction matters and is the
		// caller's to make, because "reachable from the network" is not a decision a
		// socket class should take on its own.
		//
		// Binds WITHOUT SO_REUSEADDR. That is a deliberate difference from most
		// server code, and the reason is that SO_REUSEADDR on Windows lets a second
		// process bind a port that is already being listened on, which turns "the port
		// is taken" into "two servers silently split the traffic". A Login Server that
		// cannot tell it failed to start is worse than one that refuses to start.
		//
		// Listening before binding is impossible and binding before listening means a
		// window in which the socket is bound but connections are refused rather than
		// queued, so both happen here, in that order, as one operation.
		Status Listen(const Endpoint& bindEndpoint);

		// True once Listen has succeeded and Close has not been called.
		//
		// NOT the same question as "does this object hold a socket". A constructed
		// listener holds a socket and is not listening, because nothing is bound to
		// it; treating "has a handle" as "is listening" would report a server as up
		// before it could accept anything, which is exactly the kind of false "ready"
		// that turns into a refused connection nobody can explain.
		//
		// Both conditions are checked rather than just the flag: a flag that says
		// listening while the handle is gone would let Accept block on a closed
		// socket.
		bool IsListening() const noexcept;

		// The address actually bound, including the OS-assigned port when 0 was
		// requested.
		//
		// Empty until Listen succeeds. After a successful Listen this is never empty:
		// getsockname on a bound socket always reports something, and a zeroed
		// 0.0.0.0:0 here would read as a real address and invite a bug report about a
		// server listening on nothing.
		Endpoint BoundEndpoint() const noexcept;

		// ---- accept -------------------------------------------------------

		// Waits for and accepts one connection.
		//
		// On success `outHandle` receives the accepted socket and the caller owns it;
		// hand it to TcpTransport::Adopt, which takes ownership. Nothing else may
		// close it, and nothing else should either - a second closesocket on a handle
		// the OS has already recycled frees a descriptor that now belongs to
		// something else, which fails in a way that is very hard to attribute.
		//
		// `timeoutMilliseconds` of 0 or less means "use the option". A deadline that
		// passes is SUCCESS with `outHandle` left as the sentinel, because "no
		// connection arrived" is the same non-event as "no data arrived" on a
		// transport, and a caller running an accept loop must not have to invent an
		// error for every idle pass. Fault() records WouldBlock so a caller that does
		// want to know can ask.
		//
		// Returns InvalidState when the listener is not listening, which is a
		// programming error rather than a runtime condition - but it is reported by
		// value like everything else, because this tree does not use exceptions for
		// control flow.
		Status Accept(std::uintptr_t& outHandle, int timeoutMilliseconds = 0);

		// The sentinel Accept writes when nothing arrived. Equal to INVALID_SOCKET,
		// and named here so a caller can test it without knowing that.
		static constexpr std::uintptr_t kNoAcceptedSocket = ~static_cast<std::uintptr_t>(0);

		// Stops listening and closes the socket. Idempotent.
		//
		// Closes only the LISTENING socket. Connections already accepted are owned by
		// their transports and are unaffected, which is the point: a server shutting
		// down stops accepting new work without cutting off work in progress.
		void Close() noexcept;

		// ---- diagnosis ----------------------------------------------------

		// Why the most recent operation did not do what was asked, or None if it did.
		// A "what just happened" register, not a sticky error: read it immediately
		// after the call it explains.
		TransportFault Fault() const noexcept { return static_cast<TransportFault>(m_fault.load(std::memory_order_acquire)); }

		// The platform code behind the last fault, for logs. Nothing above this
		// class may branch on it; application behaviour goes through Fault().
		int NativeError() const noexcept { return m_nativeError.load(std::memory_order_acquire); }

		// Takes a Winsock reference for a subsystem that outlives any one listener -
		// a server process that binds once and accepts many has no other reason to
		// keep Winsock up between connections.
		static Status AcquireRuntime() noexcept;
		static void   ReleaseRuntime() noexcept;

	private:
		// The live handle, or the all-ones sentinel.
		//
		// The same value as kNoSocket in TcpTransport, reached through a different
		// name because this class has no business naming the other's member.
		static constexpr std::uintptr_t kNoSocket = kNoAcceptedSocket;

		// Reads the handle. Every socket operation starts here, because the handle
		// can be taken away by another thread at any moment - a server's Close()
		// racing an Accept() blocked waiting for a connection is the normal way a
		// test shuts down, not an exotic one.
		std::uintptr_t CurrentSocket() const noexcept;


		// Creates the socket and stores it, or records the fault and leaves the
		// listener without one.
		//
		// Separate from the constructor because Listen needs it too: a socket that has
		// already been BOUND cannot be bound again, so re-listening on a different
		// address means a different socket. Doing that here is what keeps Listen from
		// having to destroy a working listener just to change its address.
		Status CreateSocket() noexcept;
		// True when the handle is not the sentinel, and so is usable.
		bool HasSocket() const noexcept;

		TcpListenerOptions m_options;

		// True when this object holds a Winsock reference; false for a listener whose
		// socket could not be created, so its destructor releases nothing.
		//
		// Plain bool, not atomic: written only during construction, destruction and a
		// move, none of which can be concurrent with a use of the object.
		bool m_holdsRuntime = false;

		// The handle, guarded by m_mutex. The lock protects the handle and the
		// recorded address, and is deliberately NOT held across the blocking accept:
		// holding it would make Close() from another thread deadlock against an
		// Accept() waiting for a connection, which is precisely the shutdown case
		// that has to work. A blocking call copies the handle out under the lock and
		// then works alone; the race that leaves is benign, because a close
		// underneath an accept makes the accept fail, which is the truthful outcome.
		mutable std::mutex    m_mutex;
		std::atomic<std::uintptr_t> m_socket{kNoSocket};

		// True between a successful listen() and Close().
		//
		// Atomic rather than a plain bool because a test or a shutdown path reads
		// IsListening() from a different thread than the one that called Listen or
		// Close, and a torn read would report a state that never existed. This is the
		// ONLY piece of listener state a second thread needs, which is why Close() does
		// not take the lock to read it.
		std::atomic<bool> m_listening{false};

		std::atomic<int> m_fault{static_cast<int>(TransportFault::None)};
		std::atomic<int> m_nativeError{0};

		// The address actually bound, written by Listen under the lock and read by
		// BoundEndpoint under the same lock.
		Endpoint m_bound;
	};
}

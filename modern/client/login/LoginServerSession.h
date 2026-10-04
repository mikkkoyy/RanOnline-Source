#pragma once

// LOGIN-002: the client half of the Login Server exchange, over a real socket.
//
// WHAT IT IS RELATIVE TO LoginServerClient.
//
// LoginServerClient (LOGIN-001) owns the PROTOCOL: the request bytes, the parse of
// the reply, the resulting grid. It deliberately owns no socket - "like its
// sibling, this is protocol-focused. It takes bytes and returns bytes; it does not
// own a socket." That separation is what let the protocol be tested exhaustively
// against a loopback pair, and it is not being undone here.
//
// This type is the missing half: it owns a TcpTransport, moves bytes between the
// socket and LoginServerClient::Feed, and enforces a deadline on the whole
// exchange. So the pair reads:
//
//     LoginServerSession  -> owns the socket, decides when to read
//     LoginServerClient   -> owns the protocol, never sees a socket
//
// and the layering is the same one legacy had, where CNetClient owned the socket
// and MsgGameSvrInfo owned the message.
//
// ---------------------------------------------------------------------------
// WHY THE DEADLINE LIVES HERE AND NOT IN THE CALLER
// ---------------------------------------------------------------------------
//
// A Login Server that sends nothing leaves a naive read loop spinning forever, and
// a Login Server that dies mid-response leaves it spinning too. Both are the same
// bug from the client's point of view: there is no end to the conversation, and
// the only thing that distinguishes "not yet" from "never" is a clock.
//
// So Pump returns an OUTCOME rather than just a Status. A Status cannot express
// this: it has no member for "the peer said nothing in time" as distinct from
// "the peer sent something wrong", and overloading InvalidState to mean both would
// make the caller's error handling lie. Completion, an orderly close, a timeout
// and a fault are four different things to a caller, and they get four different
// values.
//
// Chunking is exposed too, and only for testing the framer. See Pump's contract:
// a real caller passes 0 and the transport decides. A test that wants to prove
// ConnectionFramer survives a 1-byte read has to be able to ask for a 1-byte read,
// and hard-coding that capability into the client would be worse than exposing it.
//
// LEGACY'S CONNECTION LIFETIME, AND WHY THIS IS NOT IT.
//
// Legacy reuses ONE CNetClient socket for the Login phase and then the Agent
// phase: ConnectLoginServer closes any existing connection and opens a Login one
// (s_NetClient.cpp:367-372), and ConnectGameServer closes THAT and re-points the
// same object at the chosen game server with NET_STATE_AGENT
// (s_NetClient.cpp:379-388). One connection slot, two sequential roles.
//
// The modern tree keeps them as separate objects - LoginServerClient here, and
// LoginResponseClient (WORLD-002) for the Agent phase - and this session owns a
// socket that belongs to the Login conversation alone. That is a deliberate
// difference, not an oversight: it keeps the two protocols from being able to
// corrupt each other's framing, which is the class of bug that is hardest to
// diagnose. Forcing physical socket reuse now would be premature - the milestone
// that needs it is the one that adds the Agent phase, and by then both objects
// exist and the trade-off can be made against something concrete.

#include "LoginServerClient.h"
#include "NetworkTransport.h"
#include "TcpTransport.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace Modern::Client
{
	// How an exchange ended.
	enum class LoginServerSessionOutcome : std::uint8_t
	{
		// The terminator arrived and the list is complete.
		Completed = 0,

		// The server closed the connection before the terminator.
		//
		// Its own value rather than an error code, because a server that hung up
		// mid-response is an ordinary thing that happens on a network, and a caller
		// will usually want to retry rather than report a fault.
		PeerClosed,

		// Nothing arrived within the budget.
		TimedOut,

		// A read or write failed, or the response was not something the protocol can
		// accept.
		Faulted,
	};

	const char* ToString(LoginServerSessionOutcome outcome) noexcept;

	// The result of one exchange. `status` is Ok for Completed, PeerClosed and
	// TimedOut - none of which is a fault - and carries the reason for Faulted.
	struct LoginServerExchange
	{
		LoginServerSessionOutcome outcome = LoginServerSessionOutcome::Faulted;
		Status                    status;
		std::size_t               messagesHandled = 0;
	};

	// Drives one client conversation with a Login Server over TCP.
	//
	// Not reusable across conversations: one session is one connection, and Connect
	// requires a disconnected session. A caller wanting a second exchange builds a
	// second session, which is what keeps "whose response is this" unambiguous.
	class LoginServerSession
	{
	public:
		// The protocol object is borrowed, not owned. The caller usually owns it,
		// because it wants to keep the resulting grid after the session ends - and
		// because LoginServerClient is where the grid already lives.
		explicit LoginServerSession(LoginServerClient& protocol);

		// Closes the connection. Safe to destroy while connected.
		~LoginServerSession();

		LoginServerSession(const LoginServerSession&)            = delete;
		LoginServerSession& operator=(const LoginServerSession&) = delete;

		// Connects to a Login Server at `endpoint`.
		//
		// `endpoint.ip` must be a numeric dotted-quad. That is LoginServerClient's rule
		// from LOGIN-001, and it is a rule about RAN CONFIGURATION rather than about
		// TCP: legacy calls ::inet_addr directly (s_NetClient.cpp:474) with its
		// gethostbyname branch commented out (lines 445-465), so a name would not
		// resolve - it would silently become INADDR_NONE. TcpTransport itself accepts a
		// hostname; refusing it here keeps the RAN client's configuration contract
		// intact.
		//
		// `timeoutMilliseconds` bounds the connect; 0 uses the transport's own.
		Status Connect(const Network::EndpointAddress& endpoint, int timeoutMilliseconds = 0);

		// Builds and sends the REQ_GAME_SVR request.
		//
		// Separate from Pump because the two are separately observable: a client that
		// cannot send has a different problem from one that sent and is waiting, and a
		// caller driving its own loop needs to know which happened.
		Status RequestGameServers();

		// Reads and parses until the list completes, the peer closes, the budget runs
		// out, or something faults.
		//
		// `timeoutMilliseconds` bounds the WHOLE remaining exchange, not each read. A
		// per-read timeout would let a peer that dribbles one byte at a time hold the
		// session open indefinitely, which is the same denial of service in miniature
		// that the server side refuses to allow.
		//
		// `maxChunkBytes` caps how much a single Receive asks for. 0 means "whatever
		// the transport considers reasonable". It exists so a test can drive the
		// client with 1-byte and 2-byte reads and prove that ConnectionFramer - not
		// luck - is what reassembles the response; a real caller always passes 0.
		LoginServerExchange Pump(int timeoutMilliseconds, std::size_t maxChunkBytes = 0);

		// Connect, RequestGameServers, then Pump. The whole pre-login exchange.
		LoginServerExchange Exchange(const Network::EndpointAddress& endpoint,
		                             int timeoutMilliseconds,
		                             std::size_t maxChunkBytes = 0);

		// Closes the connection and returns the protocol object to Disconnected.
		//
		// Idempotent.
		void Disconnect() noexcept;

		bool IsConnected() const noexcept { return m_transport.IsConnected(); }

		// The peer, as a resolved numeric address. Numeric because that is what RAN's
		// wire can carry, even when a name was used to connect.
		Network::Endpoint RemoteEndpoint() const noexcept { return m_transport.RemoteEndpoint(); }

		// Why the most recent socket operation did not do what was asked.
		Network::TransportFault Fault() const noexcept { return m_transport.Fault(); }

	private:
		// The protocol half. Borrowed; see the constructor.
		LoginServerClient& m_protocol;

		// The socket half. Owned for the session's whole life, so a session that is
		// destroyed mid-exchange still closes cleanly.
		Network::TcpTransport m_transport;
	};
}
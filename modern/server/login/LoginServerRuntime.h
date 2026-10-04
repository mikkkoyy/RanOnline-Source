#pragma once

// LOGIN-002: the runnable Login Server.
//
// WHAT THIS IS, AND WHAT IT DELIBERATELY IS NOT.
//
// It is the part of legacy's CLoginServer that actually carries bytes: bind,
// listen, accept, frame, answer REQ_GAME_SVR, close. It is NOT a reproduction of
// CServer, and the difference is the whole design rather than a simplification of
// one. Legacy runs an IOCP accept thread (CServerListenProc, s_CServer.h:226),
// S_HEURISTIC_NUM worker threads per CPU capped at MAX_WORKER_THREAD=6
// (s_CServer.cpp:306-319), and an update thread (s_CServer.h:238). None of that is
// reproduced, because:
//
//   * none of it is needed to serve one request, and
//   * all of it would make an automated test non-deterministic, which is the one
//     property a test suite cannot trade away.
//
// So: no threads, no IOCP, no connection table, no batching. Accept returns one
// connection and this type serves it. A caller that later needs several clients at
// once should spawn the threads where the lifetime is visible, rather than finding
// a hidden pool inside a server class.
//
// ---------------------------------------------------------------------------
// ONE MESSAGE PER CONNECTION, AND WHY THAT IS A DEVIATION WORTH NAMING
// ---------------------------------------------------------------------------
//
// Legacy KEEPS THE CONNECTION OPEN after SND_GAME_SVR_END. MsgSndGameSvrInfo
// (s_CLoginServerMsg.cpp:95-147) contains no CloseClient and no closesocket; it
// ends at LockOff(). The socket is then held open, with a NET_HEARTBEAT_CLIENT_REQ
// every ~2 minutes (s_CLoginServer.cpp:569-577, HEARTBEAT_TIME in s_CServer.h:53)
// as the only liveness mechanism, until the peer goes away or the client moves on -
// at which point ConnectGameServer closes it and re-points the same CNetClient at
// the chosen Agent (s_NetClient.cpp:379-388).
//
// This runtime closes after the terminator instead. Three reasons, in order of how
// much they matter:
//
//   1. This milestone's client has nothing to do after END. It has no Agent phase,
//      no heartbeat responder and no server-selection step yet, so leaving a socket
//      open would leave it waiting on a conversation that cannot happen - and the
//      integration test would then have to assert on a timeout rather than on a
//      result.
//
//   2. Closing makes the end of the exchange observable. Without it, "the list
//      arrived" and "the server is still thinking" are indistinguishable, and a test
//      would have to poll a flag and hope.
//
//   3. It matches what legacy effectively does a moment later, because the client's
//      next move is to drop this connection anyway.
//
// Restoring the long-lived behaviour is a later milestone's decision, and the right
// place to record it is here rather than in a test - the change is a policy change,
// not a bug fix.
//
// ---------------------------------------------------------------------------
// ONE SEND FOR THE WHOLE RESPONSE
// ---------------------------------------------------------------------------
//
// Legacy issues one WSASend per message: CLoginServer::SendClient calls SendClient2
// directly (s_CLoginServer.cpp:827), which copies dwSize bytes into a pooled IO
// buffer (s_CClientManager.cpp:489-527), so each SND_GAME_SVR is its own send call.
//
// This runtime hands LoginServerResponder's whole buffer to the transport in one
// Send. The BYTES are identical, and that is the only thing that matters: TCP has
// no message boundaries, so a reader cannot tell one 64-byte write from four, and
// ConnectionFramer on the client exists precisely so nobody has to try. Splitting
// the send to imitate the syscall pattern would mean re-deriving message
// boundaries on the send side - a second framing parser, which is the specific
// duplication this project keeps refusing.
//
// The send loop inside TcpTransport still applies, so a response larger than the
// socket buffer is delivered whole or reported as a desynchronised stream.

#include "LoginServerConfig.h"
#include "LoginServerResponder.h"
#include "NetworkConnection.h"
#include "NetworkTransport.h"
#include "TcpListener.h"
#include "TcpTransport.h"
#include "types/Result.h"

#include <cstddef>
#include <functional>
#include <string>

namespace Modern::Server
{
	// What the server is doing, for logging and for tests.
	//
	// An enum rather than free-form strings because a test needs to assert that a
	// particular thing happened, and asserting on log text is asserting on wording.
	enum class LoginServerEvent : std::uint8_t
	{
		// "Listening on <host>:<port>" - emitted by Start, once the OS has assigned
		// the port if 0 was requested.
		Listening,

		// A connection was accepted.
		ClientConnected,

		// A well-formed REQ_GAME_SVR arrived.
		RequestReceived,

		// The response, entries and terminator together, has been handed to the
		// transport. Carries the entry count.
		ListSent,

		// A connection ended without a usable request: the peer closed first, the
		// framing failed, or the message was refused. Carries a reason.
		ClientRejected,

		// A connection was closed after the exchange completed.
		ClientDisconnected,
	};

	const char* ToString(LoginServerEvent event) noexcept;

	// One thing that happened, ready to be printed.
	//
	// A struct rather than a formatted string because the caller decides how to
	// render it - the executable prints a line, a test inspects the count - and a
	// server that formatted its own log lines would be unusable for the second.
	struct LoginServerLogEntry
	{
		LoginServerEvent event = LoginServerEvent::Listening;
		std::size_t      count = 0;   // entries sent, for ListSent
		std::string      text;        // peer endpoint, or the refusal reason
	};

	// Called for every event. Must not throw, and must not block: it runs inside
	// the connection's own handling, so a slow sink is a slow server.
	using LoginServerLogSink = std::function<void(const LoginServerLogEntry&)>;

	// Why a client was refused. Reported rather than logged-and-forgotten, so a test
	// can tell "the server rejected a malformed packet" from "the server crashed".
	enum class LoginServerRefusal : std::uint8_t
	{
		None = 0,

		// The connection carried something that is not a REQ_GAME_SVR.
		UnexpectedMessage,

		// A REQ_GAME_SVR whose length is not exactly 8 bytes.
		BadRequestSize,

		// The header could not be valid, or the client never finished one.
		MalformedFrame,

		// The peer closed before sending a complete, acceptable request.
		PeerClosedFirst,

		// A write failed, so the response could not be delivered.
		SendFailed,

		// A read failed for a reason that is not an orderly close.
		ReceiveFailed,
	};

	const char* ToString(LoginServerRefusal refusal) noexcept;

	// The Login Server, over a real socket.
	class LoginServerRuntime
	{
	public:
		// The configuration is validated in the constructor's caller - Start does it -
		// so a runtime can be constructed and inspected without opening a socket.
		explicit LoginServerRuntime(LoginServerConfig config, LoginServerLogSink log = {});

		// Stops the listener. Idempotent.
		~LoginServerRuntime();

		LoginServerRuntime(const LoginServerRuntime&)            = delete;
		LoginServerRuntime& operator=(const LoginServerRuntime&) = delete;

		// Validates the configuration, binds and listens.
		//
		// The port 0 case is the reason BoundEndpoint exists: after this returns Ok,
		// read it to learn where the server actually is. A caller that requested 0 and
		// then reports the 0 it asked for has built something nothing can connect to.
		Status Start();

		// Stops listening. Already-accepted connections are unaffected - there are none,
		// because serving is synchronous, so this is really only reachable between
		// clients. Idempotent.
		void Stop() noexcept;

		bool IsRunning() const noexcept;

		// The address actually bound, including the OS-assigned port when 0 was
		// requested. Empty until Start succeeds.
		Network::Endpoint BoundEndpoint() const noexcept;

		// Accepts one connection, serves it to completion, and closes it.
		//
		// `timeoutMilliseconds` bounds the ACCEPT only. Once a connection exists, the
		// exchange itself is bounded too - by kExchangeTimeout - so this cannot hang
		// on a client that connects and then says nothing. That is the whole reason
		// the bound is here rather than left to the socket: a server that can be held
		// open by one silent client is a server that can be denied service by anyone.
		//
		// Returns Ok with ServedClientCount() unchanged when the accept times out with
		// nobody knocking. "No client yet" is not a failure, and a caller running an
		// accept loop must not have to invent an error for every idle pass.
		//
		// A client that misbehaves is counted and refused, and ServeOneClient still
		// returns Ok: the server did its job, which was to reject the packet and stay
		// up. RefusalKind() says what happened.
		Status ServeOneClient(int timeoutMilliseconds);

		// Serves up to `count` connections in sequence, giving up on the accept after
		// `timeoutMilliseconds` of nobody knocking.
		//
		// Sequential on purpose. Proving that one connection does not corrupt another
		// does not require concurrency, and a sequential test cannot be flaky for
		// scheduling reasons. A caller wanting simultaneous clients should call this
		// from several threads.
		Status ServeClients(int count, int timeoutMilliseconds);

		// Connections served to completion, and connections refused. Both monotonic.
		std::size_t ServedClientCount() const noexcept { return m_served; }
		std::size_t RefusedClientCount() const noexcept { return m_refused; }

		// Why the most recently refused client was refused. None when the last client
		// was served, or when nothing has happened yet.
		LoginServerRefusal RefusalKind() const noexcept { return m_refusal; }

		// A human-readable detail for the last refusal, for logs.
		const std::string& RefusalDetail() const noexcept { return m_refusalDetail; }

		// The configuration as given. The grid is NOT reflected back after serving -
		// this server never mutates the list it advertises, which is a Session Server's
		// job in legacy (m_sGame is refreshed from it) and is not implemented here.
		const LoginServerConfig& Config() const noexcept { return m_config; }

	private:
		// Drives one accepted connection. `connection` is already adopted and owned.
		LoginServerRefusal ServeConnection(Network::TcpTransport& connection);

		// Reads until a whole message is available, then hands it to `out`.
		//
		// `framer` is supplied by the caller rather than owned here, because it belongs
		// to a connection and must not outlive one.
		//
		// Returns false when the connection ended, failed, or ran out of its exchange
		// budget first, having recorded which in RefusalKind()/RefusalDetail().
		bool ReadMessage(Network::TcpTransport&     connection,
		                 Network::ConnectionFramer& framer,
		                 Network::Message&          out);

		void Emit(LoginServerEvent event, std::string text, std::size_t count = 0);

		LoginServerConfig     m_config;
		LoginServerLogSink    m_log;
		Network::TcpListener  m_listener;

		std::size_t m_served = 0;
		std::size_t m_refused = 0;

		LoginServerRefusal m_refusal = LoginServerRefusal::None;
		std::string        m_refusalDetail;
	};
}

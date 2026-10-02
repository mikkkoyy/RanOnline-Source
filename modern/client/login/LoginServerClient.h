#pragma once

// LOGIN-001: the client half of the Login Server game-server-list exchange.
//
// The pre-login counterpart of Modern::Client::LoginResponseClient, and a
// deliberate SEPARATE type from it. Legacy keeps the two conversations apart by
// network state: CNetClient::MessageProcess routes NET_STATE_LOGIN to
// MessageProcessLogin and NET_STATE_AGENT to MessageProcessGame
// (s_NetClientMsg.cpp:19-44), and the two handlers share no message id. Merging
// them into one state machine would be a modern convenience that contradicts the
// wire, so LoginServerPhase below does not share a type, an enum or a counter with
// LoginPhase.
//
//   LoginServerPhase  covers  Disconnected -> ... -> GameServersReady
//   LoginPhase        covers  Disconnected -> ... -> LoginAccepted/LoginRejected
//
// One nuance the source settles, and which the type reflects: legacy does NOT hold
// two sockets open. ConnectLoginServer closes any existing connection and opens a
// Login one (s_NetClient.cpp:367-372); later ConnectGameServer closes THAT and
// re-points the same CNetClient at the chosen game server with NET_STATE_AGENT
// (s_NetClient.cpp:379-388). It is one connection slot playing two sequential
// roles. This class models the Login role; selecting a server and switching to the
// Agent role is the caller's next step and is not modelled here.
//
// Transport note: like its sibling, this is protocol-focused. It takes bytes and
// returns bytes; it does not own a socket. Reusing Network::ConnectionFramer means
// arbitrary TCP fragmentation is already handled, and this layer must not add a
// second framing parser.
//
// The stream is RAW. The Login Server sends through CClientManager::SendClient2,
// which never batches and never emits a NET_COMPRESS envelope
// (s_CLoginServerMsg.cpp:130 -> s_CLoginServer.cpp:827 -> s_CClientManager.cpp:489).
// So there is no envelope layer here to unwrap, and adding one would break the wire.

#include "GameServerListProtocol.h"
#include "NetworkConnection.h"
#include "NetworkCodec.h"
#include "NetworkTypes.h"

#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Modern::Client
{
	// The Login Server conversation only.
	//
	// These states are the ones legacy's behaviour actually distinguishes. In
	// particular there is no "picked a server" state here: legacy's selection step
	// (SelectServerPage) calls ConnectGameServer, which leaves NET_STATE_LOGIN
	// entirely, and that transition belongs to the Agent phase.
	enum class LoginServerPhase : std::uint8_t
	{
		Disconnected = 0,
		ConnectingLoginServer,
		LoginServerConnected,
		RequestingGameServers,
		ReceivingGameServers,
		GameServersReady,
	};

	const char* ToString(LoginServerPhase phase) noexcept;

	// Drives the client side of the pre-login exchange.
	//
	// Not a socket owner and not a UI model: it owns the request bytes, the parse of
	// the reply, and the resulting grid.
	class LoginServerClient
	{
	public:
		// ---- connection ---------------------------------------------------

		// Begins connecting. Validates the endpoint first.
		//
		// The address must be a numeric dotted-quad: legacy calls ::inet_addr
		// directly (s_NetClient.cpp:474) with its gethostbyname branch commented out
		// (lines 436-465), so a hostname would not resolve - it would silently
		// become INADDR_NONE. Refusing it here turns a silent misconnection into a
		// reported one.
		Status BeginConnect(const Network::EndpointAddress& endpoint);

		// Completes the connect once the transport reports success.
		//
		// Split from BeginConnect because the socket outcome is the transport's to
		// report, and collapsing the two would let this class imply a connection it
		// never made.
		Status CompleteConnect();

		// Leaves the Login phase and returns to Disconnected.
		//
		// Legacy's CloseConnect tears the socket down; the modern caller owns the
		// transport, so this resets protocol state and leaves the closing to them.
		Status Disconnect();

		const Network::EndpointAddress& Endpoint() const noexcept { return m_endpoint; }
		LoginServerPhase                   Phase() const noexcept { return m_phase; }

		// ---- request ------------------------------------------------------

		// Builds REQ_GAME_SVR - eight bytes, no body - into `request`.
		//
		// Moves the phase to RequestingGameServers, matching SndReqServerInfo, which
		// both sends the request and resets the client's grid and its END flag
		// (s_NetClientMsg.cpp:316-326). The reset happens HERE for the same reason:
		// a second request must not be answered by the previous list.
		Status RequestGameServers(std::vector<Network::WireU8>& request);

		// ---- response -----------------------------------------------------

		// Feeds bytes received from the transport.
		//
		// Returns the number of messages consumed, or an error. Entries are decoded
		// and stored as they arrive; the terminator completes the list.
		//
		// An entry the client cannot place - group or number outside the grid - is
		// COUNTED AND SKIPPED, exactly as MsgGameSvrInfo does
		// (s_NetClientMsg.cpp:199-203). It is not an error, and it does not abort the
		// list, because legacy deliberately continues.
		Status Feed(const Network::WireU8* data, std::size_t size, std::size_t& messagesHandled);

		Status Feed(const std::vector<Network::WireU8>& data, std::size_t& messagesHandled)
		{
			return Feed(data.data(), data.size(), messagesHandled);
		}

		// ---- results ------------------------------------------------------

		// The received list, in canonical (group, channel) order.
		const Network::GameServerGrid& Servers() const noexcept { return m_servers; }

		// Entries that arrived but could not be placed in the grid.
		std::size_t DroppedEntryCount() const noexcept { return m_dropped; }

		// True once the terminator has been seen. GameServersReady implies it.
		bool IsComplete() const noexcept
		{
			return m_phase == LoginServerPhase::GameServersReady;
		}

		// True once the framer has latched an unrecoverable framing error. A
		// desynchronised stream cannot resync, so the caller must drop the
		// connection rather than keep parsing.
		bool IsFailed() const noexcept { return m_framer.IsFailed(); }

		void Reset() noexcept;

	private:
		// Pops every complete message currently buffered and applies it.
		//
		// Separate from Feed so the sliced feeding loop can drain between slices;
		// `messagesHandled` accumulates across calls.
		Status DrainMessages(std::size_t& messagesHandled);

		Network::ConnectionFramer    m_framer;
		Network::GameServerGrid      m_servers;
		Network::EndpointAddress     m_endpoint;
		LoginServerPhase             m_phase = LoginServerPhase::Disconnected;
		std::size_t                  m_dropped = 0;
	};
}

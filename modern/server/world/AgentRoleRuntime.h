#pragma once

// WORLD-ENTRY-001 Phase C: the Agent role, over a real socket.
//
// ---------------------------------------------------------------------------
// WHAT THIS CLASS IS, AND WHAT IT IS NOT
// ---------------------------------------------------------------------------
//
// It TRANSLATES packets into Phase B service calls. It is not the source of truth
// about anything:
//
//   - it does not decide whether a character exists        -> CharacterSelectService
//   - it does not decide whether it is the caller's         -> FindOwned, three layers
//   - it does not allocate a gaeaId                          -> FieldEntryRegistry
//   - it does not build a 2332 or a 2358                    -> the Phase A codecs
//   - it does not know what a session state means           -> AgentSession
//
// If this class ever answers "is this character allowed" by itself, the ownership
// guarantee has leaked out of the boundary it was built to live in. Every rule it
// enforces is enforced again, independently, by the service it calls.
//
// ---------------------------------------------------------------------------
// THE CLIENT-FACING HALF OF THE WORLD-ENTRY FLOW
// ---------------------------------------------------------------------------
//
//   2049 LOGIN_2     -> authenticate; LOGIN_FB (2050)   [LOGIN-001/WORLD-002 codecs]
//   2247             -> the character list;  2248
//   2244 (per id)    -> one character;         2332
//   2353             -> the selection;         2358, or 2335 if refused
//
// Every dispatch is on the message ID AND the session state. Dispatching on ID
// alone would let an unauthenticated client ask for a character list, which is
// exactly the character-existence oracle the Phase B ownership work exists to
// prevent - so the state check is not an optimisation here, it is the security
// property.
//
// Legacy dispatches on nType alone (s_CAgentServerMsg.cpp:100) and relies on
// CClientManager having populated the slot to make an out-of-order request
// meaningless. That is an emergent property of a shared mutable client manager; the
// modern AgentSession makes it explicit, which is the same choice Phase B made.
//
// ---------------------------------------------------------------------------
// EVERY RESPONSE GOES OUT IN A NET_COMPRESS ENVELOPE
// ---------------------------------------------------------------------------
//
// The investigation's compression table (§4.2) is unambiguous: Agent -> client is
// compressed, unconditionally, and client -> anything is raw. LOGIN-002's Login
// Server is the exception that proves the rule - it uses SendClient2 and never
// batches (s_CLoginServer.cpp:827) - so its messages are raw, and this role's are
// not.
//
// The wrapping is ServerBatchEncoder + Lzo1xCodec, both already in the tree and
// already proven by LOGIN-002's LoginResponder. Nothing about compression is
// reimplemented here, and no third envelope format exists.
//
// WHY THE ENVELOPE MATTERS TO THE CLIENT: 2358 carries the Field endpoint, and a
// client that cannot unwrap the envelope never sees it. Sending a real redirect
// inside a format the client does not understand would be the same as sending a
// placeholder.
//
// ---------------------------------------------------------------------------
// ONE CONVERSATION PER CONNECTION, ENDING AT THE 2358
// ---------------------------------------------------------------------------
//
// Legacy keeps the Agent socket open after the redirect, with a NET_HEARTBEAT every
// ~2 minutes as its only liveness mechanism (s_CAgentServer.cpp:569-577,
// HEARTBEAT_TIME in s_CServer.h:53) - the same choice LOGIN-002 documented and
// deliberately did not reproduce.
//
// This role closes after the 2358, for the same reason LOGIN-002 did: this
// milestone's client has no Agent-phase conversation left to have, and a socket held
// open against silence would make "the redirect arrived" indistinguishable from
// "the server is still thinking". Restoring the long-lived behaviour is a later
// milestone's decision, recorded here rather than in a test.

#include "CompressionCodec.h"
#include "MessageReader.h"
#include "MinTeaCodec.h"
#include "NetworkConnection.h"
#include "NetworkTypes.h"
#include "ServerBatchEncoder.h"
#include "TcpListener.h"
#include "TcpTransport.h"
#include "login/LoginReceiver.h"
#include "login/LoginResponder.h"
#include "types/Result.h"
#include "world/AgentSession.h"
#include "world/CharacterRepository.h"
#include "world/CharacterSelectService.h"
#include "world/WorldEntryService.h"
#include "world/WorldServerConfig.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace Modern::Server::World
{
	// What the Agent role is doing, for logs and for tests.
	//
	// An enum rather than free-form strings because a test must be able to assert
	// that a particular thing HAPPENED. Asserting on log text is asserting on
	// wording, which changes whenever someone rewords a comment.
	enum class AgentEvent : std::uint8_t
	{
		Listening,
		ClientConnected,

		// 2049 accepted and 2050 sent.
		LoginAccepted,

		// 2248 sent. Carries nChaSNum.
		ListSent,

		// 2332 sent.
		DetailSent,

		// 2358 sent - the conversation's last message. Carries the gaeaId.
		RedirectSent,

		ClientRejected,
		ClientDisconnected,
	};

	const char* ToString(AgentEvent event) noexcept;

	struct AgentLogEntry
	{
		AgentEvent  event = AgentEvent::Listening;
		std::size_t count = 0;
		std::string text;
	};

	using AgentLogSink = std::function<void(const AgentLogEntry&)>;

	// Why a client was refused.
	//
	// Split finely enough to be useful and coarsely enough to stay honest: the two
	// the brief names - wrong state and wrong ownership - are ONE value
	// (ServiceRefused) because at this layer they are the same event, a service
	// saying no. The service's own error code is in the log text, so nothing is lost,
	// and inventing an "OwnershipRefused" here would imply this class can tell them
	// apart - which would be exactly the leak the ownership boundary prevents.
	enum class AgentRefusal : std::uint8_t
	{
		None = 0,

		// Not a message this Agent conversation accepts.
		UnexpectedMessage,

		// A recognised id whose length is wrong for that message.
		BadMessageSize,

		// The frame could not be valid, or never completed.
		MalformedFrame,

		// The peer closed before the conversation finished.
		PeerClosedFirst,

		// A send failed.
		SendFailed,

		// A read failed for a reason that is not an orderly close.
		ReceiveFailed,

		// A Phase B service refused: wrong session state, unknown character, or a
		// character this account does not own. Carries the service's code in the text.
		ServiceRefused,
	};

	const char* ToString(AgentRefusal refusal) noexcept;

	// The Agent role, over a real socket.
	//
	// Synchronous by construction, exactly like LOGIN-002's LoginServerRuntime: it
	// serves one connection per ServeOneConnection call and owns no thread. A
	// caller wanting several clients at once should spawn the thread where the
	// lifetime is visible - which is what the integration test does, standing in for
	// the process boundary a real deployment has.
	class AgentRoleRuntime
	{
	public:
		// `authenticator` decides whether a credential is correct;
		// `config.accounts` maps the accepted login name to the world account that owns
		// its characters. Both are borrowed and must outlive this runtime.
		//
		// `repository` and `registry` are the Phase B state this role drives. Neither
		// is owned: a World Server owns one repository shared by both roles, and
		// sharing it is the point - the Field role must be reading the same characters
		// the Agent role authorized.
		AgentRoleRuntime(WorldServerConfig config,
		                 ILoginAuthenticator& authenticator,
		                 ICharacterRepository& repository,
		                 FieldEntryRegistry& registry,
		                 AgentLogSink log = {});

		~AgentRoleRuntime();

		AgentRoleRuntime(const AgentRoleRuntime&)            = delete;
		AgentRoleRuntime& operator=(const AgentRoleRuntime&) = delete;

		// Validates the configuration, binds and listens.
		//
		// After Ok, AgentEndpoint() reports the address actually bound - which is not
		// the one asked for when port 0 was requested, and is the only one a client can
		// connect to.
		Status Start();

		// Points the 2358 redirect at `endpoint`.
		//
		// Called by WorldServerRuntime AFTER both listeners are bound, with the Field
		// role's ACTUAL address. It exists as a setter rather than being read from the
		// configuration because the port is OS-assigned when 0 was requested: the
		// configuration cannot contain it before the bind happens.
		//
		// Until this is called, AuthorizeWorldEntry refuses - Phase B refuses an unset
		// endpoint rather than defaulting one, because a default here becomes an
		// address a client will actually dial.
		void SetAdvertisedFieldEndpoint(const FieldEndpoint& endpoint);

		void Stop() noexcept;
		bool IsRunning() const noexcept;

		Network::Endpoint AgentEndpoint() const noexcept { return m_listener.BoundEndpoint(); }

		// Accepts one connection, serves the conversation to its end, and closes it.
		//
		// `timeoutMilliseconds` bounds the ACCEPT only; once a connection exists the
		// conversation is bounded per message, so this cannot hang on a client that
		// connects and then says nothing.
		//
		// Returns Ok with ServedClientCount() unchanged when the accept times out with
		// nobody knocking - "no client yet" is not a failure, and an accept loop must
		// not have to invent an error for every idle pass.
		//
		// A client that misbehaves is counted and refused and this still returns Ok:
		// the server's job on receiving a malformed packet is to reject it and stay up.
		Status ServeOneConnection(int timeoutMilliseconds);

		std::size_t ServedClientCount() const noexcept { return m_served; }
		std::size_t RefusedClientCount() const noexcept { return m_refused; }

		AgentRefusal    RefusalKind() const noexcept { return m_refusal; }
		const std::string& RefusalDetail() const noexcept { return m_refusalDetail; }

		// The account id the most recently authenticated session received, and the
		// gaeaId the most recent 2358 carried.
		//
		// Exposed because the integration test needs to prove the repository's
		// ownership data and the wire agree, and because an operator log wants them.
		Network::WireU32 LastAuthenticatedAccountId() const noexcept { return m_lastAccountId; }
		Network::WireU32 LastRedirectGaeaId() const noexcept { return m_lastGaeaId; }

		// How many characters the most recent 2248 announced.
		std::size_t LastListCharacterCount() const noexcept { return m_lastListCount; }

	private:
		// Drives one accepted connection. `connection` is already adopted and owned.
		AgentRefusal ServeConnection(Network::TcpTransport& connection);

		// Wraps one inner message in a NET_COMPRESS envelope and sends it.
		//
		// Takes the connection's ServerBatchEncoder by reference because a batcher is
		// PER-CONNECTION state - legacy's CSendMsgBuffer is a member of each CNetUser
		// (s_CClientManager.cpp) for exactly that reason.
		Status SendEnveloped(Network::ServerBatchEncoder& batcher,
		                     Network::TcpTransport&    connection,
		                     const std::vector<Network::WireU8>& inner);

		// Handles one message. Returns the refusal that ends the conversation, or None
		// to keep going. `conversationComplete` is set when the message was the last
		// one this role sends.
		AgentRefusal Dispatch(Network::ServerBatchEncoder& batcher,
		                      Network::TcpTransport&    connection,
		                      AgentSession&             session,
		                      LoginReceiver&            receiver,
		                      const Network::Message&   message,
		                      bool&                     conversationComplete);

		void Emit(AgentEvent event, std::string text, std::size_t count = 0);

		WorldServerConfig  m_config;
		ILoginAuthenticator& m_authenticator;
		ICharacterRepository& m_repository;
		FieldEntryRegistry& m_registry;

		CharacterSelectService m_select;
		WorldEntryService      m_entry;

		AgentLogSink        m_log;
		Network::TcpListener m_listener;

		// Per-CONNECTION state, created in ServeOneConnection and destroyed with it.
		//
		// Deliberately not members. A session or a batcher that outlived its connection
		// could answer for a client that had already gone, which is invisible in
		// testing and obvious in production.
		//
		// The codec and the cipher ARE members: LZO needs a work buffer and minTea needs
		// a key schedule, and neither carries per-conversation state, so allocating one
		// per accepted socket would be cost without isolation. Everything that DOES
		// carry per-conversation state - AgentSession, ServerBatchEncoder, LoginReceiver
		// - is local to ServeOneConnection.
		Network::MinLzo1xCodec m_codec;
		Network::MinTea       m_tea;

		std::size_t m_served  = 0;
		std::size_t m_refused = 0;

		AgentRefusal m_refusal = AgentRefusal::None;
		std::string  m_refusalDetail;

		Network::WireU32 m_lastAccountId = 0;
		Network::WireU32 m_lastGaeaId    = 0;
		std::size_t      m_lastListCount = 0;
	};
}
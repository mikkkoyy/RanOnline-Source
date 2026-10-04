#pragma once

// WORLD-ENTRY-001 Phase C: the Field role, over a real socket.
//
// ---------------------------------------------------------------------------
// THE SECOND LISTENER EXISTS TO PROVE A TOPOLOGY
// ---------------------------------------------------------------------------
//
// Legacy's client opens a genuinely new connection for world entry. After the Agent
// sends 2358 with the Field's address, CNetClient::ConnectField establishes a second
// socket (s_NetClient.cpp, the NET_STATE_FIELD branch) and the Field identity 2359
// travels on THAT socket - never on the Agent's.
//
// If this role were a second logical session on the Agent's socket, the brief's
// central claim would be untested: a client that "cannot be faked with one socket"
// would be exactly that. So this is a separate TcpListener on its own port, and
// the integration test asserts the two connections are genuinely distinct sockets.
//
// ---------------------------------------------------------------------------
// EACH CONNECTION GETS ITS OWN FieldSession
// ---------------------------------------------------------------------------
//
// Built inside ServeOneConnection and destroyed with it. Two clients must not share
// world-entry state, and the cheapest way to guarantee that is for there to be no
// object for them to share.
//
// The FieldEntryRegistry IS shared, and that is correct rather than a leak: it holds
// the Agent's authorizations, which are process-wide facts about which characters
// the Agent has admitted. It holds no per-connection state - a consumed entry stays
// consumed, which is the whole replay defence - and it is guarded by the fact that
// this runtime serves one connection at a time.
//
// ---------------------------------------------------------------------------
// 2359 IS NOT TRUSTED, AND IT CARRIES NOTHING TO TRUST
// ---------------------------------------------------------------------------
//
// NET_GAME_JOIN_FIELD_IDENTITY (s_NetGlobal.h:4301-4319) is:
//
//     { nmg; EMGAME_JOINTYPE emType; DWORD dwGaeaID;
//       DWORD dwSlotFieldAgent; CRYPT_KEY ck; }
//
// No account id. No character id. No name. The only thing a client can present is
// the (gaeaId, slot) pair the Agent put in the 2358 - and it can only have that pair
// because the Agent proved the client owns the character before minting it.
//
// So "not trusting the client" here means exactly one thing: validating that pair
// against the authorization the Agent created. Everything after that - which
// character, which level, which position - is re-read from the repository by
// Phase B's FieldEntryRegistry::Claim. No field of the spawn comes from the packet.
//
// CRYPT_KEY is carried because the packet has 4 bytes of it, and it is {1,1} and
// protects nothing (the cipher that would consume it is commented out at all four
// call sites). This role does not treat it as a credential.

#include "CompressionCodec.h"
#include "MessageReader.h"
#include "NetworkConnection.h"
#include "NetworkTypes.h"
#include "ServerBatchEncoder.h"
#include "TcpListener.h"
#include "TcpTransport.h"
#include "WorldEntryProtocol.h"
#include "types/Result.h"
#include "world/CharacterRepository.h"
#include "world/FieldSession.h"
#include "world/WorldEntryService.h"
#include "world/WorldServerConfig.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace Modern::Server::World
{
	enum class FieldEvent : std::uint8_t
	{
		Listening,
		ClientConnected,

		// 2359 accepted against an Agent authorization.
		IdentityAccepted,

		// 2333 sent - this role's whole purpose. Carries the gaeaId.
		SpawnSent,

		ClientRejected,
		ClientDisconnected,
	};

	const char* ToString(FieldEvent event) noexcept;

	struct FieldLogEntry
	{
		FieldEvent  event = FieldEvent::Listening;
		std::size_t count = 0;
		std::string text;
	};

	using FieldLogSink = std::function<void(const FieldLogEntry&)>;

	enum class FieldRefusal : std::uint8_t
	{
		None = 0,

		// Not a 2359.
		UnexpectedMessage,

		// A 2359 whose length is not 24 bytes.
		BadMessageSize,

		MalformedFrame,
		PeerClosedFirst,
		SendFailed,
		ReceiveFailed,

		// The 2359 named a pair the Agent never authorized, or one already spent, or
		// one the repository now contradicts. Carries Phase B's code in the text.
		//
		// ONE value for all of those, and the distinction is load-bearing rather than
		// lazy: a client that could tell "expired" from "never existed" from "someone
		// else's" would be probing the registry, and the registry is the thing standing
		// between a guess and a spawn.
		IdentityRejected,
	};

	const char* ToString(FieldRefusal refusal) noexcept;

	// The Field role, over a real socket.
	class FieldRoleRuntime
	{
	public:
		// The repository and registry are the shared Phase B state; see the header for
		// why sharing them is correct here while the FieldSession is not shared.
		FieldRoleRuntime(WorldServerConfig config,
		                 ICharacterRepository& repository,
		                 FieldEntryRegistry& registry,
		                 FieldLogSink log = {});

		~FieldRoleRuntime();

		FieldRoleRuntime(const FieldRoleRuntime&)            = delete;
		FieldRoleRuntime& operator=(const FieldRoleRuntime&) = delete;

		Status Start();
		void   Stop() noexcept;
		bool   IsRunning() const noexcept;

		// Named FieldBoundEndpoint() rather than FieldEndpoint(): a member function called
		// FieldEndpoint would shadow the FieldEndpoint TYPE that
		// AgentRoleRuntime::SetAdvertisedFieldEndpoint takes, and the two would stop
		// compiling together.
	Network::Endpoint FieldBoundEndpoint() const noexcept { return m_listener.BoundEndpoint(); }

		// Accepts one connection, validates its 2359, sends at most one 2333, closes.
		//
		// "At most one": a refused identity produces no spawn at all, and a second
		// 2359 on the same connection is refused - so a client cannot ask twice and get
		// two spawns, or ask with one identity after succeeding with another.
		Status ServeOneConnection(int timeoutMilliseconds);

		std::size_t ServedClientCount() const noexcept { return m_served; }
		std::size_t RefusedClientCount() const noexcept { return m_refused; }

		FieldRefusal       RefusalKind() const noexcept { return m_refusal; }
		const std::string& RefusalDetail() const noexcept { return m_refusalDetail; }

		Network::WireU32 LastSpawnGaeaId() const noexcept { return m_lastGaeaId; }
		std::size_t      LastSpawnCharacterId() const noexcept { return m_lastCharacterId; }

	private:
		FieldRefusal ServeConnection(Network::TcpTransport& connection);

		Status SendEnveloped(Network::ServerBatchEncoder& batcher,
		                     Network::TcpTransport&    connection,
		                     const std::vector<Network::WireU8>& inner);

		void Emit(FieldEvent event, std::string text, std::size_t count = 0);

		WorldServerConfig    m_config;
		ICharacterRepository& m_repository;
		FieldEntryRegistry&   m_registry;

		FieldLogSink m_log;
		Network::TcpListener m_listener;

		// Per-conversation cipher state is the batcher's, created in ServeOneConnection.
		// LZO's work buffer is not per-conversation, so the codec is a member.
		Network::MinLzo1xCodec m_codec;

		std::size_t m_served  = 0;
		std::size_t m_refused = 0;

		FieldRefusal m_refusal = FieldRefusal::None;
		std::string  m_refusalDetail;

		Network::WireU32 m_lastGaeaId      = 0;
		std::size_t      m_lastCharacterId = 0;
	};
}
#pragma once

// WORLD-ENTRY-002a: the Field role, over a real socket.
//
// The Field CONNECTION IS THE GAMEPLAY CONNECTION.
//
// WORLD-ENTRY-001's Field role read exactly one message per connection: the 2359,
// answered with a 2333, then closed. That was correct then, because the 2358 was the
// last thing the client needed from the Field. It is no longer correct, because 3032
// arrives on THAT SAME socket afterwards. Closing after the spawn would make the
// movement milestone unreachable on the transport it is supposed to use.
//
// So the connection is now long-lived: it carries the 2359, then the 2333, then any
// number of 3032s, until the client goes away.
//
// ---------------------------------------------------------------------------
// WHY THERE IS A THREAD PER CONNECTION NOW
// ---------------------------------------------------------------------------
//
// Previously one Field connection at a time was enough, because each ended
// immediately. Now a client holds its connection open, so a role that served them
// one at a time would never reach the second client: the first would still be
// connected and the second would sit in the backlog until it timed out.
//
// So this role owns an accept thread plus one worker thread per connection, and
// Stop() joins every one of them. That is the same shape a real server has, and it is
// the reason WORLD-ENTRY-001's single-threaded role did not need it.
//
// NOT a busy loop: a worker blocks in ReadOneMessage, which is bounded per message.
// NOT a hidden thread pool: the threads are counted, owned and joined here, so a test
// can assert that none survive Stop().
//
// ---------------------------------------------------------------------------
// BROADCAST SCOPE, AND THE HONEST LIMIT OF IT
// ---------------------------------------------------------------------------
//
// Legacy broadcasts through `SendMsgViewAround` (GLChar.h:629) - VIEW-RANGE scoped,
// over GLLandMan's character lists and sectors, and `NetMsgFB.dwGaeaID` names whose
// character it was.
//
// The sector system is not built, so this role broadcasts to EVERY currently
// authorized Field session and documents that as a STAGED approximation of view
// visibility, not a reproduction of it. With two clients on loopback that is exactly
// legacy's behaviour; with a populated world it would over-deliver.
//
// What IS exact: the payload. `dwGaeaID` is the AUTHORITATIVE character that changed,
// read off the server-side record, and `dwActState` is the full authoritative word -
// never the request. A receiver can therefore always tell whose movement this is.
//
// ---------------------------------------------------------------------------
// 2359 IS STILL NOT TRUSTED, AND 3032 CARRIES NO IDENTITY AT ALL
// ---------------------------------------------------------------------------
//
// NET_GAME_JOIN_FIELD_IDENTITY (s_NetGlobal.h:4301-4319) is
// { emType, dwGaeaID, dwSlotFieldAgent, CRYPT_KEY } and NET_GAME_JOIN_FIELD_
// IDENTITY is validated exactly as WORLD-ENTRY-001 established: Phase B's
// FieldEntryRegistry::Claim, unchanged.
//
// 3032 (GLMSG::SNETPC_MOVESTATE) is { dwActState } and NOTHING ELSE - 12 bytes, no
// id, no account, no name. So a client cannot name the character it is moving. The
// `dwGaeaID` in the 3033 it receives back comes from the connection's own authorized
// character, which is why the reply is not forgeable even though the request has no
// identity to forge.

#include "CompressionCodec.h"
#include "CompressionCodec.h"
#include "GotoProtocol.h"
#include "MessageReader.h"
#include "MessageReader.h"
#include "MovementStateProtocol.h"
#include "NetworkConnection.h"
#include "NetworkTypes.h"
#include "ServerBatchEncoder.h"
#include "TcpListener.h"
#include "TcpTransport.h"
#include "WorldEntryProtocol.h"
#include "types/Result.h"
#include "world/CharacterRepository.h"
#include "world/FieldSession.h"
#include "world/MovementStateService.h"
#include "world/WorldEntryService.h"
#include "world/WorldServerConfig.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace Modern::Server::World
{
	enum class FieldEvent : std::uint8_t
	{
		Listening,
		ClientConnected,

		// 2359 accepted against an Agent authorization.
		IdentityAccepted,

		// 2333 sent.
		SpawnSent,

		// WORLD-ENTRY-002a: a 3032 changed the authoritative state and 3033 went out.
		// Carries the gaeaId whose state changed.
		MoveStateSent,

		// A 3032 arrived that did NOT change the authoritative state, so nothing was
		// sent - legacy's `dwOldActState != m_dwActState` behaviour.
		MoveStateUnchanged,

		// WORLD-ENTRY-002f: a 3034 was accepted and 3035 went out. Carries the gaeaId
		// that is now walking.
		GotoAccepted,

		// WORLD-ENTRY-002f: a 3034 was refused and NOTHING was sent - which is exactly
		// what legacy does for an unreachable destination, a dead character and a
		// desynchronised client alike. The log line is the only difference from
		// silence, and it is the difference an operator has.
		GotoRejected,

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

		// Not a message this Field connection accepts.
		UnexpectedMessage,

		// A recognised id whose length is wrong for that message.
		BadMessageSize,

		MalformedFrame,
		PeerClosedFirst,
		SendFailed,
		ReceiveFailed,

		// The 2359 named a pair the Agent never authorized, or one already spent.
		IdentityRejected,

		// A 3032 arrived before the connection was spawned. Separate from
		// IdentityRejected because the cause is different and an operator reading a log
		// needs to tell "you are not logged in" from "that entry has expired".
		NotSpawned,

		// WORLD-ENTRY-002f: a 3034 arrived before the connection was spawned. Its own
		// value rather than a reuse of NotSpawned, because the count it feeds is a
		// MOVEMENT count, and an operator chasing "why will nobody move" should not
		// have to read two refusal kinds to find it.
		GotoBeforeSpawn,
	};

	const char* ToString(FieldRefusal refusal) noexcept;

	class FieldRoleRuntime
	{
	public:
		FieldRoleRuntime(WorldServerConfig config,
		                 ICharacterRepository& repository,
		                 FieldEntryRegistry& registry,
		                 const MovementStateService& movement,
		                 FieldLogSink log = {});

		~FieldRoleRuntime();

		FieldRoleRuntime(const FieldRoleRuntime&)            = delete;
		FieldRoleRuntime& operator=(const FieldRoleRuntime&) = delete;

		// Binds AND starts the accept thread.
		//
		// The Field role no longer has a "serve one connection" call: connections are
		// long-lived now, so serving one to completion from the caller's thread would
		// block every other client. See the header.
		Status Start();

		// Stops accepting, closes every live connection so blocked reads return, and
		// joins every thread. Idempotent, and leaves nothing running.
		void Stop() noexcept;

		bool IsRunning() const noexcept;

		Network::Endpoint FieldBoundEndpoint() const noexcept { return m_listener.BoundEndpoint(); }

		// Connections served to completion, and connections refused. Both monotonic.
		// Written by worker threads, so read them only after Stop().
		std::size_t ServedClientCount() const noexcept { return m_served.load(); }
		std::size_t RefusedClientCount() const noexcept { return m_refused.load(); }

		// The most recent refusal. Written by worker threads; read after Stop().
		FieldRefusal       RefusalKind() const noexcept { return m_refusal.load(); }
		const std::string& RefusalDetail() const noexcept { return m_refusalDetail; }

		Network::WireU32 LastSpawnGaeaId() const noexcept { return m_lastGaeaId.load(); }
		std::size_t      LastSpawnCharacterId() const noexcept { return m_lastCharacterId.load(); }

		// 3033s sent for state changes, and 3032s that changed nothing.
		std::size_t MoveStateSentCount() const noexcept { return m_moveSent.load(); }
		std::size_t MoveStateUnchangedCount() const noexcept { return m_moveUnchanged.load(); }

		// Currently authorized Field sessions. For tests and for the operator log.
// Currently authorized Field sessions. For tests and for the operator log.
		std::size_t AuthorizedSessionCount() const;

		// WORLD-ENTRY-002f: the movement world, so a test can observe where a
		// character actually IS.
		//
		// Exposed because RAN transmits no authoritative position after the initial
		// spawn - there is no per-tick position packet, which is a MEASURED fact and
		// not an omission (002c section 10). The only way to assert "movement happened
		// after the 3035" is therefore to ask the server, and asking the server must
		// not mean inventing a packet.
		const WorldMovementRuntime& Movement() const noexcept { return m_movementWorld; }

		// The same world, mutably, for a runtime that owns it. Only the owning
		// WorldServerRuntime and the tests take this; a reader wants the const form
		// above and has no business walking an actor.
		WorldMovementRuntime& MovementWorld() noexcept { return m_movementWorld; }

		//
		// A caller that injects elapsed time itself must NOT start the ticker: the two
		// would both advance every actor and each movement would happen twice. See
		// WorldMovementRuntime.h.
		Status StartMovementTicker();
		void   StopMovementTicker() noexcept;

		// WORLD-ENTRY-002f: where navigation meshes come from.
		//
		// BORROWED and optional. Installed before Start(); a Field role with no map
		// source spawns characters that cannot walk and refuses every 3034 with a
		// reason naming the map. That is the correct behaviour for a server started
		// without an asset root, and the reason it is not an error here.
		//
		// The production implementation is `Movement::MapRegistryMeshSource`, which
		// borrows a `Map::MapRegistry` that has already loaded.
		void ConfigureMovement(const Movement::INavigationMapSource* maps) noexcept
		{
			m_movementWorld.SetMapSource(maps);
		}

		// 3035s sent and 3034s refused. Monotonic; read after Stop().
		std::size_t GotoSentCount() const noexcept { return m_gotoSent.load(); }
		std::size_t GotoRefusedCount() const noexcept { return m_gotoRefused.load(); }

	private:
		// One live client connection.
		//
		// Owned by its worker thread for the connection's life, and held in the
		// registry by shared_ptr so a broadcast from ANOTHER thread can reach it safely.
		struct Peer
		{
			// Owned per connection, not shared with the role or with other peers.
			//
			// Forced by the Lzo1xCodec contract rather than chosen for tidiness: the
			// codec carries an LZO work buffer, so two threads compressing at the same
			// moment produce a corrupt envelope - which the peer would see as a framing
			// error on a stream that is perfectly valid, on a socket it did nothing
			// wrong on. It lives beside the transport it serves so its lifetime is
			// obviously the connection's.
			Network::MinLzo1xCodec codec;

			Network::TcpTransport transport;

			// Guards `transport` against concurrent sends.
			//
			// The worker thread sends its own replies while another worker's broadcast
			// may be writing to the same socket. TcpTransport is not documented as safe
			// for that, so the mutex is what makes a two-writer socket correct rather
			// than merely usually-fine.
			std::mutex sendMutex;

			// Constructed with the shared Phase B state rather than default-constructed:
			// FieldSession takes its repository, registry and session id, and two Field
			// sessions must never disagree about who owns which character.
			explicit Peer(ICharacterRepository& repository, FieldEntryRegistry& registry,
			            Network::WireU64 sessionId)
				: session(repository, registry, sessionId)
			{
			}

			FieldSession session;

			// Set once the 2359 has been accepted, so a broadcast can skip a
			// connection that has not earned a spawn yet.
			std::atomic<bool> spawned{false};
		};

		using PeerPtr = std::shared_ptr<Peer>;

		// The accept loop. Runs on its own thread from Start() until m_stopAccept.
		void AcceptLoop();

		// Serves one accepted connection to its end, on its own thread.
		void ServePeer(PeerPtr peer);

		// Reads one message from `peer`, filling `reader` as a side effect.
		Network::ReadResult ReadMessage(PeerPtr peer, Network::ConnectionFramer& reader,
		                       Network::Message& message, int budgetMilliseconds);

		// The 2359 path: validate and spawn. Returns whether the connection may continue.
		bool HandleIdentity(PeerPtr peer, Network::ServerBatchEncoder& batcher,
		                    const Network::Message& message);

// The 3032 path. Returns whether the connection may continue.
		bool HandleMoveState(PeerPtr peer, Network::ServerBatchEncoder& batcher,
		                     const Network::Message& message);

		// WORLD-ENTRY-002f: the 3034 path. Returns whether the connection may
		// continue.
		bool HandleGoto(PeerPtr peer, Network::ServerBatchEncoder& batcher,
		                const Network::Message& message);

		// Sends 3033 for `broadcast` to every authorized peer EXCEPT `exclude`.
		//
		// `exclude` is the peer that already received its own copy synchronously, so
		// without it the mover would get the message twice.
		void BroadcastMoveState(const PeerPtr& exclude,
		                        const Network::MovementState::MoveStateBroadcast& broadcast);

		// Sends 3035 to every authorized peer EXCEPT `exclude`, for the same reason
		// and by the same shape as BroadcastMoveState. One implementation, not two:
		// the copy-the-peer-list-under-the-lock dance is identical, and a second copy
		// would be a second thing to keep correct.
		void BroadcastGoto(const PeerPtr& exclude,
		                   const Network::Goto::GotoBroadcast& broadcast);

		Status SendEnveloped(Network::ServerBatchEncoder& batcher, PeerPtr peer,
		                     const std::vector<Network::WireU8>& inner);

		// Sends one already-enveloped frame to a peer, under its send mutex.
		Status SendRaw(PeerPtr peer, const std::vector<Network::WireU8>& bytes);

		void Emit(FieldEvent event, std::string text, std::size_t count = 0);

		WorldServerConfig      m_config;
		ICharacterRepository&  m_repository;
		FieldEntryRegistry&    m_registry;
const MovementStateService& m_movement;

		// WORLD-ENTRY-002f: the movement world.
		//
		// Owned by the role because the role is what has the characters in it. The
		// speed seam and the map source are BORROWED through it, so changing where
		// meshes come from is a wiring change and not a new object graph.
		WorldMovementRuntime m_movementWorld;

		// The GOTO rule, built from the movement-state service this role already owns
		// so the two movement paths cannot disagree about speed. Held by value, so its
		// borrow of the service cannot outlive it.
		GotoService m_gotoService;

		FieldLogSink           m_log;
		Network::TcpListener   m_listener;

		// Per-conversation LZO work buffers; the codec itself holds no conversation
		// state and is shared.
		// There is deliberately NO codec member on the role.
		//
		// MinLzo1xCodec holds an LZO work buffer and CompressionCodec.h states
		// outright that instances are "NOT internally synchronised; a server with
		// concurrent senders should hold one per thread". One shared member would be a
		// documented violation the moment a second connection existed - and the failure
		// would be a corrupted compressed envelope on somebody else's socket, which is
		// about as hard to attribute as a bug gets. Each Peer owns its own instead.

		std::thread m_acceptThread;

		// The live peers, and the threads serving them. Guarded by m_peersMutex, which
		// is held only to copy or mutate the list - never across a blocking read or a
		// send, so a broadcast cannot stall a connection's own replies.
		mutable std::mutex              m_peersMutex;
		std::vector<PeerPtr>            m_peers;
		std::vector<std::thread>        m_workerThreads;

		std::atomic<bool> m_stopAccept{false};

		// Monotonic peer id, so two Field sessions never share a session id.
		std::atomic<Network::WireU64> m_peerCounter{1};

		std::atomic<std::size_t> m_served{0};
		std::atomic<std::size_t> m_refused{0};
		std::atomic<std::size_t> m_moveSent{0};
		std::atomic<std::size_t> m_moveUnchanged{0};
std::atomic<std::size_t> m_gotoSent{0};
		std::atomic<std::size_t> m_gotoRefused{0};
		std::atomic<Network::WireU32> m_lastGaeaId{0};
		std::atomic<std::size_t> m_lastCharacterId{0};

		std::atomic<FieldRefusal> m_refusal{FieldRefusal::None};
		std::string               m_refusalDetail;
	};
}

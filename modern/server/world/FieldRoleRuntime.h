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
#include "AttackDamageProtocol.h"
#include "AttackService.h"
#include "DamageResolution.h"
#include "ResourceSyncService.h"
#include "ServerBatchEncoder.h"
#include "TcpListener.h"
#include "TcpTransport.h"
#include "UpdateStateProtocol.h"
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
#include <functional>
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

	// WORLD-ENTRY-002h: a 3046 was sent to the owning client.
	ResourceUpdateSent,

	// WORLD-ENTRY-002h: a 3053 was broadcast to other clients.
	ResourceBroadcastSent,

	// WORLD-ENTRY-002i: a 3036 was accepted and 3037 went out. Carries the ATTACKER's
	// gaeaId - the id on the wire is always the attacker's, never the target's.
	//
	// "Accepted" means the attack was VALID and in range. It does NOT mean anything
	// was hit: this milestone applies no damage, and 3037 carries an animation and a
	// target, not a result.
	AttackAccepted,

	// WORLD-ENTRY-002i: a 3036 was refused as out of range, so 3041 went to the
	// attacker and 3042 to everyone else - the ONE refusal legacy announces
	// (GLCharMsg.cpp:352-363). Every other attack refusal is silent on the wire.
	AttackAvoidSent,

	// WORLD-ENTRY-002k: a 3036 landed and a 3043 went out, carrying the damage the
	// resource layer ACTUALLY applied. Carries the TARGET's gaeaId, because a
	// damage event is about who lost HP.
	AttackDamageSent,

	// WORLD-ENTRY-002i: a 3036 was refused SILENTLY - unknown target, a mob, no id, or
	// an unspawned attacker. Nothing went on the wire, exactly as legacy's E_FAIL
	// branches do. Mirrors GotoRejected: the log line is the only difference from
	// silence, and it is the difference an operator has.
	AttackRejected,

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

		// WORLD-ENTRY-002i: a 3036 arrived before the connection was spawned. Its own
		// value for the same reason as GotoBeforeSpawn - the count it feeds is an
		// ATTACK count, and an operator should not have to read two refusal kinds.
		AttackBeforeSpawn,
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

	// WORLD-ENTRY-002i: 3036s accepted (a 3037 went out) and 3036s refused.
	// Monotonic; read after Stop().
	std::size_t AttackAcceptedCount() const noexcept { return m_attackAccepted.load(); }
	std::size_t AttackRefusedCount() const noexcept { return m_attackRefused.load(); }

	// WORLD-ENTRY-002i: how many 3041s were sent - that is, how many refusals were
	// of the ANNOUNCED kind. Every other refusal increments AttackRefusedCount()
	// alone, so this is the count of refusals a client could actually observe.
	std::size_t AttackAvoidSentCount() const noexcept { return m_attackAvoidSent.load(); }

	// WORLD-ENTRY-002k: 3043s sent (damage applied), and resolutions that produced
	// no damage packet at all - a dead target, or a refused roll.
	std::size_t AttackDamageSentCount() const noexcept
	{
		return m_attackDamageSent.load();
	}
	std::size_t AttackDamageRefusedCount() const noexcept
	{
		return m_attackDamageRefused.load();
	}

	// WORLD-ENTRY-002k: the damage roll source, injectable.
	//
	// A SEAM, not a generator. Legacy rolls `rand()/RAND_MAX` per strike
	// (GLDefine.h:11); nothing in the modern server generates randomness yet, and
	// an unseeded one would make every attack test a coin flip. The default
	// returns a fixed always-hit roll so the boundary is exercised end to end; a
	// test or a later milestone supplies its own.
	void SetDamageRollSource(std::function<float()> source) noexcept
	{
		m_rollSource = std::move(source);
	}

	// The rule itself, exposed so a test can exercise validation without a socket.
	// Read-only: the Field role owns the only instance.
	// Named AttackRules, NOT Attack: a member called `Attack` would hide the
	// Attack NAMESPACE inside this class, so every `Attack::AttackCodec` and
	// `AttackService` reference below and in the .cpp would resolve to the member
	// function instead. That is a compile error at best and a silent mis-resolution
	// at worst, and it is not worth a name collision to save four characters.
	const AttackService& AttackRules() const noexcept { return m_attackService; }

	// WORLD-ENTRY-002h: the authoritative resource synchronisation service.
	// Exposed for tests to inject time and observe frames.
	ResourceSyncService& ResourceSync() noexcept { return m_resources; }
	const ResourceSyncService& ResourceSync() const noexcept { return m_resources; }

	// WORLD-ENTRY-002h: 3046 and 3053 sent counts. Monotonic; read after Stop().
	std::size_t UpdateStateSentCount() const noexcept { return m_updateStateSent.load(); }
	std::size_t UpdateStateBrdSentCount() const noexcept { return m_updateStateBrdSent.load(); }

	// WORLD-ENTRY-002h: the last error RegisterSession returned, or None. Read after
	// Stop(). Exposed because a failed registration is invisible on the wire.
	ErrorCode ResourceRegisterFailure() const noexcept
	{
		return m_resourceRegisterFailure.load(std::memory_order_acquire);
	}

	// WORLD-ENTRY-002h: whether the resource ticker thread is running. True between a
	// successful StartResourceTicker and the Stop that actually joins it.
	bool ResourceTickerRunning() const noexcept
	{
		return m_resourceTickerRunning.load(std::memory_order_acquire);
	}

	// WORLD-ENTRY-002h: starts/stops the resource recovery ticker. Mirrors the
	// movement ticker API.
	Status StartResourceTicker();
	void   StopResourceTicker() noexcept;

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

		// WORLD-ENTRY-002i: the 3036 path. Returns whether the connection may
		// continue.
		//
		// A malformed 3036 drops the connection exactly as a malformed 3032 or 3034
		// does; a well-formed one never does, whatever the rule decides. Legacy's
		// out-of-range branch returns E_FAIL after sending 3041/3042, but E_FAIL
		// there means "this attack did not happen", not "this client is
		// misbehaving" - and dropping a well-formed connection over a failed range
		// check would be a behaviour change, not a reproduction.
		bool HandleAttack(PeerPtr peer, Network::ServerBatchEncoder& batcher,
		                  const Network::Message& message);

		// Resolves `gaeaId` to a live spawned peer, or nullptr.
		//
		// The authoritative spawned-character lookup. A peer is a target only once it
		// has earned a spawn AND carries a gaeaId - the same condition 3033, 3035 and
		// 3053 already use to decide who may receive a broadcast. Returns a
		// shared_ptr copied out from under the lock, so the caller never holds a
		// reference into the registry and cannot race an Unregister.
		PeerPtr FindTargetPeer(Network::WireU32 gaeaId);

		// than being treated as sitting at the world origin.
		// The server's authoritative position for sessionId.
		//
		// The movement runtime's actor position is used ONLY when that actor has a
		// navigation mesh behind it. A meshless role still attaches a character, so a
		// Snapshot succeeds, but the actor was never created and its position is a
		// placeholder - so every meshless character would report the SAME position,
		// which silently disables any distance rule built on it. That is not
		// hypothetical: it made the attack range check inert until a test caught it.
		// In that case the character's authored savePosition is used instead;
		// with no character either, allback.
		//
		// In a world where every map resolves, nothing about this changes.
		Vector3 AuthoritativePosition(Network::WireU64 sessionId,
		                               const WorldCharacter* character,
		                               const Vector3& fallback) const;

		// WORLD-ENTRY-002k: the ANNOUNCED refusal - 3041 to the attacker and 3042 to
		// everyone else.
		//
		// One helper for two callers, because legacy sends the identical pair from
		// the out-of-range branch (GLCharMsg.cpp:352-363) and from the MISS branch
		// (GLChar::AvoidProc, GLChar.cpp:2468-2476). A miss is not a different
		// event on the wire; it is the same one for a different reason.
		bool SendAttackAvoid(PeerPtr peer, Network::ServerBatchEncoder& batcher,
		                     const AttackResult& result);

		// WORLD-ENTRY-002k: resolves one accepted attack, applies it, and reports.
		//
		// `targetPeer` is excluded from the 3044 broadcast: the victim learns of its
		// own HP change through the 3046 that ApplyDamage already emitted, and two
		// differently-shaped statements of one event would be a protocol invention.
		void ApplyAttackDamage(PeerPtr peer, Network::ServerBatchEncoder& batcher,
		                       const AttackResult& accepted);

		// Sends 3044 to every authorized peer EXCEPT `exclude`. Mirrors BroadcastGoto.
		void BroadcastAttackDamage(const PeerPtr& exclude,
		                           const Network::Attack::AttackDamageBroadcast& broadcast);

		// WORLD-ENTRY-002k: one roll in [0,1] from the configured source.
		float NextRoll() const noexcept;

		// Sends 3037 to every authorized peer EXCEPT `exclude`. Mirrors BroadcastGoto.
		void BroadcastAttack(const PeerPtr& exclude,
		                     const Network::Attack::AttackBroadcast& broadcast);

		// WORLD-ENTRY-002i: sends 3042 to every authorized peer EXCEPT `exclude`.
		// Mirrors BroadcastGoto.
		void BroadcastAttackAvoid(const PeerPtr& exclude,
		                          const Network::Attack::AttackAvoidBroadcast& broadcast);

		// WORLD-ENTRY-002h: broadcasts a 3053 (StateBroadcast) to all authorized
		// peers except `exclude`. Mirrors BroadcastMoveState exactly.
		void BroadcastResourceState(const PeerPtr& exclude,
		                            const std::vector<Network::WireU8>& packet);

		// WORLD-ENTRY-002h: writes the authoritative pools back to the repository.
		// Invoked from the resource sync service's write-back sink.
		void WriteBackPools(WorldCharacterId characterId,
		                    const Network::RanWire::DwPair& hp,
		                    const Network::RanWire::DwPair& mp,
		                    const Network::RanWire::DwPair& sp);

		Status SendEnveloped(Network::ServerBatchEncoder& batcher, PeerPtr peer,
		                     const std::vector<Network::WireU8>& inner);

		// WORLD-ENTRY-002h: the resource ticker loop. Runs on its own thread,
		// advancing resource pools at a fixed slice rate. Mirrors the movement
		// ticker loop pattern.
		void ResourceTickerLoop();

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

	// WORLD-ENTRY-002i: the ATTACK validation rule. Owned by the role for the same
	// reason as m_gotoService - the role is what knows who is connected - and it is
	// stateless, so this is for reachability rather than for state.
	AttackService m_attackService;

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

	// WORLD-ENTRY-002i: 3036 outcomes. Monotonic; written by worker threads.
std::atomic<std::size_t> m_attackAccepted{0};
std::atomic<std::size_t> m_attackRefused{0};
	std::atomic<std::size_t> m_attackAvoidSent{0};

	// WORLD-ENTRY-002k: damage outcomes. Monotonic; written by worker threads.
std::atomic<std::size_t> m_attackDamageSent{0};
std::atomic<std::size_t> m_attackDamageRefused{0};

	// WORLD-ENTRY-002k: the roll seam. Empty means "use the fixed always-hit
	// default"; see NextRoll.
std::function<float()> m_rollSource;
		std::atomic<std::size_t> m_gotoRefused{0};
		std::atomic<Network::WireU32> m_lastGaeaId{0};
		std::atomic<std::size_t> m_lastCharacterId{0};

		std::atomic<FieldRefusal> m_refusal{FieldRefusal::None};
		std::string               m_refusalDetail;

		// WORLD-ENTRY-002h: authoritative resource sync.
		ResourceSyncService m_resources;

		// Resource recovery ticker - mirrors the movement ticker pattern.
		// Uses atomic running flag with exchange semantics for correct start/stop lifecycle.
		std::thread m_resourceTickerThread;
		std::atomic<bool> m_stopResourceTicker{false};
		std::atomic<bool> m_resourceTickerRunning{false};

		// 3046/3053 sent counts. Written by sinks on ticker/worker threads.
		std::atomic<std::size_t> m_updateStateSent{0};
		std::atomic<std::size_t> m_updateStateBrdSent{0};

	// WORLD-ENTRY-002h: the last RegisterSession error. None means every spawn so
	// far was tracked by the resource service.
	//
	// Registering before the 2333 is what makes this always None in practice: a
	// refusal now fails the spawn, so a client that reads a spawn has a session.
	// The field is kept because "never observed" and "observed and recovered" are
	// different claims, and only the second one is provable from a counter.
	std::atomic<ErrorCode> m_resourceRegisterFailure{ErrorCode::None};
	};
}

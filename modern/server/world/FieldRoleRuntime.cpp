#include "world/FieldRoleRuntime.h"

#include "NetworkCodec.h"

#include <utility>

namespace Modern::Server::World
{
	using namespace Modern::Network;

namespace
{
		// Bounds ONE message read, not the whole conversation.
		//
		// Per message, because the conversation is now unbounded - a client may hold a
		// Field connection open and send a 3032 at any time. A whole-conversation budget
		// would mean either an enormous one or closing idle clients, and both are wrong
		// for a connection that is supposed to stay open.
		constexpr int kMessageTimeoutMilliseconds = 10000;

		// How long Accept waits before it re-checks m_stopAccept.
		//
		// Short, so Stop() is prompt rather than waiting out a long poll. A long accept
		// would make shutdown feel broken and is the one thing a test would notice.
		constexpr int kAcceptSliceMilliseconds = 200;

		constexpr std::size_t kReadBufferSize = 2048;

		// WORLD-ENTRY-002h: resource ticker constants - mirror the movement ticker
		// pattern for consistent timing behavior.
		constexpr int kTickerSliceMilliseconds = 4;
		constexpr float kMaximumTickSeconds = 1.0f;
	}

const char* ToString(FieldEvent event) noexcept
	{
		switch (event)
		{
		case FieldEvent::Listening:            return "Listening";
		case FieldEvent::ClientConnected:      return "ClientConnected";
		case FieldEvent::IdentityAccepted:    return "IdentityAccepted";
		case FieldEvent::SpawnSent:           return "SpawnSent";
		case FieldEvent::MoveStateSent:       return "MoveStateSent";
		case FieldEvent::MoveStateUnchanged:  return "MoveStateUnchanged";
		case FieldEvent::GotoAccepted:        return "GotoAccepted";
		case FieldEvent::GotoRejected:        return "GotoRejected";
		case FieldEvent::ResourceUpdateSent:  return "ResourceUpdateSent";
		case FieldEvent::ResourceBroadcastSent: return "ResourceBroadcastSent";
	case FieldEvent::AttackAccepted:        return "AttackAccepted";
	case FieldEvent::AttackAvoidSent:       return "AttackAvoidSent";
	case FieldEvent::AttackDamageSent:      return "AttackDamageSent";
	case FieldEvent::AttackRejected:        return "AttackRejected";
		case FieldEvent::ClientRejected:      return "ClientRejected";
		case FieldEvent::ClientDisconnected:  return "ClientDisconnected";
		}
		return "Unrecognised";
	}

	const char* ToString(FieldRefusal refusal) noexcept
	{
		switch (refusal)
		{
		case FieldRefusal::None:             return "None";
		case FieldRefusal::UnexpectedMessage: return "UnexpectedMessage";
		case FieldRefusal::BadMessageSize:   return "BadMessageSize";
		case FieldRefusal::MalformedFrame:   return "MalformedFrame";
		case FieldRefusal::PeerClosedFirst:  return "PeerClosedFirst";
		case FieldRefusal::SendFailed:       return "SendFailed";
		case FieldRefusal::ReceiveFailed:    return "ReceiveFailed";
		case FieldRefusal::IdentityRejected: return "IdentityRejected";
		case FieldRefusal::NotSpawned:       return "NotSpawned";
		case FieldRefusal::AttackBeforeSpawn: return "AttackBeforeSpawn";
	case FieldRefusal::GotoBeforeSpawn:  return "GotoBeforeSpawn";
		}
		return "Unrecognised";
	}

	FieldRoleRuntime::FieldRoleRuntime(WorldServerConfig config,
	                                   ICharacterRepository& repository,
	                                   FieldEntryRegistry& registry,
	                                   const MovementStateService& movement,
	                                   FieldLogSink log)
		: m_config(std::move(config))
		, m_repository(repository)
		, m_registry(registry)
		, m_movement(movement)
		// WORLD-ENTRY-002f. The movement world reads its rules from the SAME service
		// instance this role already owns, and the GOTO rule borrows that same service.
		// Two services would be two answers to "what speed does this character walk
		// at", and they would disagree the first time a 3032 and a 3034 arrived close
		// enough together for the ordering to matter.
		, m_movementWorld()
	, m_gotoService(m_movement)
		, m_log(std::move(log))
	{
		m_movementWorld.SetMovementStateService(&m_movement);
		m_movementWorld.SetGotoService(&m_gotoService);
	}
	FieldRoleRuntime::~FieldRoleRuntime()
	{
		Stop();
	}

	void FieldRoleRuntime::Emit(FieldEvent event, std::string text, std::size_t count)
	{
		if (m_log)
		{
			FieldLogEntry entry;
			entry.event = event;
			entry.count = count;
			entry.text  = std::move(text);
			m_log(entry);
		}
	}

	Status FieldRoleRuntime::Start()
	{
		if (const Status status = m_config.Validate(); status.IsError())
		{
			m_refusalDetail = "invalid configuration";
			return status;
		}

		if (const Status status = m_listener.Listen(m_config.fieldBind); status.IsError())
		{
			m_refusalDetail = "bind/listen failed: ";
			m_refusalDetail += status.GetMessage();
			return status;
		}

		m_config.fieldBind = m_listener.BoundEndpoint();

		// The accept thread starts here rather than on first use, so a role that is
		// running is a role that can accept - and a test can tell the two apart.
		m_stopAccept.store(false, std::memory_order_release);
		m_acceptThread = std::thread([this] { AcceptLoop(); });

		Emit(FieldEvent::Listening, m_config.fieldBind.host + ":" +
		                                    std::to_string(m_config.fieldBind.port));

		// WORLD-ENTRY-002f: the movement ticker is NOT started here. A caller that
		// wants it calls StartMovementTicker, and a test that injects elapsed time
		// itself must not: a ticker running alongside an injected tick would advance
		// every actor twice. Deliberate, and the reason is in WorldMovementRuntime.h.
		return Ok();
	}

	void FieldRoleRuntime::Stop() noexcept
	{
		// WORLD-ENTRY-002h: the resource ticker counts as "running" for the purposes
		// of this early-out. A role whose ticker was started without a listening
		// field (the headless resource tests do exactly that) used to return here and
		// leave a JOINABLE thread behind, which is std::terminate when the runtime is
		// destroyed - the destructor calls Stop(), and Stop() is the only thing that
		// ever joins.
		if (!m_listener.IsListening() && !m_acceptThread.joinable() &&
		    !m_resourceTickerRunning.load(std::memory_order_acquire))
		{
			return;
		}

		// Stopped FIRST, before anything else, because it is the only thread that is
		// not driven by this function's own shutdown sequence and the only one that
		// can call into the resource service while sessions are being torn down.
		StopResourceTicker();

		m_stopAccept.store(true, std::memory_order_release);

		// Close the LISTENER first so no new connection is accepted, then break every
		// live connection so the worker threads blocked in Receive return, then join.
		//
		// The order matters: closing peers before the listener would leave a window in
		// which an accepted connection is never served and its thread never starts.
		m_listener.Close();

		{
			const std::lock_guard<std::mutex> lock(m_peersMutex);
			for (const PeerPtr& peer : m_peers)
			{
				// Safe from another thread by TcpTransport's own contract: a Disconnect
				// underneath a blocked Receive makes that Receive return.
				peer->transport.Disconnect();
			}
		}

		// WORLD-ENTRY-002h: the resource ticker is stopped at the TOP of this
		// function, before the listener and the peers, so it cannot advance
		// resources for a session that is about to be unregistered.

		if (m_acceptThread.joinable())
		{
			m_acceptThread.join();
		}

		for (std::thread& worker : m_workerThreads)
		{
			if (worker.joinable())
			{
				worker.join();
			}
		}
		m_workerThreads.clear();

		{
			const std::lock_guard<std::mutex> lock(m_peersMutex);
			m_peers.clear();
		}
	}

	bool FieldRoleRuntime::IsRunning() const noexcept
	{
		return m_listener.IsListening();
	}

	std::size_t FieldRoleRuntime::AuthorizedSessionCount() const
	{
		const std::lock_guard<std::mutex> lock(m_peersMutex);
		std::size_t                       count = 0;
		for (const PeerPtr& peer : m_peers)
		{
			if (peer->spawned.load(std::memory_order_acquire))
			{
				++count;
			}
		}
		return count;
	}

	void FieldRoleRuntime::AcceptLoop()
	{
		while (!m_stopAccept.load(std::memory_order_acquire))
		{
			std::uintptr_t raw = TcpListener::kNoAcceptedSocket;
			const Status    accepted =
			    m_listener.Accept(raw, kAcceptSliceMilliseconds);

			if (accepted.IsError())
			{
				// Stop() closes the listener underneath this loop, which is how the
				// accept fails on shutdown. Anything else is a real fault worth counting.
				if (!m_stopAccept.load(std::memory_order_acquire))
				{
					m_refusal.store(FieldRefusal::ReceiveFailed, std::memory_order_release);
					m_refusalDetail = "accept failed";
				}
				return;
			}

			if (raw == TcpListener::kNoAcceptedSocket)
			{
				// Nobody knocked. Not a failure and not a refusal - the slice exists so
				// Stop() is observed promptly.
				continue;
			}

			Result<TcpTransport> adopted = TcpTransport::Adopt(raw);
			if (adopted.IsError())
			{
				// Adopt deliberately leaves the handle open on failure, so closing it is
				// this function's job; otherwise a socket leaks per occurrence, silently.
				TcpTransport::CloseOwnedHandle(raw);
				m_refused.fetch_add(1, std::memory_order_relaxed);
				m_refusal.store(FieldRefusal::ReceiveFailed, std::memory_order_release);
				m_refusalDetail = "could not adopt the accepted socket";
				continue;
			}

			PeerPtr peer = std::make_shared<Peer>(m_repository, m_registry, m_peerCounter++);
			peer->transport = std::move(adopted.GetValue());
			peer->session.SetMovementStateService(m_movement);
// WORLD-ENTRY-002f: the session's WINDOW onto the movement world.
		//
		// Set per peer rather than once in the role's constructor because the session is
		// what owns the actor's identity - the map source, the path and the position all
		// hang off the SESSION id - and a session that never learns where the world is
		// cannot answer a 3034 at all. Without this line every GOTO is refused as
		// InvalidState, which on the wire is indistinguishable from a refusal.
		peer->session.SetMovementRuntime(&m_movementWorld);

			// Registered BEFORE the worker starts, so a broadcast from another thread
			// cannot arrive for a connection this role has not yet heard of.
			{
				const std::lock_guard<std::mutex> lock(m_peersMutex);
				m_peers.push_back(peer);
			}

			m_workerThreads.emplace_back([this, peer] { ServePeer(peer); });

			const Endpoint peerEndpoint = peer->transport.RemoteEndpoint();
			Emit(FieldEvent::ClientConnected,
			     peerEndpoint.host + ":" + std::to_string(peerEndpoint.port));
		}
	}

	void FieldRoleRuntime::ServePeer(PeerPtr peer)
	{
		ConnectionFramer   framer;
		ServerBatchEncoder batcher(peer->codec);
		bool               refused = false;

		for (;;)
		{
			Message message;
			const ReadResult read =
			    ReadMessage(peer, framer, message, kMessageTimeoutMilliseconds);

			if (!read.IsOk())
			{
				// Two of the read outcomes are NOT refusals, and conflating them with
				// the rest would be a behaviour change rather than a refactor.
				//
				// Before 002a a Field connection was one short conversation: it served
				// the client's messages and the client went away. An orderly close was
				// the success case, and ServedClientCount counted it.
				//
				// Now the connection is long-lived, so a client that logs off, or that
				// simply sits in the world saying nothing, also ends here. Neither broke
				// a rule, so neither may be reported as one: a "refused" counter that
				// climbed on every disconnect would be worse than useless, because it
				// would hide the refusals that matter.
				if (read.outcome == ReadOutcome::PeerClosed ||
				    read.outcome == ReadOutcome::TimedOut)
				{
					m_refusal.store(FieldRefusal::PeerClosedFirst, std::memory_order_release);
					m_refusalDetail = read.detail;
					break;
				}

				switch (read.outcome)
				{
				case ReadOutcome::TransportFault:
					m_refusal.store(FieldRefusal::ReceiveFailed, std::memory_order_release);
					m_refusalDetail = read.detail;
					break;
				default:
					m_refusal.store(FieldRefusal::MalformedFrame, std::memory_order_release);
					m_refusalDetail = read.detail;
					break;
				}
				refused = true;
				break;
			}

			// ---- 3032 -------------------------------------------------------
			// Dispatched before 2359 on purpose: a movement message is the thing this
			// connection exists for after the spawn, and recognising it first keeps the
			// "not spawned yet" refusal from being reported as an unexpected id.
if (MovementState::MovementStateCodec::IsMoveState(message.header.type))
			{
				if (!HandleMoveState(peer, batcher, message))
				{
					refused = true;
					break;
				}
				continue;
			}

			// ---- 3034 -------------------------------------------------------
			//
			// Dispatched with 3032 and BEFORE 2359, for the same reason: a movement
			// message is what this connection exists for after the spawn, and recognising
			// it first keeps the "not spawned yet" refusal from being reported as an
			// unexpected id.
			if (Goto::GotoCodec::IsGoto(message.header.type))
			{
				if (!HandleGoto(peer, batcher, message))
				{
					refused = true;
					break;
				}
				continue;
			}
			// ---- 3036 -------------------------------------------------------
			//
			// Dispatched with 3032 and 3034, and BEFORE 2359, for the same reason: a
			// gameplay message is what this connection exists for once it has spawned,
			// and recognising it first keeps the "not spawned yet" refusal from being
			// reported as an unexpected id.
			//
			// Only a MALFORMED 3036 refuses the connection. A well-formed one that the
			// rule rejects - out of range, unknown target, a mob - leaves the
			// connection exactly as usable as it was, because a client that aimed at
			// nothing has not misbehaved.
			if (Network::Attack::AttackCodec::IsAttack(message.header.type))
			{
				if (!HandleAttack(peer, batcher, message))
				{
					refused = true;
					break;
				}
				continue;
			}

			// ---- 2359 --------------------------------------------------------
			if (WorldEntryCodec::IsFieldIdentity(message.header.type))
			{
				if (peer->spawned.load(std::memory_order_acquire))
				{
					// A second 2359 on a spawned connection. Refused rather than
					// re-validated: the authorization it would claim is already spent, and
					// re-pointing a live session at another character is exactly what the
					// single-use rule exists to prevent.
					m_refusal.store(FieldRefusal::UnexpectedMessage,
					                std::memory_order_release);
					m_refusalDetail = "second 2359 on an already-spawned connection";
					refused = true;
					break;
				}

				if (!HandleIdentity(peer, batcher, message))
				{
					refused = true;
					break;
				}

				// Counted HERE, at the moment the session was earned, rather than once
				// at the end of the connection.
				//
				// Phase C counted one conversation per connection and the number is
				// unchanged in meaning, but the shape had to move: this connection may
				// live for minutes now, and it may yet be dropped for misbehaving. What
				// "served" means is "entered the world", and that fact is decided here and
				// never changes. Counting it at the end would make the answer depend on
				// how the client hung up.
				m_served.fetch_add(1, std::memory_order_relaxed);
				continue;
			}

			m_refusal.store(FieldRefusal::UnexpectedMessage, std::memory_order_release);
			m_refusalDetail = "unexpected message id " +
			                  std::to_string(message.header.type) +
			                  " on the Field connection";
			refused = true;
			break;
		}

		// The two counters have disjoint meanings, and the gap between them is real.
		//
		// m_served is incremented at the spawn above, so it is not touched here. What is
		// left is m_refused, which counts connections that ended because the client broke
		// a rule. A connection that merely closed, or that sat idle past the message
		// budget, is in NEITHER counter - it misbehaved in no way, and letting it into
		// m_refused would drown the refusals a caller actually needs to see.
		if (refused)
		{
			m_refused.fetch_add(1, std::memory_order_relaxed);
			Emit(FieldEvent::ClientRejected, m_refusalDetail);
		}

		// Settle the claim and drop the peer from the registry, so a disconnecting
		// client stops being a broadcast target and stops holding a gaeaId's
		// authorization spent-but-present.
		peer->session.SettleClaim();
		peer->spawned.store(false, std::memory_order_release);

		// WORLD-ENTRY-002h: unregister the resource session.
		(void)m_resources.UnregisterSession(peer->session.GaeaId());

		{
			const std::lock_guard<std::mutex> lock(m_peersMutex);
			for (auto it = m_peers.begin(); it != m_peers.end(); ++it)
			{
				if (*it == peer)
				{
					m_peers.erase(it);
					break;
				}
			}
		}

		peer->transport.Disconnect();
		Emit(FieldEvent::ClientDisconnected, "");
	}

	ReadResult FieldRoleRuntime::ReadMessage(PeerPtr peer, ConnectionFramer& reader,
	                                         Message& message, int budgetMilliseconds)
	{
		// The deadline loop is Network::ReadOneMessage's, shared with the Agent role and
		// with LOGIN-002. No movement-specific read loop exists, and adding one would be
		// a second framing parser.
		return ReadOneMessage(peer->transport, reader, message, budgetMilliseconds);
	}

	bool FieldRoleRuntime::HandleIdentity(PeerPtr peer, ServerBatchEncoder& batcher,
	                                      const Message& message)
	{
		const std::vector<WireU8> frame = ReconstructFrame(message);

		FieldIdentity identity;
		if (const Status status = WorldEntryCodec::DecodeFieldIdentity(frame, identity);
		    status.IsError())
		{
			m_refusal.store(FieldRefusal::BadMessageSize, std::memory_order_release);
			m_refusalDetail = "2359 malformed (dwSize " + std::to_string(frame.size()) +
			                  ", expected " + std::to_string(WorldEntry::kIdentitySize) + ")";
			return false;
		}

		// Phase B's validation, unchanged: format, pending authorization, slot, join
		// type, replay, staleness, repository ownership, and that the character still
		// holds THIS gaeaId. This layer decides nothing about whether the claim is good.
		//
		// The clock is this role's own monotonic session counter. Phase B's sessions
		// deliberately read no clock so a test can age an authorization without
		// sleeping; over a real socket the honest input is real elapsed time, and the
		// staleness rule is exercised directly in the Phase B tests.
		const WireU64 nowMs = static_cast<WireU64>(m_served.load() + m_refused.load());
		if (const Status status = peer->session.ValidateIdentity(identity, nowMs);
		    status.IsError())
		{
			m_refusal.store(FieldRefusal::IdentityRejected, std::memory_order_release);
			m_refusalDetail = "2359 refused: ";
			m_refusalDetail += status.GetMessage();
			return false;
		}

		Emit(FieldEvent::IdentityAccepted, "", peer->session.GaeaId());

		// The spawn packet itself is Phase B's, built from the repository record the
		// validation just re-read.
		FieldSpawnResult spawn;
		if (const Status status = peer->session.BuildSpawn(spawn); status.IsError())
		{
			m_refusal.store(FieldRefusal::IdentityRejected, std::memory_order_release);
			m_refusalDetail = "spawn refused: ";
			m_refusalDetail += status.GetMessage();
			return false;
		}

		// WORLD-ENTRY-002h: the authoritative resource session is REGISTERED BEFORE
		// the 2333 goes out, and a refusal here refuses the spawn.
		//
		// The ordering is the point. A spawn packet is what tells a client - and every
		// caller acting for it - that the character is in the world, so the moment the
		// 2333 is on the wire the session must already be findable. Registering after
		// the send leaves a window in which a client that has already read its spawn
		// gets NotFound from Spend/Restore/ApplyDamage for its own character, which
		// presents as "the server ignores my HP" and is a race, not a rule.
		//
		// It is therefore FATAL to the spawn here, unlike the movement Attach below.
		// The Attach is allowed to fail because a missing mesh still leaves a playable
		// character whose GOTO answers name the map; a missing resource session leaves
		// a character with no authoritative pools at all, and there is nothing later
		// that can repair it - the session is registered once, at spawn.
		const auto characterId = spawn.character.id;
		if (const Status registered = m_resources.RegisterSession(
		        spawn.character,
		        [this, peer](const std::vector<WireU8>& frame) {
			        ServerBatchEncoder batcher(peer->codec);
			        (void)SendEnveloped(batcher, peer, frame);
			        m_updateStateSent.fetch_add(1, std::memory_order_relaxed);
		        },
		        [this, peer](const std::vector<WireU8>& frame) {
			        BroadcastResourceState(peer, frame);
			        m_updateStateBrdSent.fetch_add(1, std::memory_order_relaxed);
		        },
		        [this, characterId](const Network::RanWire::DwPair& hp,
			                    const Network::RanWire::DwPair& mp,
			                    const Network::RanWire::DwPair& sp) {
			        WriteBackPools(characterId, hp, mp, sp);
		        });
		    registered.IsError())
		{
			m_resourceRegisterFailure.store(registered.GetCode(), std::memory_order_release);
			m_refusal.store(FieldRefusal::IdentityRejected, std::memory_order_release);
			m_refusalDetail = "resource registration refused: ";
			m_refusalDetail += registered.GetMessage();
			return false;
		}

		m_lastGaeaId.store(spawn.gaeaId, std::memory_order_release);
		m_lastCharacterId.store(spawn.character.id.value, std::memory_order_release);

		// The flag is raised BEFORE the packet is sent, not after it.
		//
		// It gates every broadcast: 3033, 3035 and 3053 all skip a peer whose
		// `spawned` flag is still false. A client whose Spawn() has returned has,
		// by construction, already read its 2333 - so setting the flag after the
		// send left a window in which a fully spawned, fully connected player was
		// invisible to everyone else's broadcast, and the only symptom was a 3053
		// that never arrived for a character standing right there. The packet tells
		// the CLIENT it spawned; the flag tells the SERVER, and it must be true no
		// later than the moment the client can act on it.
		peer->spawned.store(true, std::memory_order_release);

		if (const Status sent = SendEnveloped(batcher, peer, spawn.frame); sent.IsError())
		{
			// Both effects of this spawn are undone HERE rather than left to the
			// peer teardown, because the teardown only runs if a worker is still
			// alive - and the honest place to undo a step is immediately after the
			// step whose failure made it wrong.
			peer->spawned.store(false, std::memory_order_release);
			(void)m_resources.UnregisterSession(spawn.gaeaId);

			m_refusal.store(FieldRefusal::SendFailed, std::memory_order_release);
			m_refusalDetail = "send failed: ";
			m_refusalDetail += sent.GetMessage();
			return false;
		}

Emit(FieldEvent::SpawnSent, "", spawn.gaeaId);

		// WORLD-ENTRY-002f: give the character an actor on its map. A failure here is
		// deliberately NOT fatal and does not stop the spawn: the 2333 has already gone
		// out, the client is in the map, and every GOTO it sends will be answered with a
		// reason naming the map. Refusing the spawn would turn a missing asset into a
		// failed login - the wrong diagnosis for the same underlying fault.
		(void)m_movementWorld.Attach(peer->session.SessionId(), spawn.character);
		return true;
	}

	bool FieldRoleRuntime::HandleMoveState(PeerPtr peer, ServerBatchEncoder& batcher,
	                                       const Message& message)
	{
		const std::vector<WireU8> frame = ReconstructFrame(message);

		MovementState::MoveStateRequest request;
		if (const Status status =
		        MovementState::MovementStateCodec::DecodeMoveStateRequest(frame, request);
		    status.IsError())
		{
			m_refusal.store(FieldRefusal::BadMessageSize, std::memory_order_release);
			m_refusalDetail = "3032 malformed (dwSize " + std::to_string(frame.size()) +
			                  ", expected " +
			                  std::to_string(MovementState::kRequestSize) + ")";
			return false;
		}

		MovementStateChange change;
		if (const Status status =
		        peer->session.ApplyMoveState(request.actState, change);
		    status.IsError())
		{
			// NotSpawned for the pre-entry case, which is the one a caller can actually
			// provoke; InvalidState would mean the session was never wired.
			m_refusal.store(status.GetCode() == ErrorCode::NotAllowed ? FieldRefusal::NotSpawned
			                                                          : FieldRefusal::MalformedFrame,
			                std::memory_order_release);
			m_refusalDetail = "3032 refused: ";
			m_refusalDetail += status.GetMessage();
			// No Emit here: returning false makes ServePeer emit ClientRejected once,
			// with this same detail, when it accounts for the connection. Emitting in
			// both places would report one refused client twice.
			return false;
		}

		if (!change.changed)
		{
			// Legacy sends NOTHING when the state did not change
			// (GLCharMsg.cpp:203). No 3033, and no broadcast - which is the observable
			// behaviour this milestone has to get right, not an optimisation.
			m_moveUnchanged.fetch_add(1, std::memory_order_relaxed);
			Emit(FieldEvent::MoveStateUnchanged, "", change.gaeaId);
			return true;
		}

		// The AUTHORITATIVE word and the AUTHORITATIVE id, both read off the server-side
		// record by the service. The request's bits are never echoed verbatim.
		MovementState::MoveStateBroadcast broadcast;
		broadcast.gaeaId   = change.gaeaId;
		broadcast.actState = change.actState;

		std::vector<WireU8> packet;
		if (const Status status =
		        MovementState::MovementStateCodec::AppendMoveStateBroadcast(packet, broadcast);
		    status.IsError())
		{
			m_refusal.store(FieldRefusal::SendFailed, std::memory_order_release);
			m_refusalDetail = "could not encode 3033";
			return false;
		}

		// The mover gets its own copy synchronously, so a client that moved always
		// learns the authoritative result on the connection it moved from.
		if (const Status sent = SendEnveloped(batcher, peer, packet); sent.IsError())
		{
			m_refusal.store(FieldRefusal::SendFailed, std::memory_order_release);
			m_refusalDetail = "send failed: ";
			m_refusalDetail += sent.GetMessage();
			return false;
		}

		// And everyone else who is authorized learns it too.
		BroadcastMoveState(peer, broadcast);

		m_moveSent.fetch_add(1, std::memory_order_relaxed);
Emit(FieldEvent::MoveStateSent, "", change.gaeaId);

		// WORLD-ENTRY-002f: the ticker reads the run flag to choose walk or run speed,
		// and this is the only place the authoritative word changes on a 3032. Copied
		// into the movement snapshot immediately, so a state change is visible to the
		// walk on its very next slice rather than one slice late.
		(void)m_movementWorld.SetActState(peer->session.SessionId(), change.actState);
		return true;
	}

// ---------------------------------------------------------------------------
	// WORLD-ENTRY-002f: the 3034 path
	// ---------------------------------------------------------------------------

	bool FieldRoleRuntime::HandleGoto(PeerPtr peer, ServerBatchEncoder& batcher,
	                                   const Message& message)
	{
		const std::vector<WireU8> frame = ReconstructFrame(message);

		Goto::GotoRequest request;
		if (const Status status = Goto::GotoCodec::DecodeGotoRequest(frame, request);
		    status.IsError())
		{
			// A 3034 of the wrong length is a protocol fault and the connection is
			// dropped, exactly as a malformed 3032 is.
			//
			// The codec has already refused a non-finite coordinate, which is worth
			// stating: a NaN in vCurPos would make the 60-unit comparison false and
			// silently DISABLE the anti-teleport check, so it is refused at the boundary
			// rather than reaching a comparison it cannot survive.
			m_refusal.store(FieldRefusal::BadMessageSize, std::memory_order_release);
			m_refusalDetail = "3034 malformed (dwSize " + std::to_string(frame.size()) +
			                  ", expected " + std::to_string(Goto::kRequestSize) + ")";
			return false;
		}

		GotoResult result;
		if (const Status status = peer->session.ApplyGoto(request, result);
		    status.IsError())
		{
			// NotAllowed is the pre-spawn case, which is the one a caller can actually
			// provoke; anything else is a wiring fault.
			m_refusal.store(status.GetCode() == ErrorCode::NotAllowed
			                    ? FieldRefusal::GotoBeforeSpawn
			                    : FieldRefusal::MalformedFrame,
			                std::memory_order_release);
			m_refusalDetail = "3034 refused: ";
			m_refusalDetail += status.GetMessage();
			// No Emit here: returning false makes ServePeer emit ClientRejected once, with
			// this same detail, when it accounts for the connection. Emitting in both
			// places would report one refused client twice.
			return false;
		}

		// The authoritative word the rule derived, copied into the session's own
		// character so the two records cannot drift. A 3034 influences exactly one bit
		// of it and the rule has already applied that.
		if (const Status status = peer->session.AdoptGotoActState(result.actState);
		    status.IsError())
		{
			m_refusal.store(FieldRefusal::MalformedFrame, std::memory_order_release);
			m_refusalDetail = "3034 produced a corrupt movement state";
			return false;
		}

		if (!result.accepted)
		{
			// SILENT, which is the whole of legacy's failure behaviour: no 3035, no
			// error packet, nothing at all back to the client. An unreachable destination,
			// a dead character and a desynchronised client are indistinguishable from each
			// other on the wire, and 002c measured exactly that. Inventing a rejection
			// packet would be a new network message.
			//
			// The CONNECTION is unaffected. A bad destination is not a misbehaving client,
			// and it may send another.
			m_gotoRefused.fetch_add(1, std::memory_order_relaxed);
			Emit(FieldEvent::GotoRejected, result.detail, peer->session.GaeaId());
			return true;
		}

		// GLCharMsg.cpp:311-318, in order. Every value is the SERVER's:
		//
		//   dwGaeaID   the session's own authorized entity id. A 3034 carries no id, so
		//              there is nothing to forge.
		//   dwActState the authoritative word AFTER the run bit, never the request.
		//   vCurPos    the server's position, which is how a drifted client learns where
		//              the server thinks it is.
		//   vTarPos    the RAW requested target, not the probe's resolution.
		//   fDelay     0.0f, the only value this route ever sends.
		Goto::GotoBroadcast broadcast;
		broadcast.gaeaId   = peer->session.GaeaId();
		broadcast.actState = result.actState;
		broadcast.currentPosition = RanWire::Vector3{ result.authoritativeCurrent.x,
		                                              result.authoritativeCurrent.y,
		                                              result.authoritativeCurrent.z };
		broadcast.targetPosition = RanWire::Vector3{ result.authoritativeTarget.x,
		                                             result.authoritativeTarget.y,
		                                             result.authoritativeTarget.z };
		broadcast.delay = 0.0f;

		std::vector<WireU8> packet;
		if (const Status status = Goto::GotoCodec::AppendGotoBroadcast(packet, broadcast);
		    status.IsError())
		{
			m_refusal.store(FieldRefusal::SendFailed, std::memory_order_release);
			m_refusalDetail = "could not encode 3035";
			return false;
		}

		// The mover gets its own copy synchronously, on the connection it moved from.
		if (const Status sent = SendEnveloped(batcher, peer, packet); sent.IsError())
		{
			m_refusal.store(FieldRefusal::SendFailed, std::memory_order_release);
			m_refusalDetail = "send failed: ";
			m_refusalDetail += sent.GetMessage();
			return false;
		}

		// And everyone else who is authorized learns it too - the same staged
		// approximation of view range the 3033 broadcast already documents.
		BroadcastGoto(peer, broadcast);

		m_gotoSent.fetch_add(1, std::memory_order_relaxed);
		Emit(FieldEvent::GotoAccepted, "", broadcast.gaeaId);
		return true;
	}

	void FieldRoleRuntime::BroadcastGoto(const PeerPtr& exclude,
	                                    const Goto::GotoBroadcast& broadcast)
	{
		std::vector<WireU8> packet;
		if (const Status status = Goto::GotoCodec::AppendGotoBroadcast(packet, broadcast);
		    status.IsError())
		{
			// Already validated a moment ago by the caller, so this cannot fail; refused
			// rather than ignored so a future change that breaks it is visible.
			return;
		}

		// The peer list is COPIED under the lock and then used without it - the same
		// reasoning as BroadcastMoveState, and deliberately not folded into a shared
		// helper: the two broadcasts carry different packet types, and one more level of
		// indirection on the send path would cost more readability than the duplication
		// it saved.
		std::vector<PeerPtr> targets;
		{
			const std::lock_guard<std::mutex> lock(m_peersMutex);
			targets = m_peers;
		}

		for (const PeerPtr& peer : targets)
		{
			if (peer == exclude)
			{
				continue;
			}

			// Only sessions that have earned a spawn are told. A connection mid-handshake
			// has no character in the world to have moved.
			if (!peer->spawned.load(std::memory_order_acquire))
			{
				continue;
			}

			ServerBatchEncoder batcher(peer->codec);
			(void)SendEnveloped(batcher, peer, packet);
		}
	}


		FieldRoleRuntime::PeerPtr FieldRoleRuntime::FindTargetPeer(Network::WireU32 gaeaId)
	{
		if (gaeaId == 0)
		{
			return nullptr;
		}

		// The list is COPIED under the lock and searched without it, for the reason
		// BroadcastGoto gives: holding m_peersMutex across a search that a peer could
		// be unregistering from would be a lock-order hazard, and the copy is a vector
		// of shared_ptr, so a peer that disconnects mid-search simply fails to match.
		std::vector<PeerPtr> targets;
		{
			const std::lock_guard<std::mutex> lock(m_peersMutex);
			targets = m_peers;
		}

		for (const PeerPtr& peer : targets)
		{
			// `spawned` is the same gate every broadcast uses. It is written under the
			// spawn path and read here, so it is loaded rather than assumed - a peer
			// mid-spawn must never be attackable before it has a world position.
			if (!peer->spawned.load(std::memory_order_acquire))
			{
				continue;
			}

			// A gaeaId of 0 is never a valid identity: WorldEntryProtocol reserves it
			// as kInvalidGaeaId, so refusing it here cannot exclude a real character.
			if (peer->session.GaeaId() == gaeaId)
			{
				return peer;
			}
		}

		return nullptr;
	}

	Vector3 FieldRoleRuntime::AuthoritativePosition(Network::WireU64 sessionId,
	                                                const WorldCharacter* character,
	                                                const Vector3& fallback) const
	{
		ActorSnapshot snapshot;

		// `hasMesh` is checked alongside the snapshot, and that check is the whole
		// point of this function.
		//
		// WorldMovementRuntime::Attach only calls `Actor::Create` when a mesh
		// resolves. With no map source, every character is still ATTACHED - so a
		// Snapshot succeeds - but its actor was never created, and the actor's
		// position is still its default placeholder rather than a place in the
		// world. Taking snapshot.position in that state reports every meshless
		// character as standing at the same spot, which silently made the attack
		// range check inert: every pair measured distance zero, every attack was
		// in range, and nothing could ever be refused for being too far.
		//
		// This was found by a test that could not provoke an out-of-range refusal
		// at all - the acceptance path kept winning. The fix is to prefer the
		// actor's position only when the actor has a real one, and to fall back to
		// the character's authored save position otherwise. In a world with meshes
		// nothing changes.
		if (m_movementWorld.Snapshot(sessionId, snapshot) && snapshot.hasMesh)
		{
			return snapshot.position;
		}

		// No actor yet - a character that has spawned but never been attached, or a
		// test runtime with no map source at all. The character's own stored position
		// is still a real position, and using the world origin instead would put
		// every meshless character at (0,0,0) and make them all attackable from
		// anywhere near the origin.
		if (character != nullptr)
		{
			return Vector3{ character->savePosition.x, character->savePosition.y,
			                character->savePosition.z };
		}

		return fallback;
	}

	bool FieldRoleRuntime::HandleAttack(PeerPtr peer, ServerBatchEncoder& batcher,
	                                    const Message& message)
	{
		const std::vector<WireU8> frame = Network::Attack::AttackCodec::ReconstructFrame(message);

		Network::Attack::AttackRequest request;
		if (const Status status = Network::Attack::AttackCodec::DecodeAttackRequest(frame, request);
		    status.IsError())
		{
			// A 3036 of the wrong length is a protocol fault and the connection is
			// dropped, exactly as a malformed 3032 or 3034 is. This is the ONE case
			// where an attack costs the client its connection.
			m_refusal.store(FieldRefusal::BadMessageSize, std::memory_order_release);
			m_refusalDetail = "3036 malformed (dwSize " + std::to_string(frame.size()) +
			                  ", expected " + std::to_string(Network::Attack::kRequestSize) + ")";
			return false;
		}

		const bool spawned = peer->spawned.load(std::memory_order_acquire);

		AttackRequest ruleRequest;
		ruleRequest.targetCrow = request.targetCrow;
		ruleRequest.targetId   = request.targetId;
		ruleRequest.aniSel     = request.aniSel;
		ruleRequest.flags      = request.flags;

		// The attacker's OWN position, from the movement runtime. There is no
		// client-supplied position on a 3036, so the range check has nothing to trust
		// and needs no anti-teleport companion the way 3034's 60-unit rule does.
		const Vector3 attackerPosition =
		    AuthoritativePosition(peer->session.SessionId(), peer->session.Character(),
		                          Vector3{});

		// Resolve the target through the authoritative peer registry.
		TargetView target;
		if (const PeerPtr targetPeer = FindTargetPeer(request.targetId);
		    targetPeer != nullptr)
		{
			target.exists = true;
			target.position =
			    AuthoritativePosition(targetPeer->session.SessionId(),
			                          targetPeer->session.Character(), Vector3{});
		}

		const AttackResult result =
		    m_attackService.Evaluate(peer->session.GaeaId(), spawned, attackerPosition,
		                             ruleRequest, target);

		if (!result.accepted)
		{
			// SILENT unless the refusal was the announced one. Legacy sends
			// 3041/3042 on the out-of-range branch (GLCharMsg.cpp:352-363) and
			// NOTHING for an unknown target, an unspawned attacker or a mob
			// (GLCharMsg.cpp:340's E_FAIL). That asymmetry is legacy's; inventing a
			// rejection packet for the silent cases would be a new network message.
			m_attackRefused.fetch_add(1, std::memory_order_relaxed);

			if (!result.announced)
			{
				// Silent. The counter and this log line are the whole of it, which is
				// exactly what legacy leaves an operator for these branches.
				Emit(FieldEvent::AttackRejected, result.detail, result.attackerGaeaId);
				return true;
			}

			// Out of range: the ANNOUNCED refusal. A miss takes this same path
			// below - legacy sends the identical 3041/3042 pair from
			// GLChar::AvoidProc (GLChar.cpp:2468-2476) as from the range branch
			// (GLCharMsg.cpp:352-363), so one helper serves both.
			return SendAttackAvoid(peer, batcher, result);
		}

		// Accepted. NO DAMAGE: 3037 is an animation and a target, not a hit.
		//
		// `dwGaeaID` is the ATTACKER's own id, never the target's - the same
		// convention 3033 and 3035 follow, and the only value on the wire that
		// identifies who swung.
		Network::Attack::AttackBroadcast broadcast;
		broadcast.gaeaId     = result.attackerGaeaId;
		broadcast.targetCrow = result.targetCrow;
		broadcast.targetId   = result.targetId;
		broadcast.aniSel     = result.aniSel;

		std::vector<WireU8> packet;
		if (const Status status = Network::Attack::AttackCodec::AppendAttackBroadcast(packet, broadcast);
		    status.IsError())
		{
			m_refusal.store(FieldRefusal::SendFailed, std::memory_order_release);
			m_refusalDetail = "could not encode 3037";
			return false;
		}

		// The attacker does NOT get its own copy here.
		//
		// This differs from 3035 and 3033 on purpose. Legacy sends the accepted-attack
		// broadcast with SendMsgViewAround, which excludes the sender - the swinging
		// character animates locally and needs no packet - whereas GotoService's
		// 3035 is explicitly the mover's own authoritative answer. Guessing that the
		// two routes share a delivery rule would be inventing one.
		BroadcastAttack(peer, broadcast);

		m_attackAccepted.fetch_add(1, std::memory_order_relaxed);
		Emit(FieldEvent::AttackAccepted, result.detail, broadcast.gaeaId);

		// WORLD-ENTRY-002k: the attack is ALLOWED and the animation is out - now the
		// strike resolves. Legacy keeps the two apart the same way: the animation is
		// broadcast from MsgAttack, and the damage comes from AttackProcess
		// (GLChar.cpp:2848) via DamageProc.
		//
		// The ATTACKER is excluded from the 3044 broadcast, not the victim. DamageProc
		// sends the 3043 to the attacker's own connection and the 3044 with
		// SendMsgViewAround, which excludes the actor (GLChar.cpp:2526-2540). The
		// victim DOES receive the broadcast - that is how it sees the number land -
		// and it is told its new HP separately, through the 3046 ApplyDamage emits.
		ApplyAttackDamage(peer, batcher, result);

		return true;
	}

	void FieldRoleRuntime::BroadcastAttack(const PeerPtr& exclude,
	                                       const Network::Attack::AttackBroadcast& broadcast)
	{
		std::vector<WireU8> packet;
		if (const Status status = Network::Attack::AttackCodec::AppendAttackBroadcast(packet, broadcast);
		    status.IsError())
		{
			// Already validated a moment ago by the caller, so this cannot fail;
			// refused rather than ignored so a future break is visible.
			return;
		}

		std::vector<PeerPtr> targets;
		{
			const std::lock_guard<std::mutex> lock(m_peersMutex);
			targets = m_peers;
		}

		for (const PeerPtr& peer : targets)
		{
			if (peer == exclude)
			{
				continue;
			}

			if (!peer->spawned.load(std::memory_order_acquire))
			{
				continue;
			}

			ServerBatchEncoder batcher(peer->codec);
			(void)SendEnveloped(batcher, peer, packet);
		}
	}

	void FieldRoleRuntime::BroadcastAttackAvoid(const PeerPtr& exclude,
	                                             const Network::Attack::AttackAvoidBroadcast& broadcast)
	{
		std::vector<WireU8> packet;
		if (const Status status =
		        Network::Attack::AttackCodec::AppendAttackAvoidBroadcast(packet, broadcast);
		    status.IsError())
		{
			return;
		}

		std::vector<PeerPtr> targets;
		{
			const std::lock_guard<std::mutex> lock(m_peersMutex);
			targets = m_peers;
		}

		for (const PeerPtr& peer : targets)
		{
			if (peer == exclude)
			{
				continue;
			}

			if (!peer->spawned.load(std::memory_order_acquire))
			{
				continue;
			}

			ServerBatchEncoder batcher(peer->codec);
			(void)SendEnveloped(batcher, peer, packet);
		}
	}

	bool FieldRoleRuntime::SendAttackAvoid(PeerPtr peer, ServerBatchEncoder& batcher,
	                                       const AttackResult& result)
	{
		Network::Attack::AttackAvoid avoid;
		avoid.targetCrow = result.targetCrow;
		avoid.targetId   = result.targetId;

		std::vector<WireU8> packet;
		if (const Status status = Network::Attack::AttackCodec::AppendAttackAvoid(packet, avoid);
		    status.IsError())
		{
			m_refusal.store(FieldRefusal::SendFailed, std::memory_order_release);
			m_refusalDetail = "could not encode 3041";
			return false;
		}

		// The attacker's own copy, synchronously, on the connection it asked
		// from - the same shape HandleGoto uses for 3035.
		if (const Status sent = SendEnveloped(batcher, peer, packet); sent.IsError())
		{
			m_refusal.store(FieldRefusal::SendFailed, std::memory_order_release);
			m_refusalDetail = "send failed: ";
			m_refusalDetail += sent.GetMessage();
			return false;
		}

		// And everyone else who is authorized learns it too, matching legacy's
		// SendMsgViewAround on the same branch.
		Network::Attack::AttackAvoidBroadcast avoidBroadcast;
		avoidBroadcast.gaeaId     = result.attackerGaeaId;
		avoidBroadcast.targetCrow = result.targetCrow;
		avoidBroadcast.targetId   = result.targetId;
		BroadcastAttackAvoid(peer, avoidBroadcast);

		m_attackAvoidSent.fetch_add(1, std::memory_order_relaxed);
		Emit(FieldEvent::AttackAvoidSent, result.detail, result.attackerGaeaId);
		return true;
	}

	void FieldRoleRuntime::ApplyAttackDamage(PeerPtr peer, ServerBatchEncoder& batcher,
	                                         const AttackResult& accepted)
	{
		// ---- resolve ---------------------------------------------------------
		//
		// Every roll is INJECTED. `DamageResolution` is pure and generates nothing;
		// legacy's RANDOM_POS (GLDefine.h:11) is produced once per strike in
		// PreStrikeProc (GLChar.cpp:2416) and here by the role's roll source.
		DamageInput input;
		input.attackerGaeaId = accepted.attackerGaeaId;
		input.targetGaeaId   = accepted.targetId;
		input.targetCrow     = accepted.targetCrow;

		input.hitRoll        = NextRoll();
		input.damageRoll     = NextRoll();
		input.criticalRoll   = NextRoll();
		input.crushingRoll   = NextRoll();
		input.reflectionRoll = NextRoll();

		// PROTOTYPE statistics. See DamageResolution.h: legacy's are item-derived
		// and the modern Field path has none, so these are placeholders and NOT
		// recovered RAN values. Only the SHAPE is right.
		input.stats.lowDamage  = kPrototypeLowDamage;
		input.stats.highDamage = kPrototypeHighDamage;
		input.stats.hit        = kPrototypeHit;
		input.stats.avoid      = kPrototypeAvoid;

		DamageResult damage = DamageResolution::Resolve(/*attackerPresent=*/true,
		                                                /*targetPresent=*/true, input);

		if (damage.outcome == DamageOutcome::Avoided)
		{
			// A miss is the SAME wire event as an out-of-range attack: legacy's
			// AvoidProc sends 3041/3042, not a damage packet. Reused deliberately.
			AttackResult avoided = accepted;
			avoided.detail = "the attack missed";
			(void) SendAttackAvoid(peer, batcher, avoided);
			return;
		}

		if (damage.outcome != DamageOutcome::Hit)
		{
			// Refused: nothing goes on the wire at all.
			m_attackDamageRefused.fetch_add(1, std::memory_order_relaxed);
			return;
		}

		// ---- apply -----------------------------------------------------------
		//
		// The APPLIED amount, not the requested one.
		//
		// `ApplyDamage` returns GLCHARLOGIC::RECEIVE_DAMAGE's difference actually
		// lost (GLogixExPC.cpp:2093) and returns 0 when the target is already dead
		// (ResourceSyncService.cpp:373). Both matter: an overkill must be reported
		// as the HP that actually went, and a dead target must produce NO damage
		// packet at all.
		const Network::WireU32 applied =
		    m_resources.ApplyDamage(accepted.targetId, damage.requestedDamage);

		if (applied == 0)
		{
			m_attackDamageRefused.fetch_add(1, std::memory_order_relaxed);
			return;
		}

		damage.appliedDamage = applied;

		// ---- 3043 to the attacker -------------------------------------------
		//
		// The attacker gets its OWN result, synchronously. 3044 below goes to
		// everyone else.
		Network::Attack::AttackDamage toAttacker;
		toAttacker.targetCrow = accepted.targetCrow;
		toAttacker.targetId   = accepted.targetId;
		toAttacker.damage     = static_cast<Network::WireI32>(applied);
		toAttacker.damageFlag = damage.damageFlag;

		std::vector<WireU8> damagePacket;
		if (const Status status =
		        Network::Attack::AttackDamageCodec::AppendAttackDamage(damagePacket,
		                                                                toAttacker);
		    status.IsError())
		{
			m_refusal.store(FieldRefusal::SendFailed, std::memory_order_release);
			m_refusalDetail = "could not encode 3043";
			return;
		}

		if (const Status sent = SendEnveloped(batcher, peer, damagePacket); sent.IsError())
		{
			m_refusal.store(FieldRefusal::SendFailed, std::memory_order_release);
			m_refusalDetail = "send failed: ";
			m_refusalDetail += sent.GetMessage();
			return;
		}

		// ---- 3044 to everyone else ------------------------------------------
		//
		// The ATTACKER is excluded, not the victim.
		//
		// DamageProc sends the 3043 to m_dwClientID - the attacker's own
		// connection - and the 3044 with SendMsgViewAround, which excludes the
		// actor (GLChar.cpp:2526-2540). So the victim DOES receive the damage
		// broadcast; that is how it sees the number land on its own character. The
		// attacker does not, because it already has the 3043.
		//
		// Excluding the victim instead leaves a two-player world in which nobody
		// ever receives a 3044 - which is exactly the wrong guess this test caught.
		Network::Attack::AttackDamageBroadcast toView;
		toView.gaeaId     = accepted.attackerGaeaId;
		toView.targetCrow = accepted.targetCrow;
		toView.targetId   = accepted.targetId;
		toView.damage     = static_cast<Network::WireI32>(applied);
		toView.damageFlag = damage.damageFlag;

		BroadcastAttackDamage(peer, toView);

		m_attackDamageSent.fetch_add(1, std::memory_order_relaxed);
		Emit(FieldEvent::AttackDamageSent, damage.detail, accepted.targetId);
	}

	void FieldRoleRuntime::BroadcastAttackDamage(
	    const PeerPtr& exclude, const Network::Attack::AttackDamageBroadcast& broadcast)
	{
		std::vector<WireU8> packet;
		if (const Status status = Network::Attack::AttackDamageCodec::AppendAttackDamageBroadcast(
		        packet, broadcast);
		    status.IsError())
		{
			return;
		}

		std::vector<PeerPtr> targets;
		{
			const std::lock_guard<std::mutex> lock(m_peersMutex);
			targets = m_peers;
		}

		for (const PeerPtr& peer : targets)
		{
			if (peer == exclude)
			{
				continue;
			}

			if (!peer->spawned.load(std::memory_order_acquire))
			{
				continue;
			}

			ServerBatchEncoder batcher(peer->codec);
			(void)SendEnveloped(batcher, peer, packet);
		}
	}

	float FieldRoleRuntime::NextRoll() const noexcept
	{
		// The roll source is a SEAM, not a generator.
		//
		// Legacy rolls `rand()/RAND_MAX` per strike (GLDefine.h:11). Nothing in the
		// modern server generates randomness yet, and wiring an unseeded one in
		// would make every TCP test that involves an attack a coin flip - a
		// refused-then-retried suite is worse than no suite. So the default source
		// returns a fixed value that always HITS (a hit rate of 20-99 always
		// exceeds roll*100 for roll 0), the boundary is exercised end to end, and
		// the seedable generator is a later milestone.
		return m_rollSource ? m_rollSource() : 0.0f;
	}

	Status FieldRoleRuntime::StartMovementTicker()
	{
		return m_movementWorld.StartTicker();
	}

void FieldRoleRuntime::StopMovementTicker() noexcept
{
	m_movementWorld.StopTicker();
}

// WORLD-ENTRY-002h: resource recovery ticker - mirrors the movement ticker
// pattern. Idempotent start.
Status FieldRoleRuntime::StartResourceTicker()
{
	if (m_resourceTickerRunning.exchange(true, std::memory_order_acq_rel))
	{
		// Refused rather than silently ignored: a second Start would be a second
		// thread advancing the same resource pools, and two threads advancing
		// the same state is a data race with no correct outcome.
		return Status(ErrorCode::AlreadyExists);
	}

	m_stopResourceTicker.store(false, std::memory_order_release);
	m_resourceTickerThread = std::thread([this] { ResourceTickerLoop(); });
	return Ok();
}

void FieldRoleRuntime::StopResourceTicker() noexcept
{
	if (!m_resourceTickerRunning.exchange(false, std::memory_order_acq_rel))
	{
		return;
	}

	m_stopResourceTicker.store(true, std::memory_order_release);
	if (m_resourceTickerThread.joinable())
	{
		m_resourceTickerThread.join();
	}
}

void FieldRoleRuntime::ResourceTickerLoop()
{
	auto previous = std::chrono::steady_clock::now();

	while (!m_stopResourceTicker.load(std::memory_order_acquire))
	{
		// MEASURED, not assumed. The delta between two real instants drives
		// recovery. A character at max HP recovers 0; a character at half HP
		// recovers proportional to elapsed time. This loop runs at a fixed
		// slice rate, not as fast as possible, which is the same design as
		// the movement ticker.
		std::this_thread::sleep_for(std::chrono::milliseconds(kTickerSliceMilliseconds));

		const auto now = std::chrono::steady_clock::now();

		float elapsed = std::chrono::duration<float>(now - previous).count();
		previous = now;

		if (elapsed < 0.0f)
		{
			// Not reachable with a monotonic clock, and handled rather than passed
			// on: a negative delta would subtract from pools.
			elapsed = 0.0f;
		}
		if (elapsed > kMaximumTickSeconds)
		{
			elapsed = kMaximumTickSeconds;
		}

		if (elapsed > 0.0f)
		{
			(void)m_resources.Advance(elapsed);
		}
	}
}

void FieldRoleRuntime::BroadcastMoveState(const PeerPtr& exclude,
	                                          const MovementState::MoveStateBroadcast& broadcast)
{
	std::vector<WireU8> packet;
	if (const Status status =
	        MovementState::MovementStateCodec::AppendMoveStateBroadcast(packet, broadcast);
	    status.IsError())
	{
		// Already validated a moment ago by the caller, so this cannot fail; refused
		// rather than ignored so a future change that breaks it is visible.
		return;
	}

	// The peer list is COPIED under the lock and then used without it.
	//
	// Holding m_peersMutex across a send would mean one slow client's broadcast
	// stalled every other connection's replies AND its own disconnect handling. The
	// copy is a vector of shared_ptr, so a peer that disconnects mid-broadcast simply
	// fails its send and is dropped.
	std::vector<PeerPtr> targets;
	{
		const std::lock_guard<std::mutex> lock(m_peersMutex);
		targets = m_peers;
	}

	for (const PeerPtr& peer : targets)
	{
		if (peer == exclude)
		{
			continue;
		}

		// Only sessions that have earned a spawn receive movement. A connection that
		// is mid-handshake has no character in the world to have moved.
		if (!peer->spawned.load(std::memory_order_acquire))
		{
			continue;
		}

		// Wrapped in an envelope like every other server->client message, because
		// NetCompressCodec is the established path and a second envelope format
		// would be the duplication this milestone must avoid.
		ServerBatchEncoder batcher(peer->codec);
		(void)SendEnveloped(batcher, peer, packet);
	}
}

// WORLD-ENTRY-002h: broadcasts a 3053 (StateBroadcast) to all authorized peers
// except `exclude`. Mirrors BroadcastMoveState exactly.
void FieldRoleRuntime::BroadcastResourceState(const PeerPtr& exclude,
	                                          const std::vector<WireU8>& packet)
{
	std::vector<PeerPtr> targets;
	{
		const std::lock_guard<std::mutex> lock(m_peersMutex);
		targets = m_peers;
	}

	for (const PeerPtr& peer : targets)
	{
		if (peer == exclude)
		{
			continue;
		}

		if (!peer->spawned.load(std::memory_order_acquire))
		{
			continue;
		}

		ServerBatchEncoder batcher(peer->codec);
		(void)SendEnveloped(batcher, peer, packet);
	}
}

// WORLD-ENTRY-002h: writes the authoritative pools back to the repository.
	// Invoked from the resource sync service's write-back sink.
	void FieldRoleRuntime::WriteBackPools(WorldCharacterId characterId,
	                                        const Network::RanWire::DwPair& hp,
	                                        const Network::RanWire::DwPair& mp,
	                                        const Network::RanWire::DwPair& sp)
	{
		auto found = m_repository.Find(characterId);
		if (found.IsError())
		{
			return;
		}
		WorldCharacter record = found.GetValue();
		record.hp = hp;
		record.mp = mp;
		record.sp = sp;
		(void)m_repository.Replace(record);
	}

Status FieldRoleRuntime::SendEnveloped(ServerBatchEncoder& batcher, PeerPtr peer,
	                                       const std::vector<WireU8>& inner)
{
	// The per-peer codec is NOT internally synchronised (CompressionCodec.h:
	// "NOT internally synchronised; a server with concurrent senders should hold
	// one per thread"). Holding the peer's send mutex across the WHOLE encode-
	// and-send - not just the transport call - prevents the resource ticker's
	// broadcast from compressing into the same LZO work buffer the peer's own
	// worker thread is using at the same time.
	const std::lock_guard<std::mutex> lock(peer->sendMutex);

	std::vector<WireU8> pending;
	const BatchAction   action = batcher.Add(inner, pending);

	if (action == BatchAction::Buffered)
	{
		if (!batcher.Flush(pending))
		{
			return Status(ErrorCode::InvalidState);
		}
	}

	return peer->transport.Send(pending.data(), pending.size());
}
}

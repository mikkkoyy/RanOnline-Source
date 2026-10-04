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
	}

	const char* ToString(FieldEvent event) noexcept
	{
		switch (event)
		{
		case FieldEvent::Listening:           return "Listening";
		case FieldEvent::ClientConnected:     return "ClientConnected";
		case FieldEvent::IdentityAccepted:   return "IdentityAccepted";
		case FieldEvent::SpawnSent:          return "SpawnSent";
		case FieldEvent::MoveStateSent:      return "MoveStateSent";
		case FieldEvent::MoveStateUnchanged: return "MoveStateUnchanged";
		case FieldEvent::ClientRejected:      return "ClientRejected";
		case FieldEvent::ClientDisconnected: return "ClientDisconnected";
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
		, m_log(std::move(log))
	{
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
		return Ok();
	}

	void FieldRoleRuntime::Stop() noexcept
	{
		if (!m_listener.IsListening() && !m_acceptThread.joinable())
		{
			return;
		}

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

		if (const Status sent = SendEnveloped(batcher, peer, spawn.frame); sent.IsError())
		{
			m_refusal.store(FieldRefusal::SendFailed, std::memory_order_release);
			m_refusalDetail = "send failed: ";
			m_refusalDetail += sent.GetMessage();
			return false;
		}

		m_lastGaeaId.store(spawn.gaeaId, std::memory_order_release);
		m_lastCharacterId.store(spawn.character.id.value, std::memory_order_release);
		peer->spawned.store(true, std::memory_order_release);
		Emit(FieldEvent::SpawnSent, "", spawn.gaeaId);
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
		return true;
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

	Status FieldRoleRuntime::SendEnveloped(ServerBatchEncoder& batcher, PeerPtr peer,
	                                       const std::vector<WireU8>& inner)
	{
		// Same Add-then-Flush as the Agent role: a 16-byte 3033 is far below the
		// 1000-byte flush trigger, so Add buffers it and the frame must be flushed
		// explicitly. These are SENT, not held.
		std::vector<WireU8> pending;
		const BatchAction   action = batcher.Add(inner, pending);

		if (action == BatchAction::Buffered)
		{
			if (!batcher.Flush(pending))
			{
				return Status(ErrorCode::InvalidState);
			}
		}

		return SendRaw(peer, pending);
	}

	Status FieldRoleRuntime::SendRaw(PeerPtr peer, const std::vector<WireU8>& bytes)
	{
		const std::lock_guard<std::mutex> lock(peer->sendMutex);
		return peer->transport.Send(bytes.data(), bytes.size());
	}
}
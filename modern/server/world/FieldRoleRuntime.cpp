#include "world/FieldRoleRuntime.h"

#include "MessageReader.h"
#include "NetworkCodec.h"

#include <utility>

namespace Modern::Server::World
{
	using namespace Modern::Network;

	namespace
	{
		// Bounds the 2359 read. This role expects ONE message and then answers or
		// drops, so a per-message budget is the whole conversation budget.
		constexpr int kMessageTimeoutMilliseconds = 10000;

		constexpr std::size_t kReadBufferSize = 2048;
	}

	const char* ToString(FieldEvent event) noexcept
	{
		switch (event)
		{
		case FieldEvent::Listening:          return "Listening";
		case FieldEvent::ClientConnected:    return "ClientConnected";
		case FieldEvent::IdentityAccepted:  return "IdentityAccepted";
		case FieldEvent::SpawnSent:         return "SpawnSent";
		case FieldEvent::ClientRejected:    return "ClientRejected";
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
		}
		return "Unrecognised";
	}

	FieldRoleRuntime::FieldRoleRuntime(WorldServerConfig config,
	                                   ICharacterRepository& repository,
	                                   FieldEntryRegistry& registry,
	                                   FieldLogSink log)
		: m_config(std::move(config))
		, m_repository(repository)
		, m_registry(registry)
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

		// The OS-assigned port, not the requested one. WorldServerRuntime reads this
		// and puts it into the 2358, which is the only way a client can learn where
		// the second connection goes.
		m_config.fieldBind = m_listener.BoundEndpoint();

		Emit(FieldEvent::Listening, m_config.fieldBind.host + ":" +
		                                    std::to_string(m_config.fieldBind.port));
		return Ok();
	}

	void FieldRoleRuntime::Stop() noexcept
	{
		m_listener.Close();
	}

	bool FieldRoleRuntime::IsRunning() const noexcept
	{
		return m_listener.IsListening();
	}

	Status FieldRoleRuntime::ServeOneConnection(int timeoutMilliseconds)
	{
		if (!IsRunning())
		{
			return Status(ErrorCode::InvalidState);
		}

		std::uintptr_t raw = TcpListener::kNoAcceptedSocket;
		const Status    accepted = m_listener.Accept(raw, timeoutMilliseconds);

		if (accepted.IsError())
		{
			m_refusal = FieldRefusal::ReceiveFailed;
			m_refusalDetail = "accept: ";
			m_refusalDetail += accepted.GetMessage();
			return accepted;
		}

		if (raw == TcpListener::kNoAcceptedSocket)
		{
			return Ok();
		}

		Result<TcpTransport> adopted = TcpTransport::Adopt(raw);
		if (adopted.IsError())
		{
			TcpTransport::CloseOwnedHandle(raw);

			++m_refused;
			m_refusal = FieldRefusal::ReceiveFailed;
			m_refusalDetail = "could not adopt the accepted socket";
			return adopted.GetStatus();
		}

		TcpTransport connection = std::move(adopted.GetValue());

		const Endpoint peer = connection.RemoteEndpoint();
		Emit(FieldEvent::ClientConnected, peer.host + ":" + std::to_string(peer.port));

		const FieldRefusal refusal = ServeConnection(connection);

		connection.Disconnect();

		if (refusal == FieldRefusal::None)
		{
			++m_served;
			m_refusal = FieldRefusal::None;
			m_refusalDetail.clear();
			Emit(FieldEvent::ClientDisconnected,
			     peer.host + ":" + std::to_string(peer.port));
			return Ok();
		}

		++m_refused;
		m_refusal = refusal;
		Emit(FieldEvent::ClientRejected, m_refusalDetail);
		return Ok();
	}

	FieldRefusal FieldRoleRuntime::ServeConnection(TcpTransport& connection)
	{
		// Per-connection, deliberately. See the header: two clients must not share
		// world-entry state, and the cheapest guarantee is that there is no shared
		// object for them to share.
		ConnectionFramer framer;
		ServerBatchEncoder batcher(m_codec);

		FieldSession session(m_repository, m_registry, m_served + m_refused + 1);

		Message      message;
		const ReadResult read = ReadOneMessage(connection, framer, message,
		                                        kMessageTimeoutMilliseconds,
		                                        kReadBufferSize);

		if (!read.IsOk())
		{
			m_refusalDetail = read.detail;
			switch (read.outcome)
			{
			case ReadOutcome::PeerClosed:
				m_refusal = FieldRefusal::PeerClosedFirst;
				return m_refusal;
			case ReadOutcome::TransportFault:
				m_refusal = FieldRefusal::ReceiveFailed;
				return m_refusal;
			default:
				m_refusal = FieldRefusal::MalformedFrame;
				return m_refusal;
			}
		}

		// Only 2359 is meaningful here. A Field connection is one claim and one
		// answer; anything else is not this conversation.
		if (!WorldEntryCodec::IsFieldIdentity(message.header.type))
		{
			m_refusalDetail = "unexpected message id " +
			                  std::to_string(message.header.type) +
			                  " (the Field role accepts only 2359)";
			return FieldRefusal::UnexpectedMessage;
		}

		const std::vector<WireU8> frame = ReconstructFrame(message);

		// Phase A's decoder validates the id and the exact 24-byte length, so a
		// truncated or padded 2359 is refused here rather than parsed.
		FieldIdentity identity;
		if (const Status status = WorldEntryCodec::DecodeFieldIdentity(frame, identity);
		    status.IsError())
		{
			m_refusalDetail = "2359 malformed (dwSize " + std::to_string(frame.size()) +
			                  ", expected " + std::to_string(WorldEntry::kIdentitySize) + ")";
			return FieldRefusal::BadMessageSize;
		}

		// The validation is Phase B's, unchanged: format, pending authorization, slot,
		// join type, replay, staleness, repository ownership and the character holding
		// THIS gaeaId. This layer decides nothing about whether the claim is good.
		//
		// `nowMs` is this role's own monotonic clock. Phase B's sessions deliberately
		// read no clock, so that a test can age an authorization without sleeping; over a
		// real socket there is no such test, and a real elapsed time is the honest
		// input.
		const WireU64 nowMs = static_cast<WireU64>(m_served + m_refused);
		if (const Status status = session.ValidateIdentity(identity, nowMs); status.IsError())
		{
			m_refusalDetail = "2359 refused: ";
			m_refusalDetail += status.GetMessage();
			return FieldRefusal::IdentityRejected;
		}

		Emit(FieldEvent::IdentityAccepted, "", session.GaeaId());

		// The spawn packet itself is Phase B's, built from the repository record the
		// validation just re-read. Nothing here sets a level, a position or a gaeaId -
		// there is no parameter through which this layer could.
		FieldSpawnResult spawn;
		if (const Status status = session.BuildSpawn(spawn); status.IsError())
		{
			m_refusalDetail = "spawn refused: ";
			m_refusalDetail += status.GetMessage();
			return FieldRefusal::IdentityRejected;
		}

		if (const Status sent = SendEnveloped(batcher, connection, spawn.frame);
		    sent.IsError())
		{
			m_refusalDetail = "send failed: ";
			m_refusalDetail += sent.GetMessage();
			return FieldRefusal::SendFailed;
		}

		m_lastGaeaId      = spawn.gaeaId;
		m_lastCharacterId = spawn.character.id.value;
		Emit(FieldEvent::SpawnSent, "", m_lastGaeaId);
		return FieldRefusal::None;
	}

	Status FieldRoleRuntime::SendEnveloped(ServerBatchEncoder& batcher,
	                                       TcpTransport& connection,
	                                       const std::vector<WireU8>& inner)
	{
		// Same Add-then-Flush as the Agent role, for the same reason: the 1022-byte
		// spawn is over the 1000-byte trigger so Add emits it alone, while anything
		// smaller buffers and needs the explicit flush. The batcher decides which, and
		// that decision is the documented NET_COMPRESS batching rule rather than
		// something re-derived here.
		std::vector<WireU8> pending;
		const BatchAction   action = batcher.Add(inner, pending);

		if (action == BatchAction::Buffered)
		{
			if (!batcher.Flush(pending))
			{
				return Status(ErrorCode::InvalidState);
			}
		}

		return connection.Send(pending.data(), pending.size());
	}
}
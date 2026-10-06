#include "world/WorldEntryConnections.h"

#include "NetworkCodec.h"

namespace Modern::Client
{
	namespace
	{
		// Read buffer for one receive. Larger than any response in this phase, so a
		// response usually arrives whole - and the fragmentation tests prove the framer
		// handles it when it does not.
		constexpr std::size_t kReadBufferSize = 2048;
	}

	// ---------------------------------------------------------------------------
	// AgentConnection
	// ---------------------------------------------------------------------------

	AgentConnection::~AgentConnection()
	{
		Disconnect();
	}

	Status AgentConnection::Connect(const Network::EndpointAddress& endpoint,
	                                 int timeoutMilliseconds)
	{
		Network::Endpoint target;
		target.host = endpoint.ip;
		target.port = endpoint.port;

		if (const Status status = m_transport.Connect(target, timeoutMilliseconds);
		    status.IsError())
		{
			return status;
		}
		return Ok();
	}

	Status AgentConnection::Send(const std::vector<Network::WireU8>& bytes)
	{
		if (!m_transport.IsConnected())
		{
			return Status(ErrorCode::InvalidState);
		}
		return m_transport.Send(bytes.data(), bytes.size());
	}

	Status AgentConnection::PumpUntil(WorldEntryPhase untilPhase, int timeoutMilliseconds,
	                                  std::size_t maxChunkBytes)
	{
		if (!m_transport.IsConnected())
		{
			return Status(ErrorCode::InvalidState);
		}

		const std::size_t chunk =
		    (maxChunkBytes == 0 || maxChunkBytes > kReadBufferSize) ? kReadBufferSize
		                                                             : maxChunkBytes;

		const auto deadline = std::chrono::steady_clock::now() +
		                     std::chrono::milliseconds(timeoutMilliseconds > 0
		                                                  ? timeoutMilliseconds
		                                                  : 1);

		for (;;)
		{
			// The termination condition is the CALLER's, which is what makes a hang
			// impossible: this loop cannot spin forever waiting for something it decided
			// to wait for.
			if (m_protocol.Phase() == untilPhase)
			{
				return Ok();
			}

			const auto   now = std::chrono::steady_clock::now();
			if (now >= deadline)
			{
				return Status(ErrorCode::NotFound); // "not yet", reported as a timeout
			}

			const auto left =
			    std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();
			if (left <= 0)
			{
				return Status(ErrorCode::NotFound);
			}

			const int slice = left < 250 ? static_cast<int>(left) : 250;

			std::vector<Network::WireU8> buffer(chunk);
			std::size_t                  received = 0;

			const Status status =
			    m_transport.Receive(buffer.data(), buffer.size(), received, slice);

			if (status.IsError())
			{
				// A fault is reported as itself. A peer that closed is NOT a fault: the
				// Agent role closes after the 2358 (see AgentRoleRuntime.h), so an
				// orderly close is the normal end of this conversation.
				if (m_transport.Fault() == Network::TransportFault::PeerClosed)
				{
					return Ok();
				}
				return status;
			}

			if (received == 0)
			{
				continue;
			}

			std::size_t               handled = 0;
			const Status              fed =
			    m_protocol.FeedAgent(buffer.data(), received, handled);
			m_handled += handled;

			if (fed.IsError())
			{
				return fed;
			}
		}
	}

	void AgentConnection::Disconnect() noexcept
	{
		m_transport.Disconnect();
	}

	// ---------------------------------------------------------------------------
	// FieldConnection
	// ---------------------------------------------------------------------------

	FieldConnection::~FieldConnection()
	{
		Disconnect();
	}

	Status FieldConnection::Connect(const Network::FieldRedirect& redirect,
	                                int connectTimeout)
	{
		// The endpoint is the SERVER's, taken verbatim from the 2358. There is no
		// fallback and no default: if the packet did not name an endpoint, this fails,
		// and a client that could connect without a redirect would prove nothing about
		// 2358.
		if (redirect.fieldIp.empty() || redirect.servicePort <= 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		Network::Endpoint target;
		target.host = redirect.fieldIp;
		target.port = static_cast<Network::WireU16>(redirect.servicePort);

		return m_transport.Connect(target, connectTimeout);
	}

	Status FieldConnection::SendIdentity(const Network::FieldIdentity& identity)
	{
		if (!m_transport.IsConnected())
		{
			return Status(ErrorCode::InvalidState);
		}

		std::vector<Network::WireU8> request;
		if (const Status status = WorldEntryClient::BuildFieldIdentity(identity, request);
		    status.IsError())
		{
			return status;
		}
		return SendRaw(request);
	}

	Status FieldConnection::SendRaw(const std::vector<Network::WireU8>& bytes)
	{
		return m_transport.Send(bytes.data(), bytes.size());
	}

	Status FieldConnection::SendMoveState(Network::WireU32 actState)
	{
		if (!m_transport.IsConnected())
		{
			return Status(ErrorCode::InvalidState);
		}

		std::vector<Network::WireU8> request;
		if (const Status status = m_protocol.BuildMoveState(actState, request);
		    status.IsError())
		{
			return status;
		}

		// Raw, and for the same reason SendIdentity is: client -> server is never
		// enveloped. Wrapping a 3032 would give the server's framer bytes it could not
		// read, and the resulting failure would look like a framing bug in Phase C's
		// code rather than like a wrong choice here.
		return m_transport.Send(request.data(), request.size());
	}

	Status FieldConnection::PumpUntilMoveCount(std::size_t wantedCount,
	                                           int         timeoutMilliseconds,
	                                           std::size_t maxChunkBytes)
	{
		if (!m_transport.IsConnected())
		{
			return Status(ErrorCode::InvalidState);
		}

		// Already satisfied before a single byte is read.
		//
		// Without this a caller asking for a count it has already reached would block
		// for the whole budget and then report NotFound - which is indistinguishable
		// from the server having stayed silent. Both are "no", and a caller that asked
		// for nothing must be told "yes" immediately.
		if (m_protocol.MoveStateCount() >= wantedCount)
		{
			return Ok();
		}

		const std::size_t chunk =
		    (maxChunkBytes == 0 || maxChunkBytes > kReadBufferSize) ? kReadBufferSize
		                                                             : maxChunkBytes;

		const auto deadline = std::chrono::steady_clock::now() +
		                     std::chrono::milliseconds(timeoutMilliseconds > 0
		                                                  ? timeoutMilliseconds
		                                                  : 1);

		for (;;)
		{
			if (m_protocol.MoveStateCount() >= wantedCount)
			{
				return Ok();
			}

			const auto   now = std::chrono::steady_clock::now();
			if (now >= deadline)
			{
				return Status(ErrorCode::NotFound);
			}

			const auto left =
			    std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();
			if (left <= 0)
			{
				return Status(ErrorCode::NotFound);
			}

			const int slice = left < 250 ? static_cast<int>(left) : 250;

			std::vector<Network::WireU8> buffer(chunk);
			std::size_t                  received = 0;

			const Status status =
			    m_transport.Receive(buffer.data(), buffer.size(), received, slice);

			if (status.IsError())
			{
				// A peer that closed before delivering the 3033 is NotFound here, not a
				// fault: "the answer never arrived" is the fact the caller needs, and it
				// is the same fact a timeout reports. Matching PumpUntilSpawn's handling
				// of the same event keeps the two pumps comparable.
				return status;
			}

			if (received == 0)
			{
				continue;
			}

			std::size_t handled = 0;
			if (const Status fed = m_protocol.FeedField(buffer.data(), received, handled);
			    fed.IsError())
			{
				m_handled += handled;
				return fed;
			}
			m_handled += handled;
		}
	}

Status FieldConnection::SendGoto(WireU32 requestedActState, Vector3 claimedCurrent,
	                                 Vector3 requestedTarget)
	{
		if (!m_transport.IsConnected())
		{
			return Status(ErrorCode::InvalidState);
		}

		std::vector<Network::WireU8> request;
		if (const Status status =
		        m_protocol.BuildGoto(requestedActState, claimedCurrent, requestedTarget, request);
		    status.IsError())
		{
			return status;
		}

		// Raw, and for the same reason SendIdentity is: client -> server is never
		// enveloped. Wrapping a 3034 would give the server's framer bytes it could not
		// read, and the resulting failure would look like a framing bug rather than like
		// a wrong choice here.
		return m_transport.Send(request.data(), request.size());
	}

	Status FieldConnection::PumpUntilGotoCount(std::size_t wantedCount, int timeoutMilliseconds,
	                                          std::size_t maxChunkBytes)
	{
		if (!m_transport.IsConnected())
		{
			return Status(ErrorCode::InvalidState);
		}

		// Already satisfied before a single byte is read. Without this a caller asking
		// for a count it has already reached would block for the whole budget and then
		// report NotFound - indistinguishable from the server having stayed silent, and
		// both are "no".
		if (m_protocol.GotoCount() >= wantedCount)
		{
			return Ok();
		}

		const std::size_t chunk =
		    (maxChunkBytes == 0 || maxChunkBytes > kReadBufferSize) ? kReadBufferSize
		                                                             : maxChunkBytes;

		const auto deadline = std::chrono::steady_clock::now() +
		                     std::chrono::milliseconds(timeoutMilliseconds > 0
		                                                  ? timeoutMilliseconds
		                                                  : 1);

		for (;;)
		{
			if (m_protocol.GotoCount() >= wantedCount)
			{
				return Ok();
			}

			const auto now = std::chrono::steady_clock::now();
			if (now >= deadline)
			{
				return Status(ErrorCode::NotFound);
			}

			const auto left =
			    std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();
			if (left <= 0)
			{
				return Status(ErrorCode::NotFound);
			}

			const int slice = left < 250 ? static_cast<int>(left) : 250;

			std::vector<Network::WireU8> buffer(chunk);
			std::size_t                  received = 0;

			const Status status =
			    m_transport.Receive(buffer.data(), buffer.size(), received, slice);

			if (status.IsError())
			{
				// "The answer never arrived" is the fact the caller needs, and a peer that
				// closed before delivering is the same fact as a timeout. Matching
				// PumpUntilMoveCount's handling keeps the two pumps comparable.
				return status;
			}

			if (received == 0)
			{
				continue;
			}

			std::size_t handled = 0;
			if (const Status fed = m_protocol.FeedField(buffer.data(), received, handled);
			    fed.IsError())
			{
				m_handled += handled;
				return fed;
			}
			m_handled += handled;
		}
	}

	Status FieldConnection::PumpUntilSpawn(int timeoutMilliseconds,
	                                       std::size_t maxChunkBytes)
	{
		if (!m_transport.IsConnected())
		{
			return Status(ErrorCode::InvalidState);
		}

		const std::size_t chunk =
		    (maxChunkBytes == 0 || maxChunkBytes > kReadBufferSize) ? kReadBufferSize
		                                                             : maxChunkBytes;

		const auto deadline = std::chrono::steady_clock::now() +
		                     std::chrono::milliseconds(timeoutMilliseconds > 0
		                                                  ? timeoutMilliseconds
		                                                  : 1);

		for (;;)
		{
			if (m_protocol.Spawn().received)
			{
				return Ok();
			}

			const auto   now = std::chrono::steady_clock::now();
			if (now >= deadline)
			{
				return Status(ErrorCode::NotFound);
			}

			const auto left =
			    std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();
			if (left <= 0)
			{
				return Status(ErrorCode::NotFound);
			}

			const int slice = left < 250 ? static_cast<int>(left) : 250;

			std::vector<Network::WireU8> buffer(chunk);
			std::size_t                  received = 0;

			const Status status =
			    m_transport.Receive(buffer.data(), buffer.size(), received, slice);

			if (status.IsError())
			{
				// The Field role closes after the 2333, so an orderly close here is the
				// normal end - not a fault.
				if (m_transport.Fault() == Network::TransportFault::PeerClosed)
				{
					return m_protocol.Spawn().received ? Ok()
					                                   : Status(ErrorCode::NotFound);
				}
				return status;
			}

			if (received == 0)
			{
				continue;
			}

			std::size_t  handled = 0;
			const Status fed     = m_protocol.FeedField(buffer.data(), received, handled);
			m_handled += handled;

			if (fed.IsError())
			{
				return fed;
			}
		}
	}

	void FieldConnection::Disconnect() noexcept
	{
		m_transport.Disconnect();
	}
}

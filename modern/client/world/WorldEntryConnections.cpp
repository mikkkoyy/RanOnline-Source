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
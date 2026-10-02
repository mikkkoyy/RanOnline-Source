#include "login/LoginServerClient.h"

namespace Modern::Client
{
	using namespace Modern::Network;

	const char* ToString(LoginServerPhase phase) noexcept
	{
		switch (phase)
		{
		case LoginServerPhase::Disconnected:          return "Disconnected";
		case LoginServerPhase::ConnectingLoginServer: return "ConnectingLoginServer";
		case LoginServerPhase::LoginServerConnected:  return "LoginServerConnected";
		case LoginServerPhase::RequestingGameServers: return "RequestingGameServers";
		case LoginServerPhase::ReceivingGameServers:  return "ReceivingGameServers";
		case LoginServerPhase::GameServersReady:      return "GameServersReady";
		}
		return "Unrecognised";
	}

	void LoginServerClient::Reset() noexcept
	{
		m_framer.Reset();
		m_servers.Clear();
		m_endpoint = EndpointAddress{};
		m_phase  = LoginServerPhase::Disconnected;
		m_dropped = 0;
	}

	Status LoginServerClient::BeginConnect(const EndpointAddress& endpoint)
	{
		// Validated before any state changes, so a refused endpoint cannot leave the
		// client half-connected.
		if (!EndpointAddress::IsNumericIPv4(endpoint.ip))
		{
			return Status(ErrorCode::InvalidArgument);
		}
		if (endpoint.port == 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// Any previous exchange is abandoned, as legacy's CloseConnect-before-connect
		// does (s_NetClient.cpp:367-372).
		Reset();

		m_endpoint = endpoint;
		m_phase = LoginServerPhase::ConnectingLoginServer;
		return Ok();
	}

	Status LoginServerClient::CompleteConnect()
	{
		if (m_phase != LoginServerPhase::ConnectingLoginServer)
		{
			return Status(ErrorCode::InvalidState);
		}
		m_phase = LoginServerPhase::LoginServerConnected;
		return Ok();
	}

	Status LoginServerClient::Disconnect()
	{
		Reset();
		return Ok();
	}

	Status LoginServerClient::RequestGameServers(std::vector<WireU8>& request)
	{
		if (m_phase != LoginServerPhase::LoginServerConnected &&
		    m_phase != LoginServerPhase::RequestingGameServers &&
		    m_phase != LoginServerPhase::ReceivingGameServers &&
		    m_phase != LoginServerPhase::GameServersReady)
		{
			// Legacy can send the request as soon as the Login connection exists;
			// asking before that has no meaning to the protocol.
			return Status(ErrorCode::InvalidState);
		}

		// Clear before sending.
		//
		// SndReqServerInfo zeroes m_sGame and sets m_bGameServerInfoEnd = FALSE in the
		// same breath as sending (s_NetClientMsg.cpp:316-326). Doing it here, before
		// the request bytes are handed over, means a re-request cannot be answered
		// by the previous list even if the caller drops the response.
		m_servers.Clear();
		m_dropped = 0;
		m_framer.Reset();

		request.clear();
		if (const Status status = GameServerListCodec::AppendRequest(request); status.IsError())
		{
			return status;
		}

		m_phase = LoginServerPhase::RequestingGameServers;
		return Ok();
	}

	Status LoginServerClient::Feed(const WireU8* data, std::size_t size,
	                               std::size_t& messagesHandled)
	{
		messagesHandled = 0;

		if (data == nullptr && size != 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		if (m_phase != LoginServerPhase::RequestingGameServers &&
		    m_phase != LoginServerPhase::ReceivingGameServers)
		{
			// Nothing should arrive before a request was made. Legacy's dispatcher is
			// state-gated the same way (s_NetClientMsg.cpp:28-44), so accepting this
			// would let a server talk a client into a state it never asked for.
			return Status(ErrorCode::InvalidState);
		}

		// Feed in slices bounded by the framer's remaining room, draining complete
		// messages between them.
		//
		// Capacity limits BUFFERED PARTIAL DATA, not stream length: a full
		// 200-entry response is 11208 bytes and is perfectly legal, but handing all
		// of it to Feed() in one call would exceed a 2048-byte capacity and be
		// refused as Oversized. Slicing keeps a large burst - or a loopback
		// transport that delivers the whole response at once - working, and it
		// reuses the existing framer rather than adding a second parser.
		std::size_t offset = 0;
		while (offset < size)
		{
			const std::size_t room      = m_framer.Capacity() - m_framer.Buffered();
			const std::size_t remaining = size - offset;
			const std::size_t slice     = remaining < room ? remaining : room;

			if (slice == 0)
			{
				// No room and nothing drainable: the framer holds a partial message
				// larger than its own capacity, which Feed() would already have
				// refused. Treated as corruption rather than looped on.
				return Status(ErrorCode::InvalidArgument);
			}

			if (const FrameStatus fed = m_framer.Feed(data + offset, slice); fed != FrameStatus::Ok)
			{
				return Status(ErrorCode::InvalidArgument);
			}
			offset += slice;

			if (const Status status = DrainMessages(messagesHandled); status.IsError())
			{
				return status;
			}
		}

		return DrainMessages(messagesHandled);
	}

	Status LoginServerClient::DrainMessages(std::size_t& messagesHandled)
	{
		// ConnectionFramer owns message boundaries and survives arbitrary TCP
		// fragmentation. No second framing parser here.
		Message message;
		while (m_framer.Next(message) == FrameStatus::Ok)
		{
			++messagesHandled;

			// The frame as bytes, header included: every decoder validates dwSize and
			// nType, and both live in the header that Message splits out.
			std::vector<WireU8> frame;
			frame.reserve(static_cast<std::size_t>(message.header.size));
			Codec::WriteU32(frame, message.header.size);
			Codec::WriteU32(frame, message.header.type);
			frame.insert(frame.end(), message.payload.begin(), message.payload.end());

			if (GameServerListCodec::IsEntry(message.header.type))
			{
				GameServerInfo info;
				if (const Status status = GameServerListCodec::DecodeEntry(frame, info);
				    status.IsError())
				{
					// A malformed ENTRY is fatal, unlike an out-of-range one.
					//
					// The distinction is legacy's: a well-formed entry that simply
					// cannot be placed is skipped (s_NetClientMsg.cpp:199-203), whereas
					// bytes that do not decode to a valid entry are corruption, and
					// continuing would mean guessing at a server's address.
					return status;
				}

				// Out-of-grid entries are counted, not fatal - the proven behaviour.
				if (m_servers.Add(info).IsError())
				{
					++m_dropped;
				}

				m_phase = LoginServerPhase::ReceivingGameServers;
				continue;
			}

			MessageId bareId = 0;
			if (const Status status = GameServerListCodec::DecodeBare(frame, bareId);
			    status.IsError())
			{
				// Neither a valid entry nor a valid bare message: corrupt.
				return status;
			}

			if (GameServerListCodec::IsListEnd(bareId))
			{
				// The terminator completes the list. It is the ONLY completion signal
				// - the response carries no count - so this is what moves the client
				// out of "still arriving", including for an empty list.
				m_phase = LoginServerPhase::GameServersReady;
			}
			// A REQ_GAME_SVR arriving inbound is not part of the response and is
			// ignored, as MessageProcessLogin's default would.
		}

		if (m_framer.IsFailed())
		{
			return Status(ErrorCode::InvalidArgument);
		}
		return Ok();
	}
}
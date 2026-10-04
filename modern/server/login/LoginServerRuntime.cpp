#include "LoginServerRuntime.h"

#include "MessageReader.h"

namespace Modern::Server
{
	using namespace Modern::Network;

	namespace
	{
		// How long one exchange may take once a connection exists.
		//
		// An upper bound on the WHOLE exchange, not per read: a client that dribbles
		// one byte at a time must not be able to hold a connection open indefinitely by
		// never quite hitting an idle timeout. Every blocking call is given a slice of
		// this budget and the loop checks what is left.
		//
		// The loop itself now lives in Network::ReadOneMessage, shared with the Agent
		// and Field roles; this constant is the LOGIN-002 budget handed to it.
		constexpr int kExchangeTimeoutMilliseconds = 10000;
	}

	const char* ToString(LoginServerEvent event) noexcept
	{
		switch (event)
		{
		case LoginServerEvent::Listening:         return "Listening";
		case LoginServerEvent::ClientConnected:   return "ClientConnected";
		case LoginServerEvent::RequestReceived:   return "RequestReceived";
		case LoginServerEvent::ListSent:          return "ListSent";
		case LoginServerEvent::ClientRejected:    return "ClientRejected";
		case LoginServerEvent::ClientDisconnected: return "ClientDisconnected";
		}
		return "Unrecognised";
	}

	const char* ToString(LoginServerRefusal refusal) noexcept
	{
		switch (refusal)
		{
		case LoginServerRefusal::None:              return "None";
		case LoginServerRefusal::UnexpectedMessage: return "UnexpectedMessage";
		case LoginServerRefusal::BadRequestSize:    return "BadRequestSize";
		case LoginServerRefusal::MalformedFrame:    return "MalformedFrame";
		case LoginServerRefusal::PeerClosedFirst:   return "PeerClosedFirst";
		case LoginServerRefusal::SendFailed:        return "SendFailed";
		case LoginServerRefusal::ReceiveFailed:     return "ReceiveFailed";
		}
		return "Unrecognised";
	}

	LoginServerRuntime::LoginServerRuntime(LoginServerConfig config, LoginServerLogSink log)
		: m_config(std::move(config)),
		  m_log(std::move(log))
	{
	}

	LoginServerRuntime::~LoginServerRuntime()
	{
		Stop();
	}

	void LoginServerRuntime::Emit(LoginServerEvent event, std::string text, std::size_t count)
	{
		// The sink is called inside the connection's own handling, so a caller that
		// throws from here would unwind through socket code. This tree reports failure
		// by value and does not use exceptions for control flow, so the sink is given
		// the same contract as everything else and is simply not required to throw.
		if (m_log)
		{
			LoginServerLogEntry entry;
			entry.event = event;
			entry.count = count;
			entry.text  = std::move(text);
			m_log(entry);
		}
	}

	Status LoginServerRuntime::Start()
	{
		// Validated before a socket exists, so a bad configuration is reported as a bad
		// configuration rather than as whatever the socket layer made of it.
		if (const Status status = m_config.Validate(); status.IsError())
		{
			m_refusalDetail = "invalid configuration";
			return status;
		}

		if (const Status status = m_listener.Listen(m_config.bind); status.IsError())
		{
			m_refusalDetail = "bind/listen failed: ";
			m_refusalDetail += status.GetMessage();
			return status;
		}

		// The address the OS actually assigned, not the one that was asked for. With
		// port 0 the two differ and only this one can be connected to.
		m_config.bind = m_listener.BoundEndpoint();

		Emit(LoginServerEvent::Listening, m_config.bind.host + ":" +
		                                     std::to_string(m_config.bind.port));
		return Ok();
	}

	void LoginServerRuntime::Stop() noexcept
	{
		m_listener.Close();
	}

	bool LoginServerRuntime::IsRunning() const noexcept
	{
		return m_listener.IsListening();
	}

	Endpoint LoginServerRuntime::BoundEndpoint() const noexcept
	{
		return m_listener.BoundEndpoint();
	}

	Status LoginServerRuntime::ServeOneClient(int timeoutMilliseconds)
	{
		if (!IsRunning())
		{
			return Status(ErrorCode::InvalidState);
		}

		std::uintptr_t raw = TcpListener::kNoAcceptedSocket;
		const Status    accepted = m_listener.Accept(raw, timeoutMilliseconds);

		if (accepted.IsError())
		{
			m_refusal = LoginServerRefusal::ReceiveFailed;
			m_refusalDetail = "accept: ";
			m_refusalDetail += accepted.GetMessage();
			return accepted;
		}

		if (raw == TcpListener::kNoAcceptedSocket)
		{
			// Nobody knocked. Not a failure, and not a refusal either: nothing was
			// received and nothing was rejected. An accept loop must be able to spin on
			// this without inventing an error for every idle pass.
			return Ok();
		}

		Result<TcpTransport> adopted = TcpTransport::Adopt(raw);
		if (adopted.IsError())
		{
			// Adopt does not close the handle when it fails - that is deliberate, so
			// that the caller keeps the decision - which makes closing it here this
			// function's job. Without that, a socket would leak once per occurrence,
			// silently, which is the worst shape a leak can have.
			TcpTransport::CloseOwnedHandle(raw);

			m_refused++;
			m_refusal = LoginServerRefusal::ReceiveFailed;
			m_refusalDetail = "could not adopt the accepted socket";
			return adopted.GetStatus();
		}

		TcpTransport connection = std::move(adopted.GetValue());

		const Endpoint peer = connection.RemoteEndpoint();
		Emit(LoginServerEvent::ClientConnected, peer.host + ":" + std::to_string(peer.port));

		const LoginServerRefusal refusal = ServeConnection(connection);

		// Closed here rather than left to the transport's destructor so that the
		// ClientDisconnected event and the actual close are adjacent, and so the peer
		// observes the close before ServeOneClient returns.
		connection.Disconnect();

		if (refusal == LoginServerRefusal::None)
		{
			++m_served;
			m_refusal = LoginServerRefusal::None;
			m_refusalDetail.clear();
			Emit(LoginServerEvent::ClientDisconnected, peer.host + ":" + std::to_string(peer.port));
			return Ok();
		}

		++m_refused;
		m_refusal = refusal;
		Emit(LoginServerEvent::ClientRejected, m_refusalDetail);
		// Still Ok. The server's job on receiving a malformed packet is to reject it
		// and stay up; reporting an error here would make a caller that is only
		// serving clients treat a hostile client as a server failure.
		return Ok();
	}

	Status LoginServerRuntime::ServeClients(int count, int timeoutMilliseconds)
	{
		if (!IsRunning())
		{
			return Status(ErrorCode::InvalidState);
		}

		for (int served = 0; served < count; ++served)
		{
			// One refused client still counts as a client handled: the loop must not
			// spin, and must not stop early either, or a hostile connection could end
			// the batch before the good ones arrive.
			const std::size_t before = ServedClientCount() + RefusedClientCount();
			if (const Status status = ServeOneClient(timeoutMilliseconds); status.IsError())
			{
				return status;
			}
			if (ServedClientCount() + RefusedClientCount() == before)
			{
				// The accept timed out with nobody knocking, so there is nothing left to
				// wait for within this call's contract.
				break;
			}
		}
		return Ok();
	}

	LoginServerRefusal LoginServerRuntime::ServeConnection(TcpTransport& connection)
	{
		// A framer per connection, created here and destroyed with it.
		//
		// Deliberately NOT a member. A framer outliving its connection could complete a
		// message out of bytes left over from a client that had already gone, which is
		// precisely the kind of state leak that is invisible in testing and obvious in
		// production. Recreating one per connection costs an allocation and removes the
		// entire class of question.
		ConnectionFramer framer;

		Message message;
		if (!ReadMessage(connection, framer, message))
		{
			// ReadMessage records the reason in m_refusalDetail; it knows more about
			// what went wrong than this frame can reconstruct.
			return m_refusal;
		}

		// The frame as bytes, header included. ValidateRequest and the bare decoders all
		// read dwSize and nType, and both live in the header that Message splits out -
		// so the header has to be written back to reconstruct the frame.
		std::vector<WireU8> frame = ReconstructFrame(message);

		// The id is checked BEFORE the size, so a wrong id reports as a wrong id rather
		// than as a length problem. Legacy's MsgProcess dispatches on nType alone
		// (s_CLoginServerMsg.cpp:28-38) and its default case ignores anything else,
		// which is the right shape: an unexpected message is not a protocol error, it is
		// simply not this conversation.
		if (!GameServerListCodec::IsRequest(message.header.type))
		{
			m_refusalDetail = "unexpected message id " +
			                  std::to_string(message.header.type);
			return LoginServerRefusal::UnexpectedMessage;
		}

		// Then the size. Legacy answers ANY message carrying this id regardless of
		// dwSize (s_CLoginServerMsg.cpp:36-38); GameServerListCodec::ValidateRequest is
		// modern hardening, and a request must be exactly the bare 8-byte header it has
		// always been.
		if (const Status status = GameServerListCodec::ValidateRequest(frame); status.IsError())
		{
			m_refusalDetail = "REQ_GAME_SVR with dwSize " +
			                  std::to_string(message.header.size) + " (expected 8)";
			return LoginServerRefusal::BadRequestSize;
		}

		Emit(LoginServerEvent::RequestReceived, "");

		// The response is built by LOGIN-001's responder, unchanged. No serialization
		// is duplicated here: the sparse-grid walk, the IsAdvertisable filter, the
		// group-then-channel order and the unconditional terminator all live in
		// LoginServerResponder::BuildResponse and were proven by LOGIN-001's tests.
		//
		// RAW, with no NET_COMPRESS envelope - see GameServerListProtocol.h, which
		// proves from CLoginServer::SendClient -> SendClient2 that the Login Server
		// never touches the compressing path.
		LoginServerResponder responder;
		std::vector<WireU8> response;
		if (const Status status = responder.BuildResponse(m_config.servers, response);
		    status.IsError())
		{
			m_refusalDetail = "could not build response: ";
			m_refusalDetail += status.GetMessage();
			return LoginServerRefusal::SendFailed;
		}

		if (const Status status = connection.Send(response.data(), response.size());
		    status.IsError())
		{
			m_refusalDetail = "send failed: ";
			m_refusalDetail += status.GetMessage();
			m_refusalDetail += " (fault ";
			m_refusalDetail += ToString(connection.Fault());
			m_refusalDetail += ")";
			return LoginServerRefusal::SendFailed;
		}

		// One event for the whole response, carrying the entry count. The terminator is
		// not a separate send - see the header - so reporting it separately would be
		// reporting an implementation detail as an event.
		Emit(LoginServerEvent::ListSent, "", responder.LastEntryCount());
		return LoginServerRefusal::None;
	}

	bool LoginServerRuntime::ReadMessage(TcpTransport& connection, ConnectionFramer& framer,
	                                     Message& out)
	{
		// The loop itself is Network::ReadOneMessage's, shared with the Agent and
		// Field roles added by WORLD-ENTRY-001 Phase C. Four copies of a deadline loop
		// would drift, and the drift would be invisible until it was not.
		//
		// This wrapper keeps LOGIN-002's refusal vocabulary, which its tests assert
		// on: the shared reader reports four outcomes and this maps them onto the
		// LoginServerRefusal values that were already part of this class's contract.
		const ReadResult result = ReadOneMessage(connection, framer, out,
		                                         kExchangeTimeoutMilliseconds);

		if (result.IsOk())
		{
			return true;
		}

		m_refusalDetail = result.detail;

		switch (result.outcome)
		{
		case ReadOutcome::PeerClosed:
			// An orderly close before a complete request.
			m_refusal = LoginServerRefusal::PeerClosedFirst;
			return false;
		case ReadOutcome::TransportFault:
			m_refusal = LoginServerRefusal::ReceiveFailed;
			return false;
		case ReadOutcome::TimedOut:
		case ReadOutcome::Oversized:
		case ReadOutcome::MalformedHeader:
		default:
			// TimedOut and both framing failures are MalformedFrame here, exactly as
			// the pre-Phase-C private loop reported them.
			m_refusal = LoginServerRefusal::MalformedFrame;
			return false;
		}
	}
}

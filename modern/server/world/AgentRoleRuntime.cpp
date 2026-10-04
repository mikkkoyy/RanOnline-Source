#include "world/AgentRoleRuntime.h"

#include "LoginProtocol.h"
#include "LoginResponseProtocol.h"
#include "NetworkCodec.h"

#include <utility>

namespace Modern::Server::World
{
	using namespace Modern::Network;

	namespace
	{
		// Bounds ONE message read, not the whole conversation.
		//
		// Per message rather than per conversation, because the conversation is five
		// messages and a client that stalls between two of them has stalled, not
		// finished slowly. A whole-conversation budget would let a client that sent
		// four messages instantly and then went quiet hold the connection for the
		// remainder of one long timeout instead of being dropped promptly.
		constexpr int kMessageTimeoutMilliseconds = 10000;

		// Read buffer for one receive. Larger than any Agent response except a 2332
		// (1176 bytes) plus its 12-byte envelope, so those usually arrive whole - but
		// "usually" is what ConnectionFramer exists for.
		constexpr std::size_t kReadBufferSize = 2048;
	}

	const char* ToString(AgentEvent event) noexcept
	{
		switch (event)
		{
		case AgentEvent::Listening:           return "Listening";
		case AgentEvent::ClientConnected:     return "ClientConnected";
		case AgentEvent::LoginAccepted:       return "LoginAccepted";
		case AgentEvent::ListSent:            return "ListSent";
		case AgentEvent::DetailSent:          return "DetailSent";
		case AgentEvent::RedirectSent:        return "RedirectSent";
		case AgentEvent::ClientRejected:      return "ClientRejected";
		case AgentEvent::ClientDisconnected:  return "ClientDisconnected";
		}
		return "Unrecognised";
	}

	const char* ToString(AgentRefusal refusal) noexcept
	{
		switch (refusal)
		{
		case AgentRefusal::None:              return "None";
		case AgentRefusal::UnexpectedMessage: return "UnexpectedMessage";
		case AgentRefusal::BadMessageSize:    return "BadMessageSize";
		case AgentRefusal::MalformedFrame:    return "MalformedFrame";
		case AgentRefusal::PeerClosedFirst:   return "PeerClosedFirst";
		case AgentRefusal::SendFailed:        return "SendFailed";
		case AgentRefusal::ReceiveFailed:     return "ReceiveFailed";
		case AgentRefusal::ServiceRefused:    return "ServiceRefused";
		}
		return "Unrecognised";
	}

	AgentRoleRuntime::AgentRoleRuntime(WorldServerConfig config,
	                                   ILoginAuthenticator& authenticator,
	                                   ICharacterRepository& repository,
	                                   FieldEntryRegistry& registry,
	                                   AgentLogSink log)
		: m_config(std::move(config))
		, m_authenticator(authenticator)
		, m_repository(repository)
		, m_registry(registry)
		, m_select(repository)
		, m_entry(repository, registry)
		, m_log(std::move(log))
	{
	}

	AgentRoleRuntime::~AgentRoleRuntime()
	{
		Stop();
	}

	void AgentRoleRuntime::Emit(AgentEvent event, std::string text, std::size_t count)
	{
		if (m_log)
		{
			AgentLogEntry entry;
			entry.event = event;
			entry.count = count;
			entry.text  = std::move(text);
			m_log(entry);
		}
	}

	Status AgentRoleRuntime::Start()
	{
		if (const Status status = m_config.Validate(); status.IsError())
		{
			m_refusalDetail = "invalid configuration";
			return status;
		}

		if (const Status status = m_listener.Listen(m_config.agentBind); status.IsError())
		{
			m_refusalDetail = "bind/listen failed: ";
			m_refusalDetail += status.GetMessage();
			return status;
		}

		// The address the OS actually assigned, not the one that was asked for. With
		// port 0 the two differ and only this one can be connected to.
		m_config.agentBind = m_listener.BoundEndpoint();

		Emit(AgentEvent::Listening, m_config.agentBind.host + ":" +
		                                    std::to_string(m_config.agentBind.port));
		return Ok();
	}

	void AgentRoleRuntime::Stop() noexcept
	{
		m_listener.Close();
	}

	void AgentRoleRuntime::SetAdvertisedFieldEndpoint(const FieldEndpoint& endpoint)
	{
		m_entry.SetFieldEndpoint(endpoint);
	}

	bool AgentRoleRuntime::IsRunning() const noexcept
	{
		return m_listener.IsListening();
	}

	Status AgentRoleRuntime::ServeOneConnection(int timeoutMilliseconds)
	{
		if (!IsRunning())
		{
			return Status(ErrorCode::InvalidState);
		}

		std::uintptr_t raw = TcpListener::kNoAcceptedSocket;
		const Status    accepted = m_listener.Accept(raw, timeoutMilliseconds);

		if (accepted.IsError())
		{
			m_refusal = AgentRefusal::ReceiveFailed;
			m_refusalDetail = "accept: ";
			m_refusalDetail += accepted.GetMessage();
			return accepted;
		}

		if (raw == TcpListener::kNoAcceptedSocket)
		{
			// Nobody knocked. Not a failure and not a refusal: nothing was received and
			// nothing was rejected.
			return Ok();
		}

		Result<TcpTransport> adopted = TcpTransport::Adopt(raw);
		if (adopted.IsError())
		{
			// Adopt deliberately does not close the handle when it fails, so that the
			// caller keeps the decision - which makes closing it here this function's
			// job. Without that, a socket would leak once per occurrence, silently,
			// which is the worst shape a leak can have.
			TcpTransport::CloseOwnedHandle(raw);

			++m_refused;
			m_refusal = AgentRefusal::ReceiveFailed;
			m_refusalDetail = "could not adopt the accepted socket";
			return adopted.GetStatus();
		}

		TcpTransport connection = std::move(adopted.GetValue());

		const Endpoint peer = connection.RemoteEndpoint();
		Emit(AgentEvent::ClientConnected, peer.host + ":" + std::to_string(peer.port));

		const AgentRefusal refusal = ServeConnection(connection);

		// Closed here rather than left to the destructor so the Disconnected event and
		// the actual close are adjacent, and so the peer observes the close before
		// ServeOneConnection returns.
		connection.Disconnect();

		if (refusal == AgentRefusal::None)
		{
			++m_served;
			m_refusal = AgentRefusal::None;
			m_refusalDetail.clear();
			Emit(AgentEvent::ClientDisconnected,
			     peer.host + ":" + std::to_string(peer.port));
			return Ok();
		}

		++m_refused;
		m_refusal = refusal;
		Emit(AgentEvent::ClientRejected, m_refusalDetail);
		// Still Ok: rejecting a hostile client and staying up is the server working,
		// not failing. Reporting an error here would make a caller that is only serving
		// clients treat a bad packet as a server fault.
		return Ok();
	}

	AgentRefusal AgentRoleRuntime::ServeConnection(TcpTransport& connection)
	{
		// Everything per-conversation, created here and destroyed with the connection.
		ConnectionFramer framer;
		ServerBatchEncoder batcher(m_codec);
		LoginReceiver receiver(m_authenticator, m_config.expectedEncryptKey);

		// The session id is this role's own monotonic counter. It is NOT a socket
		// handle and is not claimed to be: it only has to be unique among the sessions
		// this runtime creates, so that a gaeaId authorization can name the Agent
		// conversation that asked for it.
		AgentSession session(m_served + m_refused + 1);

		for (;;)
		{
			Message message;
			const ReadResult read = ReadOneMessage(connection, framer, message,
			                                        kMessageTimeoutMilliseconds,
			                                        kReadBufferSize);

			if (!read.IsOk())
			{
				m_refusalDetail = read.detail;
				switch (read.outcome)
				{
				case ReadOutcome::PeerClosed:
					m_refusal = AgentRefusal::PeerClosedFirst;
					return m_refusal;
				case ReadOutcome::TransportFault:
					m_refusal = AgentRefusal::ReceiveFailed;
					return m_refusal;
				default:
					m_refusal = AgentRefusal::MalformedFrame;
					return m_refusal;
				}
			}

			bool conversationComplete = false;
			const AgentRefusal refusal =
			    Dispatch(batcher, connection, session, receiver, message, conversationComplete);

			if (refusal != AgentRefusal::None)
			{
				return refusal;
			}
			if (conversationComplete)
			{
				// The 2358 has been sent. Legacy would keep the socket open with
				// heartbeats; see the header for why this closes instead.
				return AgentRefusal::None;
			}
		}
	}

	Status AgentRoleRuntime::SendEnveloped(ServerBatchEncoder& batcher,
	                                       TcpTransport& connection,
	                                       const std::vector<WireU8>& inner)
	{
		// The same sequence LoginResponder uses, for the same reason: a 28-byte 2248
		// is far below the 1000-byte flush trigger, so Add normally buffers it and the
		// frame must be flushed explicitly. These responses are SENT, not held - legacy
		// emits LOGIN_FB on the tick, alone (s_CAgentServerMsgLogin.cpp) - so waiting
		// for a partner message would leave the client waiting.
		std::vector<WireU8> pending;
		const BatchAction   action = batcher.Add(inner, pending);

		if (action == BatchAction::Buffered)
		{
			if (!batcher.Flush(pending))
			{
				return Status(ErrorCode::InvalidState);
			}
		}

		if (const Status status = connection.Send(pending.data(), pending.size());
		    status.IsError())
		{
			return status;
		}
		return Ok();
	}

	AgentRefusal AgentRoleRuntime::Dispatch(ServerBatchEncoder& batcher,
	                                         TcpTransport& connection,
	                                         AgentSession& session,
	                                         LoginReceiver& receiver,
	                                         const Message& message,
	                                         bool& conversationComplete)
	{
		conversationComplete = false;

		const std::vector<WireU8> frame = ReconstructFrame(message);

		// ---- 2049 LOGIN_2 ---------------------------------------------------
		//
		// The ID is checked before anything else, so a wrong id reports as a wrong id
		// rather than as a length problem or a state problem. Legacy's MsgProcess
		// dispatches on nType alone and its default case ignores anything else, which
		// is the right shape: an unexpected message is not a protocol error, it is
		// simply not this conversation.
		if (message.header.type == LoginProtocol::kLoginMessageId ||
		    LoginProtocol::IsLoginMessage(frame.data(), frame.size()))
		{
			LoginVerdict verdict;
			if (const Status status = receiver.HandleLoginFrame(frame, m_tea, verdict);
			    status.IsError())
			{
				m_refusalDetail = "LOGIN_2 could not be decoded: ";
				m_refusalDetail += status.GetMessage();
				return AgentRefusal::MalformedFrame;
			}

			if (receiver.LastFailureWasSharedSecret())
			{
				// Legacy DROPS the connection on a shared-secret mismatch rather than
				// replying (s_CAgentServerMsgLogin.cpp:664-677). Reproduced: a
				// credential failure gets an answer, a "wrong cluster" does not.
				m_refusalDetail = "LOGIN_2 shared secret mismatch";
				return AgentRefusal::ServiceRefused;
			}

			// The login name decides the WORLD ACCOUNT. Nothing in the login exchange
			// knows a world account id exists - see WorldServerConfig.h - so the mapping
			// is explicit and a name with no binding cannot be authenticated.
			const WorldAccountBinding* binding =
			    verdict.accepted ? m_config.FindAccount(receiver.LastRequest().userId)
			                     : nullptr;

			LoginVerdict effective = verdict;
			if (verdict.accepted && binding == nullptr)
			{
				// The credential was correct but there is no world account behind the
				// name. That is a refusal, not a server error: without an account id
				// there is nothing to check ownership against, and authenticating
				// anyway would produce a session whose every character request fails.
				effective.accepted = false;
				effective.reason   = LoginRejectReason::UnknownAccount;
			}

			// BuildResponse produces the enveloped LOGIN_FB through the shared batcher.
			LoginResponder responder(batcher);
			std::vector<WireU8> feedback;
			if (const Status status = responder.BuildResponse(effective, "", 0, feedback);
			    status.IsError())
			{
				m_refusalDetail = "could not build LOGIN_FB: ";
				m_refusalDetail += status.GetMessage();
				return AgentRefusal::SendFailed;
			}

			// The envelope LoginResponder produced IS the frame - it already went
			// through ServerBatchEncoder - so it is sent directly rather than wrapped a
			// second time. Wrapping it again would put two NET_COMPRESS headers on the
			// wire, and the client's unwrapper would hand the inner one to the framer as
			// if it were a message.
			if (const Status status = connection.Send(feedback.data(), feedback.size());
			    status.IsError())
			{
				m_refusalDetail = "send failed: ";
				m_refusalDetail += status.GetMessage();
				return AgentRefusal::SendFailed;
			}

			if (!effective.accepted)
			{
				m_refusalDetail = "login refused: ";
				m_refusalDetail += effective.reason == LoginRejectReason::BadPassword ? "BadPassword" : "UnknownAccount";
				return AgentRefusal::ServiceRefused;
			}

			// The session's account comes from the SERVER's table, never from the
			// packet. The client named itself; the server decided what that name means.
			const Status authenticated = session.CompleteAuthentication(
			    binding->accountId, receiver.LastRequest().userId, false);
			if (authenticated.IsError())
			{
				m_refusalDetail = "session refused authentication: ";
				m_refusalDetail += authenticated.GetMessage();
				return AgentRefusal::ServiceRefused;
			}

			m_lastAccountId = binding->accountId.value;
			Emit(AgentEvent::LoginAccepted, binding->userId);
			return AgentRefusal::None;
		}

		// ---- 2247 request the character list --------------------------------
		if (CharacterListCodec::IsRequestAll(message.header.type))
		{
			// A 2247 is a bare 8-byte header (s_NetGlobal.h:3970 has its nChannel
			// commented out), so anything else is not this message.
			if (frame.size() != CharacterList::kBareMessageSize)
			{
				m_refusalDetail = "2247 with dwSize " + std::to_string(frame.size()) +
				                  " (expected 8)";
				return AgentRefusal::BadMessageSize;
			}

			CharacterListResult list;
			const Status       status = m_select.BuildCharacterList(session, list);
			if (status.IsError())
			{
				// Wrong state, an account with more characters than this build can
				// announce, or an encode failure. The service's code is in the text;
				// this layer deliberately does not try to classify it further, because
				// distinguishing "not yours" from "no such character" HERE would be an
				// ownership oracle built one layer too high.
				m_refusalDetail = "2247 refused: ";
				m_refusalDetail += status.GetMessage();
				return AgentRefusal::ServiceRefused;
			}

			if (const Status sent = SendEnveloped(batcher, connection, list.frame);
			    sent.IsError())
			{
				m_refusalDetail = "send failed: ";
				m_refusalDetail += sent.GetMessage();
				return AgentRefusal::SendFailed;
			}

			m_lastListCount = list.expectedDetailCount;
			Emit(AgentEvent::ListSent, "", list.expectedDetailCount);
			return AgentRefusal::None;
		}

		// ---- 2244 request one character's detail ----------------------------
		if (CharacterListCodec::IsRequestOne(message.header.type))
		{
			WireU32 characterId = 0;
			if (const Status status = CharacterListCodec::ValidateRequestOne(frame, characterId);
			    status.IsError())
			{
				m_refusalDetail = "2244 malformed";
				return AgentRefusal::BadMessageSize;
			}

			std::vector<WireU8> detail;
			const Status        status = m_select.BuildCharacterDetail(session, characterId, detail);
			if (status.IsError())
			{
				m_refusalDetail = "2244 for " + std::to_string(characterId) + " refused: ";
				m_refusalDetail += status.GetMessage();
				return AgentRefusal::ServiceRefused;
			}

			if (const Status sent = SendEnveloped(batcher, connection, detail); sent.IsError())
			{
				m_refusalDetail = "send failed: ";
				m_refusalDetail += sent.GetMessage();
				return AgentRefusal::SendFailed;
			}

			Emit(AgentEvent::DetailSent, "");
			return AgentRefusal::None;
		}

		// ---- 2353 the selection ---------------------------------------------
		if (WorldEntryCodec::IsGameJoin(message.header.type))
		{
			WireI32 characterNumber = 0;
			if (const Status status = WorldEntryCodec::DecodeGameJoin(frame, characterNumber);
			    status.IsError())
			{
				m_refusalDetail = "2353 malformed";
				return AgentRefusal::BadMessageSize;
			}

			// Negative and zero ids are refused by the service rather than clamped here:
			// the field is a signed int and the server decides what it means.
			const Status selected =
			    m_select.SelectCharacter(session, static_cast<WireU32>(characterNumber));
			if (selected.IsError())
			{
				m_refusalDetail = "selection of " + std::to_string(characterNumber) +
				                  " refused: ";
				m_refusalDetail += selected.GetMessage();
				return AgentRefusal::ServiceRefused;
			}

			// The authorization, and with it the gaeaId allocation, is Phase B's. This
			// role only turns the result into the 2358 the client will dial.
			const Result<FieldRedirect> redirect = m_entry.AuthorizeWorldEntry(session);
			if (redirect.IsError())
			{
				m_refusalDetail = "world entry refused: ";
				m_refusalDetail += redirect.GetMessage();
				return AgentRefusal::ServiceRefused;
			}

			std::vector<WireU8> packet;
			if (const Status status =
			        WorldEntryCodec::AppendFieldRedirect(packet, redirect.GetValue());
			    status.IsError())
			{
				m_refusalDetail = "could not encode 2358: ";
				m_refusalDetail += status.GetMessage();
				return AgentRefusal::SendFailed;
			}

			if (const Status sent = SendEnveloped(batcher, connection, packet); sent.IsError())
			{
				m_refusalDetail = "send failed: ";
				m_refusalDetail += sent.GetMessage();
				return AgentRefusal::SendFailed;
			}

			m_lastGaeaId = redirect.GetValue().gaeaId;

			// The last message this role sends. The conversation ends here.
			conversationComplete = true;
			Emit(AgentEvent::RedirectSent, "", m_lastGaeaId);
			return AgentRefusal::None;
		}

		m_refusalDetail = "unexpected message id " + std::to_string(message.header.type);
		return AgentRefusal::UnexpectedMessage;
	}
}
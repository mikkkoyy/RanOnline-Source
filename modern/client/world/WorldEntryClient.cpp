#include "world/WorldEntryClient.h"

#include "LoginProtocol.h"
#include "LoginResponseProtocol.h"
#include "NetworkCodec.h"
#include "UpdateStateProtocol.h"

namespace Modern::Client
{
	using namespace Modern::Network;

	namespace
	{
		// Upper bound on the UNWRAPPED inner stream of one envelope.
		//
		// A hostile or truncated envelope must not be able to make this allocate
		// without limit. The largest legal inner message in this phase is a 1176-byte
		// 2332, so 2048 - the protocol's own maximum packet size - leaves room for a
		// batch of small messages without being a number invented to be generous.
		constexpr std::size_t kMaxInnerBytes = Protocol::kMaxPacketSize;


	}

	const World001LoginClient& loginCodec() noexcept
	{
		// EncodeLoginRequest is stateless - it builds bytes from its arguments and owns
		// nothing - so a shared instance is safe and avoids asking every caller to
		// construct one.
		static const World001LoginClient codec;
		return codec;
	}
	const char* ToString(WorldEntryPhase phase) noexcept
	{
		switch (phase)
		{
		case WorldEntryPhase::Disconnected:        return "Disconnected";
		case WorldEntryPhase::LoggingIn:           return "LoggingIn";
		case WorldEntryPhase::Authenticated:       return "Authenticated";
		case WorldEntryPhase::CharacterListReady:  return "CharacterListReady";
		case WorldEntryPhase::SelectingCharacter:  return "SelectingCharacter";
		case WorldEntryPhase::RedirectReceived:    return "RedirectReceived";
		case WorldEntryPhase::LoginRejected:       return "LoginRejected";
		}
		return "Unrecognised";
	}

	Status WorldEntryClient::BuildLogin(const LoginRequestData& data,
	                                    std::vector<WireU8>& request)
	{
		if (m_phase != WorldEntryPhase::Disconnected)
		{
			// One login per conversation. A second attempt would be answered by a
			// session that already has an account, and the server refuses it - so
			// refusing here reports the client bug where it is.
			return Status(ErrorCode::NotAllowed);
		}

		// The login request bytes are WORLD-001's codec, unchanged. Re-encoding a
		// login packet here would be a second place for the garbage-token stripping and
		// the minTea field cipher to disagree with themselves.
		if (const Status status = loginCodec().EncodeLoginRequest(
		            data, World001LoginClient::DeterministicToken(0), request);
		    status.IsError())
		{
			return status;
		}

		m_phase = WorldEntryPhase::LoggingIn;
		return Ok();
	}

	Status WorldEntryClient::BuildRequestCharacterList(std::vector<WireU8>& request)
	{
		if (m_phase != WorldEntryPhase::Authenticated)
		{
			// Same ordering the server enforces, checked here so a client bug reads as
			// a client bug rather than as a server refusal three steps later.
			return Status(ErrorCode::NotAllowed);
		}
		return CharacterListCodec::AppendRequestAll(request);
	}

	Status WorldEntryClient::BuildRequestCharacterDetail(WireU32 characterId,
	                                                     std::vector<WireU8>& request)
	{
		if (m_phase != WorldEntryPhase::CharacterListReady)
		{
			return Status(ErrorCode::NotAllowed);
		}
		return CharacterListCodec::AppendRequestOne(request, characterId);
	}

	Status WorldEntryClient::BuildSelectCharacter(WireU32 characterId,
	                                              std::vector<WireU8>& request)
	{
		// Legal from CharacterListReady: a selection needs the list, because the ids in
		// the list are what the client is choosing from.
		if (m_phase != WorldEntryPhase::CharacterListReady)
		{
			return Status(ErrorCode::NotAllowed);
		}

		if (const Status status = WorldEntryCodec::AppendGameJoin(
		        request, static_cast<WireI32>(characterId));
		    status.IsError())
		{
			return status;
		}

		m_phase = WorldEntryPhase::SelectingCharacter;
		return Ok();
	}

	Status WorldEntryClient::BuildFieldIdentity(const FieldIdentity& identity,
	                                             std::vector<WireU8>& request)
	{
		// Phase A's codec, which refuses an address-less or over-long identity at the
		// boundary. The client does not get to send a malformed 2359.
		return WorldEntryCodec::AppendFieldIdentity(request, identity);
	}

	Status WorldEntryClient::BuildMoveState(WireU32 actState, std::vector<WireU8>& request)
	{
		// The shared codec builds the 12-byte NET_MSG_GCTRL_MOVESTATE and refuses
		// anything else, so a client cannot put a wrong-sized 3032 on the wire.
		//
		// `actState` is passed through verbatim, deliberately. See the header: filtering
		// the client-owned bits here would make the server's authority rule untestable.
		return MovementState::MovementStateCodec::AppendMoveStateRequest(
		    request, MovementState::MoveStateRequest{actState});
	}

	Status WorldEntryClient::BuildGoto(WireU32 requestedActState, Vector3 claimedCurrent,
	                                 Vector3 requestedTarget, std::vector<WireU8>& request)
	{
		// The shared codec builds the 36-byte NET_MSG_GCTRL_GOTO and refuses anything
		// else, so a client cannot put a wrong-sized 3034 on the wire - and refuses a
		// non-finite coordinate, which would otherwise reach the server's 60-unit
		// comparison as a NaN and silently pass it.
		//
		// Both positions are sent as the CLIENT believes them. See the header: the
		// server owns the position, and `vCurPos` exists to be CHECKED against it.
		Network::Goto::GotoRequest message;
		message.actState        = requestedActState;
		message.currentPosition = Network::RanWire::Vector3{ claimedCurrent.x,
			                                                claimedCurrent.y,
			                                                claimedCurrent.z };
		message.targetPosition  = Network::RanWire::Vector3{ requestedTarget.x,
			                                                requestedTarget.y,
			                                                requestedTarget.z };

		return Network::Goto::GotoCodec::AppendGotoRequest(request, message);
	}

	void WorldEntryClient::Reset() noexcept
	{
		m_phase            = WorldEntryPhase::Disconnected;
		m_agentFramer.Reset();
		m_fieldFramer.Reset();
		m_agentRaw.clear();
		m_fieldRaw.clear();
		m_ids = CharacterIdList{};
		m_expectedDetails = 0;
		m_details.clear();
		m_redirect     = FieldRedirect{};
		m_hasRedirect  = false;
		m_spawn        = WorldSpawnState{};
		// The 3033 stream resets with the connection that carried it, for the same
		// reason the Field framer does. Leaving the count behind would let a second
		// world entry inherit the first one's broadcasts and make MoveStateCount()
		// answer a question about two different sessions.
		m_moveState      = WorldMoveStateState{};
		m_moveStateCount = 0;
		// The 3035 stream resets with the connection that carried it, for the same
		// reason the 3033 stream does: leaving a count behind would let a second world
		// entry inherit the first one's broadcasts, and GotoCount() would then answer a
		// question about two different sessions.
		m_goto      = WorldGotoState{};
		m_gotoCount = 0;
		m_agentFailed  = false;
		m_fieldFailed  = false;
	}

	Status WorldEntryClient::ConsumeEnvelopes(std::vector<WireU8>& pendingRaw,
	                                          ConnectionFramer&  framer,
	                                          std::size_t         maxInnerBytes)
	{
		for (;;)
		{
			if (pendingRaw.size() < kCompressEnvelopeSize)
			{
				// Not even a full envelope header yet. Wait for more.
				return Ok();
			}

			// The envelope's own dwSize says how long the whole frame is. Nothing may be
			// unwrapped before that many bytes exist, and a dwSize that is already
			// impossible is refused here rather than after a long wait for bytes that
			// will make it valid.
			const WireU32 declared = Codec::ReadU32(pendingRaw.data());
			if (declared < kCompressEnvelopeSize || declared > maxInnerBytes)
			{
				return Status(ErrorCode::InvalidArgument);
			}

			if (pendingRaw.size() < declared)
			{
				return Ok();
			}

			// DecodeEnvelope validates the type is 170 and that the payload is non-empty.
			// It does NOT validate the payload's contents - an envelope claiming to be
			// compressed has not been proven so until LZO has accepted it.
			std::vector<WireU8> inner;
			if (const Status status = NetCompress::DecodeServerToClientEnvelope(
			        m_codec, pendingRaw.data(), declared, maxInnerBytes, inner);
			    status.IsError())
			{
				return status;
			}

			// The consumed envelope is removed only after a SUCCESSFUL unwrap. A failed
			// unwrap leaves the bytes in place so the caller can drop the connection
			// knowing exactly where it went wrong, rather than silently skipping 20
			// bytes and trying to frame the garbage behind them.
			pendingRaw.erase(pendingRaw.begin(),
			                 pendingRaw.begin() + static_cast<std::ptrdiff_t>(declared));

			// The inner stream is fed to the framer, which is what turns it into whole
			// messages. One envelope may carry SEVERAL (investigation §4.2: Agent/Field
			// batch up to COMPRESS_PACKET_SIZE bytes), so this is a Feed and not a
			// single decode.
			if (const FrameStatus fed = framer.Feed(inner.data(), inner.size());
			    fed != FrameStatus::Ok)
			{
				return Status(ErrorCode::InvalidArgument);
			}
		}
	}

	Status WorldEntryClient::DrainAgentMessages(std::size_t& messagesHandled)
	{
		for (;;)
		{
			Message message;
			const FrameStatus next = m_agentFramer.Next(message);
			if (next == FrameStatus::NeedMoreData)
			{
				return Ok();
			}
			if (next != FrameStatus::Ok)
			{
				m_agentFailed = true;
				return Status(ErrorCode::InvalidArgument);
			}

			++messagesHandled;
			const std::vector<WireU8> frame = ReconstructFrame(message);

			// ---- 2050 LOGIN_FB ------------------------------------------------
			if (message.header.type == 2050)
			{
				LoginFeedback feedback;
				if (const Status status = LoginResponse::Decode(frame.data(), frame.size(),
				                                               feedback);
				    status.IsError())
				{
					m_agentFailed = true;
					return status;
				}

				// The verdict is the SERVER's. A client that treated its own credentials
				// as authoritative would log in with a password the server rejected.
				if (!feedback.IsSuccess())
				{
					m_phase = WorldEntryPhase::LoginRejected;
					continue;
				}

				m_phase = WorldEntryPhase::Authenticated;
				continue;
			}

			// ---- 2248 the character list -------------------------------------
			if (CharacterListCodec::IsIdList(message.header.type))
			{
				if (const Status status = CharacterListCodec::DecodeIdList(frame, m_ids);
				    status.IsError())
				{
					m_agentFailed = true;
					return status;
				}

				// Completion is by COUNT, never by a terminator - there is no
				// end-of-list message in this protocol.
				m_expectedDetails = CharacterListCodec::ExpectedDetailCount(m_ids);
				m_phase           = WorldEntryPhase::CharacterListReady;
				continue;
			}

			// ---- 2332 one character's detail ---------------------------------
			if (CharacterListCodec::IsCharacterDetail(message.header.type))
			{
				CharacterDetail detail;
				if (const Status status =
				        CharacterListCodec::DecodeCharacterDetail(frame, detail);
				    status.IsError())
				{
					m_agentFailed = true;
					return status;
				}
				m_details.push_back(detail);
				continue;
			}

			// ---- 2358 the redirect -------------------------------------------
			if (WorldEntryCodec::IsFieldRedirect(message.header.type))
			{
				if (const Status status =
				        WorldEntryCodec::DecodeFieldRedirect(frame, m_redirect);
				    status.IsError())
				{
					m_agentFailed = true;
					return status;
				}

				m_hasRedirect = true;
				m_phase       = WorldEntryPhase::RedirectReceived;
				continue;
			}

			// ---- 2335 the selection was refused ------------------------------
			if (WorldEntryCodec::IsJoinFailure(message.header.type))
			{
				WireI32 reason = 0;
				if (const Status status =
				        WorldEntryCodec::DecodeJoinFailure(frame, reason);
				    status.IsError())
				{
					m_agentFailed = true;
					return status;
				}
				// Stays in SelectingCharacter: the server refused, and a client that
				// advanced to RedirectReceived here would try to dial a Field endpoint
				// it was never given.
				continue;
			}

			// Anything else is not this conversation. Legacy's MsgProcess default case
			// ignores unknown ids (s_NetClientMsg.cpp:19-44) rather than aborting, and
			// a real client must survive a message it does not implement yet.
		}
	}

	Status WorldEntryClient::DrainFieldMessages(std::size_t& messagesHandled)
	{
		for (;;)
		{
			Message message;
			const FrameStatus next = m_fieldFramer.Next(message);
			if (next == FrameStatus::NeedMoreData)
			{
				return Ok();
			}
			if (next != FrameStatus::Ok)
			{
				m_fieldFailed = true;
				return Status(ErrorCode::InvalidArgument);
			}

			++messagesHandled;
			const std::vector<WireU8> frame = ReconstructFrame(message);

			// ---- 3033: authoritative movement state ---------------------------------
			//
			// Checked BEFORE the spawn test, because 3033 now arrives on the SAME
			// long-lived connection as 2333 rather than on a conversation of its own. A
			// Field client that has spawned and then moved receives both, interleaved,
			// and treating the 3033 as "not a spawn" would drop it on the floor.
			if (MovementState::MovementStateCodec::IsMoveStateBroadcast(message.header.type))
			{
				MovementState::MoveStateBroadcast broadcast;
				if (const Status status =
				        MovementState::MovementStateCodec::DecodeMoveStateBroadcast(
				            frame, broadcast);
				    status.IsError())
				{
					// Terminal, and differently so from an unknown id. The id is one this
					// client recognises, so a bad one is a protocol fault worth reporting
					// rather than something to skip past - and the stream cannot be trusted
					// to be in the right place afterwards.
					m_fieldFailed = true;
					return status;
				}

				WorldMoveStateState received;
				received.received = true;
				received.gaeaId   = broadcast.gaeaId;
				received.actState = broadcast.actState;
				received.frame    = frame;

			m_moveState = received;
			++m_moveStateCount;
			continue;
		}

		// ---- 3035: an accepted GOTO -----------------------------------------
		//
		// Checked BEFORE the spawn test for the same reason the 3033 is: all
		// three arrive on ONE long-lived Field connection, interleaved. A client
		// that has spawned and then moved receives 2333, 3033 and 3035 in
		// whatever order the server produced them.
		if (Goto::GotoCodec::IsGotoBroadcast(message.header.type))
		{
			Goto::GotoBroadcast broadcast;
			if (const Status status =
			        Goto::GotoCodec::DecodeGotoBroadcast(frame, broadcast);
			    status.IsError())
			{
				// Terminal, and differently so from an unknown id. The id is one
				// this client recognises, so a bad one is a protocol fault worth
				// reporting rather than something to skip past - and the stream
				// cannot be trusted to be in the right place afterwards.
				m_fieldFailed = true;
				return status;
			}

			// Copied field by field rather than aliased, so what a test asserts on
			// is plainly what arrived.
			WorldGotoState received;
			received.received          = true;
			received.gaeaId            = broadcast.gaeaId;
			received.actState          = broadcast.actState;
			received.currentPositionX  = broadcast.currentPosition.x;
			received.currentPositionY  = broadcast.currentPosition.y;
			received.currentPositionZ  = broadcast.currentPosition.z;
			received.targetPositionX   = broadcast.targetPosition.x;
			received.targetPositionY   = broadcast.targetPosition.y;
			received.targetPositionZ   = broadcast.targetPosition.z;
			received.delay             = broadcast.delay;
			received.frame             = frame;

			m_goto = received;
			++m_gotoCount;
			continue;
		}

// ---- 3053: HP broadcast -----------------------------------------------
		//
		// Checked BEFORE the spawn test, for the same reason as 3033/3035: all
		// arrive on ONE long-lived Field connection, interleaved.
		if (UpdateState::UpdateStateCodec::IsStateBroadcast(message.header.type))
		{
			UpdateState::StateBroadcast broadcast;
			if (const Status status =
			        UpdateState::UpdateStateCodec::DecodeStateBroadcast(frame, broadcast);
			    status.IsError())
			{
				m_fieldFailed = true;
				return status;
			}

			WorldUpdateStateBrdState received;
			received.received = true;
			received.gaeaId   = broadcast.gaeaId;
			received.hpNow    = broadcast.hp.now;
			received.hpMax    = broadcast.hp.max;
			received.safeTime = broadcast.safeTime;
			received.frame    = frame;

			m_updateStateBrd = received;
			++m_updateStateBrdCount;
			continue;
		}

		// ---- 3046: authoritative resource state (UPDATE_STATE) ----------
		//
		// Checked BEFORE the spawn test, for the same reason as 3033/3035.
		if (UpdateState::UpdateStateCodec::IsStateUpdate(message.header.type))
		{
			UpdateState::StateUpdate update;
			if (const Status status =
			        UpdateState::UpdateStateCodec::DecodeStateUpdate(frame, update);
			    status.IsError())
			{
				m_fieldFailed = true;
				return status;
			}

			WorldUpdateStateState received;
			received.received     = true;
			received.hpNow        = update.hp.now;
			received.hpMax        = update.hp.max;
			received.mpNow        = update.mp.now;
			received.mpMax        = update.mp.max;
			received.spNow        = update.sp.now;
			received.spMax        = update.sp.max;
			received.cpNow        = update.cp.now;
			received.cpMax        = update.cp.max;
			received.characterName = update.name;
			received.gaeaId        = update.gaeaId;
			received.charId        = update.charId;
			received.safeTime      = update.safeTime;
			received.frame         = frame;

			m_updateState = received;
			++m_updateStateCount;
			continue;
		}

		if (!WorldEntryCodec::IsSpawn(message.header.type))
			{
				// Not a spawn. Ignored rather than treated as an error, for the same
				// reason the Agent side ignores unknown ids.
				continue;
			}

			// Decoded with Phase A's codec, which requires exactly 1022 bytes with id
			// 2333 - so a truncated or padded spawn is refused here rather than parsed.
			SpawnState spawn;
			if (const Status status = WorldEntryCodec::DecodeSpawn(frame, spawn);
			    status.IsError())
			{
				m_fieldFailed = true;
				return status;
			}

			// Copied field by field rather than aliased, so what the test asserts on is
			// plainly what arrived.
			WorldSpawnState received;
			received.received       = true;
			received.userId         = spawn.userId;
			received.clientId       = spawn.clientId;
			received.gaeaId         = spawn.gaeaId;
			received.accountId      = spawn.accountId;
			received.characterId    = spawn.characterId;
			received.characterName  = spawn.characterName;
			received.characterClass = spawn.characterClass;
			received.school         = spawn.school;
			received.level          = spawn.level;
			received.hp             = spawn.hp.now;
			received.mp             = spawn.mp.now;
			received.sp             = spawn.sp.now;
			received.mapId          = spawn.mapId.value;
			received.positionX      = spawn.position.x;
			received.positionY      = spawn.position.y;
			received.positionZ      = spawn.position.z;
			received.startMapId     = spawn.startMapId.value;
			received.startGate      = spawn.startGate;
			received.frame          = frame;

			m_spawn = received;
		}
	}

	Status WorldEntryClient::FeedAgent(const WireU8* data, std::size_t size,
	                                   std::size_t& messagesHandled)
	{
		if (m_agentFailed)
		{
			return Status(ErrorCode::InvalidState);
		}

		m_agentRaw.insert(m_agentRaw.end(), data, data + size);

		if (const Status status = ConsumeEnvelopes(m_agentRaw, m_agentFramer, kMaxInnerBytes);
		    status.IsError())
		{
			m_agentFailed = true;
			return status;
		}

		return DrainAgentMessages(messagesHandled);
	}

	Status WorldEntryClient::FeedField(const WireU8* data, std::size_t size,
	                                   std::size_t& messagesHandled)
	{
		if (m_fieldFailed)
		{
			return Status(ErrorCode::InvalidState);
		}

		m_fieldRaw.insert(m_fieldRaw.end(), data, data + size);

		if (const Status status = ConsumeEnvelopes(m_fieldRaw, m_fieldFramer, kMaxInnerBytes);
		    status.IsError())
		{
			m_fieldFailed = true;
			return status;
		}

		return DrainFieldMessages(messagesHandled);
	}
}

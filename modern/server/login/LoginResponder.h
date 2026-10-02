#pragma once

// WORLD-002: the server side of the login response.
//
// Turns a login verdict into bytes on the wire, using the V030 boundary exactly as
// it stands. Nothing here re-implements batching, LZO or the NET_COMPRESS
// envelope: this class only decides WHAT goes into the batch, and ServerBatchEncoder
// plus NetCompress decide how that batch is framed and sent.
//
// Legacy provenance for the flow being reproduced:
//
//   CAgentServer::MsgLogInBack (s_CAgentServerMsgLogin.cpp:1087-1324)
//     fills NET_LOGIN_FEEDBACK_DATA from the DB result
//     -> SendClient(dwClient, &nlfd)                    exactly ONE per branch
//        -> CNetUser::addMsg                             queue
//        -> CClientManager::SendClientFinal              flush each tick
//           -> CSendMsgBuffer::getSendSize               LZO + NET_COMPRESS
//              -> SendClient2 -> ::WSASend
//
// The four SendClient calls in MsgLogInBack are mutually exclusive branches, each
// returning immediately after its own send, so a LOGIN_FB is always a
// single-message batch. This class does the same: one response, one batch, one
// envelope. It does NOT try to coalesce a response with a server-list message,
// because legacy does not.
//
// Database access is out of scope. The verdict comes from the WORLD-001
// ILoginAuthenticator, which WORLD-002 only maps onto a wire result code.

#include "CompressionCodec.h"
#include "LoginResponseProtocol.h"
#include "NetworkTypes.h"
#include "ServerBatchEncoder.h"

#include "login/LoginReceiver.h"
#include "types/Result.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Modern::Server
{
	// Maps a login verdict onto the wire result code.
	//
	// Only a small subset of EM_LOGIN_FB_SUB is reachable here, and deliberately so:
	// inventing semantics for the Daum and region-specific codes (GidError, SsnHead,
	// Adult, the Thai play-time codes) would be exactly the guessing WORLD-002 must
	// avoid. Those exist in the enum because the CLIENT must be able to name them if
	// a RAN server sends them, not because this server emits them.
	Network::LoginFeedbackResult ToWireResult(LoginRejectReason reason) noexcept;

	// Produces the login response frame.
	//
	// Borrowed references, not owned: the codec and the per-connection batcher both
	// outlive one response, and a ServerBatchEncoder is per-connection state in
	// legacy (CSendMsgBuffer is a member of each CNetUser).
	class LoginResponder
	{
	public:
		LoginResponder(Network::MinLzo1xCodec& codec,
		               Network::ServerBatchEncoder& batcher) noexcept
			: m_batcher(batcher)
		{
			(void) codec; // the batcher owns the codec; this ctor takes it for clarity only
		}

		explicit LoginResponder(Network::ServerBatchEncoder& batcher) noexcept
			: m_batcher(batcher)
		{
		}

		// Builds the response frame for a verdict and pushes it through the V030
		// boundary, producing a complete NET_COMPRESS envelope ready for one write.
		//
		// `channel` and `email` are the account metadata the response carries; both are
		// optional and default to empty/zero, matching the failure paths where legacy
		// leaves them at the constructor's zero.
		Status BuildResponse(const LoginVerdict& verdict,
		                              const std::string& email,
		                              std::int32_t channel,
		                              std::vector<Network::WireU8>& out);

		// Same, with explicit account metadata for the success path.
		Status BuildResponse(const Network::LoginFeedback& feedback,
		                              std::vector<Network::WireU8>& out);

		// True when the last BuildResponse produced a LOGIN_FB of EM_LOGIN_FB_SUB_OK.
		bool LastWasSuccess() const noexcept { return m_lastSuccess; }

	private:
		Network::ServerBatchEncoder& m_batcher;

		// Scratch for one serialized LOGIN_FB before it is handed to the batcher.
		// A member so a response path allocates once, not per message.
		std::vector<Network::WireU8> m_message;

		bool m_lastSuccess = false;
	};

	// The minimal client-facing login state this milestone needs.
	//
	// Deliberately not a state-machine framework: WORLD-002 adds exactly the states
	// the traced protocol actually passes through, and nothing speculative. The
	// transition LOGIN_FB -> server-list is NOT modelled here - see the report: the
	// server list is requested to a DIFFERENT server before login, so it is not a
	// continuation of this exchange.
	enum class LoginPhase : std::uint8_t
	{
		Disconnected = 0,
		LoginSent,
		LoginResponseReceived,
		LoginAccepted,
		LoginRejected,
	};

	const char* ToString(LoginPhase phase) noexcept;
}
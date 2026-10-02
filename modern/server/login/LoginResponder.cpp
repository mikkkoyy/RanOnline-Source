#include "login/LoginResponder.h"

namespace Modern::Server
{
	using namespace Modern::Network;

	LoginFeedbackResult ToWireResult(LoginRejectReason reason) noexcept
	{
		switch (reason)
		{
		case LoginRejectReason::None:
			// The caller should not have produced a verdict with no reason; treating
			// it as success is the safe reading, and BuildResponse rejects it anyway
			// if the verdict is not accepted.
			return LoginFeedbackResult::Ok;

		case LoginRejectReason::BadPassword:
			// Legacy distinguishes a wrong id/password (5) from an offline/idle
			// state (21); WORLD-001's authenticator reports BadPassword for a known
			// account with a wrong password, which is the id/password case.
			return LoginFeedbackResult::Incorrect;

		case LoginRejectReason::UnknownAccount:
			// Legacy has no "no such account" code: an unknown id and a wrong
			// password both arrive as 5. Reporting 1 would be more informative but
			// would be an invention, so the legacy code is used.
			return LoginFeedbackResult::Incorrect;
		}
		return LoginFeedbackResult::Fail;
	}

	const char* ToString(LoginPhase phase) noexcept
	{
		switch (phase)
		{
		case LoginPhase::Disconnected:          return "Disconnected";
		case LoginPhase::LoginSent:             return "LoginSent";
		case LoginPhase::LoginResponseReceived: return "LoginResponseReceived";
		case LoginPhase::LoginAccepted:         return "LoginAccepted";
		case LoginPhase::LoginRejected:         return "LoginRejected";
		}
		return "Unrecognised";
	}

	Status LoginResponder::BuildResponse(const LoginFeedback& feedback,
	                                             std::vector<Network::WireU8>& out)
	{
		out.clear();
		m_lastSuccess = feedback.IsSuccess();

		// Serialize the response into a scratch buffer, then let the V030 batcher
		// decide how it is framed. This class never touches LZO or the envelope.
		m_message.clear();
		if (const Status status =
		        Network::LoginResponse::Append(m_message, feedback);
		    status.IsError())
		{
			return status;
		}

		std::vector<Network::WireU8> pending;
		const BatchAction action = m_batcher.Add(m_message, pending);

		// A 120-byte response is far below the 1000-byte flush trigger, so Add
		// normally buffers it. If something else was already pending and pushed the
		// batch over the trigger, Add has already emitted that frame - in which case
		// it is what must go out now. Otherwise flush explicitly, because a LOGIN_FB
		// is sent rather than held: legacy emits it on the tick, alone.
		if (action == BatchAction::Buffered)
		{
			if (!m_batcher.Flush(pending))
			{
				return Status(ErrorCode::InvalidState);
			}
		}

		out = std::move(pending);
		return Ok();
	}

	Status LoginResponder::BuildResponse(const LoginVerdict& verdict,
	                                             const std::string& email,
	                                             std::int32_t channel,
	                                             std::vector<Network::WireU8>& out)
	{
		LoginFeedback feedback;
		feedback.result = verdict.accepted ? LoginFeedbackResult::Ok
		                                  : ToWireResult(verdict.reason);

		// Success paths populate the account metadata; failure paths leave it zeroed,
		// matching legacy where only the OK branch copies email and points across.
		if (verdict.accepted)
		{
			feedback.email = email;
			// nExtremeM/W carry the extreme-class creation credit. Legacy forwards
			// them only under KR_PARAM/KRT_PARAM/TW_PARAM/HK_PARAM/_RELEASED
			// (s_CAgentServerMsgLogin.cpp:1092-1098); ASURA builds KR_PARAM, so they
			// ARE forwarded on this path. The modern server has no source for that
			// credit yet, so zero is sent - which is what legacy sends for an account
			// with no credit recorded.
			feedback.extremeM = 0;
			feedback.extremeW = 0;
			(void) channel; // channel is validated by the receiver, not echoed back
		}

		return BuildResponse(feedback, out);
	}
}
#include "LoginReceiver.h"

namespace Modern::Server
{
	void InMemoryLoginAuthenticator::AddAccount(const std::string& userId,
	                                           const std::string& password)
	{
		for (Account& account : m_accounts)
		{
			if (account.userId == userId)
			{
				account.password = password;
				return;
			}
		}
		m_accounts.push_back(Account{ userId, password });
	}

	LoginVerdict InMemoryLoginAuthenticator::Authenticate(const Network::LoginRequest& request)
	{
		LoginVerdict verdict{};

		for (const Account& account : m_accounts)
		{
			if (account.userId != request.userId)
			{
				continue;
			}
			if (account.password == request.password)
			{
				verdict.accepted = true;
				verdict.reason = LoginRejectReason::None;
			}
			else
			{
				verdict.accepted = false;
				verdict.reason = LoginRejectReason::BadPassword;
			}
			return verdict;
		}

		verdict.accepted = false;
		verdict.reason = LoginRejectReason::UnknownAccount;
		return verdict;
	}

	Status LoginReceiver::HandleLoginFrame(const std::vector<Network::WireU8>& frame,
	                                               const Network::MinTea& tea,
	                                               LoginVerdict& out)
	{
		return HandleLoginFrame(frame, tea,
		                         Network::LoginProtocol::PasswordDecryptWidth::Full21, out);
	}

	Status LoginReceiver::HandleLoginFrame(const std::vector<Network::WireU8>& frame,
	                                               const Network::MinTea& tea,
	                                               Network::LoginProtocol::PasswordDecryptWidth width,
	                                               LoginVerdict& out)
	{
		out = LoginVerdict{};
		m_sharedSecretFailed = false;
		m_lastRequest = Network::LoginRequest{};

		// Decode first: framing, garbage identity, size and TEA all live in the
		// protocol layer, and a malformed packet must be rejected before any
		// account is consulted.
		Network::LoginRequest request;
		if (const Status status =
		        Network::LoginProtocol::Decode(frame.data(), frame.size(), tea, width, request);
		    status.IsError())
		{
			return status;
		}

		// The shared-secret check, in legacy's position: after the fields are
		// decrypted, before anything looks at the account (s_CAgentServerMsgLogin.cpp:664).
		if (!m_expectedEncryptKey.empty() && request.encryptKey != m_expectedEncryptKey)
		{
			m_sharedSecretFailed = true;
			out.accepted = false;
			out.reason = LoginRejectReason::None; // legacy sends SUB_FAIL, not a credential reason
			return Ok();
		}

		out = m_authenticator.Authenticate(request);
		m_lastRequest = request;
		return Ok();
	}
}
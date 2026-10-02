#pragma once

// WORLD-001: the server-side login application boundary.
//
// Two things live here, and the line between them is the point:
//
//   ILoginAuthenticator   WHO is this user. Knows accounts, storage, hashing.
//   LoginReceiver         WHAT did the packet say. Knows framing, decoding.
//
// Neither knows about the other's concerns. LoginReceiver cannot see a socket
// and the authenticator cannot see a NET_MSG_GENERIC - which is what makes the
// protocol testable without a network and the account logic testable without a
// codec.
//
// Scope, deliberately: this is credential checking only. Character list, lobby
// discovery, world entry, character creation, session handoff and the login
// feedback packet are all later verticals. LoginResult therefore says "accepted
// or rejected" and nothing else, and no code here implies a character exists.
//
// Persistence is NOT implemented. ILoginAuthenticator is the seam where a real
// account store will go; WORLD-001 ships a deterministic in-memory
// implementation so the vertical is testable end to end without a database.

#include "LoginProtocol.h"

#include "types/Result.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Modern::Server
{
	// Why a login was refused. Distinct values because the operator log and any
	// future rate-limiting need to tell them apart; a single "invalid" would make
	// a credential-stuffing signal indistinguishable from a typo.
	enum class LoginRejectReason : std::uint8_t
	{
		None = 0,
		UnknownAccount,
		BadPassword,
	};

	// What the authenticator decided.
	struct LoginVerdict
	{
		bool             accepted = false;
		LoginRejectReason reason   = LoginRejectReason::None;
	};

	// The seam a real account store will implement.
	//
	// Takes the already-decoded plaintext credentials. It has no idea they
	// arrived as TEA over a garbage-prefixed frame, and it must never be handed
	// ciphertext: the split is deliberate so that a future implementation cannot
	// accidentally compare encrypted bytes.
	class ILoginAuthenticator
	{
	public:
		virtual ~ILoginAuthenticator() = default;

		// `request` has already been framed, garbage-stripped and TEA-decrypted by
		// LoginReceiver. An empty password is a legitimate attempt (the account may
		// have one), so emptiness is the authenticator's judgement, not the
		// protocol's.
		virtual LoginVerdict Authenticate(const Network::LoginRequest& request) = 0;
	};

	// A fixed set of accounts held in memory.
	//
	// Deterministic and dependency-free so the vertical can be tested without a
	// database. NOT a substitute for one: it holds credentials in process memory
	// in plaintext, which is fine for tests and unacceptable in production, and
	// that is precisely why it sits behind ILoginAuthenticator.
	class InMemoryLoginAuthenticator final : public ILoginAuthenticator
	{
	public:
		// Adds or replaces an account. Passwords are stored as given; this class
		// performs no hashing because legacy performs none on this path (see
		// s_CDbActionUser.cpp:396-410, where the decrypted password goes straight
		// to COdbcManager::UserCheck).
		void AddAccount(const std::string& userId, const std::string& password);

		LoginVerdict Authenticate(const Network::LoginRequest& request) override;

	private:
		struct Account
		{
			std::string userId;
			std::string password;
		};

		std::vector<Account> m_accounts;
	};

	// Applies the shared-secret check that legacy performs before touching the
	// account database, then delegates.
	//
	// Legacy compares the client's szEnCrypt against a passphrase the Login server
	// generated and pushed over the server backbone (V028 B.4); on mismatch it
	// replies EM_LOGIN_FB_SUB_FAIL and drops the connection
	// (s_CAgentServerMsgLogin.cpp:664-677). That check is a proof the client is
	// talking to the same server cluster, not a credential, which is why it is
	// modelled here rather than inside the authenticator.
	class LoginReceiver
	{
	public:
		// `expectedEncryptKey` is the passphrase this server cluster published. An
		// empty expectation disables the check, which exists for tests that are
		// about framing rather than membership.
		explicit LoginReceiver(ILoginAuthenticator& authenticator,
		                       std::string expectedEncryptKey = {}) noexcept
			: m_authenticator(authenticator),
			  m_expectedEncryptKey(std::move(expectedEncryptKey))
		{
		}

		// Handles one already-framed login message. `frame` must be exactly the bytes
		// of a single NET_MSG_GENERIC, which is what ConnectionFramer produces.
		//
		// `tea` is injected so the receiver owns no cipher state and tests can
		// share one.
		Status HandleLoginFrame(const std::vector<Network::WireU8>& frame,
		                                 const Network::MinTea& tea,
		                                 LoginVerdict& out);

		// As above, with an explicit password decrypt window for
		// bFeatureRegisterUseMD5.
		Status HandleLoginFrame(const std::vector<Network::WireU8>& frame,
		                                 const Network::MinTea& tea,
		                                 Network::LoginProtocol::PasswordDecryptWidth width,
		                                 LoginVerdict& out);

		// True when `frame` is a login message at all. Lets a caller dispatch
		// without attempting to decode a message of another type.
		static bool IsLoginFrame(const std::vector<Network::WireU8>& frame) noexcept
		{
			return Network::LoginProtocol::IsLoginMessage(frame.data(), frame.size());
		}

		// The shared-secret rejection is reported through the protocol, not through
		// LoginRejectReason, because legacy drops the connection rather than
		// replying with a credential failure. Callers distinguish it with this.
		bool LastFailureWasSharedSecret() const noexcept { return m_sharedSecretFailed; }

		// The decoded request from the last accepted frame. Exposed for tests and
		// for the later vertical that will build the login feedback packet.
		const Network::LoginRequest& LastRequest() const noexcept { return m_lastRequest; }

	private:
		ILoginAuthenticator& m_authenticator;
		std::string          m_expectedEncryptKey;
		bool                 m_sharedSecretFailed = false;
		Network::LoginRequest m_lastRequest{};
	};
}
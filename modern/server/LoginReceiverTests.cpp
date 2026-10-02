// WORLD-001: the server-side login receiver and authentication boundary.
//
// Tests 12, 13 and 16. The split under test is the point of this file:
//
//   LoginReceiver       framing -> decode -> shared secret -> verdict
//   ILoginAuthenticator credentials -> verdict
//
// So most tests here deliberately swap the authenticator: a stub that accepts
// everything proves the RECEIVER accepted a well-formed packet, and a stub that
// rejects everything proves rejection is the authenticator's call rather than
// something the protocol layer decides. Conflating those two is the mistake that
// would make an account-policy change look like a protocol change.
//
// The final test is the deterministic end-to-end path: client builder ->
// LoopbackTransport -> framer -> receiver -> authenticator. No socket, no
// database, no clock, no RNG.

#include "TestHarness.h"

#include "LoginProtocol.h"
#include "login/LoginReceiver.h"
#include "LoopbackTransport.h"
#include "MinTeaCodec.h"
#include "NetworkCodec.h"
#include "NetworkConnection.h"
#include "NetCompressCodec.h"
#include "NetworkTypes.h"
#include "login/World001LoginClient.h"

#include <cstring>

namespace
{
	using namespace Modern;
	using namespace Modern::Network;

	const MinTea& Tea()
	{
		static const MinTea instance;
		return instance;
	}

	// ENCRYPT_KEY is 12, so szEnCrypt holds at most 12 characters plus a NUL.
// Legacy truncates anything longer (StringCchCopy into ENCRYPT_KEY+1), and a
// passphrase that does not fit the field is not a meaningful test case.
	const char* const kSharedSecret = "clustersecr1";

	// Accepts anything. Isolates receiver behaviour from account policy.
	class AcceptAllAuthenticator final : public Server::ILoginAuthenticator
	{
	public:
		Server::LoginVerdict Authenticate(const LoginRequest& request) override
		{
			m_last = request;
			m_calls++;
			Server::LoginVerdict verdict{};
			verdict.accepted = true;
			return verdict;
		}

		int         m_calls = 0;
		LoginRequest m_last{};
	};

	// Rejects everything.
	class RejectAllAuthenticator final : public Server::ILoginAuthenticator
	{
	public:
		Server::LoginVerdict Authenticate(const LoginRequest&) override
		{
			m_calls++;
			Server::LoginVerdict verdict{};
			verdict.accepted = false;
			verdict.reason = Server::LoginRejectReason::BadPassword;
			return verdict;
		}

		int m_calls = 0;
	};

	// Encodes a login frame the way the shipped client would.
	std::vector<WireU8> BuildFrame(const std::string& user,
	                               const std::string& password,
	                               const std::string& encryptKey,
	                               const std::string& token)
	{
		Client::LoginRequestData data;
		data.userId = user;
		data.password = password;
		data.randomPassword = "54321";
		data.encryptKey = encryptKey;
		data.channel = 0;

		const Client::World001LoginClient client;
		std::vector<WireU8> frame;
		(void) client.EncodeLoginRequest(data, token, frame);
		return frame;
	}
}

// ---------------------------------------------------------------------------
// Test 13 - valid credentials
// ---------------------------------------------------------------------------

MODERN_TEST(LoginReceiver_ValidCredentialsAreAccepted)
{
	Server::InMemoryLoginAuthenticator auth;
	auth.AddAccount("tester", "pw123");

	Server::LoginReceiver receiver(auth, kSharedSecret);

	const std::vector<WireU8> frame = BuildFrame("tester", "pw123", kSharedSecret, "K9IHANA");

	Server::LoginVerdict verdict{};
	CHECK(receiver.HandleLoginFrame(frame, Tea(), verdict).IsOk());
	CHECK(verdict.accepted);
	CHECK(verdict.reason == Server::LoginRejectReason::None);
	CHECK(!receiver.LastFailureWasSharedSecret());

	// The receiver handed the authenticator exactly the decoded credentials.
	CHECK_EQ(receiver.LastRequest().userId, std::string("tester"));
	CHECK_EQ(receiver.LastRequest().password, std::string("pw123"));
	CHECK_EQ(receiver.LastRequest().randomPassword, std::string("54321"));
	CHECK_EQ(receiver.LastRequest().encryptKey, std::string(kSharedSecret));
}

MODERN_TEST(LoginReceiver_EveryGarbageTokenAuthenticates)
{
	// The token is transport noise; it must not affect the account decision.
	for (const char* token : LoginProtocol::GarbageTokens())
	{
		Server::InMemoryLoginAuthenticator auth;
		auth.AddAccount("tester", "pw123");

		Server::LoginReceiver receiver(auth, kSharedSecret);
		const std::vector<WireU8> frame = BuildFrame("tester", "pw123", kSharedSecret, token);

		Server::LoginVerdict verdict{};
		CHECK(receiver.HandleLoginFrame(frame, Tea(), verdict).IsOk());
		CHECK(verdict.accepted);
	}
}

// ---------------------------------------------------------------------------
// Test 12 - invalid credentials
// ---------------------------------------------------------------------------

MODERN_TEST(LoginReceiver_WrongPasswordIsRejectedAsBadPassword)
{
	Server::InMemoryLoginAuthenticator auth;
	auth.AddAccount("tester", "pw123");

	Server::LoginReceiver receiver(auth, kSharedSecret);
	const std::vector<WireU8> frame = BuildFrame("tester", "WRONG", kSharedSecret, "K9IHANA");

	Server::LoginVerdict verdict{};
	CHECK(receiver.HandleLoginFrame(frame, Tea(), verdict).IsOk());
	CHECK(!verdict.accepted);
	CHECK(verdict.reason == Server::LoginRejectReason::BadPassword);
}

MODERN_TEST(LoginReceiver_UnknownAccountIsDistinguishedFromBadPassword)
{
	// The two must be tellable apart: an operator watching for credential
	// stuffing needs to see which one is happening.
	Server::InMemoryLoginAuthenticator auth;
	auth.AddAccount("tester", "pw123");

	Server::LoginReceiver receiver(auth, kSharedSecret);

	const std::vector<WireU8> unknown = BuildFrame("nobody", "pw123", kSharedSecret, "K9IHANA");
	Server::LoginVerdict verdict{};
	CHECK(receiver.HandleLoginFrame(unknown, Tea(), verdict).IsOk());
	CHECK(!verdict.accepted);
	CHECK(verdict.reason == Server::LoginRejectReason::UnknownAccount);

	const std::vector<WireU8> wrong = BuildFrame("tester", "nope", kSharedSecret, "K9IHANA");
	CHECK(receiver.HandleLoginFrame(wrong, Tea(), verdict).IsOk());
	CHECK(!verdict.accepted);
	CHECK(verdict.reason == Server::LoginRejectReason::BadPassword);
}

// ---------------------------------------------------------------------------
// The shared-secret check, and boundary separation
// ---------------------------------------------------------------------------

MODERN_TEST(LoginReceiver_WrongSharedSecretIsRefusedBeforeAuthentication)
{
	// Legacy drops the connection on mismatch (s_CAgentServerMsgLogin.cpp:664-677)
	// and does so BEFORE consulting the account store. The authenticator must
	// therefore never be called.
	AcceptAllAuthenticator auth;
	Server::LoginReceiver receiver(auth, kSharedSecret);

	const std::vector<WireU8> frame = BuildFrame("tester", "pw123", "wrongpassphrase", "K9IHANA");

	Server::LoginVerdict verdict{};
	CHECK(receiver.HandleLoginFrame(frame, Tea(), verdict).IsOk());
	CHECK(!verdict.accepted);
	CHECK(receiver.LastFailureWasSharedSecret());

	// Even though the authenticator would have accepted.
	CHECK_EQ(auth.m_calls, 0);
}

MODERN_TEST(LoginReceiver_EmptyExpectedSecretSkipsTheCheck)
{
	// Exists so framing tests need not fabricate a passphrase.
	AcceptAllAuthenticator auth;
	Server::LoginReceiver receiver(auth);

	const std::vector<WireU8> frame = BuildFrame("tester", "pw123", "anything", "K9IHANA");

	Server::LoginVerdict verdict{};
	CHECK(receiver.HandleLoginFrame(frame, Tea(), verdict).IsOk());
	CHECK(verdict.accepted);
	CHECK(!receiver.LastFailureWasSharedSecret());
}

MODERN_TEST(LoginReceiver_MalformedPacketNeverReachesTheAuthenticator)
{
	// A receiver that cannot decode must not fall back to authenticating
	// something. Legacy decodes before it looks at accounts.
	AcceptAllAuthenticator auth;
	Server::LoginReceiver receiver(auth, kSharedSecret);

	// Truncated.
	std::vector<WireU8> frame = BuildFrame("tester", "pw123", kSharedSecret, "K9IHANA");
	frame.resize(frame.size() - 1);

	Server::LoginVerdict verdict{};
	CHECK(receiver.HandleLoginFrame(frame, Tea(), verdict).IsError());
	CHECK_EQ(auth.m_calls, 0);

	// Unknown garbage token.
	frame = BuildFrame("tester", "pw123", kSharedSecret, "K9IHANA");
	frame[kMessageHeaderSize] = 'Z';
	CHECK(receiver.HandleLoginFrame(frame, Tea(), verdict).IsError());
	CHECK_EQ(auth.m_calls, 0);
}

MODERN_TEST(LoginReceiver_NonLoginMessageIsNotHandled)
{
	// A heartbeat, framed but not a login. The receiver must not treat it as one.
	AcceptAllAuthenticator auth;
	Server::LoginReceiver receiver(auth, kSharedSecret);

	MessageHeader header;
	header.type = WellKnownMessage::kHeartbeatClientReq;
	std::vector<WireU8> frame;
	(void) Codec::EncodeMessage(header, {}, frame);

	CHECK(!Server::LoginReceiver::IsLoginFrame(frame));

	// And feeding it to the receiver is an error, not a silent acceptance.
	Server::LoginVerdict verdict{};
	CHECK(receiver.HandleLoginFrame(frame, Tea(), verdict).IsError());
	CHECK_EQ(auth.m_calls, 0);
}

MODERN_TEST(LoginReceiver_AuthenticatorDecisionIsPropagatedUnchanged)
{
	// The receiver must not second-guess the authenticator. A policy that rejects
	// valid credentials must be visible as-is.
	RejectAllAuthenticator auth;
	Server::LoginReceiver receiver(auth, kSharedSecret);

	const std::vector<WireU8> frame = BuildFrame("tester", "pw123", kSharedSecret, "K9IHANA");

	Server::LoginVerdict verdict{};
	CHECK(receiver.HandleLoginFrame(frame, Tea(), verdict).IsOk());
	CHECK(!verdict.accepted);
	CHECK(verdict.reason == Server::LoginRejectReason::BadPassword);
	CHECK_EQ(auth.m_calls, 1);
}

MODERN_TEST(LoginReceiver_Md5WindowDoesNotChangeTheVerdict)
{
	// Same finding as the protocol test, seen through the application boundary: the
	// password decrypt window is a transport detail and must not alter who may log
	// in.
	Server::InMemoryLoginAuthenticator auth;
	auth.AddAccount("tester", "pw123");
	Server::LoginReceiver receiver(auth, kSharedSecret);

	const std::vector<WireU8> frame = BuildFrame("tester", "pw123", kSharedSecret, "K9IHANA");

	Server::LoginVerdict full{};
	Server::LoginVerdict truncated{};
	CHECK(receiver.HandleLoginFrame(frame, Tea(),
	                                LoginProtocol::PasswordDecryptWidth::Full21, full).IsOk());
	CHECK(receiver.HandleLoginFrame(frame, Tea(),
	                                LoginProtocol::PasswordDecryptWidth::Truncated20, truncated).IsOk());

	CHECK(full.accepted);
	CHECK(truncated.accepted);
	CHECK_EQ(full.reason, truncated.reason);
}

// ---------------------------------------------------------------------------
// Test 16 - deterministic loopback integration
// ---------------------------------------------------------------------------

MODERN_TEST(Login_EndToEndOverLoopbackTransport)
{
	// The whole client -> server vertical, with no socket and no clock.
	//
	//   World001LoginClient -> raw bytes -> LoopbackTransport -> ConnectionFramer
	//   -> LoginReceiver -> ILoginAuthenticator
	//
	// The single send and the single recv are the modern stand-in for
	// CNetClient::SendBuffer2's one ::send per message. If the login ever started
	// needing an envelope, this path would have to change - which is the point.
	Server::InMemoryLoginAuthenticator auth;
	auth.AddAccount("tester", "pw123");

	auto [clientTransport, serverTransport] = LoopbackTransport::CreatePair();

	Server::LoginReceiver receiver(auth, kSharedSecret);

	Client::LoginRequestData data;
	data.userId = "tester";
	data.password = "pw123";
	data.randomPassword = "54321";
	data.encryptKey = kSharedSecret;
	data.channel = 0;

	const Client::World001LoginClient loginClient;
	std::vector<WireU8> frame;
	CHECK(loginClient.EncodeLoginRequest(data, Client::World001LoginClient::DeterministicToken(0), frame)
	          .IsOk());

	// One raw write, exactly as the shipped client does: CNetClient::SendBuffer2
	// passes one message's dwSize straight to ::send with no envelope.
	CHECK(clientTransport.Send(frame.data(), frame.size()).IsOk());

	// The server reads whatever arrived and frames it.
	ConnectionFramer framer;

	WireU8 chunk[256];
	std::size_t received = 0;
	{
		const Status status = serverTransport.Receive(chunk, sizeof(chunk), received);
		CHECK(status.IsOk());
	}

	CHECK_GT(received, std::size_t{ 0 });
	CHECK_EQ(received, frame.size());
	CHECK(framer.Feed(chunk, received) == FrameStatus::Ok);

	Message message;
	CHECK(framer.Next(message) == FrameStatus::Ok);
	CHECK_EQ(static_cast<WireU32>(message.header.type),
	         static_cast<WireU32>(LoginProtocol::kLoginMessageId));
	CHECK_EQ(static_cast<WireU32>(message.header.size), static_cast<WireU32>(frame.size()));
	CHECK(framer.Next(message) == FrameStatus::NeedMoreData);

	// The bytes the server received are the bytes the client sent, byte for byte.
	// This is the direction guard: if login ever needed a NET_COMPRESS envelope,
	// the framing here would differ and this comparison would fail.
	CHECK_EQ(std::memcmp(chunk, frame.data(), frame.size()), 0);
	CHECK(!NetCompress::IsEnvelope(Codec::ReadU32(chunk + 4)));

	// Finally decode and authenticate what arrived.
	Server::LoginVerdict verdict{};
	CHECK(receiver.HandleLoginFrame(frame, Tea(), verdict).IsOk());
	CHECK(verdict.accepted);
	CHECK_EQ(receiver.LastRequest().userId, std::string("tester"));
}
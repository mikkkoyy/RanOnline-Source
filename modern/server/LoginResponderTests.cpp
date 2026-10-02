// WORLD-002: the server -> client login response, end to end.
//
// The complete vertical, with no socket and no clock:
//
//   InMemoryLoginAuthenticator -> LoginResponder -> ServerBatchEncoder
//     -> NetCompress (LZO 1X) -> bytes -> LoginResponseClient -> LoginPhase
//
// This is the test that would catch a mistake in the direction split: if the
// response were accidentally sent raw, or accidentally encrypted like the request,
// the client would not decode it and this test fails.

#include "TestHarness.h"

#include "CompressionCodec.h"
#include "LoginResponseProtocol.h"
#include "login/LoginResponseClient.h"
#include "LoopbackTransport.h"
#include "NetworkTypes.h"
#include "ServerBatchEncoder.h"
#include "login/LoginReceiver.h"
#include "login/LoginResponder.h"

#include <cstring>

namespace
{
	using namespace Modern;
	using namespace Modern::Network;
	// LoginResponder / LoginVerdict live in Modern::Server. LoginPhase exists in BOTH
	// Modern::Server and Modern::Client - they are peers on the wire, deliberately not
	// shared - so every LoginPhase reference below is Client:: qualified.
	using namespace Modern::Server;

	const char* const kSharedSecret = "clustersecr1";
}

MODERN_TEST(LoginResponder_SuccessProducesAnEnvelopeTheClientAccepts)
{
	MinLzo1xCodec codec;
	ServerBatchEncoder batcher(codec);
	LoginResponder responder(batcher);

	Server::LoginVerdict verdict{};
	verdict.accepted = true;
	verdict.reason = Server::LoginRejectReason::None;

	std::vector<WireU8> frame;
	CHECK(responder.BuildResponse(verdict, "hero@example.com", 0, frame).IsOk());
	CHECK(responder.LastWasSuccess());
	CHECK(!frame.empty());

	// It went out enveloped, which is the whole point of this direction.
	CompressEnvelope envelope;
	CHECK(NetCompress::DecodeEnvelope(frame.data(), frame.size(), envelope).IsOk());

	Client::LoginResponseClient client(codec);
	client.NoteLoginSent();
	CHECK(client.Phase() == Client::LoginPhase::LoginSent);

	std::size_t handled = 0;
	CHECK(client.Feed(frame, handled).IsOk());
	CHECK(handled >= 1);

	CHECK(client.Phase() == Client::LoginPhase::LoginAccepted);
	CHECK(client.Feedback().IsSuccess());
	CHECK_EQ(client.Feedback().email, std::string("hero@example.com"));
	CHECK_EQ(static_cast<WireU32>(client.Feedback().result), WireU32{ 0 });
}

MODERN_TEST(LoginResponder_RejectionProducesARefusablyIncorrectCode)
{
	MinLzo1xCodec codec;
	ServerBatchEncoder batcher(codec);
	LoginResponder responder(batcher);

	Server::LoginVerdict verdict{};
	verdict.accepted = false;
	verdict.reason = Server::LoginRejectReason::BadPassword;

	std::vector<WireU8> frame;
	CHECK(responder.BuildResponse(verdict, "", 0, frame).IsOk());
	CHECK(!responder.LastWasSuccess());

	Client::LoginResponseClient client(codec);
	client.NoteLoginSent();

	std::size_t handled = 0;
	CHECK(client.Feed(frame, handled).IsOk());

	CHECK(client.Phase() == Client::LoginPhase::LoginRejected);
	CHECK(!client.Feedback().IsSuccess());
	// Wrong id/password, which is what legacy reports for both an unknown account
	// and a bad password - it has no distinct "no such account" code.
	CHECK(client.Feedback().result == LoginFeedbackResult::Incorrect);
	CHECK(client.Feedback().email.empty());
}

MODERN_TEST(LoginResponder_UnknownAccountMapsToTheLegacyIncorrectCode)
{
	// Recorded explicitly because it looks like an information leak and is not: the
	// legacy code space has no unknown-account value, so reporting one would be an
	// invention.
	CHECK(Server::ToWireResult(Server::LoginRejectReason::UnknownAccount) ==
	      LoginFeedbackResult::Incorrect);
	CHECK(Server::ToWireResult(Server::LoginRejectReason::BadPassword) ==
	      LoginFeedbackResult::Incorrect);
	CHECK(Server::ToWireResult(Server::LoginRejectReason::None) == LoginFeedbackResult::Ok);
}

MODERN_TEST(LoginResponder_FullLoginExchangeOverLoopback)
{
	// The complete path including the WORLD-001 request, so the two halves are
	// proven to compose: login request out, response back, client accepts.
	Server::InMemoryLoginAuthenticator auth;
	auth.AddAccount("tester", "pw123");

	LoginReceiver receiver(auth, kSharedSecret);
	MinLzo1xCodec codec;
	ServerBatchEncoder batcher(codec);
	LoginResponder responder(batcher);

// ---- client -> server: raw login request (WORLD-001, unchanged) ----
	//
	// The request path needs a MinTea, not the MinLzo1xCodec used for the envelope.
	// Two different ciphers, deliberately: the request fields are minTea-protected,
	// the response is not encrypted at all.
	MinTea tea;

	LoginRequest request;
	request.userId = "tester";
	request.password = "pw123";
	request.randomPassword = "54321";
	request.encryptKey = kSharedSecret;
	request.channel = 0;

	std::vector<WireU8> requestFrame;
	CHECK(LoginProtocol::Encode(request, "K9IHANA", tea, requestFrame).IsOk());

	// Not enveloped - the request direction is raw.
	CHECK(!NetCompress::IsEnvelope(Codec::ReadU32(requestFrame.data() + 4)));

	// ---- server decodes and authenticates ----
	LoginVerdict verdict{};
	CHECK(receiver.HandleLoginFrame(requestFrame, tea, verdict).IsOk());
	CHECK(verdict.accepted);
	CHECK(!receiver.LastFailureWasSharedSecret());

	// ---- server -> client: enveloped response ----
	std::vector<WireU8> responseFrame;
	CHECK(responder.BuildResponse(verdict, "tester@example.com", 0, responseFrame).IsOk());
	CHECK(NetCompress::IsEnvelope(Codec::ReadU32(responseFrame.data() + 4)));

	// ---- client decodes ----
	Client::LoginResponseClient client(codec);
	client.NoteLoginSent();

	std::size_t handled = 0;
	CHECK(client.Feed(responseFrame, handled).IsOk());
	CHECK(handled >= 1);

	CHECK(client.Phase() == Client::LoginPhase::LoginAccepted);
	CHECK_EQ(client.Feedback().email, std::string("tester@example.com"));
	CHECK(client.Feedback().IsSuccess());
	CHECK_EQ(client.Feedback().chaRemain, std::uint16_t{ 0 });
}

MODERN_TEST(LoginResponseClient_SurvivesByteAtATimeDelivery)
{
	// The case that breaks in the field and never in a naive test: the envelope
	// arrives across many reads, and a message can straddle two of them.
	MinLzo1xCodec codec;
	ServerBatchEncoder batcher(codec);
	LoginResponder responder(batcher);

	Server::LoginVerdict verdict{};
	verdict.accepted = true;

	std::vector<WireU8> frame;
	CHECK(responder.BuildResponse(verdict, "a@b.com", 0, frame).IsOk());
	CHECK(frame.size() > 12);

	Client::LoginResponseClient client(codec);
	client.NoteLoginSent();

	bool accepted = false;
	for (std::size_t i = 0; i < frame.size(); ++i)
	{
		std::size_t handled = 0;
		CHECK(client.Feed(frame.data() + i, 1, handled).IsOk());
		if (client.Phase() == Client::LoginPhase::LoginAccepted)
		{
			accepted = true;
		}
		CHECK(!client.IsFailed());
	}
	CHECK(accepted);
	CHECK_EQ(client.Feedback().email, std::string("a@b.com"));
}

MODERN_TEST(LoginResponseClient_MalformedEnvelopeIsDiscardedNotFatal)
{
	// Legacy drops a bad envelope and moves on (RcvMsgBuffer.cpp:145-160), so a
	// single bad frame must not wedge the connection.
	MinLzo1xCodec codec;
	Client::LoginResponseClient client(codec);
	client.NoteLoginSent();

	// Claims NET_MSG_COMPRESS, declares 12 bytes, but carries a broken LZO payload.
	std::vector<WireU8> bad(64, 0);
	bad[0] = 64;                                     // dwSize = 64
	bad[4] = static_cast<WireU8>(WellKnownMessage::kCompress & 0xFF);
	bad[5] = static_cast<WireU8>((WellKnownMessage::kCompress >> 8) & 0xFF);
	bad[6] = static_cast<WireU8>((WellKnownMessage::kCompress >> 16) & 0xFF);
	bad[7] = static_cast<WireU8>((WellKnownMessage::kCompress >> 24) & 0xFF);
	bad[8] = 1; // bCompress = true
	bad[12] = 0xAB;

	std::size_t handled = 0;
	CHECK(client.Feed(bad, handled).IsOk()); // dropped, not fatal
	CHECK(client.EnvelopeErrorCount() >= 1);
	CHECK(!client.IsFailed());
	CHECK(client.Phase() == Client::LoginPhase::LoginSent);

	// A good envelope afterwards still works.
	ServerBatchEncoder batcher(codec);
	LoginResponder responder(batcher);
	Server::LoginVerdict verdict{};
	verdict.accepted = true;
	std::vector<WireU8> good;
	CHECK(responder.BuildResponse(verdict, "ok@b.com", 0, good).IsOk());
	CHECK(client.Feed(good, handled).IsOk());
	CHECK(client.Phase() == Client::LoginPhase::LoginAccepted);
}

MODERN_TEST(LoginResponseClient_IgnoresNonLoginMessagesInTheBatch)
{
	// A batch can carry messages this handler does not own. Skipping them keeps the
	// handler honest rather than guessing at ids it was not given.
	MinLzo1xCodec codec;
	ServerBatchEncoder batcher(codec);

	MessageHeader other;
	other.type = WellKnownMessage::kHeartbeatServerAns;
	std::vector<WireU8> heartbeat;
	CHECK(Codec::EncodeMessage(other, { 1, 2, 3 }, heartbeat).IsOk());

	LoginResponder responder(batcher);
	Server::LoginVerdict verdict{};
	verdict.accepted = true;
	std::vector<WireU8> fb;
	CHECK(responder.BuildResponse(verdict, "x@y.com", 0, fb).IsOk());

	Client::LoginResponseClient client(codec);
	client.NoteLoginSent();
	std::size_t handled = 0;

	// The heartbeat alone must not produce a login phase change.
	CHECK(client.Feed(heartbeat, handled).IsOk());
	CHECK(client.Phase() == Client::LoginPhase::LoginSent);

	// And the response is still accepted afterwards.
	CHECK(client.Feed(fb, handled).IsOk());
	CHECK(client.Phase() == Client::LoginPhase::LoginAccepted);
}

MODERN_TEST(LoginResponseClient_PhaseNamesAreDistinct)
{
	// A log that cannot distinguish "no answer yet" from "refused" is worse than
	// no log.
	CHECK(std::string(ToString(Client::LoginPhase::Disconnected)) !=
	      std::string(ToString(Client::LoginPhase::LoginSent)));
	CHECK(std::string(ToString(Client::LoginPhase::LoginAccepted)) !=
	      std::string(ToString(Client::LoginPhase::LoginRejected)));
	CHECK_EQ(std::string(ToString(Client::LoginPhase::LoginSent)), std::string("LoginSent"));
}

MODERN_TEST(LoginResponseClient_RefusesAnAbsurdDeclaredSizeWithoutBufferingIt)
{
	// dwSize is attacker-controlled, so a huge value must be refused rather than
	// waited for. The handler is given a complete 64-byte frame that still never
	// becomes a valid envelope, and must not sit waiting for 4 GB.
	MinLzo1xCodec codec;
	Client::LoginResponseClient client(codec);
	client.NoteLoginSent();

	std::vector<WireU8> hostile(64, 0);
	hostile[0] = 0xFF; hostile[1] = 0xFF; hostile[2] = 0xFF; hostile[3] = 0xFF; // dwSize = 0xFFFFFFFF
	hostile[4] = static_cast<WireU8>(WellKnownMessage::kCompress & 0xFF);
	hostile[5] = static_cast<WireU8>((WellKnownMessage::kCompress >> 8) & 0xFF);
	hostile[6] = static_cast<WireU8>((WellKnownMessage::kCompress >> 16) & 0xFF);
	hostile[7] = static_cast<WireU8>((WellKnownMessage::kCompress >> 24) & 0xFF);
	hostile[8] = 1; // bCompress

	std::size_t handled = 0;
	CHECK(client.Feed(hostile, handled).IsOk());
	CHECK(client.EnvelopeErrorCount() >= 1);
	CHECK(!client.IsFailed());

	// Still usable afterwards: the bad size did not wedge the connection.
	ServerBatchEncoder batcher(codec);
	LoginResponder responder(batcher);
	Server::LoginVerdict verdict{};
	verdict.accepted = true;
	std::vector<WireU8> good;
	CHECK(responder.BuildResponse(verdict, "after@b.com", 0, good).IsOk());
	CHECK(client.Feed(good, handled).IsOk());
	CHECK(client.Phase() == Client::LoginPhase::LoginAccepted);
}

MODERN_TEST(LoginResponseClient_AcceptsAnEnvelopeSplitAcrossTwoReads)
{
	// The split lands in the middle of the compressed payload, so the size field is
	// already known and the body is still arriving. That is the "incomplete, not
	// malformed" case a naive decoder gets wrong.
	MinLzo1xCodec codec;
	ServerBatchEncoder batcher(codec);
	LoginResponder responder(batcher);

	Server::LoginVerdict verdict{};
	verdict.accepted = true;
	std::vector<WireU8> frame;
	CHECK(responder.BuildResponse(verdict, "split@b.com", 0, frame).IsOk());

	const std::size_t mid = frame.size() / 2;

	Client::LoginResponseClient client(codec);
	client.NoteLoginSent();

	std::size_t handled = 0;
	CHECK(client.Feed(frame.data(), mid, handled).IsOk());
	CHECK(client.Phase() == Client::LoginPhase::LoginSent); // nothing complete yet

	CHECK(client.Feed(frame.data() + mid, frame.size() - mid, handled).IsOk());
	CHECK(client.Phase() == Client::LoginPhase::LoginAccepted);
	CHECK_EQ(client.Feedback().email, std::string("split@b.com"));
}

MODERN_TEST(LoginResponseClient_SkipsForeignMessagesInsideARealBatch)
{
	// The earlier test feeds a RAW foreign message. This one puts the foreign message
	// inside a genuine envelope alongside LOGIN_FB, which is what the real batching
	// layer produces, and requires the foreign one to be skipped rather than to
	// abort the batch.
	MinLzo1xCodec codec;
	ServerBatchEncoder batcher(codec);

	MessageHeader other;
	other.type = WellKnownMessage::kHeartbeatServerAns;
	std::vector<WireU8> heartbeat;
	CHECK(Codec::EncodeMessage(other, { 1, 2, 3 }, heartbeat).IsOk());

	// The RAW LOGIN_FB message, not the responder's enveloped output - an envelope
	// is a finished network product and must never be nested inside another one.
	LoginFeedback feedback;
	feedback.result = LoginFeedbackResult::Ok;
	feedback.email = "batch@b.com";
	std::vector<WireU8> fb;
	CHECK(LoginResponse::Append(fb, feedback).IsOk());

	// Queue the foreign message FIRST, then the login feedback, then flush once, so
	// a single envelope carries both in that order. Each Add takes ONE message; its
	// second argument is the pending-output vector, not a second message.
	ServerBatchEncoder batcher2(codec);
	std::vector<WireU8> scratch;
	CHECK(batcher2.Add(heartbeat, scratch) == BatchAction::Buffered);
	CHECK(batcher2.Add(fb, scratch) == BatchAction::Buffered);
	CHECK_EQ(batcher2.Count(), std::size_t{ 2 });

	std::vector<WireU8> envelope;
	CHECK(batcher2.Flush(envelope));
	CHECK(NetCompress::IsEnvelope(Codec::ReadU32(envelope.data() + 4)));

	Client::LoginResponseClient client(codec);
	client.NoteLoginSent();
	std::size_t handled = 0;
	CHECK(client.Feed(envelope, handled).IsOk());
	CHECK(client.Phase() == Client::LoginPhase::LoginAccepted);
	CHECK_EQ(client.Feedback().email, std::string("batch@b.com"));
}

MODERN_TEST(LoginResponseClient_ResetClearsBufferedBytesAndPhase)
{
	// Reset has to clear the transport buffer, or bytes from a previous connection
	// would be prepended to the next one after a reconnect.
	MinLzo1xCodec codec;
	ServerBatchEncoder batcher(codec);
	LoginResponder responder(batcher);
	Server::LoginVerdict verdict{};
	verdict.accepted = true;
	std::vector<WireU8> frame;
	CHECK(responder.BuildResponse(verdict, "r@b.com", 0, frame).IsOk());

	Client::LoginResponseClient client(codec);
	client.NoteLoginSent();
	std::size_t handled = 0;

	// Leave a partial envelope buffered.
	CHECK(client.Feed(frame.data(), frame.size() / 2, handled).IsOk());
	client.Reset();
	CHECK(client.Phase() == Client::LoginPhase::Disconnected);

	// The stale bytes are gone, so a fresh whole frame is decoded cleanly.
	CHECK(client.Feed(frame, handled).IsOk());
	CHECK(client.Phase() == Client::LoginPhase::LoginAccepted);
	CHECK_EQ(client.Feedback().email, std::string("r@b.com"));
}

MODERN_TEST(LoginResponder_DoesNotTouchTheV030CompressionLayer)
{
	// The boundary is preserved: the responder's output is a NET_COMPRESS envelope
	// produced by the V030 layer, and LoginResponder adds no framing of its own.
	// Verified structurally by confirming the envelope header is exactly V030's and
	// that the responder never emits a bare NET_MSG_GENERIC as a final product.
	MinLzo1xCodec codec;
	ServerBatchEncoder batcher(codec);
	LoginResponder responder(batcher);

	Server::LoginVerdict verdict{};
	verdict.accepted = true;
	std::vector<WireU8> frame;
	CHECK(responder.BuildResponse(verdict, "", 0, frame).IsOk());

	// Envelope present.
	CHECK(NetCompress::IsEnvelope(Codec::ReadU32(frame.data() + 4)));
	// Its dwSize covers the whole frame.
	CHECK_EQ(static_cast<std::size_t>(Codec::ReadU32(frame.data())), frame.size());
	// 12-byte envelope, per V030.
	CompressEnvelope envelope;
	CHECK(NetCompress::DecodeEnvelope(frame.data(), frame.size(), envelope).IsOk());
	CHECK_EQ(envelope.payloadOffset, kCompressEnvelopeSize);
}
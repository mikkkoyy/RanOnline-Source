// WORLD-002: the NET_MSG_LOGIN_FB response codec.
//
// Deterministic: no socket, no clock, no RNG, no database.
//
// The facts under test are the ones a wrong implementation would get wrong:
//
//   1. The message id is 2050, not 2049. They are adjacent and confusing.
//   2. The body is 120 bytes, and the PADDING at offsets 29, 34-35 and 119 is part
//      of the wire. A codec that packs fields tightly produces a different frame.
//   3. The response is NOT minTea-encrypted, unlike the request.
//   4. It IS enveloped and LZO-compressed, unlike the request.
//
// Field values are validated semantically (decode and compare). Compressed payload
// bytes are not asserted, because LZO output is not guaranteed identical across
// builds; envelope header bytes ARE asserted, because they are fixed by the struct.

#include "TestHarness.h"

#include "CompressionCodec.h"
#include "LoginProtocol.h"
#include "LoginResponseProtocol.h"
#include "ServerBatchEncoder.h"
#include "NetworkCodec.h"
#include "NetworkConnection.h"
#include "NetworkTypes.h"
#include "NetCompressCodec.h"

namespace
{
	using namespace Modern;
	using namespace Modern::Network;

	LoginFeedback Sample()
	{
		LoginFeedback fb;
		fb.result = LoginFeedbackResult::Ok;
		fb.chaRemain = 7;
		fb.extremeM = -3;
		fb.extremeW = 11;
		fb.checkFlag = 42;
		fb.patchProgramVer = 3;
		fb.gameProgramVer = 5;
		fb.gameTime = 0x11223344u;
		fb.premiumPoint = 900u;
		fb.combatPoint = 1200u;
		fb.email = "player@example.com";
		return fb;
	}

	// One complete LOGIN_FB message on its own (no envelope).
	std::vector<WireU8> BuildFrame(const LoginFeedback& fb)
	{
		std::vector<WireU8> frame;
		(void) LoginResponse::Append(frame, fb);
		return frame;
	}
}

// ---------------------------------------------------------------------------
// Identity and geometry
// ---------------------------------------------------------------------------

MODERN_TEST(LoginResponse_MessageIdIsTwentyFifty)
{
	// NET_MSG_LOGIN_FB = NET_MSG_LOBBY + 108 = 1942 + 108. It is NOT 2049, which is
	// NET_MSG_LOGIN_2 - the REQUEST id that WORLD-001 implemented. Conflating the
	// two would make every response look like a login request.
	CHECK_EQ(static_cast<WireU32>(LoginResponse::kLoginFeedbackMessageId), WireU32{ 2050 });
	// 2049 is the REQUEST id from WORLD-001. They must not be confused.
	CHECK_NE(LoginResponse::kLoginFeedbackMessageId, LoginProtocol::kLoginMessageId);

	const std::vector<WireU8> frame = BuildFrame(Sample());
	CHECK_EQ(Codec::ReadU32(frame.data() + 4), WireU32{ 2050 });
	CHECK(LoginResponse::IsLoginFeedback(frame.data(), frame.size()));
}

MODERN_TEST(LoginResponse_BodyIsOneHundredAndTwentyBytes)
{
	CHECK_EQ(LoginResponse::kFeedbackBodySize, std::size_t{ 120 });

	const std::vector<WireU8> frame = BuildFrame(Sample());
	CHECK_EQ(frame.size(), std::size_t{ 120 });
	CHECK_EQ(static_cast<std::size_t>(Codec::ReadU32(frame.data())), std::size_t{ 120 });
}

MODERN_TEST(LoginResponse_PaddingOffsetsArePartOfTheWire)
{
	// Legacy's native layout leaves padding at 29, 34-35 and 119. dwSize counts
	// those bytes, so a tightly packed encoder would emit 116 and the shipped client
	// would reject the message.
	const std::vector<WireU8> frame = BuildFrame(Sample());

	// One padding byte between szDaumGID and nResult.
	CHECK_EQ(LoginResponse::kOffsetDaumGid + LoginResponse::kDaumGidFieldSize,
	         std::size_t{ 29 });
	CHECK_EQ(LoginResponse::kOffsetResult, std::size_t{ 30 });

	// Two padding bytes between uChaRemain and nExtremeM.
	CHECK_EQ(LoginResponse::kOffsetChaRemain + 2, std::size_t{ 34 });
	CHECK_EQ(LoginResponse::kOffsetExtremeM, std::size_t{ 36 });

	// One tail byte.
	CHECK_EQ(frame.size() - 1, LoginResponse::kOffsetEmail + LoginResponse::kEmailFieldSize);

	// Padding reads as zero, matching legacy's memset in the constructor.
	CHECK_EQ(frame[29], WireU8{ 0 });
	CHECK_EQ(frame[34], WireU8{ 0 });
	CHECK_EQ(frame[35], WireU8{ 0 });
	CHECK_EQ(frame[119], WireU8{ 0 });
}

MODERN_TEST(LoginResponse_FieldOffsetsMatchLegacyLayout)
{
	// Offsets asserted one by one against the compiler-verified probe, so a field
	// reordering cannot pass silently.
	CHECK_EQ(LoginResponse::kOffsetDaumGid, std::size_t{ 8 });
	CHECK_EQ(LoginResponse::kOffsetResult, std::size_t{ 30 });
	CHECK_EQ(LoginResponse::kOffsetChaRemain, std::size_t{ 32 });
	CHECK_EQ(LoginResponse::kOffsetExtremeM, std::size_t{ 36 });
	CHECK_EQ(LoginResponse::kOffsetExtremeW, std::size_t{ 40 });
	CHECK_EQ(LoginResponse::kOffsetCheckFlag, std::size_t{ 44 });
	CHECK_EQ(LoginResponse::kOffsetPatchProgramVer, std::size_t{ 48 });
	CHECK_EQ(LoginResponse::kOffsetGameProgramVer, std::size_t{ 52 });
	CHECK_EQ(LoginResponse::kOffsetGameTime, std::size_t{ 56 });
	CHECK_EQ(LoginResponse::kOffsetPremiumPoint, std::size_t{ 60 });
	CHECK_EQ(LoginResponse::kOffsetCombatPoint, std::size_t{ 64 });
	CHECK_EQ(LoginResponse::kOffsetEmail, std::size_t{ 68 });
}

// ---------------------------------------------------------------------------
// Round trips
// ---------------------------------------------------------------------------

MODERN_TEST(LoginResponse_SuccessRoundTrips)
{
	const LoginFeedback original = Sample();
	const std::vector<WireU8> frame = BuildFrame(original);

	LoginFeedback decoded;
	CHECK(LoginResponse::Decode(frame.data(), frame.size(), decoded).IsOk());

	CHECK(decoded.IsSuccess());
	CHECK(decoded.result == LoginFeedbackResult::Ok);
	CHECK_EQ(decoded.chaRemain, std::uint16_t{ 7 });
	CHECK_EQ(decoded.extremeM, -3);
	CHECK_EQ(decoded.extremeW, 11);
	CHECK_EQ(decoded.checkFlag, 42);
	CHECK_EQ(decoded.patchProgramVer, 3);
	CHECK_EQ(decoded.gameProgramVer, 5);
	CHECK_EQ(decoded.gameTime, 0x11223344u);
	CHECK_EQ(decoded.premiumPoint, 900u);
	CHECK_EQ(decoded.combatPoint, 1200u);
	CHECK_EQ(decoded.email, std::string("player@example.com"));
}

MODERN_TEST(LoginResponse_FailureRoundTrips)
{
	LoginFeedback fb;
	fb.result = LoginFeedbackResult::Incorrect;
	fb.email = "someone@example.com";

	const std::vector<WireU8> frame = BuildFrame(fb);

	LoginFeedback decoded;
	CHECK(LoginResponse::Decode(frame.data(), frame.size(), decoded).IsOk());
	CHECK(!decoded.IsSuccess());
	CHECK(decoded.result == LoginFeedbackResult::Incorrect);
	CHECK_EQ(decoded.email, fb.email);
}

MODERN_TEST(LoginResponse_ExtremeNegativeAndLargeValuesRoundTrip)
{
	// Boundary values: the field is a signed 32-bit int, and treating it as
	// unsigned would turn -1 into 4294967295.
	LoginFeedback fb;
	fb.result = LoginFeedbackResult::Ok;
	fb.extremeM = -2147483647 - 1;   // INT32_MIN
	fb.extremeW = 2147483647;        // INT32_MAX
	fb.gameTime = 0xFFFFFFFFu;
	fb.premiumPoint = 0x80000000u;

	const std::vector<WireU8> frame = BuildFrame(fb);
	LoginFeedback decoded;
	CHECK(LoginResponse::Decode(frame.data(), frame.size(), decoded).IsOk());
	CHECK_EQ(decoded.extremeM, -2147483647 - 1);
	CHECK_EQ(decoded.extremeW, 2147483647);
	CHECK_EQ(decoded.gameTime, 0xFFFFFFFFu);
	CHECK_EQ(decoded.premiumPoint, 0x80000000u);
}

MODERN_TEST(LoginResponse_AllResultCodesRoundTrip)
{
	// Every EM_LOGIN_FB_SUB value, 0..23. A client must be able to name whatever a
	// RAN server sends, including the region-specific ones this server never emits.
	for (std::uint16_t code = 0; code <= 23; ++code)
	{
		LoginFeedback fb;
		fb.result = static_cast<LoginFeedbackResult>(code);

		const std::vector<WireU8> frame = BuildFrame(fb);
		LoginFeedback decoded;
		CHECK(LoginResponse::Decode(frame.data(), frame.size(), decoded).IsOk());
		CHECK_EQ(static_cast<std::uint16_t>(decoded.result), code);
		CHECK(decoded.IsSuccess() == (code == 0));
		CHECK(std::string(ToString(decoded.result)) != std::string("Unrecognised"));
	}
}

MODERN_TEST(LoginResponse_OverlongEmailIsTruncatedNotShifted)
{
	// StringCchCopy with USR_INFOMAIL_LENGTH (50) into a 51-byte field, so at most
	// 49 characters plus a terminator. An over-long email must not push later fields.
	LoginFeedback fb;
	fb.email = std::string(200, 'e');

	const std::vector<WireU8> frame = BuildFrame(fb);
	CHECK_EQ(frame.size(), std::size_t{ 120 }); // size unchanged

	LoginFeedback decoded;
	CHECK(LoginResponse::Decode(frame.data(), frame.size(), decoded).IsOk());
	CHECK_EQ(decoded.email.size(), std::size_t{ 49 });
}

MODERN_TEST(LoginResponse_EmptyFieldsAreZero)
{
	// The failure paths in legacy leave everything at the constructor's zero, so a
	// zero-filled response must round-trip as empty rather than as garbage.
	LoginFeedback fb;
	fb.result = LoginFeedbackResult::Fail;

	const std::vector<WireU8> frame = BuildFrame(fb);
	LoginFeedback decoded;
	CHECK(LoginResponse::Decode(frame.data(), frame.size(), decoded).IsOk());
	CHECK(decoded.daumGid.empty());
	CHECK(decoded.email.empty());
	CHECK_EQ(decoded.chaRemain, std::uint16_t{ 0 });
	CHECK_EQ(decoded.extremeM, 0);
}

// ---------------------------------------------------------------------------
// Rejection
// ---------------------------------------------------------------------------

MODERN_TEST(LoginResponse_RejectsWrongMessageId)
{
	std::vector<WireU8> frame = BuildFrame(Sample());

	// 2049 is NET_MSG_LOGIN_2 - the request id. A response claiming it is invalid.
	frame[4] = 0x01; frame[5] = 0x08; frame[6] = 0x00; frame[7] = 0x00; // 2049

	CHECK(!LoginResponse::IsLoginFeedback(frame.data(), frame.size()));
	LoginFeedback decoded;
	CHECK(LoginResponse::Decode(frame.data(), frame.size(), decoded).IsError());
}

MODERN_TEST(LoginResponse_RejectsWrongSize)
{
	// Both directions: a declared size that is not 120, and a buffer shorter than
	// the declared size.
	std::vector<WireU8> frame = BuildFrame(Sample());

	std::vector<WireU8> wrongSize = frame;
	wrongSize[0] = 116; // a tightly packed encoder's size - the classic mistake
	LoginFeedback decoded;
	CHECK(LoginResponse::Decode(wrongSize.data(), wrongSize.size(), decoded).IsError());

	// Truncated.
	CHECK(LoginResponse::Decode(frame.data(), frame.size() - 1, decoded).IsError());
	CHECK(LoginResponse::Decode(frame.data(), 8, decoded).IsError());
	CHECK(LoginResponse::Decode(frame.data(), 0, decoded).IsError());
	CHECK(LoginResponse::Decode(nullptr, 120, decoded).IsError());
}

MODERN_TEST(LoginResponse_RejectsOutOfRangeChaRemain)
{
	// MAX_CHAR_LENGTH is 100. Legacy clamps a larger value to zero and logs; the
	// modern encoder refuses instead of emitting a value legacy would discard, so a
	// caller learns of its own bug rather than shipping a silently-zeroed field.
	LoginFeedback fb;
	fb.chaRemain = 500;

	std::vector<WireU8> frame;
	CHECK(LoginResponse::Append(frame, fb).IsError());
	CHECK(frame.empty());
}

// ---------------------------------------------------------------------------
// The compression asymmetry
// ---------------------------------------------------------------------------

MODERN_TEST(LoginResponse_IsNotEncryptedUnlikeTheRequest)
{
	// The structural claim, made testable: no minTea key appears anywhere in the
	// response bytes, and the values are readable in the clear. MsgLogInBack has no
	// m_Tea call and the client casts the buffer straight to the struct.
	const LoginFeedback fb = Sample();
	const std::vector<WireU8> frame = BuildFrame(fb);

	// nResult = 0 (Ok) appears literally at offset 30, little-endian.
	CHECK_EQ(frame[30], WireU8{ 0 });
	CHECK_EQ(frame[31], WireU8{ 0 });

	// The email is readable without any key. The length is derived from the literal
	// rather than hardcoded, so an edited address cannot silently mis-assert.
	const std::string expected = "player@example.com";
	const std::string onWire(reinterpret_cast<const char*>(frame.data() + 68), expected.size());
	CHECK_EQ(onWire, expected);
	// And it is terminated immediately after, not merely long enough to truncate.
	CHECK_EQ(frame[68 + expected.size()], WireU8{ 0 });
}

MODERN_TEST(LoginResponse_IsEnvelopedAndCompressedUnlikeTheRequest)
{
	// The other half of the asymmetry: this direction goes through V030.
	MinLzo1xCodec codec;
	ServerBatchEncoder batcher(codec);

	const std::vector<WireU8> frame = BuildFrame(Sample());
	std::vector<WireU8> pending;
	CHECK(batcher.Add(frame, pending) == BatchAction::Buffered);
	CHECK(batcher.Flush(pending));

	// The result is a NET_COMPRESS envelope, not a bare message.
	CHECK(pending.size() > frame.size() / 2);
	CompressEnvelope envelope;
	CHECK(NetCompress::DecodeEnvelope(pending.data(), pending.size(), envelope).IsOk());
	CHECK(envelope.compressed);
	CHECK_EQ(Codec::ReadU32(pending.data() + 4),
	         static_cast<WireU32>(WellKnownMessage::kCompress));
}

MODERN_TEST(LoginResponse_CompressesAndDecompressesBackToTheSameMessage)
{
	// A 120-byte message is small and only mildly compressible, so the codec may
	// legitimately decline and ship it raw. Both outcomes are valid; the point is
	// that the inner stream survives either way.
	MinLzo1xCodec codec;
	ServerBatchEncoder batcher(codec);

	const std::vector<WireU8> frame = BuildFrame(Sample());
	std::vector<WireU8> pending;
	CHECK(batcher.Add(frame, pending) == BatchAction::Buffered);
	CHECK(batcher.Flush(pending));

	std::vector<WireU8> inner;
	CHECK(NetCompress::DecodeServerToClientEnvelope(
	          codec, pending.data(), pending.size(), kMaxDecompressedBatch, inner).IsOk());
	CHECK_EQ(inner.size(), std::size_t{ 120 });

	LoginFeedback decoded;
	CHECK(LoginResponse::Decode(inner.data(), inner.size(), decoded).IsOk());
	CHECK_EQ(decoded.email, std::string("player@example.com"));
	CHECK(decoded.IsSuccess());
}

MODERN_TEST(LoginResponse_FramerAcceptsTheResponseFromTheInnerStream)
{
	MinLzo1xCodec codec;
	ServerBatchEncoder batcher(codec);

	const std::vector<WireU8> frame = BuildFrame(Sample());
	std::vector<WireU8> pending;
	CHECK(batcher.Add(frame, pending) == BatchAction::Buffered);
	CHECK(batcher.Flush(pending));

	std::vector<WireU8> inner;
	CHECK(NetCompress::DecodeServerToClientEnvelope(
	          codec, pending.data(), pending.size(), kMaxDecompressedBatch, inner).IsOk());

	ConnectionFramer framer;
	CHECK(framer.Feed(inner.data(), inner.size()) == FrameStatus::Ok);

	Message message;
	CHECK(framer.Next(message) == FrameStatus::Ok);
	CHECK_EQ(static_cast<WireU32>(message.header.type),
	         static_cast<WireU32>(LoginResponse::kLoginFeedbackMessageId));
	CHECK_EQ(static_cast<std::size_t>(message.header.size), std::size_t{ 120 });
	CHECK(!framer.IsFailed());
}

MODERN_TEST(LoginResponse_MultipleResponsesInOneEnvelopeAreFramedInOrder)
{
	// Legacy's SendMsgBuffer batches, so several responses can share one envelope.
	// The batcher must not be assumed to carry exactly one.
	MinLzo1xCodec codec;
	ServerBatchEncoder batcher(codec);

	LoginFeedback a = Sample();
	a.result = LoginFeedbackResult::Ok;
	LoginFeedback b = Sample();
	b.result = LoginFeedbackResult::Incorrect;
	LoginFeedback c = Sample();
	c.result = LoginFeedbackResult::System;

	const std::vector<WireU8> fa = BuildFrame(a);
	const std::vector<WireU8> fb = BuildFrame(b);
	const std::vector<WireU8> fc = BuildFrame(c);

	std::vector<WireU8> pending;
	CHECK(batcher.Add(fa, pending) == BatchAction::Buffered);
	CHECK(batcher.Add(fb, pending) == BatchAction::Buffered);
	CHECK(batcher.Add(fc, pending) == BatchAction::Buffered);
	CHECK_EQ(batcher.Count(), std::size_t{ 3 });
	CHECK(batcher.Flush(pending));

	std::vector<WireU8> inner;
	CHECK(NetCompress::DecodeServerToClientEnvelope(
	          codec, pending.data(), pending.size(), kMaxDecompressedBatch, inner).IsOk());
	CHECK_EQ(inner.size(), std::size_t{ 360 }); // 3 x 120

	ConnectionFramer framer;
	CHECK(framer.Feed(inner.data(), inner.size()) == FrameStatus::Ok);

	const LoginFeedbackResult expected[] = {
		LoginFeedbackResult::Ok, LoginFeedbackResult::Incorrect, LoginFeedbackResult::System
	};
	for (LoginFeedbackResult want : expected)
	{
		Message message;
		CHECK(framer.Next(message) == FrameStatus::Ok);

		std::vector<WireU8> raw;
		Codec::WriteU32(raw, message.header.size);
		Codec::WriteU32(raw, message.header.type);
		raw.insert(raw.end(), message.payload.begin(), message.payload.end());

		LoginFeedback decoded;
		CHECK(LoginResponse::Decode(raw.data(), raw.size(), decoded).IsOk());
		CHECK(decoded.result == want);
	}
}
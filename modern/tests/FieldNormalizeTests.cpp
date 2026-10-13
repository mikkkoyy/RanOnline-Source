// SKILL-023: regression tests for the inbound field-frame
// garbage-token normalization boundary.
//
// ===========================================================================
// WHAT THESE TESTS PROVE
// ===========================================================================
//
// The ASURA client splices one of five garbage tokens into EVERY
// non-login client->server message, immediately after the 8-byte
// header, and inflates dwSize by the token's length. The tests
// below cover, against the REAL codecs and the REAL framer:
//
//   * each of the five exact tokens, on the fixed-size messages
//     (3032/3034/3036/2359) and on the variable-length 3303;
//   * correct removal and declared-size correction;
//   * payload preservation, byte for byte, after the token;
//   * a token-free frame passing through untouched;
//   * a token-free frame whose payload deliberately BEGINS with
//     an exact token - the collision case - surviving untouched;
//   * every malformed shape (short frame, dwSize below the header,
//     dwSize above the frame, truncated token, unknown token-like
//     prefix, token-only payload, token + garbage byte) being left
//     for the message-specific decoder to reject, never guessed at;
//   * the login envelope (2049) and the compression envelope (170)
//     being out of bounds for this boundary by name;
//   * back-to-back frames on one TCP stream, fed one byte at a
//     time, so a partial frame is provably never normalized;
//   * the SKILL-021 zero-target sizing quirk (dwSize 159 with
//     wTARGET_NUM 0) surviving normalization.
//
// No test here fabricates a successful skill cast: normalization
// moves bytes, and the 3303 tests assert the SKILL-021 decoder's
// own fields, nothing more.

#include "FieldFrameNormalization.h"

#include "AttackProtocol.h"
#include "GotoProtocol.h"
#include "LoginProtocol.h"
#include "MessageReader.h"
#include "MovementStateProtocol.h"
#include "NetworkCodec.h"
#include "NetworkConnection.h"
#include "NetworkTypes.h"
#include "SkillProtocol.h"
#include "TestHarness.h"
#include "WorldEntryProtocol.h"

#include <cstddef>
#include <cstring>
#include <string>
#include <vector>

namespace ModernTests
{
namespace
{
	using namespace Modern;
	using namespace Modern::Network;

	// The wire header layout these tests re-write by hand: dwSize at
	// offset 0, nType at offset 4, 8 bytes in total. The protocol
	// layer expresses these as literals (Codec::DecodeMessage reads
	// `bytes + 4` directly), so the test file names them itself.
	constexpr std::size_t kWireSizeOffset = 0;
	constexpr std::size_t kWireIdOffset = 4;

	// ---- helpers ------------------------------------------------------

	// Splices a garbage token into a clean frame exactly where the
	// ASURA client puts it: immediately after the 8-byte header,
	// with dwSize inflated by the token's length (s_NetClient.cpp:
	// 893-922, SendMsgAddGarbageValue).
	std::vector<WireU8> SpliceGarbageToken(
	    const std::vector<WireU8>& cleanFrame, const char* token)
	{
		const std::size_t tokenLength = std::strlen(token);
		std::vector<WireU8> frame;
		frame.reserve(cleanFrame.size() + tokenLength);

		const WireU32 declaredSize =
		    static_cast<WireU32>(cleanFrame.size() + tokenLength);
		const WireU32 type = Codec::ReadU32(cleanFrame.data() +
		                                    kWireIdOffset);
		for (int byte = 0; byte < 4; ++byte)
		{
			frame.push_back(static_cast<WireU8>((declaredSize >> (8 * byte)) & 0xFFu));
		}
		for (int byte = 0; byte < 4; ++byte)
		{
			frame.push_back(static_cast<WireU8>((type >> (8 * byte)) & 0xFFu));
		}
		frame.insert(frame.end(), token, token + tokenLength);
		frame.insert(frame.end(),
		             cleanFrame.begin() + kMessageHeaderSize, cleanFrame.end());
		return frame;
	}

	// A finite float whose little-endian wire bytes are exactly `bits`.
	// Used to build a frame whose payload deliberately begins with
	// ASCII that spells a garbage token.
	float FloatFromBits(WireU32 bits)
	{
		float value = 0.0f;
		std::memcpy(&value, &bits, sizeof(value));
		return value;
	}

	// The payload bytes of `frame`, for byte-for-byte comparison.
	std::vector<WireU8> PayloadOf(const std::vector<WireU8>& frame)
	{
		return std::vector<WireU8>(frame.begin() + kMessageHeaderSize,
		                            frame.end());
	}

	bool SameBytes(const std::vector<WireU8>& left,
	               const std::vector<WireU8>& right)
	{
		return left.size() == right.size() &&
		       std::memcmp(left.data(), right.data(), left.size()) == 0;
	}

	// Overwrites the little-endian WireU32 at `offset` in `frame` -
	// Codec::WriteU32 APPENDS, and these tests need to re-write the
	// header of a frame that already exists.
	void OverwriteU32(std::vector<WireU8>& frame, std::size_t offset,
	                   WireU32 value)
	{
		for (int byte = 0; byte < 4; ++byte)
		{
			frame[offset + static_cast<std::size_t>(byte)] =
			    static_cast<WireU8>((value >> (8 * byte)) & 0xFFu);
		}
	}

	// The five tokens, each spliced into `cleanFrame`, normalized,
	// and appended to `normalized` as the normalized payloads -
	// proving removal, size correction and preservation in one
	// pass.
	void NormalizeEachToken(const std::vector<WireU8>& cleanFrame,
	                        std::vector<std::vector<WireU8>>& normalized)
	{
		for (const char* token : LoginProtocol::GarbageTokens())
		{
			const std::vector<WireU8> spliced =
			    SpliceGarbageToken(cleanFrame, token);
			Message message;
			REQUIRE(Codec::DecodeMessage(spliced.data(), spliced.size(),
			                             message)
			            .IsOk());
			REQUIRE(FieldFrame::NormalizeInbound(message).IsOk());
			normalized.push_back(message.payload);
		}
	}

	// ---- the five exact tokens -----------------------------------------

	// Each of the five tokens is removed from a 3034, and dwSize is
	// corrected to the clean 36 bytes - the declared size the GOTO
	// codec requires.
	MODERN_TEST(FieldNormalize_EachOfTheFiveTokensIsRemovedAndTheSizeCorrected)
	{
		const Goto::GotoRequest request{
			MovementState::kActRun,
			RanWire::Vector3{ 100.5f, -20.25f, 3.75f },
			RanWire::Vector3{ 150.0f, 0.0f, 9.0f } };
		std::vector<WireU8> clean;
		REQUIRE(Goto::GotoCodec::AppendGotoRequest(clean, request).IsOk());
		REQUIRE(clean.size() == Goto::kRequestSize);

		std::size_t tokenIndex = 0;
		for (const char* token : LoginProtocol::GarbageTokens())
		{
			const std::vector<WireU8> spliced =
			    SpliceGarbageToken(clean, token);
			REQUIRE(spliced.size() ==
			        clean.size() + std::strlen(token));

			Message message;
			REQUIRE(Codec::DecodeMessage(spliced.data(), spliced.size(),
			                             message).IsOk());
			// dwSize arrived inflated by the token length.
			REQUIRE(message.header.size ==
			        static_cast<WireU32>(clean.size() + std::strlen(token)));

			REQUIRE(FieldFrame::NormalizeInbound(message).IsOk());

			CHECK_EQ(message.header.size,
			         static_cast<WireU32>(Goto::kRequestSize));
			CHECK_EQ(message.payload.size(),
			         static_cast<std::size_t>(Goto::kRequestSize -
			                                  kMessageHeaderSize));

			// The normalized frame is byte-identical to the clean
			// frame, header included - so the corrected little-endian
			// dwSize is on the wire, not just in the struct.
			CHECK(SameBytes(ReconstructFrame(message), clean));
			++tokenIndex;
		}
		CHECK_EQ(tokenIndex, static_cast<std::size_t>(5));
	}

	// The payload behind the token survives normalization byte for
	// byte, on every dispatched message shape: the fixed-size 3032,
	// 3034, 3036 and 2359, and the variable-length 3303.
	MODERN_TEST(FieldNormalize_PayloadIsPreservedByteForByte)
	{
		// 3032 MOVESTATE.
		{
			const MovementState::MoveStateRequest request{
				MovementState::kActRun | MovementState::kActPeaceMode };
			std::vector<WireU8> clean;
			REQUIRE(
			    MovementState::MovementStateCodec::AppendMoveStateRequest(
			        clean, request)
			        .IsOk());
			std::vector<std::vector<WireU8>> payloads;
			NormalizeEachToken(clean, payloads);
			const std::vector<WireU8> expected = PayloadOf(clean);
			for (const std::vector<WireU8>& payload : payloads)
			{
				CHECK(SameBytes(payload, expected));
			}
		}

		// 3036 ATTACK.
		{
			const Attack::AttackRequest request{ Attack::kCrowPc, 77u, 3u,
			                                     0u };
			std::vector<WireU8> clean;
			REQUIRE(Attack::AttackCodec::AppendAttackRequest(clean, request)
			            .IsOk());
			std::vector<std::vector<WireU8>> payloads;
			NormalizeEachToken(clean, payloads);
			const std::vector<WireU8> expected = PayloadOf(clean);
			for (const std::vector<WireU8>& payload : payloads)
			{
				CHECK(SameBytes(payload, expected));
			}
		}

		// 2359 JOIN_FIELD_IDENTITY.
		{
			const FieldIdentity identity{
				WorldEntry::kJoinTypeFirst, 0x12345678u, 0x0ABCDEF0u,
				RanWire::DefaultCryptKey() };
			std::vector<WireU8> clean;
			REQUIRE(WorldEntryCodec::AppendFieldIdentity(clean, identity)
			            .IsOk());
			REQUIRE(clean.size() == WorldEntry::kIdentitySize);
			std::vector<std::vector<WireU8>> payloads;
			NormalizeEachToken(clean, payloads);
			const std::vector<WireU8> expected = PayloadOf(clean);
			for (const std::vector<WireU8>& payload : payloads)
			{
				CHECK(SameBytes(payload, expected));
			}
		}

		// 3303 REQ_SKILL, one target - the variable-length shape.
		{
			Skill::SkillRequest request{
				RanWire::NativeId{ 0x00140007u },
				RanWire::Vector3{ 100.0f, 0.0f, 200.0f } };
			request.targets.push_back(Skill::SkillTarget{ 2u, 0x1234u });
			std::vector<WireU8> clean;
			REQUIRE(Skill::SkillCodec::AppendSkillRequest(clean, request)
			            .IsOk());
			REQUIRE(clean.size() == Skill::kRequestHeaderSize + 4);
			std::vector<std::vector<WireU8>> payloads;
			NormalizeEachToken(clean, payloads);
			const std::vector<WireU8> expected = PayloadOf(clean);
			for (const std::vector<WireU8>& payload : payloads)
			{
				CHECK(SameBytes(payload, expected));
			}
		}
	}

	// ---- token-free frames ---------------------------------------------

	// An ordinary token-free frame is never modified: same header,
	// same payload, same reconstructed bytes.
	MODERN_TEST(FieldNormalize_TokenFreeFrameIsUnchanged)
	{
		const Goto::GotoRequest request{
			MovementState::kActRun,
			RanWire::Vector3{ 1.0f, 2.0f, 3.0f },
			RanWire::Vector3{ 4.0f, 5.0f, 6.0f } };
		std::vector<WireU8> clean;
		REQUIRE(Goto::GotoCodec::AppendGotoRequest(clean, request).IsOk());

		Message message;
		REQUIRE(Codec::DecodeMessage(clean.data(), clean.size(),
		                             message).IsOk());

		const Message copy = message;
		REQUIRE(FieldFrame::NormalizeInbound(message).IsOk());

		CHECK_EQ(message.header.size, copy.header.size);
		CHECK_EQ(message.header.type, copy.header.type);
		CHECK_EQ(message.payload.size(), copy.payload.size());
		CHECK(SameBytes(message.payload, copy.payload));
		CHECK(SameBytes(ReconstructFrame(message), clean));
	}

	// THE COLLISION GUARD.
	//
	// A token-free frame whose payload deliberately begins with the
	// exact bytes of a garbage token is NOT corrupted. "L8IDUL" is
	// 4C 38 49 44 55 4C: a dwActState of 0x4449384C spells "L8ID"
	// and a coordinate whose first two wire bytes are 55 4C spells
	// "UL". Both the 3034 and the 3303 shapes are built that way,
	// and both must survive normalization untouched and still decode.
	MODERN_TEST(FieldNormalize_APayloadBeginningWithAnExactTokenIsNotCorrupted)
	{
		// "L8IDUL" as the first six payload bytes of a 3034.
		{
			const Goto::GotoRequest request{
				0x4449384Cu, // "L8ID" little-endian
				RanWire::Vector3{ FloatFromBits(0x00004C55u), 0.0f, 0.0f },
				                                     // "UL"
				RanWire::Vector3{ 9.0f, 9.0f, 9.0f } };
			std::vector<WireU8> clean;
			REQUIRE(Goto::GotoCodec::AppendGotoRequest(clean, request)
			            .IsOk());

			// The premise: the payload really does begin with the
			// token bytes.
			REQUIRE(clean.size() > kMessageHeaderSize + 6);
			CHECK(std::memcmp(clean.data() + kMessageHeaderSize, "L8IDUL",
			                  6) == 0);

			Message message;
			REQUIRE(Codec::DecodeMessage(clean.data(), clean.size(),
			                             message).IsOk());
			REQUIRE(FieldFrame::NormalizeInbound(message).IsOk());

			// Untouched: the payload is still the legal clean size,
			// and the frame still decodes with the values it was
			// built with.
			CHECK_EQ(message.payload.size(),
			         static_cast<std::size_t>(Goto::kRequestSize -
			                                  kMessageHeaderSize));
			CHECK(SameBytes(ReconstructFrame(message), clean));

			Goto::GotoRequest decoded{};
			REQUIRE(Goto::GotoCodec::DecodeGotoRequest(
			    ReconstructFrame(message), decoded).IsOk());
			CHECK_EQ(decoded.actState, 0x4449384Cu);
		}

		// "L8IDUL" as the first six payload bytes of a 3303.
		{
			const Skill::SkillRequest request{
				RanWire::NativeId{ 0x4449384Cu }, // "L8ID"
				RanWire::Vector3{ FloatFromBits(0x00004C55u), 0.0f,
				                  0.0f } };       // "UL"
			std::vector<WireU8> clean;
			REQUIRE(Skill::SkillCodec::AppendSkillRequest(clean, request)
			            .IsOk());
			REQUIRE(clean.size() == Skill::kRequestHeaderSize);

			REQUIRE(clean.size() > kMessageHeaderSize + 6);
			CHECK(std::memcmp(clean.data() + kMessageHeaderSize, "L8IDUL",
			                  6) == 0);

			Message message;
			REQUIRE(Codec::DecodeMessage(clean.data(), clean.size(),
			                             message).IsOk());
			REQUIRE(FieldFrame::NormalizeInbound(message).IsOk());

			CHECK_EQ(message.payload.size(),
			         static_cast<std::size_t>(Skill::kRequestHeaderSize -
			                                  kMessageHeaderSize));
			CHECK(SameBytes(ReconstructFrame(message), clean));

			Skill::SkillRequest decoded{};
			REQUIRE(Skill::SkillCodec::DecodeSkillRequest(
			    ReconstructFrame(message), decoded).IsOk());
			CHECK_EQ(decoded.skillId.value, 0x4449384Cu);
			CHECK_EQ(decoded.declaredTargetCount, static_cast<WireU16>(0));
			CHECK(decoded.targets.empty());
		}
	}

	// ---- malformed shapes: fail safe, never guess ----------------------

	// A frame shorter than the 8-byte header cannot even be decoded,
	// and a hand-built Message whose declared size is below the
	// header is rejected by the boundary itself.
	MODERN_TEST(FieldNormalize_AFrameShorterThanTheHeaderIsRejected)
	{
		const std::vector<WireU8> partial = { 0x24, 0x00, 0x00 };
		Message message;
		CHECK(Codec::DecodeMessage(partial.data(), partial.size(),
		                           message).IsError());

		Message handBuilt;
		handBuilt.header.size = 3;
		handBuilt.header.type = Goto::kGotoId;
		const Status status = FieldFrame::NormalizeInbound(handBuilt);
		CHECK(status.IsError());
		CHECK_EQ(status.GetCode(), ErrorCode::InvalidArgument);
	}

	// A declared size smaller than the header is a lie about the
	// frame's own shape and is rejected before any token byte is
	// read.
	MODERN_TEST(FieldNormalize_ADeclaredSizeSmallerThanTheHeaderIsRejected)
	{
		std::vector<WireU8> frame;
		Codec::WriteU32(frame, 4); // dwSize below the header
		Codec::WriteU32(frame, Goto::kGotoId);
		Message message;
		CHECK(Codec::DecodeMessage(frame.data(), frame.size(),
		                           message).IsError());

		Message handBuilt;
		handBuilt.header.size = 4;
		handBuilt.header.type = Goto::kGotoId;
		const Status status = FieldFrame::NormalizeInbound(handBuilt);
		CHECK(status.IsError());
		CHECK_EQ(status.GetCode(), ErrorCode::InvalidArgument);
	}

	// A declared size larger than the received frame is truncation,
	// and the decoder refuses it - the boundary is never asked to
	// normalize bytes that have not arrived.
	MODERN_TEST(FieldNormalize_ADeclaredSizeLargerThanTheFrameIsRejected)
	{
		const Goto::GotoRequest request{
			MovementState::kActRun,
			RanWire::Vector3{ 1.0f, 2.0f, 3.0f },
			RanWire::Vector3{ 4.0f, 5.0f, 6.0f } };
		std::vector<WireU8> clean;
		REQUIRE(Goto::GotoCodec::AppendGotoRequest(clean, request).IsOk());

		// Promise 100 bytes, deliver the 36 the clean frame really
		// has.
		std::vector<WireU8> lying = clean;
		OverwriteU32(lying, kWireSizeOffset, 100);
		Message message;
		CHECK(Codec::DecodeMessage(lying.data(), lying.size(),
		                           message).IsError());
	}

	// A truncated token - a prefix of "K9IHANA" - matches no token,
	// strips nothing, and the decoder rejects the frame.
	MODERN_TEST(FieldNormalize_ATruncatedTokenIsNotStripped)
	{
		const Goto::GotoRequest request{
			MovementState::kActRun,
			RanWire::Vector3{ 1.0f, 2.0f, 3.0f },
			RanWire::Vector3{ 4.0f, 5.0f, 6.0f } };
		std::vector<WireU8> clean;
		REQUIRE(Goto::GotoCodec::AppendGotoRequest(clean, request).IsOk());

		const std::string partial = "K9I";
		std::vector<WireU8> frame;
		frame.reserve(clean.size() + partial.size());
		const WireU32 declaredSize =
		    static_cast<WireU32>(clean.size() + partial.size());
		const WireU32 type = Codec::ReadU32(clean.data() + kWireIdOffset);
		for (int byte = 0; byte < 4; ++byte)
		{
			frame.push_back(static_cast<WireU8>((declaredSize >> (8 * byte)) & 0xFFu));
		}
		for (int byte = 0; byte < 4; ++byte)
		{
			frame.push_back(static_cast<WireU8>((type >> (8 * byte)) & 0xFFu));
		}
		frame.insert(frame.end(), partial.begin(), partial.end());
		frame.insert(frame.end(),
		             clean.begin() + kMessageHeaderSize, clean.end());

		Message message;
		REQUIRE(Codec::DecodeMessage(frame.data(), frame.size(),
		                             message).IsOk());
		REQUIRE(FieldFrame::NormalizeInbound(message).IsOk());

		// Nothing was stripped: the payload still carries the
		// three prefix bytes, and the GOTO decoder refuses the
		// wrong size.
		CHECK_EQ(message.payload.size(),
		         static_cast<std::size_t>(Goto::kRequestSize -
		                                  kMessageHeaderSize + partial.size()));
		Goto::GotoRequest decoded{};
		CHECK(Goto::GotoCodec::DecodeGotoRequest(
		          ReconstructFrame(message), decoded)
		          .IsError());
	}

	// An unknown token-like prefix matches none of the five tokens
	// and is never stripped.
	MODERN_TEST(FieldNormalize_AnUnknownTokenLikePrefixIsNotStripped)
	{
		const Goto::GotoRequest request{
			MovementState::kActRun,
			RanWire::Vector3{ 1.0f, 2.0f, 3.0f },
			RanWire::Vector3{ 4.0f, 5.0f, 6.0f } };
		std::vector<WireU8> clean;
		REQUIRE(Goto::GotoCodec::AppendGotoRequest(clean, request).IsOk());

		const std::vector<WireU8> frame =
		    SpliceGarbageToken(clean, "XXXXXXX");
		Message message;
		REQUIRE(Codec::DecodeMessage(frame.data(), frame.size(),
		                             message).IsOk());
		REQUIRE(FieldFrame::NormalizeInbound(message).IsOk());

		CHECK_EQ(message.payload.size(),
		         static_cast<std::size_t>(Goto::kRequestSize -
		                                  kMessageHeaderSize + 7));
		CHECK(std::memcmp(message.payload.data(), "XXXXXXX", 7) == 0);
	}

	// A token followed by one byte too many: clean + g + 1 is a
	// legal size for NOTHING, so no token is stripped and the
	// decoder rejects the frame. This is the case that proves the
	// boundary never weakens a size check to make a packet pass.
	MODERN_TEST(FieldNormalize_ATokenPrefixedFrameWithExtraBytesIsNotStripped)
	{
		const Goto::GotoRequest request{
			MovementState::kActRun,
			RanWire::Vector3{ 1.0f, 2.0f, 3.0f },
			RanWire::Vector3{ 4.0f, 5.0f, 6.0f } };
		std::vector<WireU8> clean;
		REQUIRE(Goto::GotoCodec::AppendGotoRequest(clean, request).IsOk());

		std::vector<WireU8> frame = SpliceGarbageToken(clean, "K9IHANA");
		frame.push_back(0x00); // one byte past any legal shape
		OverwriteU32(frame, kWireSizeOffset,
		             static_cast<WireU32>(frame.size()));

		Message message;
		REQUIRE(Codec::DecodeMessage(frame.data(), frame.size(),
		                             message).IsOk());
		REQUIRE(FieldFrame::NormalizeInbound(message).IsOk());

		// The token is still there: stripping it would leave 29
		// payload bytes, which is not a legal GOTO size either.
		CHECK_EQ(message.payload.size(),
		         static_cast<std::size_t>(Goto::kRequestSize -
		                                  kMessageHeaderSize + 7 + 1));
		CHECK(std::memcmp(message.payload.data(), "K9IHANA", 7) == 0);
		Goto::GotoRequest decoded{};
		CHECK(Goto::GotoCodec::DecodeGotoRequest(
		          ReconstructFrame(message), decoded)
		          .IsError());
	}

	// A payload that is EXACTLY a token, with nothing behind it:
	// stripping would leave a zero-length payload, which is a legal
	// size for nothing, so it is not stripped.
	MODERN_TEST(FieldNormalize_ATokenOnlyPayloadIsNotStripped)
	{
		std::vector<WireU8> frame;
		const WireU32 declaredSize =
		    static_cast<WireU32>(kMessageHeaderSize + 6);
		const WireU32 type = Goto::kGotoId;
		for (int byte = 0; byte < 4; ++byte)
		{
			frame.push_back(static_cast<WireU8>((declaredSize >> (8 * byte)) & 0xFFu));
		}
		for (int byte = 0; byte < 4; ++byte)
		{
			frame.push_back(static_cast<WireU8>((type >> (8 * byte)) & 0xFFu));
		}
		// ONE pointer, not two literals: this toolchain does not
		// pool identical string literals, so "L8IDUL" and
		// "L8IDUL" + 6 would be two different objects and the
		// insert range would span everything between them.
		const char* const token = "L8IDUL";
		frame.insert(frame.end(), token, token + 6);

		Message message;
		REQUIRE(Codec::DecodeMessage(frame.data(), frame.size(),
		                             message).IsOk());
		REQUIRE(FieldFrame::NormalizeInbound(message).IsOk());

		CHECK_EQ(message.payload.size(), static_cast<std::size_t>(6));
		CHECK(std::memcmp(message.payload.data(), "L8IDUL", 6) == 0);
		Goto::GotoRequest decoded{};
		CHECK(Goto::GotoCodec::DecodeGotoRequest(
		          ReconstructFrame(message), decoded)
		          .IsError());
	}

	// A frame whose declared size disagrees with the payload it
	// carries is inconsistent, and the boundary refuses it rather
	// than normalize it.
	MODERN_TEST(FieldNormalize_AnInconsistentFrameIsRejected)
	{
		Message message;
		message.header.size = 100; // claims far more than it carries
		message.header.type = Goto::kGotoId;
		message.payload.assign(28, 0x00);
		const Status status = FieldFrame::NormalizeInbound(message);
		CHECK(status.IsError());
		CHECK_EQ(status.GetCode(), ErrorCode::InvalidArgument);
	}

	// An id the Field connection does not dispatch is never
	// normalized - the fail-closed choice. 3049 (UPDATE_SP) is a
	// real legacy id that is server->client only.
	MODERN_TEST(FieldNormalize_AnUndispatchedIdIsNeverNormalized)
	{
		const Goto::GotoRequest request{
			MovementState::kActRun,
			RanWire::Vector3{ 1.0f, 2.0f, 3.0f },
			RanWire::Vector3{ 4.0f, 5.0f, 6.0f } };
		std::vector<WireU8> clean;
		REQUIRE(Goto::GotoCodec::AppendGotoRequest(clean, request).IsOk());

		// Rebuild the same payload under an id the Field role
		// does not dispatch.
		std::vector<WireU8> other = SpliceGarbageToken(clean, "K9IHANA");
		OverwriteU32(other, kWireIdOffset, Goto::kGotoId + 1000);
		OverwriteU32(other, kWireSizeOffset,
		             static_cast<WireU32>(other.size()));

		Message message;
		REQUIRE(Codec::DecodeMessage(other.data(), other.size(),
		                             message).IsOk());
		REQUIRE(FieldFrame::NormalizeInbound(message).IsOk());

		CHECK_EQ(message.header.type, Goto::kGotoId + 1000);
		CHECK_EQ(message.payload.size(),
		         static_cast<std::size_t>(Goto::kRequestSize -
		                                  kMessageHeaderSize + 7));
		CHECK(std::memcmp(message.payload.data(), "K9IHANA", 7) == 0);
	}

	// ---- the two out-of-bounds envelopes -------------------------------

	// NET_MSG_LOGIN_2 carries the same token, but it is LoginProtocol's
	// to strip: this boundary must not touch login semantics.
	MODERN_TEST(FieldNormalize_TheLoginEnvelopeIsNotModified)
	{
		std::vector<WireU8> loginFrame;
		Codec::WriteU32(loginFrame, 8 + 16 + 7);
		Codec::WriteU32(loginFrame, LoginProtocol::kLoginMessageId);
		// ONE pointer, not two literals: identical string literals
		// are not pooled by this toolchain, so two occurrences of
		// "K9IHANA" can be two different objects and the insert
		// range would span everything between them.
		const char* const token = "K9IHANA";
		loginFrame.insert(loginFrame.end(), token, token + 7);
		loginFrame.insert(loginFrame.end(), 16, 0x41);

		Message message;
		REQUIRE(Codec::DecodeMessage(loginFrame.data(), loginFrame.size(),
		                             message).IsOk());
		REQUIRE(FieldFrame::NormalizeInbound(message).IsOk());

		CHECK_EQ(message.header.type, LoginProtocol::kLoginMessageId);
		CHECK_EQ(message.payload.size(), static_cast<std::size_t>(23));
		CHECK(std::memcmp(message.payload.data(), "K9IHANA", 7) == 0);
	}

	// NET_COMPRESS is the server->client compression envelope. Inbound
	// field traffic is never compressed and no compression may be
	// added to it here.
	MODERN_TEST(FieldNormalize_TheCompressionEnvelopeIsNotModified)
	{
		std::vector<WireU8> frame;
		Codec::WriteU32(frame, 8 + 16 + 9);
		Codec::WriteU32(frame, WellKnownMessage::kCompress);
		// ONE pointer, not two literals: identical string literals
		// are not pooled by this toolchain, so two occurrences of
		// "O5FDASEOT" can be two different objects and the insert
		// range would span everything between them.
		const char* const token = "O5FDASEOT";
		frame.insert(frame.end(), token, token + 9);
		frame.insert(frame.end(), 16, 0x42);

		Message message;
		REQUIRE(Codec::DecodeMessage(frame.data(), frame.size(),
		                             message).IsOk());
		REQUIRE(FieldFrame::NormalizeInbound(message).IsOk());

		CHECK_EQ(message.header.type, WellKnownMessage::kCompress);
		CHECK_EQ(message.payload.size(), static_cast<std::size_t>(25));
		CHECK(std::memcmp(message.payload.data(), "O5FDASEOT", 9) == 0);
	}

	// ---- the fixed-size field messages, token-prefixed ---------------

	// 3032 MOVESTATE and 3036 ATTACK, each with all five tokens:
	// normalized, and decoded by their own codecs with the values
	// the encoder wrote.
	MODERN_TEST(FieldNormalize_TokenPrefixedMoveStateAndAttackDecode)
	{
		for (const char* token : LoginProtocol::GarbageTokens())
		{
			// 3032.
			{
				const MovementState::MoveStateRequest request{
					MovementState::kActRun };
				std::vector<WireU8> clean;
				REQUIRE(MovementState::MovementStateCodec::
				            AppendMoveStateRequest(clean, request)
				            .IsOk());

				const std::vector<WireU8> frame =
				    SpliceGarbageToken(clean, token);
				Message message;
				REQUIRE(Codec::DecodeMessage(frame.data(), frame.size(),
				                             message).IsOk());
				REQUIRE(FieldFrame::NormalizeInbound(message).IsOk());

				MovementState::MoveStateRequest decoded{};
				REQUIRE(MovementState::MovementStateCodec::
				            DecodeMoveStateRequest(ReconstructFrame(message),
				                                   decoded)
				            .IsOk());
				CHECK_EQ(decoded.actState,
				         static_cast<WireU32>(MovementState::kActRun));
			}

			// 3036.
			{
				const Attack::AttackRequest request{ Attack::kCrowPc, 4242u,
				                                     1u, 0u };
				std::vector<WireU8> clean;
				REQUIRE(Attack::AttackCodec::AppendAttackRequest(clean,
				                                                 request)
				            .IsOk());

				const std::vector<WireU8> frame =
				    SpliceGarbageToken(clean, token);
				Message message;
				REQUIRE(Codec::DecodeMessage(frame.data(), frame.size(),
				                             message).IsOk());
				REQUIRE(FieldFrame::NormalizeInbound(message).IsOk());

				Attack::AttackRequest decoded{};
				REQUIRE(Attack::AttackCodec::DecodeAttackRequest(
				    ReconstructFrame(message), decoded).IsOk());
				CHECK_EQ(decoded.targetCrow, static_cast<WireU32>(Attack::kCrowPc));
				CHECK_EQ(decoded.targetId, 4242u);
				CHECK_EQ(decoded.aniSel, 1u);
			}
		}
	}

	// 2359 JOIN_FIELD_IDENTITY - the LOBBY-range id a token-blind
	// "id >= kGCtrlBase" rule would wrongly skip - with all five
	// tokens, decoded by the identity codec.
	MODERN_TEST(FieldNormalize_TokenPrefixedIdentityDecodes)
	{
		const FieldIdentity identity{
			WorldEntry::kJoinTypeMoveMap, 0x00F00DB1u, 0x0000C0DEu,
			RanWire::DefaultCryptKey() };
		std::vector<WireU8> clean;
		REQUIRE(WorldEntryCodec::AppendFieldIdentity(clean, identity)
		            .IsOk());
		REQUIRE(clean.size() == WorldEntry::kIdentitySize);

		for (const char* token : LoginProtocol::GarbageTokens())
		{
			const std::vector<WireU8> frame =
			    SpliceGarbageToken(clean, token);
			Message message;
			REQUIRE(Codec::DecodeMessage(frame.data(), frame.size(),
			                             message).IsOk());
			REQUIRE(FieldFrame::NormalizeInbound(message).IsOk());

			CHECK_EQ(message.header.size,
			         static_cast<WireU32>(WorldEntry::kIdentitySize));

			FieldIdentity decoded{};
			REQUIRE(WorldEntryCodec::DecodeFieldIdentity(
			    ReconstructFrame(message), decoded).IsOk());
			CHECK_EQ(decoded.joinType, WorldEntry::kJoinTypeMoveMap);
			CHECK_EQ(decoded.gaeaId, 0x00F00DB1u);
			CHECK_EQ(decoded.slotFieldAgent, 0x0000C0DEu);
		}
	}

	// ---- the variable-length skill request -----------------------------

	// 3303 with one target, token-prefixed: normalized to 67 bytes
	// and decoded by the SKILL-021 decoder, with the target the
	// encoder wrote.
	MODERN_TEST(FieldNormalize_TokenPrefixedSkillRequestDecodes)
	{
		Skill::SkillRequest request{
			RanWire::NativeId{ 0x00140007u },
			RanWire::Vector3{ 100.0f, 0.0f, 200.0f } };
		request.targets.push_back(Skill::SkillTarget{ 2u, 0x1234u });

		std::vector<WireU8> clean;
		REQUIRE(Skill::SkillCodec::AppendSkillRequest(clean, request)
		            .IsOk());
		REQUIRE(clean.size() == Skill::kRequestHeaderSize + 4);

		const std::vector<WireU8> frame =
		    SpliceGarbageToken(clean, "K9IHANA");
		Message message;
		REQUIRE(Codec::DecodeMessage(frame.data(), frame.size(),
		                             message).IsOk());
		REQUIRE(FieldFrame::NormalizeInbound(message).IsOk());

		CHECK_EQ(message.header.size, static_cast<WireU32>(clean.size()));

		Skill::SkillRequest decoded{};
		REQUIRE(Skill::SkillCodec::DecodeSkillRequest(
		    ReconstructFrame(message), decoded).IsOk());
		CHECK_EQ(decoded.skillId.value, 0x00140007u);
		CHECK_EQ(decoded.targetPosition.x, 100.0f);
		CHECK_EQ(decoded.targetPosition.y, 0.0f);
		CHECK_EQ(decoded.targetPosition.z, 200.0f);
		CHECK_EQ(decoded.declaredTargetCount, static_cast<WireU16>(1));
		CHECK_EQ(decoded.targets.size(), static_cast<std::size_t>(1));
		CHECK_EQ(decoded.targets[0].crow, static_cast<WireU16>(2));
		CHECK_EQ(decoded.targets[0].id, static_cast<WireU16>(0x1234u));
	}

	// The SKILL-021 sizing quirk: legacy sends a zero-target cast as
	// a FULL 159-byte request (dwSize 159, wTARGET_NUM 0, 96 bytes
	// of never-read array tail). Normalization must leave that shape
	// exactly as the decoder expects it: 159 bytes, zero declared
	// targets.
	MODERN_TEST(FieldNormalize_TheZeroTargetSizingQuirkSurvives)
	{
		// The clean minimal request: 63 bytes, wTARGET_NUM 0.
		const Skill::SkillRequest request{
			RanWire::NativeId{ 0x00090001u },
			RanWire::Vector3{ 50.0f, 50.0f, 50.0f } };
		std::vector<WireU8> minimal;
		REQUIRE(Skill::SkillCodec::AppendSkillRequest(minimal, request)
		            .IsOk());
		REQUIRE(minimal.size() == Skill::kRequestHeaderSize);

		// The quirk frame: the same 63 bytes, padded to the full
		// 159 the legacy client declares, with dwSize saying 159.
		std::vector<WireU8> quirk = minimal;
		quirk.resize(Skill::kRequestMaxSize, 0x00);
		OverwriteU32(quirk, kWireSizeOffset,
		             static_cast<WireU32>(Skill::kRequestMaxSize));
		REQUIRE(quirk.size() == Skill::kRequestMaxSize);

		const std::vector<WireU8> frame =
		    SpliceGarbageToken(quirk, "O5FDASEOT");
		Message message;
		REQUIRE(Codec::DecodeMessage(frame.data(), frame.size(),
		                             message).IsOk());
		REQUIRE(FieldFrame::NormalizeInbound(message).IsOk());

		CHECK_EQ(message.header.size,
		         static_cast<WireU32>(Skill::kRequestMaxSize));
		CHECK(SameBytes(ReconstructFrame(message), quirk));

		Skill::SkillRequest decoded{};
		REQUIRE(Skill::SkillCodec::DecodeSkillRequest(
		    ReconstructFrame(message), decoded).IsOk());
		CHECK_EQ(decoded.skillId.value, 0x00090001u);
		CHECK_EQ(decoded.declaredTargetCount, static_cast<WireU16>(0));
		CHECK_EQ(decoded.targetCapacity, static_cast<std::size_t>(24));
		CHECK(decoded.targets.empty());
	}

	// ---- TCP stream semantics ------------------------------------------

	// Back-to-back frames on one stream, fed ONE BYTE AT A TIME: a
	// partial frame is never normalized (the framer never hands one
	// over), and a token-prefixed frame followed by an ordinary
	// frame - and the other two orders - all decode to the same
	// values.
	MODERN_TEST(FieldNormalize_BackToBackFramesOnOneStream)
	{
		const Goto::GotoRequest first{
			MovementState::kActRun,
			RanWire::Vector3{ 1.0f, 2.0f, 3.0f },
			RanWire::Vector3{ 10.0f, 0.0f, 0.0f } };
		std::vector<WireU8> cleanFirst;
		REQUIRE(Goto::GotoCodec::AppendGotoRequest(cleanFirst, first)
		            .IsOk());

		const Goto::GotoRequest second{
			MovementState::kActRun | MovementState::kActPeaceMode,
			RanWire::Vector3{ 4.0f, 5.0f, 6.0f },
			RanWire::Vector3{ 20.0f, 0.0f, 0.0f } };
		std::vector<WireU8> cleanSecond;
		REQUIRE(Goto::GotoCodec::AppendGotoRequest(cleanSecond, second)
		            .IsOk());

		// Three stream shapes: token-prefixed then ordinary,
		// ordinary then token-prefixed, and two token-prefixed
		// frames with different tokens. Each stream is the
		// CONCATENATION of two whole frames.
		std::vector<WireU8> streamA =
		    SpliceGarbageToken(cleanFirst, "K9IHANA");
		streamA.insert(streamA.end(), cleanSecond.begin(),
		               cleanSecond.end());

		std::vector<WireU8> streamB = cleanSecond;
		// ONE temporary: begin() and end() must come from
		// the same vector, or the insert range would span
		// two unrelated heap allocations.
		const std::vector<WireU8> prefixed =
		    SpliceGarbageToken(cleanFirst, "L8IDUL");
		streamB.insert(streamB.end(), prefixed.begin(),
		               prefixed.end());

		std::vector<WireU8> streamC =
		    SpliceGarbageToken(cleanFirst, "M7HSET");
		const std::vector<WireU8> secondToken =
		    SpliceGarbageToken(cleanSecond, "N6GNET");
		streamC.insert(streamC.end(), secondToken.begin(),
		               secondToken.end());

		const std::vector<std::vector<WireU8>> streams = {
		    streamA, streamB, streamC };

		for (const std::vector<WireU8>& stream : streams)
		{
			ConnectionFramer framer;
			std::size_t decoded = 0;
			for (const WireU8 byte : stream)
			{
				REQUIRE(framer.Feed(&byte, 1) == FrameStatus::Ok);

				Message message;
				while (framer.Next(message) == FrameStatus::Ok)
				{
					REQUIRE(FieldFrame::NormalizeInbound(message).IsOk());

					Goto::GotoRequest decodedRequest{};
					REQUIRE(Goto::GotoCodec::DecodeGotoRequest(
					    ReconstructFrame(message), decodedRequest)
					            .IsOk());
					++decoded;
				}
			}
			CHECK_EQ(decoded, static_cast<std::size_t>(2));
		}
	}
}
}

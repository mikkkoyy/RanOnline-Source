// SKILL-021: codec tests for the SKILL-CAST protocol - 3303, 3304 and 3305.
//
// No socket, on purpose and for the reason GotoProtocolTests.cpp gives: these
// three packets are framed and measured independently of a transport, and the
// authority questions 3303 raises (whose character is casting, what the client
// CLAIMED, which bytes the server may read) are all settled before a byte is
// framed.
//
// ---------------------------------------------------------------------------
// WHY THESE TESTS ARE MOSTLY HAND-BUILT BYTES
// ---------------------------------------------------------------------------
//
// 3303 is the first VARIABLE-LENGTH message in the modern tree, so the usual
// "encode, decode, compare fields" round trip proves much less here than it does
// for a fixed-size packet: encoder and decoder could agree on a completely wrong
// layout - a target array two entries early, a float half an offset low, a
// length formula off by four - and the round trip would still be perfectly
// green, because both ends would be wrong the same way.
//
// So the frames here are built byte by byte from the offsets in
// GLContrlSkillMsg.h and then read back with a hand-rolled little-endian reader
// that shares no code with the codec. The round trips that do exist are
// ADDITIONAL, not the primary evidence.
//
// The cases defend:
//
//   ids        3303/3304/3305, and NOT 3034/3035/3041/3042
//   layout     the measured offsets, read back independently
//   alignment  the six UNALIGNED floats at 29/41/45/49 and the enums at 53/57
//   length     all 25 legal request lengths, and the four named illegal ones
//   bounds     wTARGET_NUM is REPORTED, never obeyed - the 159-with-zero case
//   name       33 raw bytes, never strlen'd
//   broadcast  a 32-bit caster beside 16-bit targets

#include "TestHarness.h"

#include "AttackProtocol.h"
#include "GotoProtocol.h"
#include "MovementStateProtocol.h"
#include "NetworkTypes.h"
#include "RanWirePrimitives.h"
#include "SkillProtocol.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

namespace
{
	using namespace Modern;
	using namespace Modern::Network;
	namespace SkillNS = Modern::Network::Skill;
	namespace SkillCodec = Modern::Network::Skill::SkillCodec;

	// ------------------------------------------------------------------------
	// An independent reader.
	//
	// Written out by hand rather than reusing the codec's own Codec::ReadU32, so
	// the offset and byte-order assertions are genuinely INDEPENDENT of the code
	// under test. Reading a value back with the codec that just wrote it would
	// pass even if the field were written at the wrong offset in the wrong order,
	// which is precisely the bug class these packets are most prone to.
	// ------------------------------------------------------------------------
	WireU16 PeekLE16(const std::vector<WireU8>& frame, std::size_t offset)
	{
		return static_cast<WireU16>(static_cast<WireU16>(frame[offset]) |
		                            (static_cast<WireU16>(frame[offset + 1]) << 8));
	}

	WireU32 PeekLE32(const std::vector<WireU8>& frame, std::size_t offset)
	{
		WireU32 value = 0;
		for (std::size_t i = 0; i < 4; ++i)
		{
			value |= static_cast<WireU32>(frame[offset + i]) << (8 * i);
		}
		return value;
	}

	// The same independence for a float: the wire carries an IEEE-754 single, so
	// this decomposes the bit pattern one byte at a time rather than
	// reinterpreting four bytes as a float.
	float PeekLEFloat(const std::vector<WireU8>& frame, std::size_t offset)
	{
		const WireU32 bits = PeekLE32(frame, offset);
		float         value = 0.0f;
		std::memcpy(&value, &bits, sizeof(value));
		return value;
	}

	// Write helpers, for the same reason.
	void PokeLE16(std::vector<WireU8>& frame, std::size_t offset, WireU16 value)
	{
		frame[offset]     = static_cast<WireU8>(value & 0xFFu);
		frame[offset + 1] = static_cast<WireU8>((value >> 8) & 0xFFu);
	}

	void PokeLE32(std::vector<WireU8>& frame, std::size_t offset, WireU32 value)
	{
		for (std::size_t i = 0; i < 4; ++i)
		{
			frame[offset + i] = static_cast<WireU8>((value >> (8 * i)) & 0xFFu);
		}
	}

	void PokeLEFloat(std::vector<WireU8>& frame, std::size_t offset, float value)
	{
		WireU32 bits = 0;
		std::memcpy(&bits, &value, sizeof(bits));
		PokeLE32(frame, offset, bits);
	}

	// ------------------------------------------------------------------------
	// A hand-built 3303, laid out exactly as GLContrlSkillMsg.h:155-225 declares
	// it under `#pragma pack(1)`.
	//
	// `targetEntries` is how many STARIDs to append; the caller owns the total
	// length, so this can produce legal frames AND the illegal ones (a hostile
	// wTARGET_NUM, a length that is not 63 + 4n).
	// ------------------------------------------------------------------------
	std::vector<WireU8> BuildRequest(std::size_t targetEntries, WireU16 declaredCount)
	{
		std::vector<WireU8> frame(SkillNS::kRequestHeaderSize +
		                              targetEntries * SkillNS::kTargetEntryBytes,
		                          0);

		PokeLE32(frame, 0, static_cast<WireU32>(frame.size()));
		PokeLE32(frame, 4, SkillNS::kSkillId);

		// SNATIVEID is a union of DWORD and {WORD wMainID, WORD wSubID}, so
		// wMainID is at 8 and wSubID at 10 - a shift the codec must not lose.
		PokeLE16(frame, 8, 0x0014u);
		PokeLE16(frame, 10, 0x0007u);

		// vTARPOS: x, y, z as three IEEE-754 singles.
		PokeLEFloat(frame, 12, 10.5f);
		PokeLEFloat(frame, 16, -20.25f);
		PokeLEFloat(frame, 20, 30.125f);

		PokeLE32(frame, 24, 0xDEADBEEFu); // dwFlags
		frame[28] = 0x01u;                // bDefenseSkill, ONE byte

		// The five echoed range values, four of them at ODD or 2-aligned offsets.
		PokeLEFloat(frame, 29, 3.5f);     // fSum_TarRange, at 29
		PokeLE16(frame, 33, 0x1111u);     // wBodyRadius
		PokeLE16(frame, 35, 0x2222u);     // wAttRange
		PokeLE16(frame, 37, 0x3333u);     // wSkillRangeApply
		PokeLE16(frame, 39, 0x4444u);     // wSkillRangeTar

		PokeLEFloat(frame, 41, 1.25f);    // fAttVelo, at 41
		PokeLEFloat(frame, 45, -2.5f);    // fAttVeloItem, at 45
		PokeLE32(frame, 49, 0xCAFEBABEu);// dwAnimationKey, at 49
		PokeLE32(frame, 53, 9u);         // emMType = AN_SKILL_A, at 53
		PokeLE32(frame, 57, 0u);         // emSType, at 57

		PokeLE16(frame, 61, declaredCount);

		for (std::size_t i = 0; i < targetEntries; ++i)
		{
			const std::size_t at = 63 + i * 4;
			PokeLE16(frame, at, static_cast<WireU16>(2u + i)); // wCrow = CROW_MOB + i
			PokeLE16(frame, at + 2, static_cast<WireU16>(1000u + i));
		}

		return frame;
	}

	// A 3303 with every float set to a value that is exactly representable and
	// not an integer, so a one-byte shift in any of the three would be visible.
	std::vector<WireU8> BuildRequestWithUnalignedFloats()
	{
		std::vector<WireU8> frame = BuildRequest(1, 1);
		PokeLEFloat(frame, 29, 0.125f);   // fSum_TarRange  @29
		PokeLEFloat(frame, 41, -3.375f);  // fAttVelo       @41
		PokeLEFloat(frame, 45, 12.625f);  // fAttVeloItem   @45
		return frame;
	}

	// A hand-built 3305, per GLContrlSkillMsg.h:255-304.
	std::vector<WireU8> BuildBroadcast(std::size_t targetEntries, WireU16 declaredCount)
	{
		std::vector<WireU8> frame(SkillNS::kBroadcastHeaderSize +
		                              targetEntries * SkillNS::kTargetEntryBytes,
		                          0);

		PokeLE32(frame, 0, static_cast<WireU32>(frame.size()));
		PokeLE32(frame, 4, SkillNS::kSkillBrdId);

		PokeLE32(frame, 8, 0u);                          // emCrow = CROW_PC
		PokeLE32(frame, 12, 0x11223344u);                // dwID, the FULL 32-bit caster id
		PokeLE16(frame, 16, 0x0014u);                    // skill_id.wMainID
		PokeLE16(frame, 18, 0x0007u);                    // skill_id.wSubID
		PokeLE16(frame, 20, 0x0005u);                    // wLEVEL
		PokeLEFloat(frame, 22, -5.5f);                   // vTARPOS.x @22
		PokeLEFloat(frame, 26, 6.25f);                   // vTARPOS.y
		PokeLEFloat(frame, 30, -7.125f);                 // vTARPOS.z
		PokeLE16(frame, 34, declaredCount);              // wTARNUM @34

		for (std::size_t i = 0; i < targetEntries; ++i)
		{
			const std::size_t at = 36 + i * 4;
			PokeLE16(frame, at, static_cast<WireU16>(0u + i));
			PokeLE16(frame, at + 2, static_cast<WireU16>(7000u + i));
		}

		return frame;
	}

	// =========================================================================
	// Message ids, and what they must not be confused with
	// =========================================================================

	// The ids are the whole reason this file exists alongside GotoProtocolTests:
	// 3303/3304/3305 are NET_MSG_GCTRL + 411/412/413, and a plausible-looking
	// "reuse 3034 for the skill result" would collide with GOTO and route every
	// cast request through the movement rule.
	MODERN_TEST(Skill_MessageIdsAre3303And3304And3305)
	{
		CHECK_EQ(SkillNS::kSkillId, static_cast<MessageId>(3303));
		CHECK_EQ(SkillNS::kSkillFbId, static_cast<MessageId>(3304));
		CHECK_EQ(SkillNS::kSkillBrdId, static_cast<MessageId>(3305));

		// All three are NET_MSG_GCTRL + n, so the arithmetic is two lines from
		// being one off in either direction - which would collide with 3302 or
		// 3306, both of which legacy also defines.
		CHECK_EQ(SkillNS::kSkillId, static_cast<MessageId>(Protocol::kGCtrlBase + 411));
		CHECK_EQ(SkillNS::kSkillBrdId - SkillNS::kSkillId, static_cast<MessageId>(2));

		CHECK(SkillCodec::IsSkill(3303));
		CHECK(!SkillCodec::IsSkill(3304));
		CHECK(!SkillCodec::IsSkill(3305));
		CHECK(SkillCodec::IsSkillFeedback(3304));
		CHECK(!SkillCodec::IsSkillFeedback(3303));
		CHECK(SkillCodec::IsSkillBroadcast(3305));
		CHECK(!SkillCodec::IsSkillBroadcast(3303));
	}

	// The separation the static_asserts in SkillProtocol.h make at build time,
	// restated at run time against the OTHER families' own constants rather than
	// against literals - so a change to GOTO or ATTACK fails this too.
	MODERN_TEST(Skill_MessageIdsAreSeparateFromGotoAndAttack)
	{
		namespace GotoNS = Modern::Network::Goto;
		namespace AtkNS  = Modern::Network::Attack;
		namespace MSNS   = Modern::Network::MovementState;

		const MessageId taken[] = {
		    GotoNS::kGotoId,              // 3034
		    GotoNS::kGotoBrdId,           // 3035
		    AtkNS::kAttackId,             // 3036
		    AtkNS::kAttackBrdId,          // 3037
		    AtkNS::kAttackAvoidId,        // 3041
		    AtkNS::kAttackAvoidBrdId,     // 3042
		    MSNS::kMoveStateId,           // 3032
		    MSNS::kMoveStateBrdId,        // 3033
		};

		for (const MessageId id : taken)
		{
			CHECK(!SkillCodec::IsSkill(id));
			CHECK(!SkillCodec::IsSkillFeedback(id));
			CHECK(!SkillCodec::IsSkillBroadcast(id));
		}

		// The converse, which is the direction that matters at dispatch: none of
		// the three skill predicates accepts a movement or attack id. A 3034
		// arriving on a connection must never reach the skill handler.
		CHECK(!GotoNS::GotoCodec::IsGoto(SkillNS::kSkillId));
		CHECK(!AtkNS::AttackCodec::IsAttack(SkillNS::kSkillId));
		CHECK(!MSNS::MovementStateCodec::IsMoveState(SkillNS::kSkillId));
	}

	// =========================================================================
	// Layout: the measured sizes and offsets
	// =========================================================================

	// Asserted by static_assert in SkillProtocol.h, so this case is about the
	// CONSTANTS - it fails when a constant is restated wrongly, which the
	// static_asserts would then agree with instead of contradicting.
	MODERN_TEST(Skill_MeasuredSizesAndOffsetsAreNamed)
	{
		CHECK_EQ(SkillNS::kMaxTargets, static_cast<std::size_t>(24));
		CHECK_EQ(SkillNS::kTargetEntryBytes, static_cast<std::size_t>(4));
		CHECK_EQ(SkillNS::kNameBytes, static_cast<std::size_t>(33));

		CHECK_EQ(SkillNS::kRequestHeaderSize, static_cast<std::size_t>(63));
		CHECK_EQ(SkillNS::kRequestOneTargetSize, static_cast<std::size_t>(67));
		CHECK_EQ(SkillNS::kRequestMaxSize, static_cast<std::size_t>(159));

		CHECK_EQ(SkillNS::kRequestSkillIdOffset, static_cast<std::size_t>(8));
		CHECK_EQ(SkillNS::kRequestTargetPositionOffset, static_cast<std::size_t>(12));
		CHECK_EQ(SkillNS::kRequestFlagsOffset, static_cast<std::size_t>(24));
		CHECK_EQ(SkillNS::kRequestDefenseSkillOffset, static_cast<std::size_t>(28));
		CHECK_EQ(SkillNS::kRequestSumTarRangeOffset, static_cast<std::size_t>(29));
		CHECK_EQ(SkillNS::kRequestBodyRadiusOffset, static_cast<std::size_t>(33));
		CHECK_EQ(SkillNS::kRequestAttackRangeOffset, static_cast<std::size_t>(35));
		CHECK_EQ(SkillNS::kRequestSkillRangeApplyOffset, static_cast<std::size_t>(37));
		CHECK_EQ(SkillNS::kRequestSkillRangeTarOffset, static_cast<std::size_t>(39));
		CHECK_EQ(SkillNS::kRequestAttackVelocityOffset, static_cast<std::size_t>(41));
		CHECK_EQ(SkillNS::kRequestAttackVelocityItemOffset, static_cast<std::size_t>(45));
		CHECK_EQ(SkillNS::kRequestAnimationKeyOffset, static_cast<std::size_t>(49));
		CHECK_EQ(SkillNS::kRequestAnimationMainTypeOffset, static_cast<std::size_t>(53));
		CHECK_EQ(SkillNS::kRequestAnimationSubTypeOffset, static_cast<std::size_t>(57));
		CHECK_EQ(SkillNS::kRequestTargetCountOffset, static_cast<std::size_t>(61));
		CHECK_EQ(SkillNS::kRequestTargetsOffset, static_cast<std::size_t>(63));

		CHECK_EQ(SkillNS::kFeedbackSize, static_cast<std::size_t>(45));
		CHECK_EQ(SkillNS::kFeedbackCheckOffset, static_cast<std::size_t>(8));
		CHECK_EQ(SkillNS::kFeedbackNameOffset, static_cast<std::size_t>(12));

		CHECK_EQ(SkillNS::kBroadcastHeaderSize, static_cast<std::size_t>(36));
		CHECK_EQ(SkillNS::kBroadcastMaxSize, static_cast<std::size_t>(132));
		CHECK_EQ(SkillNS::kBroadcastCrowOffset, static_cast<std::size_t>(8));
		CHECK_EQ(SkillNS::kBroadcastCasterIdOffset, static_cast<std::size_t>(12));
		CHECK_EQ(SkillNS::kBroadcastSkillIdOffset, static_cast<std::size_t>(16));
		CHECK_EQ(SkillNS::kBroadcastLevelOffset, static_cast<std::size_t>(20));
		CHECK_EQ(SkillNS::kBroadcastTargetPositionOffset, static_cast<std::size_t>(22));
		CHECK_EQ(SkillNS::kBroadcastTargetCountOffset, static_cast<std::size_t>(34));
		CHECK_EQ(SkillNS::kBroadcastTargetsOffset, static_cast<std::size_t>(36));

		// The CALCSIZE identity, stated rather than assumed:
		//     dwSize = sizeof(*this) - 4 * (24 - n) = 63 + 4n
		CHECK_EQ(SkillNS::kRequestMaxSize - SkillNS::kTargetEntryBytes *
		                                        (SkillNS::kMaxTargets - 0),
		         SkillNS::kRequestHeaderSize);
		CHECK_EQ(SkillNS::kRequestMaxSize - SkillNS::kTargetEntryBytes *
		                                        (SkillNS::kMaxTargets - 1),
		         SkillNS::kRequestOneTargetSize);
	}

	// A 67-byte 3303, built byte by byte and read back with an independent
	// reader. Every offset is named so a swap reports WHICH field moved rather
	// than that "the frame was wrong".
	MODERN_TEST(Skill_HandBuiltSixtySevenByteRequestHasEveryFieldWhereMeasured)
	{
		const std::vector<WireU8> frame = BuildRequest(1, 1);

		CHECK_EQ(frame.size(), static_cast<std::size_t>(67));

		// NET_MSG_GENERIC order: dwSize first, then nType. And dwSize is 67 -
		// the ACTUAL variable length, not sizeof(SNETPC_REQ_SKILL) = 159.
		CHECK_EQ(PeekLE32(frame, 0), static_cast<WireU32>(67));
		CHECK_EQ(PeekLE32(frame, 4), SkillNS::kSkillId);

		// SNATIVEID: wMainID at 8, wSubID at 10, the same 32 bits.
		CHECK_EQ(PeekLE16(frame, 8), static_cast<WireU16>(0x0014u));
		CHECK_EQ(PeekLE16(frame, 10), static_cast<WireU16>(0x0007u));
		CHECK_EQ(PeekLE32(frame, 8), static_cast<WireU32>(0x00070014u));

		CHECK(PeekLEFloat(frame, 12) == 10.5f);
		CHECK(PeekLEFloat(frame, 16) == -20.25f);
		CHECK(PeekLEFloat(frame, 20) == 30.125f);

		CHECK_EQ(PeekLE32(frame, 24), static_cast<WireU32>(0xDEADBEEFu));
		CHECK_EQ(frame[28], static_cast<WireU8>(0x01u));

		CHECK(PeekLEFloat(frame, 29) == 3.5f);
		CHECK_EQ(PeekLE16(frame, 33), static_cast<WireU16>(0x1111u));
		CHECK_EQ(PeekLE16(frame, 35), static_cast<WireU16>(0x2222u));
		CHECK_EQ(PeekLE16(frame, 37), static_cast<WireU16>(0x3333u));
		CHECK_EQ(PeekLE16(frame, 39), static_cast<WireU16>(0x4444u));

		CHECK(PeekLEFloat(frame, 41) == 1.25f);
		CHECK(PeekLEFloat(frame, 45) == -2.5f);
		CHECK_EQ(PeekLE32(frame, 49), static_cast<WireU32>(0xCAFEBABEu));
		CHECK_EQ(PeekLE32(frame, 53), static_cast<WireU32>(9u));
		CHECK_EQ(PeekLE32(frame, 57), static_cast<WireU32>(0u));

		CHECK_EQ(PeekLE16(frame, 61), static_cast<WireU16>(1));

		// The STARID: wCrow at 63, wID at 65, FOUR bytes total.
		CHECK_EQ(PeekLE16(frame, 63), static_cast<WireU16>(2u));
		CHECK_EQ(PeekLE16(frame, 65), static_cast<WireU16>(1000u));

		// And the decoder agrees with the independent reader, field for field.
		SkillNS::SkillRequest decoded;
		REQUIRE(SkillCodec::DecodeSkillRequest(frame, decoded).IsOk());

		CHECK_EQ(decoded.skillId.value, static_cast<WireU32>(0x00070014u));
		CHECK(decoded.targetPosition.x == 10.5f);
		CHECK(decoded.targetPosition.y == -20.25f);
		CHECK(decoded.targetPosition.z == 30.125f);
		CHECK_EQ(decoded.flags, static_cast<WireU32>(0xDEADBEEFu));
		CHECK_EQ(decoded.defenseSkill, static_cast<WireU8>(1u));
		CHECK(decoded.sumTarRange == 3.5f);
		CHECK_EQ(decoded.bodyRadius, static_cast<WireU16>(0x1111u));
		CHECK_EQ(decoded.attackRange, static_cast<WireU16>(0x2222u));
		CHECK_EQ(decoded.skillRangeApply, static_cast<WireU16>(0x3333u));
		CHECK_EQ(decoded.skillRangeTar, static_cast<WireU16>(0x4444u));
		CHECK(decoded.attackVelocity == 1.25f);
		CHECK(decoded.attackVelocityItem == -2.5f);
		CHECK_EQ(decoded.animationKey, static_cast<WireU32>(0xCAFEBABEu));
		CHECK_EQ(decoded.animationMainType, static_cast<WireI32>(9));
		CHECK_EQ(decoded.animationSubType, static_cast<WireI32>(0));
		CHECK_EQ(decoded.declaredTargetCount, static_cast<WireU16>(1));
		CHECK_EQ(decoded.targetCapacity, static_cast<std::size_t>(1));

		REQUIRE(decoded.targets.size() == static_cast<std::size_t>(1));
		CHECK_EQ(decoded.targets[0].crow, static_cast<WireU16>(2u));
		CHECK_EQ(decoded.targets[0].id, static_cast<WireU16>(1000u));
	}

	// The offsets this layout is famous for.
	//
	// Six fields sit at 1-byte-aligned addresses - fSum_TarRange at 29, fAttVelo
	// at 41, fAttVeloItem at 45, dwAnimationKey at 49, emMType at 53, emSType at
	// 57 - and the target array starts at 63, which is odd. That is not a
	// mistake in the modern tree; it is `#pragma pack(1)` in
	// GLContrlSkillMsg.h, and it is the reason nothing in the codec casts a
	// pointer into the buffer.
	//
	// The values are exactly representable and non-integral, so a one-byte shift
	// in any direction produces a different float rather than a rounded one.
	MODERN_TEST(Skill_UnalignedFloatsAt29And41And45DecodeExactly)
	{
		const std::vector<WireU8> frame = BuildRequestWithUnalignedFloats();

		// Confirm by hand that the bytes really are where they are claimed.
		CHECK(PeekLEFloat(frame, 29) == 0.125f);
		CHECK(PeekLEFloat(frame, 41) == -3.375f);
		CHECK(PeekLEFloat(frame, 45) == 12.625f);

		// And that reading them one byte early or late gives something else, so
		// this case would actually notice a shift.
		CHECK(PeekLEFloat(frame, 30) != 0.125f);
		CHECK(PeekLEFloat(frame, 42) != -3.375f);
		CHECK(PeekLEFloat(frame, 46) != 12.625f);

		SkillNS::SkillRequest decoded;
		REQUIRE(SkillCodec::DecodeSkillRequest(frame, decoded).IsOk());
		CHECK(decoded.sumTarRange == 0.125f);
		CHECK(decoded.attackVelocity == -3.375f);
		CHECK(decoded.attackVelocityItem == 12.625f);
	}

	// bDefenseSkill is a C++ bool, so ONE byte, and it is what pushes every
	// float after it off a 4-byte boundary.
	//
	// Legacy only ever tests it for truth (GLContrlSkillMsg.cpp:248 and :299), so
	// a peer that sends 0xFF is not lying about anything the server acts on - but
	// a struct field typed `bool` must not hold a byte that is neither 0 nor 1,
	// because anything comparing it against `true` afterwards is relying on that.
	MODERN_TEST(Skill_DefenseSkillIsOneByteAndIsNormalisedToZeroOrOne)
	{
		std::vector<WireU8> frame = BuildRequest(0, 0);
		frame[28] = 0xFFu;

		SkillNS::SkillRequest decoded;
		REQUIRE(SkillCodec::DecodeSkillRequest(frame, decoded).IsOk());
		CHECK_EQ(decoded.defenseSkill, static_cast<WireU8>(1u));

		frame[28] = 0x00u;
		REQUIRE(SkillCodec::DecodeSkillRequest(frame, decoded).IsOk());
		CHECK_EQ(decoded.defenseSkill, static_cast<WireU8>(0u));
	}

	// emMType and emSType are plain unscoped enums, which MSVC sizes as a signed
	// 32-bit int. 9 (AN_SKILL_A) is small enough to fit in a byte, so a codec
	// that wrongly narrowed the field would produce a 157-byte packet or read
	// three bytes of the next field as part of the enum - this case pins four
	// bytes.
	MODERN_TEST(Skill_AnimationEnumsAreFourBytesNotOne)
	{
		std::vector<WireU8> frame = BuildRequest(0, 0);

		// A value that does not fit in one byte or in a signed 16-bit.
		PokeLE32(frame, 53, 0x7FFFFFFFu); // emMType
		PokeLE32(frame, 57, 0xFFFFFFFFu); // emSType, all bits - a negative int32

		SkillNS::SkillRequest decoded;
		REQUIRE(SkillCodec::DecodeSkillRequest(frame, decoded).IsOk());
		CHECK_EQ(decoded.animationMainType, static_cast<WireI32>(0x7FFFFFFFu));
		CHECK_EQ(decoded.animationSubType, static_cast<WireI32>(-1));

		// The bytes immediately after emSType are wTARGET_NUM and the array; if
		// the enum had been read as fewer than four bytes, these would be wrong.
		CHECK_EQ(decoded.declaredTargetCount, static_cast<WireU16>(0));
		CHECK_EQ(decoded.targets.size(), static_cast<std::size_t>(0));
	}

	// =========================================================================
	// The variable length: all 25 legal sizes
	// =========================================================================

	// 63 + 4n for n in [0, 24]. Every one of them decodes, to exactly n
	// targets.
	//
	// A codec that hard-coded the full 159-byte struct would pass a 159-byte case
	// and fail every other one; a codec that accepted anything would pass the
	// refusals below. The whole legal range is walked so neither survives.
	MODERN_TEST(Skill_AllTwentyFiveLegalRequestLengthsDecodeToTheRightTargetCount)
	{
		for (std::size_t n = 0; n <= SkillNS::kMaxTargets; ++n)
		{
			const std::size_t expectedSize =
			    SkillNS::kRequestHeaderSize + n * SkillNS::kTargetEntryBytes;
			CHECK_EQ(expectedSize, SkillNS::kRequestHeaderSize + 4 * n);

			const std::vector<WireU8> frame =
			    BuildRequest(n, static_cast<WireU16>(n));

			SkillNS::SkillRequest decoded;
			REQUIRE(SkillCodec::DecodeSkillRequest(frame, decoded).IsOk());

			CHECK_EQ(frame.size(), expectedSize);
			CHECK_EQ(decoded.targets.size(), n);
			CHECK_EQ(decoded.targetCapacity, n);
			CHECK_EQ(decoded.declaredTargetCount, static_cast<WireU16>(n));
		}
	}

	// The endpoints, named, because "the loop covered them" is weaker than
	// "these two were checked".
	MODERN_TEST(Skill_ZeroTargetRequestIsSixtyThreeBytes)
	{
		const std::vector<WireU8> frame = BuildRequest(0, 0);

		CHECK_EQ(frame.size(), SkillNS::kRequestHeaderSize);
		CHECK_EQ(frame.size(), static_cast<std::size_t>(63));

		SkillNS::SkillRequest decoded;
		REQUIRE(SkillCodec::DecodeSkillRequest(frame, decoded).IsOk());
		CHECK_EQ(decoded.targets.size(), static_cast<std::size_t>(0));
		CHECK_EQ(decoded.targetCapacity, static_cast<std::size_t>(0));
		CHECK_EQ(decoded.declaredTargetCount, static_cast<WireU16>(0));
	}

	MODERN_TEST(Skill_MaximumRequestIsOneHundredAndFiftyNineBytesWithTwentyFourTargets)
	{
		const std::vector<WireU8> frame = BuildRequest(24, 24);

		CHECK_EQ(frame.size(), SkillNS::kRequestMaxSize);
		CHECK_EQ(frame.size(), static_cast<std::size_t>(159));

		SkillNS::SkillRequest decoded;
		REQUIRE(SkillCodec::DecodeSkillRequest(frame, decoded).IsOk());

		CHECK_EQ(decoded.targets.size(), SkillNS::kMaxTargets);
		CHECK_EQ(decoded.targets.size(), static_cast<std::size_t>(24));

		// The last entry is the one a short array would lose.
		REQUIRE(decoded.targets.size() == 24);
		CHECK_EQ(decoded.targets[23].crow, static_cast<WireU16>(2u + 23u));
		CHECK_EQ(decoded.targets[23].id, static_cast<WireU16>(1000u + 23u));

		// Entry i is at 63 + 4i - the array really is where it is claimed.
		CHECK_EQ(PeekLE16(frame, 63 + 23 * 4), static_cast<WireU16>(2u + 23u));
		CHECK_EQ(PeekLE16(frame, 65 + 23 * 4), static_cast<WireU16>(1000u + 23u));
	}

	// =========================================================================
	// wTARGET_NUM is REPORTED, never obeyed
	// =========================================================================

	// THE LEGACY QUIRK, and the reason for the second bound.
	//
	// GLContrlSkillMsg.h:199 sets `nmg.dwSize = sizeof(*this)` in the CONSTRUCTOR
	// and CALCSIZE is only ever called from ADDTARGET (GLContrlSkillMsg.h:216). A
	// cast with zero targets therefore goes out declaring 159 bytes, with 96
	// bytes of never-written array tail behind the header.
	//
	// Those 159 bytes are accepted - they are normal legacy traffic and refusing
	// them would break a real client. But they are NOT 24 targets, and treating
	// them as 24 would hand a later milestone 24 target ids the client never
	// sent. This is the case that separates "the length says the bytes are there"
	// from "the peer said there are that many".
	MODERN_TEST(Skill_FullSizeRequestWithZeroTargetsIsAcceptedAndItsArrayIsIgnored)
	{
		const std::vector<WireU8> frame = BuildRequest(24, 0);

		CHECK_EQ(frame.size(), static_cast<std::size_t>(159));

		SkillNS::SkillRequest decoded;
		REQUIRE(SkillCodec::DecodeSkillRequest(frame, decoded).IsOk());

		// The peer declared zero, so the list is empty. NOT 24.
		CHECK_EQ(decoded.targets.size(), static_cast<std::size_t>(0));

		// And both numbers are still visible, so nothing is hidden: the bytes
		// could have held 24, the peer claimed 0, and the discrepancy is
		// readable rather than silently resolved.
		CHECK_EQ(decoded.declaredTargetCount, static_cast<WireU16>(0));
		CHECK_EQ(decoded.targetCapacity, static_cast<std::size_t>(24));
	}

	// The other direction of the same rule, and the memory-safety one: a peer
	// that DECLARES 24 in a 63-byte frame must not cause 24 reads.
	//
	// `capacity` comes from the validated length, so the minimum is taken before
	// any loop begins. Without this the decoder would read 96 bytes past the end
	// of a 63-byte buffer - and legacy, which casts a struct straight onto the
	// packet and only clamps AFTER dereferencing, is exposed to exactly that.
	MODERN_TEST(Skill_HostileTargetCountInAShortFrameIsNotIteratedAndOverreadsNothing)
	{
		// 63 bytes: the fixed header and NOTHING after it, yet the peer claims
		// the full array.
		const std::vector<WireU8> frame = BuildRequest(0, 24);

		CHECK_EQ(frame.size(), static_cast<std::size_t>(63));

		SkillNS::SkillRequest decoded;
		REQUIRE(SkillCodec::DecodeSkillRequest(frame, decoded).IsOk());

		CHECK_EQ(decoded.targets.size(), static_cast<std::size_t>(0));
		CHECK_EQ(decoded.declaredTargetCount, static_cast<WireU16>(24));
		CHECK_EQ(decoded.targetCapacity, static_cast<std::size_t>(0));

		// The absurd case, which on the legacy SEND side underflows CALCSIZE's
		// size_t subtraction. 0xFFFF targets in a 63-byte frame must be the same
		// refusal-free empty list, not a 65535-iteration loop.
		const std::vector<WireU8> absurd = BuildRequest(0, 0xFFFFu);
		REQUIRE(SkillCodec::DecodeSkillRequest(absurd, decoded).IsOk());
		CHECK_EQ(decoded.targets.size(), static_cast<std::size_t>(0));
		CHECK_EQ(decoded.declaredTargetCount, static_cast<WireU16>(0xFFFFu));
	}

	// A count larger than the bytes, on a frame that is long enough to be legal.
	// The minimum is per-field, so a peer gets the targets the bytes back, not
	// the ones it asked for.
	MODERN_TEST(Skill_TargetCountLargerThanTheArrayYieldsOnlyWhatTheBytesHold)
	{
		// 67 bytes: room for exactly one STARID, peer claims 24.
		const std::vector<WireU8> frame = BuildRequest(1, 24);

		SkillNS::SkillRequest decoded;
		REQUIRE(SkillCodec::DecodeSkillRequest(frame, decoded).IsOk());
		CHECK_EQ(decoded.targets.size(), static_cast<std::size_t>(1));
		CHECK_EQ(decoded.targets[0].id, static_cast<WireU16>(1000u));
		CHECK_EQ(decoded.declaredTargetCount, static_cast<WireU16>(24));
		CHECK_EQ(decoded.targetCapacity, static_cast<std::size_t>(1));
	}

	// The wire sentinel. STARID::GETID maps wID == USHRT_MAX to EMTARGET_NULL, so
	// a 0xFFFF id is "no target" and must be recognisable rather than being a
	// plausible-looking id.
	MODERN_TEST(Skill_TargetIdIsSixteenBitsAndZeroFFFFIsTheNullMarker)
	{
		std::vector<WireU8> frame = BuildRequest(2, 2);
		PokeLE16(frame, 63 + 0, 0u);
		PokeLE16(frame, 65 + 0, 0xFFFFu); // the sentinel
		PokeLE16(frame, 63 + 4, 1u);
		PokeLE16(frame, 65 + 4, 0x1234u);

		SkillNS::SkillRequest decoded;
		REQUIRE(SkillCodec::DecodeSkillRequest(frame, decoded).IsOk());
		REQUIRE(decoded.targets.size() == 2);

		CHECK(decoded.targets[0].IsNull());
		CHECK(!decoded.targets[1].IsNull());
		CHECK_EQ(decoded.targets[1].id, static_cast<WireU16>(0x1234u));

		// The id is TWO bytes on the wire. A peer cannot name a 32-bit entity
		// here - STARID truncates it (GLContrlSkillMsg.h:213) - so there is
		// nowhere for the upper half to have gone.
		CHECK_EQ(decoded.targets[1].id, static_cast<WireU16>(0x1234u));
	}

	// =========================================================================
	// Refusal of malformed lengths and wrong ids
	// =========================================================================

	// The four named illegal lengths, and the general rule they instantiate.
	//
	//   62  below the fixed header - a truncated packet
	//   65  not 63 + 4n: one byte of a STARID
	//   66  not 63 + 4n: half of a STARID
	//   164 above the maximum: longer than the array can ever be
	//
	// 65 and 66 are the interesting pair. A decoder that only checked "at least
	// 63" would accept 65 and then read four bytes for a target that is not
	// there.
	MODERN_TEST(Skill_MalformedRequestLengthsAreRefused)
	{
		SkillNS::SkillRequest decoded;
		decoded.flags = 0xDEADBEEFu; // so a cleared output is visible

		const std::size_t illegal[] = { 62, 65, 66, 164 };

		for (const std::size_t length : illegal)
		{
			// Built at the illegal length directly, so the frame really is that
			// long and really declares that length.
			std::vector<WireU8> frame(length, 0);
			PokeLE32(frame, 0, static_cast<WireU32>(length));
			PokeLE32(frame, 4, SkillNS::kSkillId);

			CHECK(SkillCodec::DecodeSkillRequest(frame, decoded).IsError());

			// The output is CLEARED on refusal, so a caller that ignores the
			// status cannot act on a half-decoded request.
			CHECK_EQ(decoded.flags, static_cast<WireU32>(0));
			CHECK_EQ(decoded.targets.size(), static_cast<std::size_t>(0));
		}

		// Empty, and below the 8-byte header.
		const std::vector<WireU8> empty;
		CHECK(SkillCodec::DecodeSkillRequest(empty, decoded).IsError());

		const std::vector<WireU8> tiny(7, 0);
		CHECK(SkillCodec::DecodeSkillRequest(tiny, decoded).IsError());
	}

	// THE LIE: real bytes declaring a different length.
	//
	// `frame.size()` is the authority; a declared size is only ever COMPARED with
	// it, never used to index. Both directions matter - a frame whose header
	// understates and one that overstates - and a peer controls both.
	MODERN_TEST(Skill_DeclaredSizeMustMatchTheFrameItArrivedIn)
	{
		const std::vector<WireU8> good = BuildRequest(1, 1);

		SkillNS::SkillRequest decoded;

		// Declares 159 while carrying 67 bytes. A decoder that sized its reads
		// from the header would walk 92 bytes past the end of this buffer.
		std::vector<WireU8> overstating = good;
		PokeLE32(overstating, 0, 159u);
		CHECK(SkillCodec::DecodeSkillRequest(overstating, decoded).IsError());

		// Declares 63 while carrying 67. The converse, so the two can never
		// drift apart in either direction.
		std::vector<WireU8> understating = good;
		PokeLE32(understating, 0, 63u);
		CHECK(SkillCodec::DecodeSkillRequest(understating, decoded).IsError());

		// Declares 65 - a length that is not 63 + 4n - over a 67-byte buffer.
		std::vector<WireU8> wrongForm = good;
		PokeLE32(wrongForm, 0, 65u);
		CHECK(SkillCodec::DecodeSkillRequest(wrongForm, decoded).IsError());

		// The good frame still decodes, so the refusals above are about the
		// malformed inputs and not about a decoder that refuses everything.
		CHECK(SkillCodec::DecodeSkillRequest(good, decoded).IsOk());
		CHECK_EQ(decoded.targets.size(), static_cast<std::size_t>(1));
	}

	// The wrong message id is refused even when the length is exactly right.
	//
	// 3304 is 45 bytes and 3305 is 36 + 4n; neither is 63 + 4n in general, but
	// the REQUEST decoder must not depend on that - a 67-byte frame with 3304's
	// id in it has to fail on the id.
	MODERN_TEST(Skill_WrongMessageIdsAreRefusedByTheSkillDecoders)
	{
		SkillNS::SkillRequest    request;
		SkillNS::SkillFeedback   feedback;
		SkillNS::SkillBroadcast  broadcast;

		// 3304 and 3305 labels on a well-formed 67-byte request.
		std::vector<WireU8> mislabelled = BuildRequest(1, 1);

		std::vector<WireU8> asFb = mislabelled;
		PokeLE32(asFb, 4, SkillNS::kSkillFbId);
		CHECK(SkillCodec::DecodeSkillRequest(asFb, request).IsError());

		std::vector<WireU8> asBrd = mislabelled;
		PokeLE32(asBrd, 4, SkillNS::kSkillBrdId);
		CHECK(SkillCodec::DecodeSkillRequest(asBrd, request).IsError());

		// 3034 - the GOTO id - on a 67-byte request. A cast request must never
		// be reachable through the movement path or vice versa.
		std::vector<WireU8> asGoto = mislabelled;
		PokeLE32(asGoto, 4, static_cast<WireU32>(3034));
		CHECK(SkillCodec::DecodeSkillRequest(asGoto, request).IsError());

		// And 3303's id on the other two decoders.
		std::vector<WireU8> feedbackFrame(45, 0);
		PokeLE32(feedbackFrame, 0, 45u);
		PokeLE32(feedbackFrame, 4, SkillNS::kSkillId);
		CHECK(SkillCodec::DecodeSkillFeedback(feedbackFrame, feedback).IsError());

		std::vector<WireU8> broadcastFrame = BuildBroadcast(0, 0);
		PokeLE32(broadcastFrame, 4, SkillNS::kSkillId);
		CHECK(SkillCodec::DecodeSkillBroadcast(broadcastFrame, broadcast).IsError());
	}

	// =========================================================================
	// 3304: the feedback, and the 33-byte name
	// =========================================================================

	MODERN_TEST(Skill_FeedbackIsFortyFiveBytesWithAFourByteEnumAndThirtyThreeNameBytes)
	{
		// EMSKILL_UNKNOWN = 999, which is the value that proves the enum is four
		// bytes: it does not fit in a byte or in a signed 16-bit, so a codec
		// that narrowed the field would read three bytes of the name as part of
		// the code.
		std::vector<WireU8> frame(45, 0);
		PokeLE32(frame, 0, 45u);
		PokeLE32(frame, 4, SkillNS::kSkillFbId);
		PokeLE32(frame, 8, 999u);

		// A 33-byte name with NO NUL anywhere in it. This is the shape a peer
		// can legitimately send: the legacy field is char[33] for an id length
		// of 33 and the constructor only zeroed it, so nothing guarantees a
		// terminator. Any strlen over this would run off the end.
		for (std::size_t i = 0; i < SkillNS::kNameBytes; ++i)
		{
			frame[12 + i] = static_cast<WireU8>('A' + static_cast<WireU8>(i % 26));
		}

		// Asserted by hand first: not one byte of it is zero.
		bool anyNul = false;
		for (std::size_t i = 0; i < SkillNS::kNameBytes; ++i)
		{
			if (frame[12 + i] == 0)
			{
				anyNul = true;
			}
		}
		CHECK(!anyNul);

		SkillNS::SkillFeedback decoded;
		decoded.skillCheck = SkillNS::SkillCheck::kOk;
		REQUIRE(SkillCodec::DecodeSkillFeedback(frame, decoded).IsOk());

		CHECK_EQ(decoded.skillCheck, static_cast<WireI32>(999));
		CHECK_EQ(decoded.name.size(), SkillNS::kNameBytes);
		CHECK_EQ(decoded.name.size(), static_cast<std::size_t>(33));

		// Every byte survives, and the string is exactly 33 long - which is the
		// assertion that proves nothing NUL-scanned it and nothing grew it.
		for (std::size_t i = 0; i < SkillNS::kNameBytes; ++i)
		{
			CHECK_EQ(static_cast<WireU8>(decoded.name[i]),
			         static_cast<WireU8>('A' + static_cast<WireU8>(i % 26)));
		}
	}

	// An embedded NUL is just a byte. The field is raw, so a name that happens
	// to contain one must survive intact rather than being truncated at it.
	MODERN_TEST(Skill_FeedbackNameIsRawBytesAndAnEmbeddedNulIsNotATerminator)
	{
		std::vector<WireU8> frame(45, 0);
		PokeLE32(frame, 0, 45u);
		PokeLE32(frame, 4, SkillNS::kSkillFbId);
		PokeLE32(frame, 8, 1u); // EMSKILL_NOTLEARN

		frame[12]     = 'A';
		frame[12 + 1] = 0x00; // an embedded NUL, at position 1
		frame[12 + 2] = 'B';

		SkillNS::SkillFeedback decoded;
		REQUIRE(SkillCodec::DecodeSkillFeedback(frame, decoded).IsOk());

		// Still 33 long, not 1. A NUL-scanning reader would have stopped at
		// index 1 and reported a one-byte name.
		CHECK_EQ(decoded.name.size(), SkillNS::kNameBytes);
		CHECK_EQ(static_cast<WireU8>(decoded.name[0]), static_cast<WireU8>('A'));
		CHECK_EQ(static_cast<WireU8>(decoded.name[1]), static_cast<WireU8>(0x00));
		CHECK_EQ(static_cast<WireU8>(decoded.name[2]), static_cast<WireU8>('B'));
	}

	// The name length is EXACTLY 33 in both directions. Padding a short name or
	// cutting a long one would both be a lie about what the server meant to send.
	MODERN_TEST(Skill_FeedbackAppendRequiresExactlyThirtyThreeNameBytes)
	{
		SkillNS::SkillFeedback feedback;
		feedback.skillCheck = SkillNS::SkillCheck::kNotLearned;
		feedback.name       = std::string(SkillNS::kNameBytes, 'x');

		std::vector<WireU8> frame;
		REQUIRE(SkillCodec::AppendSkillFeedback(frame, feedback).IsOk());
		CHECK_EQ(frame.size(), SkillNS::kFeedbackSize);

		// 32 and 34 are both refused rather than adjusted.
		feedback.name = std::string(SkillNS::kNameBytes - 1, 'x');
		std::vector<WireU8> shortFrame;
		CHECK(SkillCodec::AppendSkillFeedback(shortFrame, feedback).IsError());
		CHECK_EQ(shortFrame.size(), static_cast<std::size_t>(0));

		feedback.name = std::string(SkillNS::kNameBytes + 1, 'x');
		std::vector<WireU8> longFrame;
		CHECK(SkillCodec::AppendSkillFeedback(longFrame, feedback).IsError());
		CHECK_EQ(longFrame.size(), static_cast<std::size_t>(0));
	}

	// The named EMSKILLCHECK values, restated at run time.
	//
	// Only these are named in SkillProtocol.h, and only because each one's
	// MEANING was read out of legacy. EMSKILL_UNKNOWN in particular is the
	// "the server has no data for this skill" code - GLogixExPC.cpp:4092, from
	// `if (!pSkill)` - and must never be reused as a generic failure.
	MODERN_TEST(Skill_FeedbackCodesMatchLegacyEmskillcheck)
	{
		CHECK_EQ(SkillNS::SkillCheck::kOk, static_cast<WireI32>(0));
		CHECK_EQ(SkillNS::SkillCheck::kNotLearned, static_cast<WireI32>(1));
		CHECK_EQ(SkillNS::SkillCheck::kDelayTime, static_cast<WireI32>(2));
		CHECK_EQ(SkillNS::SkillCheck::kNotHp, static_cast<WireI32>(6));
		CHECK_EQ(SkillNS::SkillCheck::kNotMp, static_cast<WireI32>(7));
		CHECK_EQ(SkillNS::SkillCheck::kNotSp, static_cast<WireI32>(8));
		CHECK_EQ(SkillNS::SkillCheck::kNotCp, static_cast<WireI32>(11));
		CHECK_EQ(SkillNS::SkillCheck::kUnknown, static_cast<WireI32>(999));
	}

	// 3304 is FIXED at 45 bytes - there is no CALCSIZE on that struct - so 44
	// and 46 are both wrong, unlike 3303 where a range is legal.
	MODERN_TEST(Skill_FeedbackDecodeRefusesWrongSizes)
	{
		std::vector<WireU8> good(45, 0);
		PokeLE32(good, 0, 45u);
		PokeLE32(good, 4, SkillNS::kSkillFbId);
		PokeLE32(good, 8, 0u);

		SkillNS::SkillFeedback decoded;
		CHECK(SkillCodec::DecodeSkillFeedback(good, decoded).IsOk());

		std::vector<WireU8> shortFrame(good.begin(), good.end() - 1);
		CHECK(SkillCodec::DecodeSkillFeedback(shortFrame, decoded).IsError());

		std::vector<WireU8> longFrame = good;
		longFrame.push_back(0);
		CHECK(SkillCodec::DecodeSkillFeedback(longFrame, decoded).IsError());

		// A 63-byte request is not a feedback, even though it is a legal size
		// for a different message on the same connection.
		const std::vector<WireU8> request = BuildRequest(0, 0);
		CHECK(SkillCodec::DecodeSkillFeedback(request, decoded).IsError());

		CHECK(SkillCodec::DecodeSkillFeedback(good, decoded).IsOk());
	}

	// =========================================================================
	// 3305: the broadcast
	// =========================================================================

	// The asymmetry this struct is most likely to get wrong: the CASTER's id is
	// a full 32-bit DWORD at offset 12, and every TARGET is still a four-byte
	// STARID with two 16-bit halves.
	//
	// Widening a target to 32 bits would make the packet 156 bytes; narrowing the
	// caster would make it 129. Both would be the right SIZE to look plausible and
	// completely unreadable to a client.
	MODERN_TEST(Skill_BroadcastHasAThirtyTwoBitCasterBesideSixteenBitTargets)
	{
		const std::vector<WireU8> frame = BuildBroadcast(24, 24);

		CHECK_EQ(frame.size(), SkillNS::kBroadcastMaxSize);
		CHECK_EQ(frame.size(), static_cast<std::size_t>(132));

		CHECK_EQ(PeekLE32(frame, 0), static_cast<WireU32>(132));
		CHECK_EQ(PeekLE32(frame, 4), SkillNS::kSkillBrdId);

		// emCrow at 8 is a FOUR-byte enum, unlike a target's wCrow.
		CHECK_EQ(PeekLE32(frame, 8), static_cast<WireU32>(0u));

		// dwID at 12 is the full 32-bit caster id: 0x11223344 does not fit in 16
		// bits, so a narrowed field would have had to drop half of it.
		CHECK_EQ(PeekLE32(frame, 12), static_cast<WireU32>(0x11223344u));

		CHECK_EQ(PeekLE16(frame, 16), static_cast<WireU16>(0x0014u));
		CHECK_EQ(PeekLE16(frame, 18), static_cast<WireU16>(0x0007u));
		CHECK_EQ(PeekLE16(frame, 20), static_cast<WireU16>(0x0005u));
		CHECK(PeekLEFloat(frame, 22) == -5.5f);
		CHECK(PeekLEFloat(frame, 26) == 6.25f);
		CHECK(PeekLEFloat(frame, 30) == -7.125f);
		CHECK_EQ(PeekLE16(frame, 34), static_cast<WireU16>(24));

		// Targets start at 36 and are four bytes each.
		CHECK_EQ(PeekLE16(frame, 36), static_cast<WireU16>(0u));
		CHECK_EQ(PeekLE16(frame, 38), static_cast<WireU16>(7000u));
		CHECK_EQ(PeekLE16(frame, 36 + 23 * 4), static_cast<WireU16>(23u));
		CHECK_EQ(PeekLE16(frame, 38 + 23 * 4), static_cast<WireU16>(7000u + 23u));

		SkillNS::SkillBroadcast decoded;
		REQUIRE(SkillCodec::DecodeSkillBroadcast(frame, decoded).IsOk());

		CHECK_EQ(decoded.crow, static_cast<WireI32>(0));
		CHECK_EQ(decoded.casterId, static_cast<WireU32>(0x11223344u));
		CHECK_EQ(decoded.skillId.value, static_cast<WireU32>(0x00070014u));
		CHECK_EQ(decoded.level, static_cast<WireU16>(0x0005u));
		CHECK(decoded.targetPosition.x == -5.5f);
		CHECK_EQ(decoded.targets.size(), static_cast<std::size_t>(24));
		REQUIRE(decoded.targets.size() == 24);
		CHECK_EQ(decoded.targets[23].id, static_cast<WireU16>(7000u + 23u));
	}

	// The same variable-length rule as 3303, with a DIFFERENT base: 36 + 4n.
	//
	// A decoder shared with 3303 would accept the wrong one somewhere - 67 bytes
	// is a legal request and an illegal broadcast - so both directions are
	// checked.
	MODERN_TEST(Skill_BroadcastAcceptsThirtySixAndOneHundredAndThirtyTwoAndRefusesTheRest)
	{
		SkillNS::SkillBroadcast decoded;

		// All 25 legal broadcast lengths.
		for (std::size_t n = 0; n <= SkillNS::kMaxTargets; ++n)
		{
			const std::vector<WireU8> frame =
			    BuildBroadcast(n, static_cast<WireU16>(n));
			REQUIRE(SkillCodec::DecodeSkillBroadcast(frame, decoded).IsOk());
			CHECK_EQ(frame.size(), SkillNS::kBroadcastHeaderSize + 4 * n);
			CHECK_EQ(decoded.targets.size(), n);
		}

		// The zero-target endpoint, named.
		const std::vector<WireU8> none = BuildBroadcast(0, 0);
		CHECK_EQ(none.size(), static_cast<std::size_t>(36));
		REQUIRE(SkillCodec::DecodeSkillBroadcast(none, decoded).IsOk());
		CHECK_EQ(decoded.targets.size(), static_cast<std::size_t>(0));
		CHECK_EQ(decoded.targetCapacity, static_cast<std::size_t>(0));

		// Below 36, not 36 + 4n, and above 132.
		const std::size_t illegal[] = { 35, 37, 38, 39, 134, 136 };
		for (const std::size_t length : illegal)
		{
			std::vector<WireU8> frame(length, 0);
			PokeLE32(frame, 0, static_cast<WireU32>(length));
			PokeLE32(frame, 4, SkillNS::kSkillBrdId);
			CHECK(SkillCodec::DecodeSkillBroadcast(frame, decoded).IsError());
		}

		// 67 bytes: a perfectly legal REQUEST, and an illegal broadcast.
		const std::vector<WireU8> requestSized = BuildRequest(1, 1);
		CHECK_EQ(requestSized.size(), static_cast<std::size_t>(67));
		CHECK(SkillCodec::DecodeSkillBroadcast(requestSized, decoded).IsError());

		// 36 bytes is an illegal REQUEST and a legal broadcast, so the two
		// decoders really are separate rather than one length rule shared.
		std::vector<WireU8> mislabelledAsRequest = none;
		PokeLE32(mislabelledAsRequest, 4, SkillNS::kSkillId);
		SkillNS::SkillRequest asRequest;
		CHECK(SkillCodec::DecodeSkillRequest(mislabelledAsRequest, asRequest).IsError());
	}

	// The two bounds again, on the broadcast. Same rule as 3303: a full-size
	// broadcast with a zero count is accepted and yields no targets.
	MODERN_TEST(Skill_BroadcastTargetCountIsAlsoReportedNotObeyed)
	{
		const std::vector<WireU8> frame = BuildBroadcast(24, 0);

		SkillNS::SkillBroadcast decoded;
		REQUIRE(SkillCodec::DecodeSkillBroadcast(frame, decoded).IsOk());
		CHECK_EQ(decoded.targets.size(), static_cast<std::size_t>(0));
		CHECK_EQ(decoded.declaredTargetCount, static_cast<WireU16>(0));
		CHECK_EQ(decoded.targetCapacity, static_cast<std::size_t>(24));
	}

	// =========================================================================
	// Round trips - ADDITIONAL evidence, not the primary evidence
	// =========================================================================

	// A round trip through the encoder, over every legal length, so the encoder
	// cannot drift from the decoder. This is the check that would catch the two
	// having quietly stopped agreeing - and it is explicitly NOT what the
	// offset cases above rely on.
	MODERN_TEST(Skill_EncoderAndDecoderAgreeAtEveryLegalLength)
	{
		SkillNS::SkillRequest request;
		request.skillId.value       = 0x00070014u;
		request.targetPosition      = {1.5f, -2.25f, 3.125f};
		request.flags               = 0x01020304u;
		request.defenseSkill        = 1u;
		request.sumTarRange         = 0.5f;
		request.bodyRadius          = 11u;
		request.attackRange         = 22u;
		request.skillRangeApply     = 33u;
		request.skillRangeTar       = 44u;
		request.attackVelocity      = 1.25f;
		request.attackVelocityItem  = 2.5f;
		request.animationKey        = 0x99AABBCCu;
		request.animationMainType   = 9;
		request.animationSubType    = 3;

		for (std::size_t n = 0; n <= SkillNS::kMaxTargets; ++n)
		{
			SkillNS::SkillRequest sized = request;
			sized.targets.clear();
			for (std::size_t i = 0; i < n; ++i)
			{
				sized.targets.push_back(
				    SkillNS::SkillTarget{ static_cast<WireU16>(i),
				                          static_cast<WireU16>(1000u + i) });
			}

			std::vector<WireU8> frame;
			REQUIRE(SkillCodec::AppendSkillRequest(frame, sized).IsOk());

			// The encoder emits the VARIABLE length, never sizeof(*this).
			CHECK_EQ(frame.size(), SkillNS::kRequestHeaderSize + 4 * n);
			CHECK_EQ(PeekLE32(frame, 0), static_cast<WireU32>(frame.size()));
			CHECK_EQ(PeekLE32(frame, 4), SkillNS::kSkillId);

			SkillNS::SkillRequest decoded;
			REQUIRE(SkillCodec::DecodeSkillRequest(frame, decoded).IsOk());

			CHECK_EQ(decoded.skillId.value, request.skillId.value);
			CHECK(decoded.targetPosition.x == request.targetPosition.x);
			CHECK(decoded.targetPosition.z == request.targetPosition.z);
			CHECK_EQ(decoded.flags, request.flags);
			CHECK_EQ(decoded.defenseSkill, request.defenseSkill);
			CHECK(decoded.sumTarRange == request.sumTarRange);
			CHECK_EQ(decoded.bodyRadius, request.bodyRadius);
			CHECK_EQ(decoded.skillRangeTar, request.skillRangeTar);
			CHECK(decoded.attackVelocity == request.attackVelocity);
			CHECK(decoded.attackVelocityItem == request.attackVelocityItem);
			CHECK_EQ(decoded.animationKey, request.animationKey);
			CHECK_EQ(decoded.animationMainType, request.animationMainType);
			CHECK_EQ(decoded.animationSubType, request.animationSubType);
			CHECK_EQ(decoded.targets.size(), n);
		}
	}

	// Append APPENDS, and each copy carries its own size and id - which is what
	// lets a framer that splits on dwSize find two 3303s rather than one frame.
	MODERN_TEST(Skill_TwoRequestsAppendIntoOneWellFormedBuffer)
	{
		SkillNS::SkillRequest first;
		first.skillId.value = 0x00010001u;
		first.targets.push_back(SkillNS::SkillTarget{ 0u, 10u });

		SkillNS::SkillRequest second;
		second.skillId.value = 0x00020002u;
		second.targets.push_back(SkillNS::SkillTarget{ 1u, 20u });
		second.targets.push_back(SkillNS::SkillTarget{ 2u, 30u });

		std::vector<WireU8> frame;
		REQUIRE(SkillCodec::AppendSkillRequest(frame, first).IsOk());
		REQUIRE(SkillCodec::AppendSkillRequest(frame, second).IsOk());

		CHECK_EQ(frame.size(), static_cast<std::size_t>(67 + 71));

		CHECK_EQ(PeekLE32(frame, 0), static_cast<WireU32>(67));
		CHECK_EQ(PeekLE32(frame, 4), SkillNS::kSkillId);
		CHECK_EQ(PeekLE32(frame, 67), static_cast<WireU32>(71));
		CHECK_EQ(PeekLE32(frame, 71), SkillNS::kSkillId);

		// Each decodes on its own, which is the property the buffer exists for.
		std::vector<WireU8> firstFrame(frame.begin(), frame.begin() + 67);
		std::vector<WireU8> secondFrame(frame.begin() + 67, frame.end());

		SkillNS::SkillRequest decodedFirst;
		SkillNS::SkillRequest decodedSecond;
		REQUIRE(SkillCodec::DecodeSkillRequest(firstFrame, decodedFirst).IsOk());
		REQUIRE(SkillCodec::DecodeSkillRequest(secondFrame, decodedSecond).IsOk());

		CHECK_EQ(decodedFirst.targets.size(), static_cast<std::size_t>(1));
		CHECK_EQ(decodedSecond.targets.size(), static_cast<std::size_t>(2));
		CHECK_EQ(decodedSecond.targets[1].id, static_cast<WireU16>(30u));
	}

	// The encoder refuses a target list longer than the array, rather than
	// truncating it: a caller that asked for 25 has a bug, and silently dropping
	// the 25th would hide it.
	MODERN_TEST(Skill_AppendRefusesMoreThanTwentyFourTargets)
	{
		SkillNS::SkillRequest request;
		for (std::size_t i = 0; i <= SkillNS::kMaxTargets; ++i)
		{
			request.targets.push_back(
			    SkillNS::SkillTarget{ static_cast<WireU16>(i), static_cast<WireU16>(i) });
		}

		std::vector<WireU8> frame;
		CHECK(SkillCodec::AppendSkillRequest(frame, request).IsError());
		CHECK_EQ(frame.size(), static_cast<std::size_t>(0));
	}

	// =========================================================================
	// Non-finite values
	// =========================================================================

	// Every one of the six floats on 3303 is compared by legacy against a
	// server-computed value with `!=` (GLCharSkillMsg.cpp:286-288 and :312-315).
	// Every comparison involving a NaN is FALSE, so a NaN in fSum_TarRange would
	// make the echo check DISAGREE where the client meant to agree - a cast
	// refused for a reason nobody can see. And vTARPOS drives ReceiveTeleport
	// directly for EMSPECA_RANDOM_EXP_RATE skills (GLCharSkillMsg.cpp:396),
	// where a NaN would become a position.
	//
	// Refused in BOTH directions, so the encoder can never emit a frame its own
	// decoder rejects - the round-trip hole GotoProtocol deliberately documents
	// is not repeated here.
	MODERN_TEST(Skill_NonFiniteFloatsAreRefusedOnTheWireAndByTheEncoder)
	{
		const std::size_t floatOffsets[] = {
		    SkillNS::kRequestSumTarRangeOffset,         // 29
		    SkillNS::kRequestAttackVelocityOffset,      // 41
		    SkillNS::kRequestAttackVelocityItemOffset,  // 45
		    SkillNS::kRequestTargetPositionOffset,      // 12 (x)
		    SkillNS::kRequestTargetPositionOffset + 4,  // 16 (y)
		    SkillNS::kRequestTargetPositionOffset + 8,  // 20 (z)
		};

		const float bad[2] = { std::numeric_limits<float>::quiet_NaN(),
		                       std::numeric_limits<float>::infinity() };

		for (const std::size_t offset : floatOffsets)
		{
			for (const float value : bad)
			{
				// On the wire - the direction a PEER controls, which is the one
				// that matters. Built from a FINITE request and then overwritten
				// with the bit pattern, which is the only way to get one there
				// once the encoder refuses it.
				std::vector<WireU8> frame = BuildRequest(0, 0);
				PokeLEFloat(frame, offset, value);

				SkillNS::SkillRequest decoded;
				decoded.flags = 0x12345678u;
				CHECK(SkillCodec::DecodeSkillRequest(frame, decoded).IsError());

				// The output is CLEARED on refusal.
				CHECK_EQ(decoded.flags, static_cast<WireU32>(0));

				// And the encoder refuses the same value, so there is no
				// round-trip hole.
				SkillNS::SkillRequest built;
				if (offset == SkillNS::kRequestSumTarRangeOffset)
				{
					built.sumTarRange = value;
				}
				else if (offset == SkillNS::kRequestAttackVelocityOffset)
				{
					built.attackVelocity = value;
				}
				else if (offset == SkillNS::kRequestAttackVelocityItemOffset)
				{
					built.attackVelocityItem = value;
				}
				else if (offset == SkillNS::kRequestTargetPositionOffset)
				{
					built.targetPosition.x = value;
				}
				else if (offset == SkillNS::kRequestTargetPositionOffset + 4)
				{
					built.targetPosition.y = value;
				}
				else
				{
					built.targetPosition.z = value;
				}

				std::vector<WireU8> emitted;
				CHECK(SkillCodec::AppendSkillRequest(emitted, built).IsError());
				CHECK_EQ(emitted.size(), static_cast<std::size_t>(0));
			}
		}
	}

	// The broadcast's own vector, for the same reason.
	MODERN_TEST(Skill_NonFiniteBroadcastPositionIsRefusedInBothDirections)
	{
		const float nanValue = std::numeric_limits<float>::quiet_NaN();

		std::vector<WireU8> frame = BuildBroadcast(0, 0);
		PokeLEFloat(frame, 22, nanValue);

		SkillNS::SkillBroadcast decoded;
		decoded.casterId = 0x11223344u;
		CHECK(SkillCodec::DecodeSkillBroadcast(frame, decoded).IsError());
		CHECK_EQ(decoded.casterId, static_cast<WireU32>(0));

		SkillNS::SkillBroadcast built;
		built.targetPosition.y = nanValue;
		std::vector<WireU8> emitted;
		CHECK(SkillCodec::AppendSkillBroadcast(emitted, built).IsError());
		CHECK_EQ(emitted.size(), static_cast<std::size_t>(0));
	}

	// Extremely large but FINITE coordinates are carried unchanged.
	//
	// Whether a destination is in range is a rule, and rules are decided after
	// decoding. A codec that clamped a value would make the bytes differ from
	// what the client sent, which is the wrong place for a range decision.
	MODERN_TEST(Skill_LargeButFiniteValuesSurviveUnchanged)
	{
		std::vector<WireU8> frame = BuildRequest(0, 0);
		PokeLEFloat(frame, 12, -100000.0f);
		PokeLEFloat(frame, 16, 100000.0f);
		PokeLEFloat(frame, 20, 0.0001f);

		SkillNS::SkillRequest decoded;
		REQUIRE(SkillCodec::DecodeSkillRequest(frame, decoded).IsOk());
		CHECK(decoded.targetPosition.x == -100000.0f);
		CHECK(decoded.targetPosition.y == 100000.0f);
		CHECK(decoded.targetPosition.z == 0.0001f);
	}
} // namespace
// WORLD-ENTRY-002k: the ATTACK_DAMAGE result codec (3043 / 3044).
//
// The sizes and offsets are asserted at compile time in AttackDamageProtocol.h,
// so these cases do not repeat that arithmetic. What they prove is the part a
// static_assert cannot: that the bytes on the wire are the ones a decoder reads
// back, that a wrong size is refused, and that the SIGNED damage field really is
// treated as signed.
//
// That last one is the case most worth having. `nDamage` is the only signed
// field in the whole ATTACK family, and a codec that read it unsigned would
// happily report a huge positive number where the sender meant something else -
// and a test that only ever sent small positive values would never notice.

#include "TestHarness.h"
#include "AttackDamageProtocol.h"

#include <vector>

namespace ModernTests
{
	using namespace Modern;
	using namespace Modern::Network;
	using namespace Modern::Network::Attack;

	namespace
	{
		// Independent little-endian accessors, per the GotoProtocolTests convention:
		// reading a wire value back with the codec that just wrote it would pass even
		// if every field were written big-endian at the wrong offset.
		void PokeLE32(std::vector<WireU8>& frame, std::size_t offset, WireU32 value)
		{
			for (std::size_t i = 0; i < 4; ++i)
			{
				frame[offset + i] = static_cast<WireU8>((value >> (8 * i)) & 0xFFu);
			}
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

		WireI32 PeekLE32Signed(const std::vector<WireU8>& frame, std::size_t offset)
		{
			WireI32 value = 0;
			for (std::size_t i = 0; i < 4; ++i)
			{
				value |= static_cast<WireI32>(
				             static_cast<WireU32>(frame[offset + i]) << (8 * i));
			}
			return value;
		}

		// A frame of `size` bytes declaring `id`, payload filled with a pattern so a
		// field landing at the wrong offset is visible.
		std::vector<WireU8> MakeFrame(MessageId id, std::size_t size)
		{
			std::vector<WireU8> frame(size);
			for (std::size_t i = 8; i < size; ++i)
			{
				frame[i] = static_cast<WireU8>(0xB0 + (i & 0x0F));
			}
			PokeLE32(frame, 0, static_cast<WireU32>(size));
			PokeLE32(frame, 4, static_cast<WireU32>(id));
			return frame;
		}
	}

	// ---- sizes and ids -------------------------------------------------------

	MODERN_TEST(AttackDamageProtocol_TheTwoMessagesAreTheSizesLegacyMeasured)
	{
		// SNETPC_ATTACK_DAMAGE     = 8 + 4*4                 = 24
		// SNETPC_ATTACK_DAMAGE_BRD = 12 (SNETPC_BROAD) + 4*4 = 28
		CHECK_EQ(kDamageSize, static_cast<std::size_t>(24));
		CHECK_EQ(kDamageBroadcastSize, static_cast<std::size_t>(28));

		CHECK_EQ(static_cast<int>(kAttackDamageId), 3043);
		CHECK_EQ(static_cast<int>(kAttackDamageBrdId), 3044);

		// The known DAMAGE_TYPE_* bits, from GLDefine.h:772-785.
		CHECK_EQ(kDamageTypeShock, 0x0001u);
		CHECK_EQ(kDamageTypeCritical, 0x0002u);
		CHECK_EQ(kDamageTypeCrushingBlow, 0x0004u);
		CHECK_EQ(kDamageTypeImmune, 0x0100u);
		CHECK_EQ(kDamageTypeIllusion, 0x0200u);
	}

	// ---- round trips ---------------------------------------------------------

	MODERN_TEST(AttackDamage_ADamageRoundTrips)
	{
		AttackDamage sent;
		sent.targetCrow = kCrowMob; // deliberately not the default
		sent.targetId   = 0xC0FFEEu;
		sent.damage     = 1234;
		sent.damageFlag = kDamageTypeCritical | kDamageTypeShock;

		std::vector<WireU8> frame;
		REQUIRE(AttackDamageCodec::AppendAttackDamage(frame, sent).IsOk());
		CHECK_EQ(frame.size(), kDamageSize);

		// Read back INDEPENDENTLY, so a wrong offset cannot hide behind a matching
		// encoder.
		CHECK_EQ(static_cast<int>(PeekLE32(frame, 0)), static_cast<int>(kDamageSize));
		CHECK_EQ(static_cast<int>(PeekLE32(frame, 4)), static_cast<int>(kAttackDamageId));
		CHECK_EQ(PeekLE32(frame, kDamageTargetCrowOffset), sent.targetCrow);
		CHECK_EQ(PeekLE32(frame, kDamageTargetIdOffset), sent.targetId);
		CHECK_EQ(PeekLE32Signed(frame, kDamageAmountOffset), sent.damage);
		CHECK_EQ(PeekLE32(frame, kDamageFlagOffset), sent.damageFlag);

		AttackDamage received;
		REQUIRE(AttackDamageCodec::DecodeAttackDamage(frame, received).IsOk());
		CHECK_EQ(received.targetCrow, sent.targetCrow);
		CHECK_EQ(received.targetId, sent.targetId);
		CHECK_EQ(received.damage, sent.damage);
		CHECK_EQ(received.damageFlag, sent.damageFlag);
	}

	MODERN_TEST(AttackDamage_ADamageBroadcastRoundTrips)
	{
		AttackDamageBroadcast sent;
		sent.gaeaId     = 77u;
		sent.targetCrow = kCrowPc;
		sent.targetId   = 88u;
		sent.damage     = 4321;
		sent.damageFlag = kDamageTypeNone;

		std::vector<WireU8> frame;
		REQUIRE(AttackDamageCodec::AppendAttackDamageBroadcast(frame, sent).IsOk());
		CHECK_EQ(frame.size(), kDamageBroadcastSize);

		CHECK_EQ(PeekLE32(frame, kDamageBroadcastGaeaIdOffset), sent.gaeaId);
		CHECK_EQ(PeekLE32(frame, kDamageBroadcastTargetCrowOffset), sent.targetCrow);
		CHECK_EQ(PeekLE32(frame, kDamageBroadcastTargetIdOffset), sent.targetId);
		CHECK_EQ(PeekLE32Signed(frame, kDamageBroadcastAmountOffset), sent.damage);
		CHECK_EQ(PeekLE32(frame, kDamageBroadcastFlagOffset), sent.damageFlag);

		AttackDamageBroadcast received;
		REQUIRE(AttackDamageCodec::DecodeAttackDamageBroadcast(frame, received).IsOk());
		CHECK_EQ(received.gaeaId, sent.gaeaId);
		CHECK_EQ(received.damage, sent.damage);
		CHECK_EQ(received.damageFlag, sent.damageFlag);
	}

	// ---- field boundaries ----------------------------------------------------

	MODERN_TEST(AttackDamage_TheDamageFieldIsSignedOnTheWire)
	{
		// The single most valuable case here. A value with the high bit set reads
		// back as a large POSITIVE number if the field were treated as unsigned, and
		// would be refused outright by the negative guard if it were decoded
		// correctly - so these two spellings cannot be confused.
		AttackDamage negative;
		negative.damage = -1;
		std::vector<WireU8> refused;
		CHECK(AttackDamageCodec::AppendAttackDamage(refused, negative).IsError());

		// The largest value a real hit can carry, which is well inside int32 but
		// would be the sign bit if it were bigger.
		AttackDamage large;
		large.damage = 0x7FFFFFFF;
		std::vector<WireU8> frame;
		REQUIRE(AttackDamageCodec::AppendAttackDamage(frame, large).IsOk());

		CHECK_EQ(PeekLE32Signed(frame, kDamageAmountOffset), 0x7FFFFFFF);

		AttackDamage received;
		REQUIRE(AttackDamageCodec::DecodeAttackDamage(frame, received).IsOk());
		CHECK_EQ(received.damage, 0x7FFFFFFF);

		// A frame carrying 0xFFFFFFFF must be REFUSED, not decoded as 4294967295.
		std::vector<WireU8> raw = MakeFrame(kAttackDamageId, kDamageSize);
		PokeLE32(raw, kDamageAmountOffset, 0xFFFFFFFFu);
		CHECK(AttackDamageCodec::DecodeAttackDamage(raw, received).IsError());

		// And the failure leaves no stale value behind.
		CHECK_EQ(received.damage, 0);
	}

	MODERN_TEST(AttackDamage_ZeroDamageIsAValidValue)
	{
		// Zero is not "no message". It is a legal int32 and must survive a round
		// trip; refusing it would make a zero-damage result unrepresentable.
		AttackDamage sent;
		sent.damage = 0;

		std::vector<WireU8> frame;
		REQUIRE(AttackDamageCodec::AppendAttackDamage(frame, sent).IsOk());

		AttackDamage received;
		REQUIRE(AttackDamageCodec::DecodeAttackDamage(frame, received).IsOk());
		CHECK_EQ(received.damage, 0);
	}

	MODERN_TEST(AttackDamage_UnknownDamageFlagBitsSurvive)
	{
		// A newer DAMAGE_TYPE_* must not break an older reader. The codec's job is
		// the wire's, and the known-bit mask is documentation, not a filter.
		const WireU32 unknownBit = 0x4000u;
		CHECK_EQ(kKnownDamageFlags & unknownBit, 0u);

		AttackDamage sent;
		sent.damageFlag = kDamageTypeCritical | unknownBit;

		std::vector<WireU8> frame;
		REQUIRE(AttackDamageCodec::AppendAttackDamage(frame, sent).IsOk());

		AttackDamage received;
		REQUIRE(AttackDamageCodec::DecodeAttackDamage(frame, received).IsOk());
		CHECK_EQ(received.damageFlag, sent.damageFlag);
	}

	// ---- malformed sizes -----------------------------------------------------

	MODERN_TEST(AttackDamage_EveryWrongSizeIsRefused)
	{
		const std::size_t sizes[]     = { kDamageSize - 1, kDamageSize + 1 };
		const std::size_t brdSizes[]  = { kDamageBroadcastSize - 1,
			                              kDamageBroadcastSize + 1 };

		for (std::size_t size : sizes)
		{
			AttackDamage out;
			CHECK(AttackDamageCodec::DecodeAttackDamage(MakeFrame(kAttackDamageId, size), out)
			          .IsError());
		}
		for (std::size_t size : brdSizes)
		{
			AttackDamageBroadcast out;
			CHECK(AttackDamageCodec::DecodeAttackDamageBroadcast(
			          MakeFrame(kAttackDamageBrdId, size), out)
			          .IsError());
		}
	}

	MODERN_TEST(AttackDamage_TheRightBytesUnderTheWrongIdAreRefused)
	{
		// 3043 is 24 bytes and so is a 3037. A frame declaring 3037 must not be
		// accepted as a damage result merely because the length matches.
		AttackDamage sent;
		sent.damage = 55;

		std::vector<WireU8> frame;
		REQUIRE(AttackDamageCodec::AppendAttackDamage(frame, sent).IsOk());
		CHECK_EQ(frame.size(), static_cast<std::size_t>(24)); // same as a 3037

		AttackDamage out;

		// And the genuinely-misfiled case: a real 3037 offered to the damage
		// decoder, which is the reverse of the same confusion.
		Attack::AttackBroadcast broadcast;
		std::vector<WireU8> attackFrame;
		REQUIRE(AttackCodec::AppendAttackBroadcast(attackFrame, broadcast).IsOk());
		CHECK_EQ(attackFrame.size(), kDamageSize);
		CHECK(AttackDamageCodec::DecodeAttackDamage(attackFrame, out).IsError());

		REQUIRE(AttackDamageCodec::DecodeAttackDamage(frame, out).IsOk());
	}

	// ---- predicates ----------------------------------------------------------

	MODERN_TEST(AttackDamage_ThePredicatesSelectOnlyTheirOwnId)
	{
		CHECK(AttackDamageCodec::IsAttackDamage(kAttackDamageId));
		CHECK(AttackDamageCodec::IsAttackDamageBroadcast(kAttackDamageBrdId));

		// 3038/3039 (cancel) and 3041/3042 (avoid) belong to other messages.
		CHECK(!AttackDamageCodec::IsAttackDamage(3041));
		CHECK(!AttackDamageCodec::IsAttackDamage(3039));
		CHECK(!AttackDamageCodec::IsAttackDamageBroadcast(3042));
		CHECK(!AttackDamageCodec::IsAttackDamage(3044));
	}
} // namespace ModernTests

// WORLD-ENTRY-002i: the ATTACK request family codec (3036/3037/3041/3042).
//
// The sizes and offsets are already asserted at compile time in
// AttackProtocol.h, so these cases do NOT repeat that arithmetic. What they prove
// is the part static_assert cannot: that the bytes a frame actually carries are the
// ones a decoder reads back, that a wrong size is refused, and that a frame
// carrying the right bytes under the WRONG id is refused too.
//
// That last one matters because RequireFrame checks the id as well as the size, and
// a peer controls both - a 24-byte frame declaring 3037 must not be accepted as a
// 3036 merely because both happen to be 24 bytes.

#include "TestHarness.h"
#include "AttackProtocol.h"
#include "NetworkCodec.h"

#include <vector>

namespace ModernTests
{
	using namespace Modern;
	using namespace Modern::Network;
	using namespace Modern::Network::Attack;

	namespace
	{
		// A little-endian 32-bit write at an ABSOLUTE offset, written out by hand
		// rather than reusing the codec's helper.
		//
		// Codec::WriteU32 APPENDS to a vector, so it cannot express "put a size at
		// offset 0" at all. And reading a wire value back with the same helper that
		// wrote it would pass even if every field were written big-endian at the
		// wrong offset - which is the class of bug these packets are most prone to.
		void PokeLE32(std::vector<WireU8>& frame, std::size_t offset, WireU32 value)
		{
			for (std::size_t i = 0; i < 4; ++i)
			{
				frame[offset + i] = static_cast<WireU8>((value >> (8 * i)) & 0xFFu);
			}
		}

		// The same independence for reading.
		WireU32 PeekLE32(const std::vector<WireU8>& frame, std::size_t offset)
		{
			WireU32 value = 0;
			for (std::size_t i = 0; i < 4; ++i)
			{
				value |= static_cast<WireU32>(frame[offset + i]) << (8 * i);
			}
			return value;
		}

		// Builds a frame of `size` bytes declaring `id`, with every payload byte a
		// recognisable pattern so a field landing at the wrong offset is visible
		// rather than accidentally equal.
		std::vector<WireU8> MakeFrame(MessageId id, std::size_t size)
		{
			std::vector<WireU8> frame(size);
			for (std::size_t i = 8; i < size; ++i)
			{
				frame[i] = static_cast<WireU8>(0xA0 + (i & 0x0F));
			}
			PokeLE32(frame, 0, static_cast<WireU32>(size));
			PokeLE32(frame, 4, static_cast<WireU32>(id));
			return frame;
		}
	}

	// ---- sizes are what legacy measured ---------------------------------------

	MODERN_TEST(AttackProtocol_TheFourMessagesAreTheSizesLegacyMeasured)
	{
		// NET_MSG_GENERIC is 8 bytes and SNETPC_BROAD is 12, so:
		//   3036 SNETPC_ATTACK          = 8 + 4*4 = 24
		//   3037 SNETPC_ATTACK_BRD      = 12 + 3*4 = 24
		//   3041 SNETPC_ATTACK_AVOID    = 8 + 2*4 = 16
		//   3042 SNETPC_ATTACK_AVOID_BRD = 12 + 2*4 = 20
		CHECK_EQ(kRequestSize, static_cast<std::size_t>(24));
		CHECK_EQ(kBroadcastSize, static_cast<std::size_t>(24));
		CHECK_EQ(kAvoidSize, static_cast<std::size_t>(16));
		CHECK_EQ(kAvoidBroadcastSize, static_cast<std::size_t>(20));

		// The ids, which are `NET_MSG_GCTRL + 144/145/149/150`.
		CHECK_EQ(static_cast<int>(kAttackId), 3036);
		CHECK_EQ(static_cast<int>(kAttackBrdId), 3037);
		CHECK_EQ(static_cast<int>(kAttackAvoidId), 3041);
		CHECK_EQ(static_cast<int>(kAttackAvoidBrdId), 3042);
	}

	// ---- round trips ----------------------------------------------------------

	MODERN_TEST(AttackProtocol_ARequestRoundTrips)
	{
		AttackRequest sent;
		sent.targetCrow = kCrowMob; // deliberately NOT the default
		sent.targetId   = 0xDEADBEEFu;
		sent.aniSel     = 0x01020304u;
		sent.flags      = 0x05060708u;

		std::vector<WireU8> frame;
		REQUIRE(AttackCodec::AppendAttackRequest(frame, sent).IsOk());
		CHECK_EQ(frame.size(), kRequestSize);
		CHECK_EQ(static_cast<int>(PeekLE32(frame, 0)), static_cast<int>(kRequestSize));
		CHECK_EQ(static_cast<int>(PeekLE32(frame, 4)), static_cast<int>(kAttackId));

		AttackRequest received;
		REQUIRE(AttackCodec::DecodeAttackRequest(frame, received).IsOk());
		CHECK_EQ(received.targetCrow, sent.targetCrow);
		CHECK_EQ(received.targetId, sent.targetId);
		CHECK_EQ(received.aniSel, sent.aniSel);
		CHECK_EQ(received.flags, sent.flags);
	}

	MODERN_TEST(AttackProtocol_ABroadcastRoundTrips)
	{
		AttackBroadcast sent;
		sent.gaeaId     = 7u;
		sent.targetCrow = kCrowPc;
		sent.targetId   = 4242u;
		sent.aniSel     = 99u;

		std::vector<WireU8> frame;
		REQUIRE(AttackCodec::AppendAttackBroadcast(frame, sent).IsOk());
		CHECK_EQ(frame.size(), kBroadcastSize);

		AttackBroadcast received;
		REQUIRE(AttackCodec::DecodeAttackBroadcast(frame, received).IsOk());
		CHECK_EQ(received.gaeaId, sent.gaeaId);
		CHECK_EQ(received.targetCrow, sent.targetCrow);
		CHECK_EQ(received.targetId, sent.targetId);
		CHECK_EQ(received.aniSel, sent.aniSel);
	}

	MODERN_TEST(AttackProtocol_AnAvoidRoundTrips)
	{
		AttackAvoid sent;
		sent.targetCrow = kCrowPc;
		sent.targetId   = 1234u;

		std::vector<WireU8> frame;
		REQUIRE(AttackCodec::AppendAttackAvoid(frame, sent).IsOk());
		CHECK_EQ(frame.size(), kAvoidSize);

		AttackAvoid received;
		REQUIRE(AttackCodec::DecodeAttackAvoid(frame, received).IsOk());
		CHECK_EQ(received.targetCrow, sent.targetCrow);
		CHECK_EQ(received.targetId, sent.targetId);
	}

	MODERN_TEST(AttackProtocol_AnAvoidBroadcastRoundTrips)
	{
		AttackAvoidBroadcast sent;
		sent.gaeaId     = 11u;
		sent.targetCrow = kCrowPc;
		sent.targetId   = 22u;

		std::vector<WireU8> frame;
		REQUIRE(AttackCodec::AppendAttackAvoidBroadcast(frame, sent).IsOk());
		CHECK_EQ(frame.size(), kAvoidBroadcastSize);

		AttackAvoidBroadcast received;
		REQUIRE(AttackCodec::DecodeAttackAvoidBroadcast(frame, received).IsOk());
		CHECK_EQ(received.gaeaId, sent.gaeaId);
		CHECK_EQ(received.targetCrow, sent.targetCrow);
		CHECK_EQ(received.targetId, sent.targetId);
	}

	// ---- exact-size validation -----------------------------------------------

	MODERN_TEST(AttackProtocol_EveryWrongSizeIsRefused)
	{
		AttackRequest    request;
		AttackBroadcast  broadcast;
		AttackAvoid      avoid;
		AttackAvoidBroadcast avoidBroadcast;

		// One byte short and one byte long for each of the four, both directions.
		const std::size_t requestSizes[]   = { kRequestSize - 1, kRequestSize + 1 };
		const std::size_t broadcastSizes[] = { kBroadcastSize - 1, kBroadcastSize + 1 };
		const std::size_t avoidSizes[]     = { kAvoidSize - 1, kAvoidSize + 1 };
		const std::size_t avoidBrdSizes[]  = { kAvoidBroadcastSize - 1,
		                                       kAvoidBroadcastSize + 1 };

		for (std::size_t size : requestSizes)
		{
			AttackRequest out;
			CHECK(AttackCodec::DecodeAttackRequest(MakeFrame(kAttackId, size), out)
			          .IsError());
		}
		for (std::size_t size : broadcastSizes)
		{
			AttackBroadcast out;
			CHECK(AttackCodec::DecodeAttackBroadcast(MakeFrame(kAttackBrdId, size), out)
			          .IsError());
		}
		for (std::size_t size : avoidSizes)
		{
			AttackAvoid out;
			CHECK(AttackCodec::DecodeAttackAvoid(MakeFrame(kAttackAvoidId, size), out)
			          .IsError());
		}
		for (std::size_t size : avoidBrdSizes)
		{
			AttackAvoidBroadcast out;
			CHECK(AttackCodec::DecodeAttackAvoidBroadcast(
			          MakeFrame(kAttackAvoidBrdId, size), out)
			          .IsError());
		}

		// Silence about the unused locals above would be a warning; the point is
		// that a correctly sized frame is accepted.
		std::vector<WireU8> good;
		REQUIRE(AttackCodec::AppendAttackRequest(good, request).IsOk());
		AttackRequest decoded;
		CHECK(AttackCodec::DecodeAttackRequest(good, decoded).IsOk());
		(void) broadcast;
		(void) avoid;
		(void) avoidBroadcast;
	}

	// ---- the id is checked as well as the size --------------------------------

	MODERN_TEST(AttackProtocol_TheRightBytesUnderTheWrongIdAreRefused)
	{
		// 3036 and 3037 are BOTH 24 bytes. A frame declaring 3037 must therefore not
		// be accepted as a 3036 just because the length matches - which is exactly
		// the class of bug the id check exists to catch.
		AttackBroadcast broadcast;
		broadcast.gaeaId = 1u;

		std::vector<WireU8> frame;
		REQUIRE(AttackCodec::AppendAttackBroadcast(frame, broadcast).IsOk());
		CHECK_EQ(frame.size(), kRequestSize); // same length as a 3036

		AttackRequest out;
		CHECK(AttackCodec::DecodeAttackRequest(frame, out).IsError());

		AttackBroadcast round;
		REQUIRE(AttackCodec::DecodeAttackBroadcast(frame, round).IsOk());
	}

	MODERN_TEST(AttackProtocol_APeerDeclaredSizeIsNeverUsedToIndex)
	{
		// The frame is 24 bytes and declares 999. Reading must be refused on the
		// real length, and the declared number must not become a read size - which
		// is the out-of-bounds this ordering prevents.
		const std::vector<WireU8> frame = MakeFrame(kAttackId, kRequestSize);
		PokeLE32(const_cast<std::vector<WireU8>&>(frame), 0, 999u);

		AttackRequest out;
		CHECK(AttackCodec::DecodeAttackRequest(frame, out).IsError());

		// And a truncated frame declaring its true length is still refused.
		std::vector<WireU8> truncated(frame.begin(), frame.end() - 1);
		CHECK(AttackCodec::DecodeAttackRequest(truncated, out).IsError());
	}

	// ---- predicates -----------------------------------------------------------

	MODERN_TEST(AttackProtocol_ThePredicatesSelectOnlyTheirOwnId)
	{
		CHECK(AttackCodec::IsAttack(kAttackId));
		CHECK(AttackCodec::IsAttackBroadcast(kAttackBrdId));
		CHECK(AttackCodec::IsAttackAvoid(kAttackAvoidId));
		CHECK(AttackCodec::IsAttackAvoidBroadcast(kAttackAvoidBrdId));

		// 3038/3039 (attack cancel) and 3043/3044 (damage) are NOT this milestone.
		// Nothing must claim them.
		CHECK(!AttackCodec::IsAttack(3038));
		CHECK(!AttackCodec::IsAttack(3043));
		CHECK(!AttackCodec::IsAttackBroadcast(3044));

		// A 3035 is not an attack.
		CHECK(!AttackCodec::IsAttack(3035));
		CHECK(!AttackCodec::IsAttackAvoid(3035));
	}
} // namespace ModernTests
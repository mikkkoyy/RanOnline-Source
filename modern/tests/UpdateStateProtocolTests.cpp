// WORLD-ENTRY-002h: layout and codec tests for 3046/3053.

#include "TestHarness.h"
#include "UpdateStateProtocol.h"
#include "NetworkCodec.h"

using Modern::Network::WireU8;
using Modern::Network::WireU32;

namespace
{
	using namespace Modern;
	using namespace Modern::Network;
	namespace UpdateStateNS = Modern::Network::UpdateState;
	namespace UpdateStateCodec = Modern::Network::UpdateState::UpdateStateCodec;

	// A little-endian 32-bit read, written out by hand rather than reusing the
	// codec's own helper. It exists so the byte-order and offset assertions are
	// genuinely INDEPENDENT of the code under test.
	WireU32 PeekLE32(const std::vector<WireU8>& frame, std::size_t offset)
	{
		WireU32 value = 0;
		for (std::size_t i = 0; i < 4; ++i)
		{
			value |= static_cast<WireU32>(frame[offset + i]) << (8 * i);
		}
		return value;
	}

	// Build a minimal valid StateUpdate.
	UpdateStateNS::StateUpdate MakeUpdate()
	{
		return UpdateStateNS::StateUpdate{
			{ 123, 456 },   // hp
			{ 789, 101112 }, // mp
			{ 131415, 161718 }, // sp
			{ 0, 0 },       // cp (dead field)
			"TestCharacter",
			0x11223344,
			0x55667788,
			false
		};
	}

	// Build a minimal valid StateBroadcast.
	UpdateStateNS::StateBroadcast MakeBroadcast()
	{
		return UpdateStateNS::StateBroadcast{
			0x11223344,
			{ 123, 456 },
			false
		};
	}
}

MODERN_TEST(UpdateStateProtocol_Layout_StateUpdate_IsExactly82Bytes)
{
	UpdateStateNS::StateUpdate u = MakeUpdate();
	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateUpdate(frame, u).IsOk());
	CHECK_EQ(frame.size(), 82u);
}

MODERN_TEST(UpdateStateProtocol_Layout_StateBroadcast_IsExactly21Bytes)
{
	UpdateStateNS::StateBroadcast b = MakeBroadcast();
	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateBroadcast(frame, b).IsOk());
	CHECK_EQ(frame.size(), 21u);
}

MODERN_TEST(UpdateStateProtocol_Layout_StateUpdate_OffsetsMatchLegacy)
{
	UpdateStateNS::StateUpdate u = MakeUpdate();
	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateUpdate(frame, u).IsOk());

	// NET_MSG_GENERIC header: dwSize@0, nType@4
	CHECK_EQ(PeekLE32(frame, 0), 82u);
	CHECK_EQ(PeekLE32(frame, 4), 3046u);

	// GLDWDATA pairs: hp@8, mp@16, sp@24, cp@32 (each 8 bytes = now,max)
	CHECK_EQ(PeekLE32(frame, 8), 123u);     // hp.now
	CHECK_EQ(PeekLE32(frame, 12), 456u);    // hp.max
	CHECK_EQ(PeekLE32(frame, 16), 789u);    // mp.now
	CHECK_EQ(PeekLE32(frame, 20), 101112u); // mp.max
	CHECK_EQ(PeekLE32(frame, 24), 131415u); // sp.now
	CHECK_EQ(PeekLE32(frame, 28), 161718u); // sp.max
	CHECK_EQ(PeekLE32(frame, 32), 0u);      // cp.now
	CHECK_EQ(PeekLE32(frame, 36), 0u);      // cp.max

	// szCharName[33]@40
	CHECK_EQ(frame[40], 'T');
	CHECK_EQ(frame[40 + 12], 'r');
	CHECK_EQ(frame[40 + 13], 0);

	// dwCharGaeaID@73
	CHECK_EQ(PeekLE32(frame, 73), 0x11223344u);
	// dwCharID@77
	CHECK_EQ(PeekLE32(frame, 77), 0x55667788u);
	// bSafeTime@81
	CHECK_EQ(frame[81], 0);
}

MODERN_TEST(UpdateStateProtocol_Layout_StateBroadcast_OffsetsMatchLegacy)
{
	UpdateStateNS::StateBroadcast b = MakeBroadcast();
	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateBroadcast(frame, b).IsOk());

	// SNETPC_BROAD base: nmg@0 (8), dwGaeaID@8
	CHECK_EQ(PeekLE32(frame, 0), 21u);
	CHECK_EQ(PeekLE32(frame, 4), 3053u);
	CHECK_EQ(PeekLE32(frame, 8), 0x11223344u);

	// sHP@12 (now,max)
	CHECK_EQ(PeekLE32(frame, 12), 123u);
	CHECK_EQ(PeekLE32(frame, 16), 456u);
	// bSafeTime@20
	CHECK_EQ(frame[20], 0);
}

MODERN_TEST(UpdateStateProtocol_Codec_RoundTrip_StateUpdate)
{
	UpdateStateNS::StateUpdate original = MakeUpdate();
	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateUpdate(frame, original).IsOk());

	UpdateStateNS::StateUpdate decoded;
	CHECK(UpdateStateCodec::DecodeStateUpdate(frame, decoded).IsOk());

	CHECK_EQ(decoded.hp.now, original.hp.now);
	CHECK_EQ(decoded.hp.max, original.hp.max);
	CHECK_EQ(decoded.mp.now, original.mp.now);
	CHECK_EQ(decoded.mp.max, original.mp.max);
	CHECK_EQ(decoded.sp.now, original.sp.now);
	CHECK_EQ(decoded.sp.max, original.sp.max);
	CHECK_EQ(decoded.cp.now, original.cp.now);
	CHECK_EQ(decoded.cp.max, original.cp.max);
	CHECK_EQ(decoded.name, original.name);
	CHECK_EQ(decoded.gaeaId, original.gaeaId);
	CHECK_EQ(decoded.charId, original.charId);
	CHECK_EQ(decoded.safeTime, original.safeTime);
}

MODERN_TEST(UpdateStateProtocol_Codec_RoundTrip_StateBroadcast)
{
	UpdateStateNS::StateBroadcast original = MakeBroadcast();
	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateBroadcast(frame, original).IsOk());

	UpdateStateNS::StateBroadcast decoded;
	CHECK(UpdateStateCodec::DecodeStateBroadcast(frame, decoded).IsOk());

	CHECK_EQ(decoded.gaeaId, original.gaeaId);
	CHECK_EQ(decoded.hp.now, original.hp.now);
	CHECK_EQ(decoded.hp.max, original.hp.max);
	CHECK_EQ(decoded.safeTime, original.safeTime);
}

MODERN_TEST(UpdateStateProtocol_Codec_BoundaryValues_UintMax)
{
	UpdateStateNS::StateUpdate u = MakeUpdate();
	u.hp.now = 0xFFFFFFFFu;
	u.hp.max = 0xFFFFFFFFu;
	u.mp.now = 0xFFFFFFFFu;
	u.mp.max = 0xFFFFFFFFu;
	u.sp.now = 0xFFFFFFFFu;
	u.sp.max = 0xFFFFFFFFu;

	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateUpdate(frame, u).IsOk());

	UpdateStateNS::StateUpdate decoded;
	CHECK(UpdateStateCodec::DecodeStateUpdate(frame, decoded).IsOk());

	CHECK_EQ(decoded.hp.now, 0xFFFFFFFFu);
	CHECK_EQ(decoded.hp.max, 0xFFFFFFFFu);
	CHECK_EQ(decoded.mp.now, 0xFFFFFFFFu);
	CHECK_EQ(decoded.mp.max, 0xFFFFFFFFu);
	CHECK_EQ(decoded.sp.now, 0xFFFFFFFFu);
	CHECK_EQ(decoded.sp.max, 0xFFFFFFFFu);
}

MODERN_TEST(UpdateStateProtocol_Codec_BoundaryValues_ZeroPools)
{
	UpdateStateNS::StateUpdate u = MakeUpdate();
	u.hp.now = u.hp.max = 0;
	u.mp.now = u.mp.max = 0;
	u.sp.now = u.sp.max = 0;

	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateUpdate(frame, u).IsOk());

	UpdateStateNS::StateUpdate decoded;
	CHECK(UpdateStateCodec::DecodeStateUpdate(frame, decoded).IsOk());

	CHECK_EQ(decoded.hp.now, 0u);
	CHECK_EQ(decoded.hp.max, 0u);
	CHECK_EQ(decoded.mp.now, 0u);
	CHECK_EQ(decoded.mp.max, 0u);
	CHECK_EQ(decoded.sp.now, 0u);
	CHECK_EQ(decoded.sp.max, 0u);
}

MODERN_TEST(UpdateStateProtocol_Codec_Name_Exact32Chars_Accepted)
{
	UpdateStateNS::StateUpdate u = MakeUpdate();
	u.name = "12345678901234567890123456789012"; // 32 chars
	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateUpdate(frame, u).IsOk());
}

MODERN_TEST(UpdateStateProtocol_Codec_Name_33Chars_Rejected)
{
	UpdateStateNS::StateUpdate u = MakeUpdate();
	u.name = "123456789012345678901234567890123"; // 33 chars
	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateUpdate(frame, u).IsError());
}

MODERN_TEST(UpdateStateProtocol_Codec_Name_Empty_Accepted)
{
	UpdateStateNS::StateUpdate u = MakeUpdate();
	u.name.clear();
	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateUpdate(frame, u).IsOk());

	// Decoded name should be empty
	UpdateStateNS::StateUpdate decoded;
	CHECK(UpdateStateCodec::DecodeStateUpdate(frame, decoded).IsOk());
	CHECK(decoded.name.empty());
}

MODERN_TEST(UpdateStateProtocol_Codec_Malformed_TooShort_StateUpdate)
{
	UpdateStateNS::StateUpdate u = MakeUpdate();
	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateUpdate(frame, u).IsOk());
	frame.pop_back(); // 81 bytes

	UpdateStateNS::StateUpdate decoded;
	CHECK(UpdateStateCodec::DecodeStateUpdate(frame, decoded).IsError());
}

MODERN_TEST(UpdateStateProtocol_Codec_Malformed_TooLong_StateUpdate)
{
	UpdateStateNS::StateUpdate u = MakeUpdate();
	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateUpdate(frame, u).IsOk());
	frame.push_back(0); // 83 bytes

	UpdateStateNS::StateUpdate decoded;
	CHECK(UpdateStateCodec::DecodeStateUpdate(frame, decoded).IsError());
}

MODERN_TEST(UpdateStateProtocol_Codec_Malformed_WrongId_StateUpdate)
{
	UpdateStateNS::StateBroadcast b = MakeBroadcast();
	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateBroadcast(frame, b).IsOk());
	// frame is 21 bytes, not 82 - decode as StateUpdate should fail
	UpdateStateNS::StateUpdate decoded;
	CHECK(UpdateStateCodec::DecodeStateUpdate(frame, decoded).IsError());
}

MODERN_TEST(UpdateStateProtocol_Codec_Malformed_WrongId_StateBroadcast)
{
	UpdateStateNS::StateUpdate u = MakeUpdate();
	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateUpdate(frame, u).IsOk());
	// frame is 82 bytes, not 21 - decode as StateBroadcast should fail
	UpdateStateNS::StateBroadcast decoded;
	CHECK(UpdateStateCodec::DecodeStateBroadcast(frame, decoded).IsError());
}

MODERN_TEST(UpdateStateProtocol_Codec_Malformed_WrongDwSize)
{
	UpdateStateNS::StateUpdate u = MakeUpdate();
	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateUpdate(frame, u).IsOk());
	// Corrupt dwSize
	frame[0] = 0xFF;
	frame[1] = 0xFF;
	frame[2] = 0xFF;
	frame[3] = 0xFF;

	UpdateStateNS::StateUpdate decoded;
	CHECK(UpdateStateCodec::DecodeStateUpdate(frame, decoded).IsError());
}

MODERN_TEST(UpdateStateProtocol_Codec_SafeTime_True_RoundTrip)
{
	UpdateStateNS::StateUpdate u = MakeUpdate();
	u.safeTime = true;
	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateUpdate(frame, u).IsOk());
	CHECK_EQ(frame[81], 1);

	UpdateStateNS::StateBroadcast b = MakeBroadcast();
	b.safeTime = true;
	std::vector<WireU8> frameB;
	CHECK(UpdateStateCodec::AppendStateBroadcast(frameB, b).IsOk());
	CHECK_EQ(frameB[20], 1);
}

MODERN_TEST(UpdateStateProtocol_Codec_Cp_DeadField_Carried)
{
	UpdateStateNS::StateUpdate u = MakeUpdate();
	u.cp.now = 0;
	u.cp.max = 0;
	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateUpdate(frame, u).IsOk());

	// cp at offset 32-39
	CHECK_EQ(PeekLE32(frame, 32), 0u);
	CHECK_EQ(PeekLE32(frame, 36), 0u);
}

MODERN_TEST(UpdateStateProtocol_Codec_Predicate_IsStateUpdate)
{
	UpdateStateNS::StateUpdate u = MakeUpdate();
	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateUpdate(frame, u).IsOk());
	CHECK(UpdateStateCodec::IsStateUpdate(3046));
	CHECK(!UpdateStateCodec::IsStateUpdate(3053));
	CHECK(!UpdateStateCodec::IsStateUpdate(3033));
}

MODERN_TEST(UpdateStateProtocol_Codec_Predicate_IsStateBroadcast)
{
	UpdateStateNS::StateBroadcast b = MakeBroadcast();
	std::vector<WireU8> frame;
	CHECK(UpdateStateCodec::AppendStateBroadcast(frame, b).IsOk());
	CHECK(UpdateStateCodec::IsStateBroadcast(3053));
	CHECK(!UpdateStateCodec::IsStateBroadcast(3046));
	CHECK(!UpdateStateCodec::IsStateBroadcast(3035));
}
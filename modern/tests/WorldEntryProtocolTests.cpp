// WORLD-ENTRY-001 Phase A: focused protocol tests for the world-entry layouts.
//
// These prove four things, and nothing else:
//
//   1. The measured sizes and offsets hold - 2332 is 1176 bytes with its fields
//      where the probe put them; 2333 is 1022 PACKED bytes, not the 1032 an
//      unpacked declaration measures.
//   2. The codecs round-trip.
//   3. Regions belonging to excluded subsystems are actually ZERO, not merely
//      labelled reserved. A reserved region that quietly carried data would be a
//      bug nobody could see, so the tests look at the bytes.
//   4. Malformed input is refused with a specific reason, never read past.
//
// There is no socket here on purpose. Phase A is layouts and codecs; the later
// phases own the connections. Framing is ConnectionFramer's job and compression
// is NetCompressCodec's, neither of which is duplicated or re-tested.

#include "TestHarness.h"

#include "CharacterListProtocol.h"
#include "NetworkTypes.h"
#include "RanWirePrimitives.h"
#include "WorldEntryProtocol.h"

#include <string>
#include <vector>

namespace
{
	using namespace Modern;
	using namespace Modern::Network;

	std::vector<WireU8> Repeat(WireU8 value, std::size_t count)
	{
		return std::vector<WireU8>(count, value);
	}

	// A little-endian 32-bit read, written out by hand rather than reusing the
	// codec's own helper.
	//
	// It exists so that the byte-order and offset assertions below are genuinely
	// INDEPENDENT of the code under test. Reading a wire value back with the codec
	// that just wrote it would pass even if every field were written big-endian at
	// the wrong offset, which is the class of bug these packets are most prone to.
	WireU32 PeekLE32(const std::vector<WireU8>& frame, std::size_t offset)
	{
		WireU32 value = 0;
		for (std::size_t i = 0; i < 4; ++i)
		{
			value |= static_cast<WireU32>(frame[offset + i]) << (8 * i);
		}
		return value;
	}

	// A single byte, for the cases where only part of a field is worth pinning.
	WireU8 PeekByte(const std::vector<WireU8>& frame, std::size_t offset)
	{
		return frame[offset];
	}

	// Builds a 76-byte character-id list without going through AppendIdList, which
	// deliberately only emits this build's 28-byte width.
	//
	// Synthesising it is the point: the 76-byte layout is an INTEROP requirement
	// (a released server sends it) rather than something this build produces, so
	// testing it through our own encoder would test the wrong thing.
	std::vector<WireU8> MakeReleasedWidthIdList(const std::vector<WireU32>& ids,
	                                            std::size_t declaredCount)
	{
		std::vector<WireU8> frame(CharacterList::kListReleasedSize, 0);
		frame[0] = static_cast<WireU8>(CharacterList::kListReleasedSize);
		frame[1] = 0;
		frame[2] = 0;
		frame[3] = 0;
		frame[4] = static_cast<WireU8>(CharacterList::kAllInfoId);
		frame[5] = static_cast<WireU8>(CharacterList::kAllInfoId >> 8);
		frame[6] = 0;
		frame[7] = 0;
		frame[8] = static_cast<WireU8>(declaredCount);
		frame[9] = 0;
		frame[10] = 0;
		frame[11] = 0;
		for (std::size_t i = 0; i < ids.size(); ++i)
		{
			for (std::size_t b = 0; b < 4; ++b)
			{
				frame[12 + i * 4 + b] = static_cast<WireU8>((ids[i] >> (8 * b)) & 0xFFu);
			}
		}
		return frame;
	}

	// A 2247 request is exactly a bare header. Checking the bytes is worth doing
	// because "8 bytes, all zero except size and type" is the whole packet.
	MODERN_TEST(WorldEntry_PhaseA_SharedPrimitivesHaveTheMeasuredSizes)
	{
		static_assert(sizeof(RanWire::NativeId) == 4, "");
		static_assert(sizeof(RanWire::DwPair) == 8, "");
		static_assert(sizeof(RanWire::LllPair) == 16, "");
		static_assert(sizeof(RanWire::StatsSix) == 12, "");
		static_assert(sizeof(RanWire::Vector3) == 12, "");
		static_assert(sizeof(RanWire::CryptKey) == 4, "");

		// The one that matters most, and the one two independent hand calculations
		// got wrong: SITEM_LOBY's members sum to 44 and its size is 48.
		CHECK_EQ(sizeof(RanWire::ItemLobbyWire), static_cast<std::size_t>(48));
		CHECK_EQ(offsetof(RanWire::ItemLobbyWire, genNum), static_cast<std::size_t>(8));
		CHECK_EQ(offsetof(RanWire::ItemLobbyWire, tailPad), static_cast<std::size_t>(44));

		// 22 slots x 48 = 1056. The superseded assumption of 44 bytes gives 968.
		CHECK_EQ(CharacterList::kSlotCount * CharacterList::kItemSlotSize,
		         static_cast<std::size_t>(1056));

		// CRYPT_KEY protects nothing; both halves are the constant 1.
		const RanWire::CryptKey key = RanWire::DefaultCryptKey();
		CHECK_EQ(static_cast<int>(key.keyDirection), 1);
		CHECK_EQ(static_cast<int>(key.key), 1);
	}

	// =========================================================================
	// 2247 - request the list
	// =========================================================================

	MODERN_TEST(WorldEntry_PhaseA_RequestAllIsABareEightByteHeader)
	{
		CHECK_EQ(CharacterList::kRequestAllId, static_cast<MessageId>(2247));
		CHECK_EQ(CharacterList::kRequestOneId, static_cast<MessageId>(2244));
		CHECK_EQ(CharacterList::kAllInfoId, static_cast<MessageId>(2248));
		CHECK_EQ(CharacterList::kCharacterDetailId, static_cast<MessageId>(2332));

		std::vector<WireU8> frame;
		CHECK(CharacterListCodec::AppendRequestAll(frame).IsOk());

		// Legacy's struct carries an `int nChannel` that is COMMENTED OUT
		// (s_NetGlobal.h:3970), so the packet is the header and nothing else. A body
		// here would be a packet no RAN server expects.
		CHECK_EQ(frame.size(), static_cast<std::size_t>(8));
		CHECK_EQ(PeekLE32(frame, 0), static_cast<WireU32>(8));
		CHECK_EQ(PeekLE32(frame, 4), static_cast<WireU32>(CharacterList::kRequestAllId));
		// 2247 == 0x08C7, so the low byte is 0xC7 and the second is 0x08. Pinned
		// explicitly because an off-by-one in the header would otherwise be invisible.
		CHECK_EQ(static_cast<int>(PeekByte(frame, 4)), 0xC7);
		CHECK_EQ(static_cast<int>(PeekByte(frame, 5)), 0x08);
		CHECK_EQ(static_cast<int>(PeekByte(frame, 6)), 0x00);
		CHECK_EQ(static_cast<int>(PeekByte(frame, 7)), 0x00);

		CHECK(CharacterListCodec::IsRequestAll(2247));
		CHECK(!CharacterListCodec::IsRequestAll(2248));
	}

	MODERN_TEST(WorldEntry_PhaseA_RequestOneCarriesTheCharacterId)
	{
		std::vector<WireU8> frame;
		CHECK(CharacterListCodec::AppendRequestOne(frame, 0x11223344u).IsOk());
		CHECK_EQ(frame.size(), static_cast<std::size_t>(12));

		WireU32 id = 0;
		CHECK(CharacterListCodec::ValidateRequestOne(frame, id).IsOk());
		CHECK_EQ(static_cast<int>(id), 0x11223344);

		// Appends rather than replaces, so a stream can be built.
		std::vector<WireU8> stream;
		CHECK(CharacterListCodec::AppendRequestOne(stream, 1).IsOk());
		CHECK(CharacterListCodec::AppendRequestOne(stream, 2).IsOk());
		CHECK_EQ(stream.size(), static_cast<std::size_t>(24));
	}

	// =========================================================================
	// 2248 - the id list, in BOTH real widths
	// =========================================================================

	MODERN_TEST(WorldEntry_PhaseA_IdListIsFixedWidthAndCarriesTheCountSeparately)
	{
		const std::vector<WireU32> ids = { 101u, 202u, 303u, 404u };

		std::vector<WireU8> frame;
		CHECK(CharacterListCodec::AppendIdList(frame, ids).IsOk());

		// 12 + 4*4. This is the width a build with no country macro emits.
		CHECK_EQ(frame.size(), static_cast<std::size_t>(28));
		CHECK_EQ(CharacterList::kListSmallSize, static_cast<std::size_t>(28));
		CHECK_EQ(PeekLE32(frame, 0), static_cast<WireU32>(28));
		CHECK_EQ(PeekLE32(frame, 4), static_cast<WireU32>(CharacterList::kAllInfoId));
		CHECK_EQ(PeekLE32(frame, 8), static_cast<WireU32>(4));
		CHECK_EQ(PeekLE32(frame, 12), 101u);
		CHECK_EQ(PeekLE32(frame, 16), 202u);
		CHECK_EQ(PeekLE32(frame, 20), 303u);
		CHECK_EQ(PeekLE32(frame, 24), 404u);

		CharacterIdList decoded;
		CHECK(CharacterListCodec::DecodeIdList(frame, decoded).IsOk());
		CHECK_EQ(decoded.ids.size(), static_cast<std::size_t>(4));
		for (std::size_t i = 0; i < ids.size(); ++i)
		{
			CHECK_EQ(decoded.ids[i], ids[i]);
		}
	}

	MODERN_TEST(WorldEntry_PhaseA_AFullListAndAPartlyFullListAreTheSameWidth)
	{
		// THE detail that matters most about 2248. NET_CHA_BBA_INFO's constructor sets
		// dwSize = sizeof(...) and nChaNum[] is a fixed-size array member, so an
		// account with one character sends the SAME 28 bytes as one with four.
		//
		// A variable-width encoder would emit 16 bytes here, which is a packet shape
		// neither this codebase nor a real client expects.
		std::vector<WireU8> full;
		std::vector<WireU8> partial;
		CHECK(CharacterListCodec::AppendIdList(full, { 1u, 2u, 3u, 4u }).IsOk());
		CHECK(CharacterListCodec::AppendIdList(partial, { 1u }).IsOk());

		CHECK_EQ(full.size(), partial.size());
		CHECK_EQ(partial.size(), static_cast<std::size_t>(28));

		// Only the count and the used slots differ.
		CHECK_EQ(PeekLE32(partial, 8), static_cast<WireU32>(1));
		CHECK_EQ(PeekLE32(partial, 12), 1u);

		// And the three unused slots are ZERO, which is what legacy's memset leaves.
		for (std::size_t slot = 1; slot < 4; ++slot)
		{
			CHECK_EQ(PeekLE32(partial, 12 + slot * 4), static_cast<WireU32>(0));
		}

		CharacterIdList decoded;
		CHECK(CharacterListCodec::DecodeIdList(partial, decoded).IsOk());

		// The trailing zeros are PADDING, not characters. Reporting them would invent
		// three characters that do not exist, and the client would then request a 2332
		// for a slot that has no character behind it.
		CHECK_EQ(decoded.ids.size(), static_cast<std::size_t>(1));
		CHECK_EQ(decoded.ids[0], 1u);
		CHECK_EQ(CharacterListCodec::ExpectedDetailCount(decoded), static_cast<std::size_t>(1));
	}

	MODERN_TEST(WorldEntry_PhaseA_DecodesTheSeventySixByteReleasedWidth)
	{
		// A released Korean server sets MAX_ONESERVERCHAR_NUM to 16 and sends 76 bytes.
		// A decoder hard-coded to 28 would read this as 28 and desynchronise on the
		// first character.
		//
		// Synthesised rather than produced by AppendIdList, which deliberately emits
		// only this build's 28-byte width: the 76-byte layout is an interop
		// requirement, not something this build generates.
		std::vector<WireU32> ids;
		for (WireU32 i = 0; i < CharacterList::kListReleasedSlots; ++i)
		{
			ids.push_back(1000u + i);
		}

		const std::vector<WireU8> frame =
		    MakeReleasedWidthIdList(ids, CharacterList::kListReleasedSlots);
		CHECK_EQ(frame.size(), static_cast<std::size_t>(76));
		CHECK_EQ(CharacterList::kListReleasedSize, static_cast<std::size_t>(76));

		CharacterIdList decoded;
		CHECK(CharacterListCodec::DecodeIdList(frame, decoded).IsOk());
		CHECK_EQ(decoded.ids.size(), CharacterList::kListReleasedSlots);
		if (decoded.ids.size() == ids.size())
		{
			for (std::size_t i = 0; i < ids.size(); ++i)
			{
				CHECK_EQ(decoded.ids[i], ids[i]);
			}
		}
		CHECK_EQ(CharacterListCodec::ExpectedDetailCount(decoded),
		         CharacterList::kListReleasedSlots);
	}

	MODERN_TEST(WorldEntry_PhaseA_EmptyIdListIsStillAValidTwentyEightBytePacket)
	{
		// An account with no characters on this server still gets a well-formed
		// packet. The width does NOT shrink with the count, because the array is a
		// fixed-size field in the struct, not a variable tail.
		std::vector<WireU8> frame;
		CHECK(CharacterListCodec::AppendIdList(frame, {}).IsOk());
		CHECK_EQ(frame.size(), static_cast<std::size_t>(28));
		CHECK_EQ(PeekLE32(frame, 8), static_cast<WireU32>(0));

		CharacterIdList decoded;
		CHECK(CharacterListCodec::DecodeIdList(frame, decoded).IsOk());
		CHECK_EQ(decoded.ids.size(), static_cast<std::size_t>(0));
		CHECK_EQ(CharacterListCodec::ExpectedDetailCount(decoded), static_cast<std::size_t>(0));
	}

	MODERN_TEST(WorldEntry_PhaseA_IdListRefusesMoreCharactersThanItsWidthHolds)
	{
		// Count above capacity is the silent-data-loss case: a decoder that trusted
		// the count would read ids out of bytes that are not there.
		CharacterIdList decoded;

		// 15 claimed in a packet that holds 4.
		{
			std::vector<WireU8> frame;
			CHECK(CharacterListCodec::AppendIdList(frame, { 1u, 2u, 3u, 4u }).IsOk());
			frame[8] = 15;
			CHECK(CharacterListCodec::DecodeIdList(frame, decoded).IsError());
		}

		// 1000 claimed in a 76-byte packet that holds 16.
		{
			const std::vector<WireU8> frame =
			    MakeReleasedWidthIdList({ 1u, 2u }, 1000);
			CHECK(CharacterListCodec::DecodeIdList(frame, decoded).IsError());
		}

		// And the encoder refuses outright rather than silently dropping characters
		// past slot 4, which would make a five-character account look like four.
		{
			std::vector<WireU8> frame;
			const std::size_t   before = frame.size();
			const std::vector<WireU32> five(5, 9u);
			CHECK(CharacterListCodec::AppendIdList(frame, five).IsError());
			CHECK_EQ(frame.size(), before);
		}
	}

	MODERN_TEST(WorldEntry_PhaseA_IdListRejectsMalformedFrames)
	{
		CharacterIdList decoded;

		// Empty.
		CHECK(CharacterListCodec::DecodeIdList({}, decoded).IsError());

		// Wrong declared size in the header.
		{
			std::vector<WireU8> frame;
			CHECK(CharacterListCodec::AppendIdList(frame, { 1u, 2u, 3u, 4u }).IsOk());
			frame[0] = 76; // claims 76 while only 28 bytes are present
			CHECK(CharacterListCodec::DecodeIdList(frame, decoded).IsError());
		}

		// Truncated: 27 of 28 bytes.
		{
			std::vector<WireU8> frame;
			CHECK(CharacterListCodec::AppendIdList(frame, { 1u, 2u, 3u, 4u }).IsOk());
			frame.resize(27);
			CHECK(CharacterListCodec::DecodeIdList(frame, decoded).IsError());
		}

		// A width that is neither 28 nor 76.
		{
			std::vector<WireU8> frame;
			CHECK(CharacterListCodec::AppendIdList(frame, { 1u, 2u, 3u, 4u }).IsOk());
			frame.resize(40);
			CHECK(CharacterListCodec::DecodeIdList(frame, decoded).IsError());
		}

		// Oversized: beyond the widest legal width.
		{
			std::vector<WireU8> frame;
			CHECK(CharacterListCodec::AppendIdList(frame, { 1u, 2u, 3u, 4u }).IsOk());
			frame.resize(200);
			CHECK(CharacterListCodec::DecodeIdList(frame, decoded).IsError());
		}

		// Negative count (0xFFFFFFFF read as a signed int).
		{
			std::vector<WireU8> frame;
			CHECK(CharacterListCodec::AppendIdList(frame, { 1u, 2u, 3u, 4u }).IsOk());
			for (int i = 0; i < 4; ++i)
			{
				frame[8 + static_cast<std::size_t>(i)] = 0xFF;
			}
			CHECK(CharacterListCodec::DecodeIdList(frame, decoded).IsError());
		}

		// Wrong message id.
		{
			std::vector<WireU8> frame;
			CHECK(CharacterListCodec::AppendIdList(frame, { 1u }).IsOk());
			frame[4] = 0xFF;
			CHECK(CharacterListCodec::DecodeIdList(frame, decoded).IsError());
		}

		// More ids than this build's width can carry.
		{
			std::vector<WireU8> frame;
			const std::vector<WireU32> tooMany(CharacterList::kLocalSlots + 1, 7u);
			CHECK(CharacterListCodec::AppendIdList(frame, tooMany).IsError());
		}
	}

	// =========================================================================
	// 2332 - the character detail
	// =========================================================================

	MODERN_TEST(WorldEntry_PhaseA_CharacterDetailHasTheMeasuredLayout)
	{
		// 1176, not the 1088 that summing the source's members produces. The 88-byte
		// difference is SITEM_LOBY's alignment padding multiplied by 22 slots.
		CHECK_EQ(sizeof(CharacterDetailWire), static_cast<std::size_t>(1168));
		CHECK_EQ(sizeof(CharacterDetailFrameWire), static_cast<std::size_t>(1176));
		CHECK_EQ(CharacterList::kCharacterDetailSize, static_cast<std::size_t>(1176));

		CHECK_EQ(offsetof(CharacterDetailWire, characterId), static_cast<std::size_t>(0));
		CHECK_EQ(offsetof(CharacterDetailWire, name), static_cast<std::size_t>(4));
		CHECK_EQ(offsetof(CharacterDetailWire, characterClass), static_cast<std::size_t>(40));
		CHECK_EQ(offsetof(CharacterDetailWire, school), static_cast<std::size_t>(44));
		CHECK_EQ(offsetof(CharacterDetailWire, hp), static_cast<std::size_t>(56));
		CHECK_EQ(offsetof(CharacterDetailWire, level), static_cast<std::size_t>(84));
		CHECK_EQ(offsetof(CharacterDetailWire, equipment), static_cast<std::size_t>(104));
		CHECK_EQ(sizeof(CharacterDetailWire::equipment), static_cast<std::size_t>(1056));
		CHECK_EQ(offsetof(CharacterDetailWire, saveMapId), static_cast<std::size_t>(1160));
	}

	MODERN_TEST(WorldEntry_PhaseA_CharacterDetailRoundTrips)
	{
		CharacterDetail detail;
		detail.characterId   = 0x0BADF00Du;
		detail.name          = "Sunflower";
		detail.characterClass = 7u;
		detail.school        = 2u;
		detail.level         = 31u;
		detail.hp.now        = 250u;
		detail.hp.max        = 300u;
		detail.saveMapId.value = 0x11223344u;

		std::vector<WireU8> frame;
		CHECK(CharacterListCodec::AppendCharacterDetail(frame, detail).IsOk());
		CHECK_EQ(frame.size(), static_cast<std::size_t>(1176));

		CharacterDetail decoded;
		CHECK(CharacterListCodec::DecodeCharacterDetail(frame, decoded).IsOk());
		CHECK(decoded == detail);
	}

	MODERN_TEST(WorldEntry_PhaseA_CharacterDetailReservedRegionsAreActuallyZero)
	{
		// Not merely "labelled reserved" - the bytes are inspected. A reserved
		// region that quietly carried data would be invisible in every other test.
		CharacterDetail detail;
		detail.characterId = 5u;
		detail.name        = "A";
		detail.hp.now      = 10u;

		std::vector<WireU8> frame;
		CHECK(CharacterListCodec::AppendCharacterDetail(frame, detail).IsOk());

		const std::size_t payload = CharacterList::kBareMessageSize;

		// The whole 1056-byte equipment array. Equipment is out of scope, so all of
		// it is zero.
		CHECK(RanWire::IsRegionZeroed(
		    frame, payload + CharacterList::kDetailOffsetEquipment,
		    CharacterList::kEquipmentSize));

		// The remaining reserved runs.
		CHECK(RanWire::IsRegionZeroed(
		    frame, payload + CharacterList::kDetailReservedAppearanceOffset,
		    CharacterList::kDetailReservedAppearanceSize));
		CHECK(RanWire::IsRegionZeroed(
		    frame, payload + CharacterList::kDetailReservedExperienceOffset,
		    CharacterList::kDetailReservedExperienceSize));
		CHECK(RanWire::IsRegionZeroed(
		    frame, payload + CharacterList::kDetailReservedBrightOffset,
		    CharacterList::kDetailReservedBrightSize));
		CHECK(RanWire::IsRegionZeroed(
		    frame, payload + CharacterList::kDetailReservedStatsOffset,
		    CharacterList::kDetailReservedStatsSize));
		CHECK(RanWire::IsRegionZeroed(
		    frame, payload + CharacterList::kDetailReservedScaleOffset,
		    CharacterList::kDetailReservedScaleSize));

		// And the interior padding, which legacy leaves indeterminate.
		CHECK(RanWire::IsRegionZeroed(frame, payload + 37, 3));
		CHECK(RanWire::IsRegionZeroed(frame, payload + 54, 2));
		CHECK(RanWire::IsRegionZeroed(frame, payload + 98, 6));

		// The name field is padded out to its full 33 bytes, not left at 2.
		CHECK(RanWire::IsRegionZeroed(frame, payload + CharacterList::kDetailOffsetName + 2,
		                              CharacterList::kNameFieldSize - 2));
	}

	MODERN_TEST(WorldEntry_PhaseA_CharacterDetailRejectsMalformedFrames)
	{
		CharacterDetail decoded;

		CHECK(CharacterListCodec::DecodeCharacterDetail({}, decoded).IsError());

		CharacterDetail detail;
		detail.characterId = 1u;
		detail.name        = "Test";

		// Truncated by one byte.
		{
			std::vector<WireU8> frame;
			CHECK(CharacterListCodec::AppendCharacterDetail(frame, detail).IsOk());
			frame.resize(1175);
			CHECK(CharacterListCodec::DecodeCharacterDetail(frame, decoded).IsError());
		}

		// Oversized.
		{
			std::vector<WireU8> frame;
			CHECK(CharacterListCodec::AppendCharacterDetail(frame, detail).IsOk());
			frame.resize(1177);
			CHECK(CharacterListCodec::DecodeCharacterDetail(frame, decoded).IsError());
		}

		// Wrong message id.
		{
			std::vector<WireU8> frame;
			CHECK(CharacterListCodec::AppendCharacterDetail(frame, detail).IsOk());
			frame[4] = 0x11;
			CHECK(CharacterListCodec::DecodeCharacterDetail(frame, decoded).IsError());
		}

		// Declared size disagreeing with the bytes.
		{
			std::vector<WireU8> frame;
			CHECK(CharacterListCodec::AppendCharacterDetail(frame, detail).IsOk());
			frame[0] = static_cast<WireU8>(1000);
			CHECK(CharacterListCodec::DecodeCharacterDetail(frame, decoded).IsError());
		}

		// A name field with no NUL anywhere in its 33 bytes. Legacy's reader would
		// run off the end of the struct here; the modern boundary refuses.
		{
			std::vector<WireU8> frame;
			CHECK(CharacterListCodec::AppendCharacterDetail(frame, detail).IsOk());
			const std::size_t nameAt = CharacterList::kBareMessageSize +
			                           CharacterList::kDetailOffsetName;
			for (std::size_t i = 0; i < CharacterList::kNameFieldSize; ++i)
			{
				frame[nameAt + i] = 'X';
			}
			CHECK(CharacterListCodec::DecodeCharacterDetail(frame, decoded).IsError());
		}
	}

	MODERN_TEST(WorldEntry_PhaseA_CharacterDetailRefusesAnOverlongNameWithoutPartialWrite)
	{
		// 33 characters cannot fit a 33-byte field with room for its terminator. The
		// append must refuse AND leave the caller's buffer untouched, because a
		// half-written frame is worse than no frame.
		CharacterDetail detail;
		detail.characterId = 1u;
		detail.name        = std::string(CharacterList::kNameFieldSize, 'A');

		std::vector<WireU8> frame;
		const std::size_t   before = frame.size();

		CHECK(CharacterListCodec::AppendCharacterDetail(frame, detail).IsError());
		CHECK_EQ(frame.size(), before);

		// One shorter is the longest that fits.
		detail.name = std::string(CharacterList::kNameFieldSize - 1, 'A');
		CHECK(CharacterListCodec::AppendCharacterDetail(frame, detail).IsOk());
		CHECK_EQ(frame.size(), static_cast<std::size_t>(1176));
	}

	MODERN_TEST(WorldEntry_PhaseA_SeveralDetailsProduceIndependentFrames)
	{
		// The list exchange is N records in a row. Each must be independently
		// decodable, and none may bleed into the next.
		std::vector<WireU8> stream;
		for (WireU32 i = 0; i < 4; ++i)
		{
			CharacterDetail detail;
			detail.characterId = 100u + i;
			detail.name        = "Char" + std::to_string(i);
			detail.level       = static_cast<WireU16>(10 + i);
			CHECK(CharacterListCodec::AppendCharacterDetail(stream, detail).IsOk());
		}
		CHECK_EQ(stream.size(), static_cast<std::size_t>(4 * 1176));

		for (WireU32 i = 0; i < 4; ++i)
		{
			const std::size_t at = static_cast<std::size_t>(i) * 1176;
			const std::vector<WireU8> slice(
			    stream.begin() + static_cast<std::ptrdiff_t>(at),
			    stream.begin() + static_cast<std::ptrdiff_t>(at + 1176));

			CharacterDetail decoded;
			CHECK(CharacterListCodec::DecodeCharacterDetail(slice, decoded).IsOk());
			CHECK_EQ(decoded.characterId, 100u + i);
			CHECK_EQ(decoded.name, "Char" + std::to_string(i));
			CHECK_EQ(static_cast<int>(decoded.level), static_cast<int>(10 + i));
		}
	}

	// =========================================================================
	// 2353 - the selection
	// =========================================================================

	MODERN_TEST(WorldEntry_PhaseA_GameJoinRoundTrips)
	{
		CHECK_EQ(WorldEntry::kGameJoinId, static_cast<MessageId>(2353));
		CHECK_EQ(WorldEntry::kGameJoinSize, static_cast<std::size_t>(12));

		std::vector<WireU8> frame;
		CHECK(WorldEntryCodec::AppendGameJoin(frame, 4242).IsOk());
		CHECK_EQ(frame.size(), static_cast<std::size_t>(12));
		CHECK_EQ(PeekLE32(frame, 0), static_cast<WireU32>(12));
		CHECK_EQ(PeekLE32(frame, 4), static_cast<WireU32>(WorldEntry::kGameJoinId));
		CHECK_EQ(PeekLE32(frame, 8), static_cast<WireU32>(4242));

		WireI32 value = 0;
		CHECK(WorldEntryCodec::DecodeGameJoin(frame, value).IsOk());
		CHECK_EQ(static_cast<int>(value), 4242);

		// Negative ids are carried, not clamped: the field is a signed int and the
		// server is the one that must reject an id it does not own.
		//
		// A SEPARATE frame, because AppendGameJoin appends and DecodeGameJoin requires
		// exactly one frame - reusing `frame` here would decode two messages at once.
		std::vector<WireU8> negative;
		CHECK(WorldEntryCodec::AppendGameJoin(negative, -1).IsOk());
		CHECK_EQ(negative.size(), static_cast<std::size_t>(12));
		CHECK(WorldEntryCodec::DecodeGameJoin(negative, value).IsOk());
		CHECK_EQ(static_cast<int>(value), -1);
	}

	MODERN_TEST(WorldEntry_PhaseA_GameJoinRejectsMalformedFrames)
	{
		WireI32 value = 0;
		CHECK(WorldEntryCodec::DecodeGameJoin({}, value).IsError());

		std::vector<WireU8> frame;
		CHECK(WorldEntryCodec::AppendGameJoin(frame, 1).IsOk());

		frame.resize(11);
		CHECK(WorldEntryCodec::DecodeGameJoin(frame, value).IsError());

		CHECK(WorldEntryCodec::AppendGameJoin(frame, 1).IsOk());
		frame[4] = 0x22;
		CHECK(WorldEntryCodec::DecodeGameJoin(frame, value).IsError());
	}

	// =========================================================================
	// 2358 - the field redirect
	// =========================================================================

	MODERN_TEST(WorldEntry_PhaseA_FieldRedirectCarriesTheRealAddressAndIdentity)
	{
		CHECK_EQ(WorldEntry::kConnectFieldId, static_cast<MessageId>(2358));
		CHECK_EQ(WorldEntry::kRedirectSize, static_cast<std::size_t>(48));

		FieldRedirect redirect;
		redirect.joinType       = WorldEntry::kJoinTypeFirst;
		redirect.gaeaId         = 0x00C0FFEEu;
		redirect.slotFieldAgent = 77u;
		redirect.servicePort    = 12002;
		redirect.fieldIp        = "127.0.0.1";

		std::vector<WireU8> frame;
		CHECK(WorldEntryCodec::AppendFieldRedirect(frame, redirect).IsOk());
		CHECK_EQ(frame.size(), static_cast<std::size_t>(48));

		// The three values a client genuinely needs are present at their measured
		// offsets - not stubbed, and not something the client is allowed to ignore.
		//
		// Read little-endian by hand rather than via the codec, so that a wrong
		// offset or a wrong byte order would actually fail here.
		CHECK_EQ(PeekLE32(frame, WorldEntry::kRedirectOffsetJoinType),
		         static_cast<WireU32>(WorldEntry::kJoinTypeFirst));
		CHECK_EQ(PeekLE32(frame, WorldEntry::kRedirectOffsetGaeaId), 0x00C0FFEEu);
		CHECK_EQ(PeekLE32(frame, WorldEntry::kRedirectOffsetSlot), 77u);

		// 12002 == 0x00002EE2, so LE gives E2 2E 00 00. Spelled out byte by byte
		// because getting this wrong is exactly the failure being tested for: a
		// big-endian port would be a redirect the client cannot connect to.
		CHECK_EQ(static_cast<int>(PeekByte(frame, WorldEntry::kRedirectOffsetPort)), 0xE2);
		CHECK_EQ(static_cast<int>(PeekByte(frame, WorldEntry::kRedirectOffsetPort + 1)), 0x2E);
		CHECK_EQ(static_cast<int>(PeekByte(frame, WorldEntry::kRedirectOffsetPort + 2)), 0x00);
		CHECK_EQ(static_cast<int>(PeekByte(frame, WorldEntry::kRedirectOffsetPort + 3)), 0x00);

		// And the address really is in the 21-byte field, NUL-terminated.
		// "127.0.0.1" is 9 characters, so byte 9 is the terminator and 10..20 are
		// zero padding - which is what makes the encoded frame byte-comparable.
		CHECK_EQ(static_cast<int>(PeekByte(frame, WorldEntry::kRedirectOffsetIp)), '1');
		CHECK_EQ(static_cast<int>(PeekByte(frame, WorldEntry::kRedirectOffsetIp + 8)), '1');
		CHECK_EQ(static_cast<int>(PeekByte(frame, WorldEntry::kRedirectOffsetIp + 9)), 0x00);
		CHECK_EQ(static_cast<int>(PeekByte(frame, WorldEntry::kRedirectOffsetIp + 20)), 0x00);

		FieldRedirect decoded;
		CHECK(WorldEntryCodec::DecodeFieldRedirect(frame, decoded).IsOk());
		CHECK_EQ(static_cast<int>(decoded.joinType), static_cast<int>(WorldEntry::kJoinTypeFirst));
		CHECK_EQ(decoded.gaeaId, redirect.gaeaId);
		CHECK_EQ(decoded.slotFieldAgent, redirect.slotFieldAgent);
		CHECK_EQ(static_cast<int>(decoded.servicePort), 12002);
		CHECK_EQ(decoded.fieldIp, redirect.fieldIp);
	}

	MODERN_TEST(WorldEntry_PhaseA_FieldRedirectRejectsMalformedFrames)
	{
		FieldRedirect decoded;

		CHECK(WorldEntryCodec::DecodeFieldRedirect({}, decoded).IsError());

		FieldRedirect redirect;
		redirect.servicePort = 12002;
		redirect.fieldIp     = "10.0.0.1";

		std::vector<WireU8> frame;
		CHECK(WorldEntryCodec::AppendFieldRedirect(frame, redirect).IsOk());

		// Truncated.
		{
			std::vector<WireU8> bad = frame;
			bad.resize(47);
			CHECK(WorldEntryCodec::DecodeFieldRedirect(bad, decoded).IsError());
		}

		// Oversized.
		{
			std::vector<WireU8> bad = frame;
			bad.resize(49);
			CHECK(WorldEntryCodec::DecodeFieldRedirect(bad, decoded).IsError());
		}

		// Wrong id.
		{
			std::vector<WireU8> bad = frame;
			bad[4] = 0x33;
			CHECK(WorldEntryCodec::DecodeFieldRedirect(bad, decoded).IsError());
		}

		// Address field with no terminator.
		{
			std::vector<WireU8> bad = frame;
			for (std::size_t i = 0; i < WorldEntry::kAddressFieldSize; ++i)
			{
				bad[WorldEntry::kRedirectOffsetIp + i] = 'X';
			}
			CHECK(WorldEntryCodec::DecodeFieldRedirect(bad, decoded).IsError());
		}

		// An address too long to fit with its terminator is refused rather than
		// truncated: a client that dialled a shortened address would fail in a way
		// that looks like a network fault.
		{
			FieldRedirect tooLong = redirect;
			tooLong.fieldIp = std::string(WorldEntry::kAddressFieldSize, '9');
			std::vector<WireU8> out;
			const std::size_t   before = out.size();
			CHECK(WorldEntryCodec::AppendFieldRedirect(out, tooLong).IsError());
			CHECK_EQ(out.size(), before);
		}
	}

	// =========================================================================
	// 2359 - the field identity
	// =========================================================================

	MODERN_TEST(WorldEntry_PhaseA_FieldIdentityRoundTrips)
	{
		CHECK_EQ(WorldEntry::kJoinFieldId, static_cast<MessageId>(2359));
		CHECK_EQ(WorldEntry::kIdentitySize, static_cast<std::size_t>(24));

		FieldIdentity identity;
		identity.joinType       = WorldEntry::kJoinTypeFirst;
		identity.gaeaId         = 0xABCDEF01u;
		identity.slotFieldAgent = 31337u;
		identity.cryptKey       = RanWire::DefaultCryptKey();

		std::vector<WireU8> frame;
		CHECK(WorldEntryCodec::AppendFieldIdentity(frame, identity).IsOk());
		CHECK_EQ(frame.size(), static_cast<std::size_t>(24));

		FieldIdentity decoded;
		CHECK(WorldEntryCodec::DecodeFieldIdentity(frame, decoded).IsOk());
		CHECK_EQ(static_cast<int>(decoded.joinType), static_cast<int>(WorldEntry::kJoinTypeFirst));
		CHECK_EQ(decoded.gaeaId, identity.gaeaId);
		CHECK_EQ(decoded.slotFieldAgent, identity.slotFieldAgent);

		// Carried for framing, and it protects nothing - see RanWirePrimitives.h.
		CHECK_EQ(static_cast<int>(decoded.cryptKey.keyDirection), 1);
		CHECK_EQ(static_cast<int>(decoded.cryptKey.key), 1);
	}

	MODERN_TEST(WorldEntry_PhaseA_FieldIdentityRejectsMalformedFrames)
	{
		FieldIdentity decoded;
		CHECK(WorldEntryCodec::DecodeFieldIdentity({}, decoded).IsError());

		FieldIdentity identity;
		identity.gaeaId = 5u;

		std::vector<WireU8> frame;
		CHECK(WorldEntryCodec::AppendFieldIdentity(frame, identity).IsOk());

		std::vector<WireU8> truncated = frame;
		truncated.resize(23);
		CHECK(WorldEntryCodec::DecodeFieldIdentity(truncated, decoded).IsError());

		std::vector<WireU8> oversized = frame;
		oversized.resize(25);
		CHECK(WorldEntryCodec::DecodeFieldIdentity(oversized, decoded).IsError());

		std::vector<WireU8> wrongId = frame;
		wrongId[4] = 0x44;
		CHECK(WorldEntryCodec::DecodeFieldIdentity(wrongId, decoded).IsError());

		std::vector<WireU8> wrongSize = frame;
		wrongSize[0] = 12;
		CHECK(WorldEntryCodec::DecodeFieldIdentity(wrongSize, decoded).IsError());
	}

	// =========================================================================
	// 2333 - the spawn
	// =========================================================================

	MODERN_TEST(WorldEntry_PhaseA_SpawnHasTheMeasuredPackedLayout)
	{
		// 1022 PACKED. The same declaration at default alignment measures 1032, which
		// is the wrong wire size by 10 bytes - the whole reason the packing context
		// is reproduced rather than assumed.
		CHECK_EQ(sizeof(SpawnWire), static_cast<std::size_t>(1022));
		CHECK_EQ(WorldEntry::kSpawnSize, static_cast<std::size_t>(1022));
		CHECK_EQ(WorldEntry::kSpawnSize, CharacterList::kBareMessageSize + 21 + 4 + 4 + 4 + 12 + 600 + 2 + 288 + 44 + 2 + 4 + 4 + 4 + 12 + 1 + 4 + 4);

		CHECK_EQ(offsetof(SpawnWire, userId), static_cast<std::size_t>(8));
		CHECK_EQ(offsetof(SpawnWire, clientId), static_cast<std::size_t>(29));
		CHECK_EQ(offsetof(SpawnWire, gaeaId), static_cast<std::size_t>(33));
		CHECK_EQ(offsetof(SpawnWire, mapId), static_cast<std::size_t>(37));
		CHECK_EQ(offsetof(SpawnWire, position), static_cast<std::size_t>(41));
		CHECK_EQ(offsetof(SpawnWire, data), static_cast<std::size_t>(53));
		CHECK_EQ(offsetof(SpawnWire, startMapId), static_cast<std::size_t>(989));
		CHECK_EQ(offsetof(SpawnWire, startGate), static_cast<std::size_t>(993));

		// The 600-byte record keeps its natural internal layout even though it is a
		// member of a packed struct. MSVC places the member with align 1 but does not
		// re-pack the member itself.
		CHECK_EQ(sizeof(SpawnWire::data), static_cast<std::size_t>(600));
		CHECK_EQ(sizeof(SpawnCharacterRecordWire), static_cast<std::size_t>(600));

		CHECK_EQ(offsetof(SpawnCharacterRecordWire, accountId), static_cast<std::size_t>(0));
		CHECK_EQ(offsetof(SpawnCharacterRecordWire, characterId), static_cast<std::size_t>(76));
		CHECK_EQ(offsetof(SpawnCharacterRecordWire, name), static_cast<std::size_t>(80));
		CHECK_EQ(offsetof(SpawnCharacterRecordWire, characterClass), static_cast<std::size_t>(120));
		CHECK_EQ(offsetof(SpawnCharacterRecordWire, school), static_cast<std::size_t>(124));
		CHECK_EQ(offsetof(SpawnCharacterRecordWire, level), static_cast<std::size_t>(144));
		CHECK_EQ(offsetof(SpawnCharacterRecordWire, hp), static_cast<std::size_t>(304));
		CHECK_EQ(offsetof(SpawnCharacterRecordWire, mp), static_cast<std::size_t>(312));
		CHECK_EQ(offsetof(SpawnCharacterRecordWire, sp), static_cast<std::size_t>(320));
	}

	MODERN_TEST(WorldEntry_PhaseA_SpawnRoundTripsTheAuthoritativeState)
	{
		CHECK_EQ(WorldEntry::kCharacterJoinId, static_cast<MessageId>(2333));

		SpawnState spawn;
		spawn.userId         = "player_one";
		spawn.clientId       = 100u;
		spawn.gaeaId         = 0x0BADF00Du;
		spawn.mapId.value    = 7u;
		spawn.position.x     = 123.5f;
		spawn.position.y     = -64.25f;
		spawn.position.z     = 0.0f;
		spawn.accountId      = 9001u;
		spawn.characterId    = 0x11223344u;
		spawn.characterName  = "Sunflower";
		spawn.characterClass = 3u;
		spawn.school         = 1u;
		spawn.level          = 42u;
		spawn.hp.now         = 500u;
		spawn.mp.now         = 250u;
		spawn.sp.now         = 100u;
		spawn.startMapId.value = 7u;
		spawn.startGate        = 12u;

		std::vector<WireU8> frame;
		CHECK(WorldEntryCodec::AppendSpawn(frame, spawn).IsOk());
		CHECK_EQ(frame.size(), static_cast<std::size_t>(1022));

		SpawnState decoded;
		CHECK(WorldEntryCodec::DecodeSpawn(frame, decoded).IsOk());

		CHECK_EQ(decoded.userId, spawn.userId);
		CHECK_EQ(decoded.clientId, spawn.clientId);
		CHECK_EQ(decoded.gaeaId, spawn.gaeaId);
		CHECK_EQ(decoded.mapId.value, spawn.mapId.value);
		CHECK_EQ(decoded.position.x, spawn.position.x);
		CHECK_EQ(decoded.position.y, spawn.position.y);
		CHECK_EQ(decoded.position.z, spawn.position.z);
		CHECK_EQ(decoded.accountId, spawn.accountId);
		CHECK_EQ(decoded.characterId, spawn.characterId);
		CHECK_EQ(decoded.characterName, spawn.characterName);
		CHECK_EQ(decoded.characterClass, spawn.characterClass);
		CHECK_EQ(static_cast<int>(decoded.school), static_cast<int>(spawn.school));
		CHECK_EQ(static_cast<int>(decoded.level), static_cast<int>(spawn.level));
		CHECK_EQ(decoded.hp.now, spawn.hp.now);
		CHECK_EQ(decoded.mp.now, spawn.mp.now);
		CHECK_EQ(decoded.sp.now, spawn.sp.now);
		CHECK_EQ(decoded.startMapId.value, spawn.startMapId.value);
		CHECK_EQ(decoded.startGate, spawn.startGate);
	}

	MODERN_TEST(WorldEntry_PhaseA_SpawnReservedRegionsAreActuallyZero)
	{
		SpawnState spawn;
		spawn.userId        = "u";
		spawn.characterName = "c";
		spawn.gaeaId        = 1u;

		std::vector<WireU8> frame;
		CHECK(WorldEntryCodec::AppendSpawn(frame, spawn).IsOk());

		// Quick slots, counts, cosmetics, last-call state and the trailing flags.
		// Skills, quests, quickslots and inventory are all out of scope, so all of
		// these must be zero rather than merely described as reserved.
		CHECK(RanWire::IsRegionZeroed(frame, WorldEntry::kSpawnReservedQuickslotOffset,
		                              WorldEntry::kSpawnReservedQuickslotSize));
		CHECK(RanWire::IsRegionZeroed(frame, WorldEntry::kSpawnReservedSlotArrayOffset,
		                              WorldEntry::kSpawnReservedSlotArraySize));
		CHECK(RanWire::IsRegionZeroed(frame, WorldEntry::kSpawnReservedCountsOffset,
		                              WorldEntry::kSpawnReservedCountsSize));
		CHECK(RanWire::IsRegionZeroed(frame, WorldEntry::kSpawnReservedCosmeticsOffset,
		                              WorldEntry::kSpawnReservedCosmeticsSize));
		CHECK(RanWire::IsRegionZeroed(frame, WorldEntry::kSpawnReservedLastCallOffset,
		                              WorldEntry::kSpawnReservedLastCallSize));
		CHECK(RanWire::IsRegionZeroed(frame, WorldEntry::kSpawnReservedTrailingOffset,
		                              WorldEntry::kSpawnReservedTrailingSize));

		// And the reserved runs inside the 600-byte record.
		const std::size_t record = WorldEntry::kSpawnOffsetData;
		CHECK(RanWire::IsRegionZeroed(frame, record + WorldEntry::kRecordReservedPreIdentityOffset,
		                              WorldEntry::kRecordReservedPreIdentitySize));
		CHECK(RanWire::IsRegionZeroed(frame, record + WorldEntry::kRecordReservedTribeOffset,
		                              WorldEntry::kRecordReservedTribeSize));
		CHECK(RanWire::IsRegionZeroed(frame, record + WorldEntry::kRecordReservedAppearanceOffset,
		                              WorldEntry::kRecordReservedAppearanceSize));
		CHECK(RanWire::IsRegionZeroed(frame, record + WorldEntry::kRecordReservedMidOffset,
		                              WorldEntry::kRecordReservedMidSize));
		CHECK(RanWire::IsRegionZeroed(frame, record + WorldEntry::kRecordReservedPostResourceOffset,
		                              WorldEntry::kRecordReservedPostResourceSize));
		CHECK(RanWire::IsRegionZeroed(frame, record + WorldEntry::kRecordReservedTailOffset,
		                              WorldEntry::kRecordReservedTailSize));

		// Interior padding inside the record, which legacy leaves indeterminate.
		CHECK(RanWire::IsRegionZeroed(frame, record + 113, 3));
		CHECK(RanWire::IsRegionZeroed(frame, record + 146, 6));
	}

	MODERN_TEST(WorldEntry_PhaseA_SpawnRejectsMalformedFrames)
	{
		SpawnState decoded;
		CHECK(WorldEntryCodec::DecodeSpawn({}, decoded).IsError());

		SpawnState spawn;
		spawn.userId        = "u";
		spawn.characterName = "c";

		std::vector<WireU8> frame;
		CHECK(WorldEntryCodec::AppendSpawn(frame, spawn).IsOk());

		// Truncated by one byte.
		{
			std::vector<WireU8> bad = frame;
			bad.resize(1021);
			CHECK(WorldEntryCodec::DecodeSpawn(bad, decoded).IsError());
		}

		// The UNPACKED size, which is the value a careless implementation would use.
		// It must be rejected, not quietly accepted.
		{
			std::vector<WireU8> bad = frame;
			bad.resize(1032);
			CHECK(WorldEntryCodec::DecodeSpawn(bad, decoded).IsError());
		}

		// Wrong id.
		{
			std::vector<WireU8> bad = frame;
			bad[4] = 0x55;
			CHECK(WorldEntryCodec::DecodeSpawn(bad, decoded).IsError());
		}

		// Declared size disagreeing with the bytes present.
		{
			std::vector<WireU8> bad = frame;
			bad[0] = static_cast<WireU8>(1000);
			CHECK(WorldEntryCodec::DecodeSpawn(bad, decoded).IsError());
		}

		// User id field with no terminator.
		{
			std::vector<WireU8> bad = frame;
			for (std::size_t i = 0; i < WorldEntry::kUserIdFieldSize; ++i)
			{
				bad[WorldEntry::kSpawnOffsetUserId + i] = 'Z';
			}
			CHECK(WorldEntryCodec::DecodeSpawn(bad, decoded).IsError());
		}
	}

	MODERN_TEST(WorldEntry_PhaseA_SpawnRefusesOverlongStringsWithoutPartialWrite)
	{
		SpawnState spawn;
		spawn.userId = std::string(WorldEntry::kUserIdFieldSize, 'a');

		std::vector<WireU8> frame;
		CHECK(WorldEntryCodec::AppendSpawn(frame, spawn).IsError());
		CHECK_EQ(frame.size(), static_cast<std::size_t>(0));

		spawn.userId        = std::string(WorldEntry::kUserIdFieldSize - 1, 'a');
		spawn.characterName = std::string(WorldEntry::kNameFieldSize, 'b');
		CHECK(WorldEntryCodec::AppendSpawn(frame, spawn).IsError());
		CHECK_EQ(frame.size(), static_cast<std::size_t>(0));

		spawn.characterName = std::string(WorldEntry::kNameFieldSize - 1, 'b');
		CHECK(WorldEntryCodec::AppendSpawn(frame, spawn).IsOk());
		CHECK_EQ(frame.size(), static_cast<std::size_t>(1022));
	}

	// =========================================================================
	// 2335 - the Agent's refusal
	// =========================================================================

	MODERN_TEST(WorldEntry_PhaseA_JoinFailureRoundTrips)
	{
		CHECK_EQ(WorldEntry::kCharacterJoinFbId, static_cast<MessageId>(2335));

		std::vector<WireU8> frame;
		CHECK(WorldEntryCodec::AppendJoinFailure(frame, 2).IsOk());
		CHECK_EQ(frame.size(), static_cast<std::size_t>(12));

		WireI32 reason = 0;
		CHECK(WorldEntryCodec::DecodeJoinFailure(frame, reason).IsOk());
		CHECK_EQ(static_cast<int>(reason), 2);

		frame.resize(11);
		CHECK(WorldEntryCodec::DecodeJoinFailure(frame, reason).IsError());
		CHECK(WorldEntryCodec::DecodeJoinFailure({}, reason).IsError());
	}

	// =========================================================================
	// Cross-packet invariants
	// =========================================================================

	MODERN_TEST(WorldEntry_PhaseA_NoMessageExceedsTheRanBuffer)
	{
		// Legacy asserts dwSize <= NET_DATA_BUFSIZE in the spawn packet's own
		// constructor, so a modern packet larger than that would be one legacy's own
		// code refuses to build.
		CHECK(CharacterList::kCharacterDetailSize <= Protocol::kDataBufferSize);
		CHECK(WorldEntry::kSpawnSize <= Protocol::kDataBufferSize);
		CHECK(WorldEntry::kRedirectSize <= Protocol::kDataBufferSize);
		CHECK(CharacterList::kListReleasedSize <= Protocol::kDataBufferSize);
	}

	MODERN_TEST(WorldEntry_PhaseA_PredicatesAreDistinct)
	{
		// A predicate that matched the wrong id would let a session route a spawn to
		// the character-list decoder, so the five sets are checked to be disjoint.
		const MessageId ids[] = {
			WorldEntry::kGameJoinId,      WorldEntry::kConnectFieldId,
			WorldEntry::kJoinFieldId,     WorldEntry::kCharacterJoinId,
			WorldEntry::kCharacterJoinFbId,
			CharacterList::kRequestAllId, CharacterList::kAllInfoId,
			CharacterList::kRequestOneId, CharacterList::kCharacterDetailId,
		};

		for (std::size_t i = 0; i < 9; ++i)
		{
			for (std::size_t j = i + 1; j < 9; ++j)
			{
				CHECK_NE(ids[i], ids[j]);
			}
		}

		CHECK(WorldEntryCodec::IsSpawn(WorldEntry::kCharacterJoinId));
		CHECK(!WorldEntryCodec::IsSpawn(WorldEntry::kGameJoinId));
		CHECK(CharacterListCodec::IsCharacterDetail(CharacterList::kCharacterDetailId));
		CHECK(!CharacterListCodec::IsCharacterDetail(WorldEntry::kCharacterJoinId));

		// 2332 and 2333 are one apart on the wire and must not be confused.
		CHECK_EQ(WorldEntry::kCharacterJoinId - CharacterList::kCharacterDetailId,
		         static_cast<MessageId>(1));
	}
}
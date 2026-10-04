#pragma once

// WORLD-ENTRY-001 Phase A: the character-list and character-detail packets.
//
//   NET_MSG_REQ_CHA_BAINFO     2247  client -> Agent   8 bytes, bare header
//   NET_MSG_CHA_BAINFO         2248  Agent  -> client  28 or 76 bytes  (see below)
//   NET_MSG_REQ_CHA_BINFO      2244  client -> Agent   12 bytes
//   NET_MSG_LOBBY_CHAR_SEL     2332  Agent  -> client  1176 bytes
//
// Proven from legacy/Lib_Network/s_NetGlobal.h and
// legacy/Lib_Network/s_CAgentServerMsg.cpp. All four are NET_MSG_LOBBY + n, with
// NET_MSG_LOBBY = NET_MSG_BASE + 950 = 1942 and NET_MSG_BASE = 992 in every one of
// the fourteen country branches (s_NetGlobal.h:644-672), so no variant had to be
// chosen and none can change these ids.
//
// ---------------------------------------------------------------------------
// THE EXCHANGE HAS NO TERMINATOR, AND THAT IS THE POINT
// ---------------------------------------------------------------------------
//
// The Agent answers 2247 with ONE 2248 carrying a count and an array of character
// ids. The client then asks for each character in turn (2244) and receives one 2332
// per id. Nothing marks the end.
//
// Legacy detects completion by COUNTING (DxLobyStage.h:150,
// `m_nStartCharNum == m_nStartCharLoad`), and there is no end-of-list message
// anywhere in the tree. This is the opposite of LOGIN-001, whose game-server list
// ends with an explicit SND_GAME_SVR_END, so a decoder written by analogy with that
// one would wait forever. Phase A therefore exposes a count and a per-character
// record and NO terminator, and the later phases are the ones that must decide how
// many records to expect.
//
// ---------------------------------------------------------------------------
// 2248 HAS TWO SIZES, AND THE DECODER MUST NOT PICK ONE
// ---------------------------------------------------------------------------
//
//     struct NET_CHA_BBA_INFO {          s_NetGlobal.h:3982-3994
//         NET_MSG_GENERIC nmg;
//         int              nChaSNum;
//         int              nChaNum[MAX_ONESERVERCHAR_NUM];
//     };
//
// MAX_ONESERVERCHAR_NUM (s_NetGlobal.h:195-199) is 16 when any of KRT_PARAM,
// _RELEASED, KR_PARAM, TW_PARAM, HK_PARAM, TH_PARAM, MYE_PARAM, MY_PARAM,
// CH_PARAM, PH_PARAM or JP_PARAM is defined, and 4 otherwise. So the packet is
//
//     8 + 4 + 4 * 16 = 76 bytes    released Korean server
//     8 + 4 + 4 *  4 = 28 bytes    everything else, including this checkout
//
// Both are real. This tree's buildable .vcxproj files define no country macro at
// all - only dead VS2003 .vcproj files ever defined KR_PARAM - so a build of THIS
// code emits 28, while a real Korean release server emits 76. A decoder that
// assumed 28 would read a 76-byte packet as 28 and desynchronise on the first
// character.
//
// So DecodeIdList derives the array length from the packet's own dwSize, accepts
// either width, and rejects anything else.
//
// THE WIDTH IS FIXED; ONLY THE COUNT VARIES
// -----------------------------------------
//
// This is the detail most easily got wrong, and it is worth stating because the
// struct makes it look otherwise. NET_CHA_BBA_INFO's constructor
// (s_NetGlobal.h:3987-3992) does
//
//     memset(this, 0, sizeof(NET_CHA_BBA_INFO));
//     nmg.dwSize = sizeof(NET_CHA_BBA_INFO);
//     nChaSNum   = 0;
//
// and nChaNum[] is a fixed-size array MEMBER, not a trailing array. So dwSize is
// always 28 or 76 and never depends on nChaSNum. An account with two characters
// sends 28 bytes carrying two ids followed by two zero slots.
//
// Therefore:
//   - AppendIdList always emits this build's full width (28) and zero-fills the
//     slots it is not using. A variable-width packet would be a shape this codebase
//     cannot itself parse.
//   - DecodeIdList accepts `nChaSNum <= capacity`, NOT `== capacity`. Requiring
//     equality would reject the most common packet RAN sends: a real account with
//     fewer than four characters.
//   - DecodeIdList reports only the `nChaSNum` slots. The trailing zeros are
//     padding, and reporting them would invent characters that do not exist.
//
// ---------------------------------------------------------------------------
// 2332 IS 1176 BYTES, NOT 1088
// ---------------------------------------------------------------------------
//
// NET_MSG_LOBBY_CHAR_SEL is GLMSG::SNETLOBBY_CHARINFO (GLContrlCharJoinMsg.h:264):
// an 8-byte header plus SCHARINFO_LOBBY, which is 1168. That total is measured,
// not summed, and the distinction matters: summing SCHARINFO_LOBBY's members gives
// 1080, because SITEM_LOBY's members sum to 44 while its size is 48 (LONGLONG forces
// 8-byte alignment). Across 22 equipment slots that is 88 bytes of error in a
// packet sent once per character.
//
// WHY THE EQUIPMENT ARRAY IS PRESENT BUT EMPTY
// ---------------------------------------------
//
// m_PutOnItems[22] is 1056 of those 1168 bytes, and equipment is explicitly out of
// WORLD-ENTRY-001 scope. It is still declared, because the packet is fixed-size and
// a 1088-byte packet is a malformed one. The region is zero-filled and documented
// as reserved. Nothing in this file interprets a single equipment field; see
// RanWirePrimitives.h for the same statement about ItemLobbyWire.
//
// FIELDS WORLD-ENTRY-001 ACTUALLY USES: character id, name, class, school, level,
// HP, and the save map. Everything else is reserved.

#include "NetworkTypes.h"
#include "RanWirePrimitives.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Modern::Network
{
	namespace CharacterList
	{
		// ---- message ids ----------------------------------------------------
		// s_NetGlobal.h:839, :843, :842, :845.
		constexpr MessageId kRequestAllId        = Protocol::kLobbyBase + 305; // 2247
		constexpr MessageId kAllInfoId           = Protocol::kLobbyBase + 306; // 2248
		constexpr MessageId kRequestOneId        = Protocol::kLobbyBase + 302; // 2244
		constexpr MessageId kCharacterDetailId   = Protocol::kLobbyBase + 390; // 2332

		static_assert(kRequestAllId == 2247, "REQ_CHA_BAINFO is 2247");
		static_assert(kAllInfoId == 2248, "CHA_BAINFO is 2248");
		static_assert(kRequestOneId == 2244, "REQ_CHA_BINFO is 2244");
		static_assert(kCharacterDetailId == 2332, "LOBBY_CHAR_SEL is 2332");

		// ---- sizes ----------------------------------------------------------
		constexpr std::size_t kBareMessageSize = 8;   // NET_MSG_GENERIC
		constexpr std::size_t kRequestOneSize  = 12;  // header + INT nChaNum

		// 2248: header + count + id array. Two widths, both real; see the note above.
		constexpr std::size_t kListHeaderSize     = 12; // 8 header + 4 count
		constexpr std::size_t kListSmallSlots     = 4;  // MAX_ONESERVERCHAR_NUM, default
		constexpr std::size_t kListReleasedSlots  = 16; // MAX_ONESERVERCHAR_NUM, released
		constexpr std::size_t kListSmallSize      = kListHeaderSize + kListSmallSlots * 4;    // 28
		constexpr std::size_t kListReleasedSize   = kListHeaderSize + kListReleasedSlots * 4; // 76

		// What THIS build emits, as opposed to what it accepts.
		//
		// No country macro is defined in any buildable project in this tree, so
		// MAX_ONESERVERCHAR_NUM resolves to 4 and a modern server emits 28 bytes.
		// AppendIdList is pinned to that; DecodeIdList accepts both widths, because
		// interop with a released 76-byte server is a requirement and emitting 76
		// ourselves is not.
		//
		// Kept separate from kListSmallSlots so the coupling is explicit: if a country
		// macro is ever added to a modern project, kLocalSlots is the one line to
		// change, and the build then refuses to pretend it can still send four.
		constexpr std::size_t kLocalSlots = kListSmallSlots;
		constexpr std::size_t kLocalSize  = kListSmallSize;
		static_assert(kLocalSize == 28,
		              "this build defines no country macro, so it emits 28 bytes");

		// The widest 2248 is still a fraction of NET_DATA_BUFSIZE (2048), which legacy
		// asserts on in the packet's own constructor.
		static_assert(kListReleasedSize <= Protocol::kDataBufferSize,
		              "the widest character-id list must fit the RAN buffer");
		static_assert(kListSmallSize == 28, "the narrow list is 28 bytes");
		static_assert(kListReleasedSize == 76, "the released list is 76 bytes");

		// ---- 2332 payload ----------------------------------------------------
		constexpr std::size_t kCharacterDetailSize = 1176; // header + SCHARINFO_LOBBY
		constexpr std::size_t kCharacterRecordSize = 1168; // SCHARINFO_LOBBY alone

		constexpr std::size_t kSlotCount          = 22;  // SLOT_TSIZE, GLItemDef.h:242
		constexpr std::size_t kItemSlotSize       = 48;  // SITEM_LOBY, measured
		constexpr std::size_t kEquipmentSize      = kSlotCount * kItemSlotSize; // 1056
		constexpr std::size_t kNameFieldSize      = 33;  // CHAR_SZNAME = CHR_ID_LENGTH

		static_assert(kEquipmentSize == 1056,
		              "22 slots x 48 bytes; the old 44-byte assumption gives 968");
		static_assert(kCharacterRecordSize == 1168, "SCHARINFO_LOBBY is 1168 bytes");
		static_assert(kCharacterDetailSize == 1176, "LOBBY_CHAR_SEL is 1176 bytes");

		// ---- 2332 field offsets, all measured --------------------------------
		//
		// SCHARINFO_LOBBY is declared OUTSIDE any #pragma pack (GLContrlBaseMsg.h:229,
		// with pack(1) starting at line 272), so it is default-aligned. The explicit
		// padding arrays below exist to reproduce that alignment exactly; if a pad were
		// wrong, the offsetof static_asserts in the wire struct would fail.
		constexpr std::size_t kDetailOffsetCharacterId   = 0;
		constexpr std::size_t kDetailOffsetName          = 4;
		constexpr std::size_t kDetailOffsetCharacterClass = 40;
		constexpr std::size_t kDetailOffsetSchool        = 44;
		constexpr std::size_t kDetailOffsetHp            = 56;
		constexpr std::size_t kDetailOffsetLevel         = 84;
		constexpr std::size_t kDetailOffsetEquipment     = 104;
		constexpr std::size_t kDetailOffsetSaveMapId     = 1160;

		// Reserved runs inside SCHARINFO_LOBBY, contiguous and gap-free.
		constexpr std::size_t kDetailReservedAppearanceOffset = 46;
		constexpr std::size_t kDetailReservedAppearanceSize    = 8;   // hair, face, sex, hairColor
		constexpr std::size_t kDetailReservedExperienceOffset = 64;
		constexpr std::size_t kDetailReservedExperienceSize    = 16;  // GLLLDATA
		constexpr std::size_t kDetailReservedBrightOffset      = 80;
		constexpr std::size_t kDetailReservedBrightSize       = 4;
		constexpr std::size_t kDetailReservedStatsOffset       = 86;
		constexpr std::size_t kDetailReservedStatsSize        = 12;  // SCHARSTATS
		constexpr std::size_t kDetailReservedScaleOffset      = 1164;
		constexpr std::size_t kDetailReservedScaleSize       = 4;

		// ---- 2248 field offsets ----------------------------------------------
		constexpr std::size_t kListOffsetCount = 8;
		constexpr std::size_t kListOffsetFirstId = 12;
	}

	// ---------------------------------------------------------------------------
	// Wire structures
	// ---------------------------------------------------------------------------

	// SCHARINFO_LOBBY - legacy GLContrlBaseMsg.h:229, default (unpacked) alignment.
	//
	// Declared so that `sizeof` and `offsetof` can be asserted by the compiler,
	// which is the mechanism that stops a packing or alignment change from silently
	// altering the wire. The codec reads and writes fields individually and
	// little-endian, in keeping with the rest of ModernNetwork: no native struct is
	// ever memcpy'd onto the wire, so the bytes do not depend on this layout - the
	// layout exists to be CHECKED, and the assertions are what make it load-bearing.
	struct CharacterDetailWire
	{
		WireU32 characterId;                            //   0
		char    name[CharacterList::kNameFieldSize];     //   4  33 bytes
		WireU8  padToClass[3];                           //  37  align 4 for the enum
		WireU32 characterClass;                         //  40  EMCHARCLASS, raw on the wire
		WireU16 school;                                  //  44
		WireU8  reservedAppearance[CharacterList::kDetailReservedAppearanceSize]; // 46
		WireU8  padToHp[2];                              //  54  align 4 for GLDWDATA
		RanWire::DwPair hp;                              //  56
		WireU8  reservedExperience[CharacterList::kDetailReservedExperienceSize];   //  64
		WireU32 reservedBright;                          //  80
		WireU16 level;                                   //  84
		WireU8  reservedStats[CharacterList::kDetailReservedStatsSize];            //  86
		WireU8  padToEquipment[6];                       //  98  align 8 for the slot array
		RanWire::ItemLobbyWire equipment[CharacterList::kSlotCount];               // 104
		RanWire::NativeId saveMapId;                     // 1160
		WireU8  reservedScaleRange[CharacterList::kDetailReservedScaleSize];      // 1164
	};

	static_assert(sizeof(CharacterDetailWire) == CharacterList::kCharacterRecordSize,
	              "SCHARINFO_LOBBY must stay 1168 bytes");
	static_assert(offsetof(CharacterDetailWire, characterId) ==
	                  CharacterList::kDetailOffsetCharacterId, "charId at 0");
	static_assert(offsetof(CharacterDetailWire, name) ==
	                  CharacterList::kDetailOffsetName, "name at 4");
	static_assert(offsetof(CharacterDetailWire, characterClass) ==
	                  CharacterList::kDetailOffsetCharacterClass, "class at 40");
	static_assert(offsetof(CharacterDetailWire, school) ==
	                  CharacterList::kDetailOffsetSchool, "school at 44");
	static_assert(offsetof(CharacterDetailWire, hp) ==
	                  CharacterList::kDetailOffsetHp, "hp at 56");
	static_assert(offsetof(CharacterDetailWire, level) ==
	                  CharacterList::kDetailOffsetLevel, "level at 84");
	static_assert(offsetof(CharacterDetailWire, equipment) ==
	                  CharacterList::kDetailOffsetEquipment, "equipment array at 104");
	static_assert(sizeof(CharacterDetailWire::equipment) ==
	                  CharacterList::kEquipmentSize, "the equipment array is 1056 bytes");
	static_assert(offsetof(CharacterDetailWire, saveMapId) ==
	                  CharacterList::kDetailOffsetSaveMapId, "saveMapId at 1160");

	// GLMSG::SNETLOBBY_CHARINFO - GLContrlCharJoinMsg.h:264, declared inside
	// `#pragma pack(1)`. The outer struct is packed even though the payload is not,
	// which removes the padding that would otherwise follow the 8-byte header.
#pragma pack(push, 1)
	struct CharacterDetailFrameWire
	{
		MessageHeader              header;
		CharacterDetailWire        data;
	};
#pragma pack(pop)

	static_assert(sizeof(CharacterDetailFrameWire) == CharacterList::kCharacterDetailSize,
	              "LOBBY_CHAR_SEL must stay 1176 bytes");
	static_assert(offsetof(CharacterDetailFrameWire, data) == 8,
	              "the payload starts immediately after the 8-byte header");

	// ---------------------------------------------------------------------------
	// Application model
	// ---------------------------------------------------------------------------

	// The authoritative subset of one character, as 2332 carries it.
	//
	// Deliberately NOT the whole 1168 bytes. Equipment, experience, stats, PK
	// brightness and scale are wire regions this milestone preserves and zeroes;
	// modelling them as fields would imply a subsystem that does not exist here.
	struct CharacterDetail
	{
		WireU32 characterId   = 0;
		std::string name;                   // at most kNameFieldSize-1 characters
		WireU32 characterClass = 0;          // EMCHARCLASS, carried raw
		WireU16 school         = 0;
		WireU16 level          = 0;
		RanWire::DwPair hp;                  // legacy fills `now` and leaves `max` at 0
		RanWire::NativeId saveMapId;

		bool operator==(const CharacterDetail& other) const noexcept
		{
			return characterId == other.characterId && name == other.name &&
			       characterClass == other.characterClass && school == other.school &&
			       level == other.level && hp.now == other.hp.now &&
			       hp.max == other.hp.max && saveMapId.value == other.saveMapId.value;
		}
	};

	// 2248's payload: how many characters the account has on this server, and which.
	//
	// A vector rather than a fixed array of 4, because the packet itself is not
	// fixed - it is 28 or 76 bytes and the width comes off the wire. A fixed array
	// would reinstate exactly the assumption the decoder is required not to make.
	struct CharacterIdList
	{
		std::vector<WireU32> ids;
	};

	namespace CharacterListCodec
	{
		// 2247. APPENDED to `out`; a bare 8-byte header with no body.
		//
		// Legacy's struct carries an `int nChannel` that is commented out
		// (s_NetGlobal.h:3970), so the packet is exactly the header. Channel choice
		// moved to the game-server-list step in LOGIN-001, and adding a body here
		// would be a packet no RAN server expects.
		Status AppendRequestAll(std::vector<WireU8>& out);

		// 2248. APPENDED. `ids.size()` may be 0..kListReleasedSlots.
		//
		// The emitted size is 12 + 4*count, which is 28 for at most 4 ids and 76 for
		// more than 4. That reproduces both legacy widths from one rule instead of
		// asking the caller to choose, because a caller that passed 16 ids against a
		// 4-slot build would otherwise produce a packet that build cannot parse.
		Status AppendIdList(std::vector<WireU8>& out, const std::vector<WireU32>& ids);

		// 2248. Derives the array length from the frame's own dwSize.
		//
		// Rejects a dwSize that is neither 28 nor 76, a count that disagrees with
		// that width, a negative count, and a frame shorter than its declared size.
		// The count check matters: without it a peer could declare 76 bytes and a
		// count of 1, and the decoder would report a list of one while silently
		// discarding fifteen ids.
		Status DecodeIdList(const std::vector<WireU8>& frame, CharacterIdList& out);

		// 2244. APPENDED. `characterId` is the id from the 2248 array.
		Status AppendRequestOne(std::vector<WireU8>& out, WireU32 characterId);

		// 2332. APPENDED, always 1176 bytes with the reserved regions zeroed.
		//
		// Refuses a name that will not fit with room for its terminator, and rolls
		// back rather than leaving a partial frame behind.
		Status AppendCharacterDetail(std::vector<WireU8>& out, const CharacterDetail& detail);

		// 2332. Requires a frame of exactly 1176 bytes with id 2332.
		//
		// Validates dwSize against the frame's actual length and against the only
		// legal width, so a truncated or oversized packet is refused rather than
		// decoded from whatever happens to be in the buffer.
		Status DecodeCharacterDetail(const std::vector<WireU8>& frame, CharacterDetail& out);

		// ---- predicates ------------------------------------------------------
		bool IsRequestAll(MessageId id) noexcept;
		bool IsIdList(MessageId id) noexcept;
		bool IsRequestOne(MessageId id) noexcept;
		bool IsCharacterDetail(MessageId id) noexcept;

		// Validates a 2244 frame: right id, and exactly 12 bytes.
		Status ValidateRequestOne(const std::vector<WireU8>& frame, WireU32& characterId);

		// Reads 2244's character id out of an already-validated frame.
		//
		// Separate from ValidateRequestOne because the server handler needs the value
		// AND the caller may want the validation on its own.
		Status ReadRequestOneId(const std::vector<WireU8>& frame, WireU32& characterId);

		// The number of 2332 records a client should expect, from a 2248 payload.
		//
		// This is the replacement for LOGIN-001's terminator. Legacy completes the
		// list by counting (DxLobyStage.h:150); exposing the expectation explicitly
		// is what lets a later phase implement that, and it is why no end-of-list
		// message appears anywhere in this file.
		std::size_t ExpectedDetailCount(const CharacterIdList& list) noexcept;
	}
}
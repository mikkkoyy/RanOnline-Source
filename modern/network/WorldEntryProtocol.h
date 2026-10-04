#pragma once

// WORLD-ENTRY-001 Phase A: selection, field redirect, field identity and spawn.
//
//   NET_MSG_LOBBY_GAME_JOIN        2353  client -> Agent  12 bytes
//   NET_MSG_CONNECT_CLIENT_FIELD  2358  Agent  -> client 48 bytes
//   NET_MSG_JOIN_FIELD_IDENTITY   2359  client -> Field  24 bytes
//   NET_MSG_LOBBY_CHAR_JOIN       2333  Field  -> client 1022 bytes
//   NET_MSG_LOBBY_CHAR_JOIN_FB    2335  Agent  -> client 12 bytes
//   NET_MSG_LOBBY_GAME_COMPLETE   2354                     8 bytes, client-synthesised
//
// All ids are NET_MSG_LOBBY + n (s_NetGlobal.h:846, :864, :870, :871, :870, :865)
// with NET_MSG_LOBBY = 1942 in every country configuration.
//
// ---------------------------------------------------------------------------
// WHICH SERVER SENDS WHAT
// ---------------------------------------------------------------------------
//
// Proven rather than assumed. The selection (2353) goes to the AGENT
// (s_CAgentServerMsg.cpp:100 dispatches it to CAgentServer::MsgGameJoin), which
// then redirects the client to a FIELD server with 2358, which answers 2333.
//
// Legacy performs the hop between them with two server-to-server messages
// (MET_MSG_GAME_JOIN_FIELDSVR 2356 and its reply 2357) that the client never sees.
// Phase A does not model those; a later phase owns the Agent/Field services. What
// matters here is that 2358 and 2359 are genuine client-visible packets carrying
// real information, not placeholders - the client must dial a second connection
// using the address 2358 gives it and identify itself with 2359.
//
// ---------------------------------------------------------------------------
// 2333 IS 1022 BYTES PACKED, NOT 1032
// ---------------------------------------------------------------------------
//
// GLMSG::SNETLOBBY_CHARJOIN (GLContrlCharJoinMsg.h:279) is declared inside
// `#pragma pack(1)`, and packing removes 10 bytes of padding relative to the same
// struct at default alignment. Reproducing the packing context is therefore part of
// getting the size right; measuring the unpacked declaration gives 1032, which is
// the wrong wire size.
//
// The payload it carries is SCHARDATA (600 bytes) and the outer fields include the
// authoritative spawn state: gaeaId (the entity id every later gameplay message
// refers to), the map, and the position. RAN sends NO rotation or angle anywhere
// in this protocol, which is why there is no heading field here.
//
// ---------------------------------------------------------------------------
// WHAT IS POPULATED AND WHAT IS RESERVED
// ---------------------------------------------------------------------------
//
// Populated: userId, clientId, gaeaId, mapId, position, startMapId, startGate, and
// inside the 600-byte character record: accountId, characterId, name, class,
// school, level, HP, MP, SP.
//
// Reserved and zero-filled: the 288 bytes of skill and action quickslots, the 44
// bytes of inventory/skill/quest/activity/codex counts, both big-head/big-hand
// flags, last-call map and position, the tracing flag, and the two cafe-class
// fields - together with most of the 600-byte character record. Equipment,
// inventory, skills, quickslots, quests and guilds are explicitly out of
// WORLD-ENTRY-001 scope, and nothing here interprets a byte of them.
//
// ---------------------------------------------------------------------------
// 2359's CRYPT KEY PROTECTS NOTHING
// ---------------------------------------------------------------------------
//
// Reproduced because it occupies 4 bytes of a fixed-size packet, not because it
// does anything. See RanWirePrimitives.h for the evidence; in short the server
// hardcodes {1,1}, the client discards whatever it is sent, and the cipher that
// would consume it is commented out at every call site.
//
// ---------------------------------------------------------------------------
// THERE IS NO WORLD-ENTRY ACKNOWLEDGEMENT
// ---------------------------------------------------------------------------
//
// NET_MSG_GAME_JOIN_OK (2355) exists in the enum but both of its send sites are
// commented out (s_CFieldServerMsg.cpp:433-447, s_CAgentServerMsg.cpp:900-912),
// and it was Field-to-Session, never Field-to-client. NET_MSG_LOBBY_GAME_COMPLETE
// (2354) is synthesised by the CLIENT once it has the whole spawn burst
// (DxGameStage.cpp:581). Neither is a server response to world entry, so neither
// is modelled here as one. The id is recorded only so that a later phase can
// recognise it if it appears.

#include "NetworkTypes.h"
#include "RanWirePrimitives.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Modern::Network
{
	namespace WorldEntry
	{
		// ---- message ids ----------------------------------------------------
		constexpr MessageId kGameJoinId        = Protocol::kLobbyBase + 411; // 2353
		constexpr MessageId kConnectFieldId    = Protocol::kLobbyBase + 416; // 2358
		constexpr MessageId kJoinFieldId       = Protocol::kLobbyBase + 417; // 2359
		constexpr MessageId kCharacterJoinId   = Protocol::kLobbyBase + 391; // 2333
		constexpr MessageId kCharacterJoinFbId = Protocol::kLobbyBase + 393; // 2335
		constexpr MessageId kGameCompleteId    = Protocol::kLobbyBase + 412; // 2354

		static_assert(kGameJoinId == 2353, "LOBBY_GAME_JOIN is 2353");
		static_assert(kConnectFieldId == 2358, "CONNECT_CLIENT_FIELD is 2358");
		static_assert(kJoinFieldId == 2359, "JOIN_FIELD_IDENTITY is 2359");
		static_assert(kCharacterJoinId == 2333, "LOBBY_CHAR_JOIN is 2333");
		static_assert(kCharacterJoinFbId == 2335, "LOBBY_CHAR_JOIN_FB is 2335");
		static_assert(kGameCompleteId == 2354, "LOBBY_GAME_COMPLETE is 2354");

		// ---- sizes ----------------------------------------------------------
		constexpr std::size_t kBareMessageSize    = 8;
		constexpr std::size_t kGameJoinSize       = 12;  // header + INT nChaNum
		constexpr std::size_t kRedirectSize       = 48;
		constexpr std::size_t kIdentitySize       = 24;
		constexpr std::size_t kSpawnSize          = 1022;
		constexpr std::size_t kJoinFailureSize    = 12;  // header + EMCHAR_JOIN_FB

		static_assert(kGameJoinSize == 12, "NET_GAME_JOIN is 12 bytes");
		static_assert(kRedirectSize == 48, "NET_CONNECT_CLIENT_TO_FIELD is 48 bytes");
		static_assert(kIdentitySize == 24, "NET_GAME_JOIN_FIELD_IDENTITY is 24 bytes");
		static_assert(kSpawnSize == 1022, "SNETLOBBY_CHARJOIN is 1022 packed bytes");
		static_assert(kJoinFailureSize == 12, "SNETLOBBY_CHARJOIN_FB is 12 bytes");

		// The spawn packet carries the character record plus quick slots and counts,
		// and must still fit RAN's own NET_DATA_BUFSIZE assertion.
		static_assert(kSpawnSize <= Protocol::kDataBufferSize,
		              "the spawn packet must fit the RAN buffer");

		// ---- EMGAME_JOINTYPE - s_NetGlobal.h:4156-4161 -----------------------
		//
		// A plain 4-byte enum. The values are read from the declaration rather than
		// invented, and are carried opaquely by the codec; which one a given
		// exchange uses is a later phase's business.
		constexpr WireI32 kJoinTypeFirst  = 0;
		constexpr WireI32 kJoinTypeMoveMap = 1;
		constexpr WireI32 kJoinTypeRebirth = 2;

		// ---- 2358 field offsets (default alignment) --------------------------
		constexpr std::size_t kRedirectOffsetJoinType = 8;
		constexpr std::size_t kRedirectOffsetGaeaId   = 12;
		constexpr std::size_t kRedirectOffsetSlot     = 16;
		constexpr std::size_t kRedirectOffsetPort     = 20;
		constexpr std::size_t kRedirectOffsetIp       = 24;
		constexpr std::size_t kAddressFieldSize       = 21; // MAX_IP_LENGTH + 1

		// ---- 2359 field offsets (default alignment) --------------------------
		constexpr std::size_t kIdentityOffsetJoinType = 8;
		constexpr std::size_t kIdentityOffsetGaeaId   = 12;
		constexpr std::size_t kIdentityOffsetSlot     = 16;
		constexpr std::size_t kIdentityOffsetKey      = 20; // CRYPT_KEY, 4 bytes

		// ---- 2333 field offsets (PACK(1) - no padding anywhere) ---------------
		constexpr std::size_t kSpawnOffsetUserId      = 8;
		constexpr std::size_t kSpawnOffsetClientId    = 29;
		constexpr std::size_t kSpawnOffsetGaeaId      = 33;
		constexpr std::size_t kSpawnOffsetMapId       = 37;
		constexpr std::size_t kSpawnOffsetPosition    = 41;
		constexpr std::size_t kSpawnOffsetData        = 53;
		constexpr std::size_t kSpawnOffsetStartMapId  = 989;
		constexpr std::size_t kSpawnOffsetStartGate   = 993;
		constexpr std::size_t kUserIdFieldSize        = 21; // USR_ID_LENGTH + 1

		// ---- SCHARDATA (600 bytes, default alignment) -------------------------
		// Measured offsets for the fields WORLD-ENTRY-001 populates. Everything
		// else in the record is reserved.
		constexpr std::size_t kRecordSize                = 600;
		constexpr std::size_t kRecordOffsetAccountId     = 0;
		constexpr std::size_t kRecordOffsetCharacterId   = 76;
		constexpr std::size_t kRecordOffsetName          = 80;
		constexpr std::size_t kRecordOffsetCharacterClass = 120;
		constexpr std::size_t kRecordOffsetSchool        = 124;
		constexpr std::size_t kRecordOffsetLevel         = 144;
		constexpr std::size_t kRecordOffsetHp            = 304;
		constexpr std::size_t kRecordOffsetMp            = 312;
		constexpr std::size_t kRecordOffsetSp            = 320;
		constexpr std::size_t kNameFieldSize             = 33; // CHAR_SZ_NAME

		// Reserved runs inside the record. Named so that a test can assert them zero
		// and a reader can see exactly how much of the 600 bytes is not populated.
		constexpr std::size_t kRecordReservedPreIdentityOffset  = 4;
		constexpr std::size_t kRecordReservedPreIdentitySize    = 72;
		constexpr std::size_t kRecordReservedTribeOffset        = 116;
		constexpr std::size_t kRecordReservedTribeSize          = 4;
		constexpr std::size_t kRecordReservedAppearanceOffset   = 126;
		constexpr std::size_t kRecordReservedAppearanceSize     = 18;
		constexpr std::size_t kRecordReservedMidOffset          = 152;
		constexpr std::size_t kRecordReservedMidSize            = 152;
		constexpr std::size_t kRecordReservedPostResourceOffset = 328;
		constexpr std::size_t kRecordReservedPostResourceSize   = 264;
		constexpr std::size_t kRecordReservedTailOffset        = 592;
		constexpr std::size_t kRecordReservedTailSize          = 8;

		// ---- 2333 reserved runs outside the record ---------------------------
		constexpr std::size_t kSpawnReservedQuickslotOffset = 653;
		constexpr std::size_t kSpawnReservedQuickslotSize   = 2;   // wSKILLQUICK_ACT
		constexpr std::size_t kSpawnReservedSlotArrayOffset = 655;
		constexpr std::size_t kSpawnReservedSlotArraySize   = 288; // 60 skill + 6 action
		constexpr std::size_t kSpawnReservedCountsOffset    = 943;
		constexpr std::size_t kSpawnReservedCountsSize      = 44;  // 11 DWORD counts
		constexpr std::size_t kSpawnReservedCosmeticsOffset = 987;
		constexpr std::size_t kSpawnReservedCosmeticsSize   = 2;   // bBIGHEAD, bBIGHAND
		constexpr std::size_t kSpawnReservedLastCallOffset  = 997;
		constexpr std::size_t kSpawnReservedLastCallSize    = 16;  // map id + position
		constexpr std::size_t kSpawnReservedTrailingOffset  = 1013;
		constexpr std::size_t kSpawnReservedTrailingSize    = 9;   // tracing + 2 cafe
	}

	// ---------------------------------------------------------------------------
	// Wire structures
	// ---------------------------------------------------------------------------

	// The 600-byte character record carried inside the spawn packet.
	//
	// Same construction as CharacterDetailWire and for the same reason: the layout
	// must be assertable so that a packing or alignment change breaks the build
	// instead of silently changing the wire. The codec does not memcpy this struct;
	// it reads and writes the individual fields below.
	struct SpawnCharacterRecordWire
	{
		WireU32 accountId;                              //   0
		WireU8  reservedPreIdentity[WorldEntry::kRecordReservedPreIdentitySize]; // 4
		WireU32 characterId;                            //  76
		char    name[WorldEntry::kNameFieldSize];       //  80
		WireU8  padToTribe[3];                          // 113
		WireU32 reservedTribe;                          // 116  EMTRIBE, reserved
		WireU32 characterClass;                         // 120
		WireU16 school;                                  // 124
		WireU8  reservedAppearance[WorldEntry::kRecordReservedAppearanceSize];     // 126
		WireU16 level;                                   // 144
		WireU8  padToMoney[6];                           // 146
		WireU8  reservedMid[WorldEntry::kRecordReservedMidSize];                  // 152
		RanWire::DwPair hp;                              // 304
		RanWire::DwPair mp;                              // 312
		RanWire::DwPair sp;                              // 320
		WireU8  reservedPostResource[WorldEntry::kRecordReservedPostResourceSize]; // 328
		WireU8  reservedTail[WorldEntry::kRecordReservedTailSize];                // 592
	};

	static_assert(sizeof(SpawnCharacterRecordWire) == WorldEntry::kRecordSize,
	              "SCHARDATA must stay 600 bytes");
	static_assert(offsetof(SpawnCharacterRecordWire, accountId) ==
	                  WorldEntry::kRecordOffsetAccountId, "accountId at 0");
	static_assert(offsetof(SpawnCharacterRecordWire, characterId) ==
	                  WorldEntry::kRecordOffsetCharacterId, "characterId at 76");
	static_assert(offsetof(SpawnCharacterRecordWire, name) ==
	                  WorldEntry::kRecordOffsetName, "name at 80");
	static_assert(offsetof(SpawnCharacterRecordWire, characterClass) ==
	                  WorldEntry::kRecordOffsetCharacterClass, "class at 120");
	static_assert(offsetof(SpawnCharacterRecordWire, school) ==
	                  WorldEntry::kRecordOffsetSchool, "school at 124");
	static_assert(offsetof(SpawnCharacterRecordWire, level) ==
	                  WorldEntry::kRecordOffsetLevel, "level at 144");
	static_assert(offsetof(SpawnCharacterRecordWire, hp) ==
	                  WorldEntry::kRecordOffsetHp, "hp at 304");
	static_assert(offsetof(SpawnCharacterRecordWire, mp) ==
	                  WorldEntry::kRecordOffsetMp, "mp at 312");
	static_assert(offsetof(SpawnCharacterRecordWire, sp) ==
	                  WorldEntry::kRecordOffsetSp, "sp at 320");

	// GLMSG::SNETLOBBY_CHARJOIN - GLContrlCharJoinMsg.h:279, inside `#pragma pack(1)`.
	//
	// The packing is the whole point of this struct. SNETLOBBY_CHARJOIN sits inside
	// pack(1) while SCHARDATA does not, so MSVC places the member with align 1
	// while the member's own internal layout keeps its natural alignment - which is
	// why the record is still 600 bytes and not 592.
#pragma pack(push, 1)
	struct SpawnWire
	{
		MessageHeader              header;
		char                       userId[WorldEntry::kUserIdFieldSize]; //   8
		WireU32                    clientId;                            //  29
		WireU32                    gaeaId;                              //  33
		RanWire::NativeId          mapId;                               //  37
		RanWire::Vector3           position;                            //  41
		SpawnCharacterRecordWire   data;                                //  53
		WireU8                     reservedQuickslot[WorldEntry::kSpawnReservedQuickslotSize]; // 653
		WireU8                     reservedSlots[WorldEntry::kSpawnReservedSlotArraySize];    // 655
		WireU8                     reservedCounts[WorldEntry::kSpawnReservedCountsSize];      // 943
		WireU8                     reservedCosmetics[WorldEntry::kSpawnReservedCosmeticsSize];// 987
		RanWire::NativeId          startMapId;                          // 989
		WireU32                    startGate;                           // 993
		RanWire::NativeId          reservedLastCallMapId;               // 997
		RanWire::Vector3           reservedLastCallPosition;             // 1001
		WireU8                     reservedTrailing[WorldEntry::kSpawnReservedTrailingSize]; // 1013
	};
#pragma pack(pop)

	static_assert(sizeof(SpawnWire) == WorldEntry::kSpawnSize,
	              "SNETLOBBY_CHARJOIN must stay 1022 packed bytes - not the 1032 "
	              "an unpacked declaration measures");
	static_assert(offsetof(SpawnWire, userId) == WorldEntry::kSpawnOffsetUserId,
	              "userId at 8");
	static_assert(offsetof(SpawnWire, clientId) == WorldEntry::kSpawnOffsetClientId,
	              "clientId at 29");
	static_assert(offsetof(SpawnWire, gaeaId) == WorldEntry::kSpawnOffsetGaeaId,
	              "gaeaId at 33");
	static_assert(offsetof(SpawnWire, mapId) == WorldEntry::kSpawnOffsetMapId,
	              "mapId at 37");
	static_assert(offsetof(SpawnWire, position) == WorldEntry::kSpawnOffsetPosition,
	              "position at 41");
	static_assert(offsetof(SpawnWire, data) == WorldEntry::kSpawnOffsetData,
	              "the character record starts at 53");
	static_assert(sizeof(SpawnWire::data) == WorldEntry::kRecordSize,
	              "the record stays 600 bytes even inside a packed struct");
	static_assert(offsetof(SpawnWire, startMapId) == WorldEntry::kSpawnOffsetStartMapId,
	              "startMapId at 989");
	static_assert(offsetof(SpawnWire, startGate) == WorldEntry::kSpawnOffsetStartGate,
	              "startGate at 993");

	// ---------------------------------------------------------------------------
	// Application model
	// ---------------------------------------------------------------------------

	// 2358 - the redirect that tells the client which Field server to dial.
	//
	// Real information, not a placeholder: gaeaId and slot are the identity the
	// client must present in 2359, and the address is the endpoint for the second
	// TCP connection. The brief for Phase B is explicit that the client must not be
	// wired to ignore this.
	struct FieldRedirect
	{
		WireI32          joinType = WorldEntry::kJoinTypeFirst;
		WireU32          gaeaId   = 0;
		WireU32          slotFieldAgent = 0;
		WireI32          servicePort    = 0;
		std::string      fieldIp;   // at most kAddressFieldSize-1 characters
	};

	// 2359 - the identity the client presents to the Field server.
	//
	// `cryptKey` is carried because the packet has it, and is expected to be {1,1};
	// it is not a secret and nothing in Phase A treats it as one.
	struct FieldIdentity
	{
		WireI32          joinType = WorldEntry::kJoinTypeFirst;
		WireU32          gaeaId   = 0;
		WireU32          slotFieldAgent = 0;
		RanWire::CryptKey cryptKey = RanWire::DefaultCryptKey();
	};

	// 2333 - the authoritative spawn state.
	struct SpawnState
	{
		std::string userId;          // at most kUserIdFieldSize-1 characters
		WireU32      clientId = 0;
		WireU32      gaeaId   = 0;   // the entity id later gameplay messages use
		RanWire::NativeId mapId;
		RanWire::Vector3  position;  // server-authoritative; RAN sends no rotation

		// The authoritative subset of the 600-byte record.
		WireU32      accountId     = 0;
		WireU32      characterId   = 0;
		std::string  characterName; // at most kNameFieldSize-1 characters
		WireU32      characterClass = 0;
		WireU16      school         = 0;
		WireU16      level          = 0;
		RanWire::DwPair hp;
		RanWire::DwPair mp;
		RanWire::DwPair sp;

		RanWire::NativeId startMapId;
		WireU32          startGate = 0;
	};

	namespace WorldEntryCodec
	{
		// 2353 - the selection. APPENDED, 12 bytes, one signed int body.
		//
		// `characterNumber` is a raw value the client supplies and the server must
		// NOT trust: it names which character to enter the world with, and
		// ownership is the server's to verify. Phase A only moves the bytes.
		Status AppendGameJoin(std::vector<WireU8>& out, WireI32 characterNumber);

		// Decodes a 2353 frame. Right id, exactly 12 bytes.
		Status DecodeGameJoin(const std::vector<WireU8>& frame, WireI32& characterNumber);

		// 2358 - APPENDED, 48 bytes.
		//
		// Refuses an address that will not fit with room for its terminator rather
		// than truncating it, because a redirected client that dials the wrong
		// address fails in a way that looks like a network problem.
		Status AppendFieldRedirect(std::vector<WireU8>& out, const FieldRedirect& redirect);

		// 2358 - requires exactly 48 bytes with id 2358.
		Status DecodeFieldRedirect(const std::vector<WireU8>& frame, FieldRedirect& out);

		// 2359 - APPENDED, 24 bytes.
		Status AppendFieldIdentity(std::vector<WireU8>& out, const FieldIdentity& identity);

		// 2359 - requires exactly 24 bytes with id 2359.
		Status DecodeFieldIdentity(const std::vector<WireU8>& frame, FieldIdentity& out);

		// 2333 - APPENDED, always 1022 bytes with reserved regions zeroed.
		//
		// Rolls back rather than leaving a partial frame behind if a fixed string
		// field will not fit.
		Status AppendSpawn(std::vector<WireU8>& out, const SpawnState& spawn);

		// 2333 - requires exactly 1022 bytes with id 2333.
		Status DecodeSpawn(const std::vector<WireU8>& frame, SpawnState& out);

		// 2335 - the Agent's refusal of a selection. APPENDED, 12 bytes.
		//
		// `reason` is a raw EMCHAR_JOIN_FB value. Legacy's own client distinguishes
		// three of them (EMCJOIN_FB_NOWLOGIN, EMCJOIN_FB_ERROR, EMCJOIN_FB_PKPOINT) and
		// a fourth party-specific one; the numeric values are not restated here
		// because this investigation did not read the enum's declaration, and
		// inventing them would be worse than carrying the field opaquely.
		Status AppendJoinFailure(std::vector<WireU8>& out, WireI32 reason);

		// 2335 - requires exactly 12 bytes with id 2335.
		Status DecodeJoinFailure(const std::vector<WireU8>& frame, WireI32& reason);

		// ---- predicates ------------------------------------------------------
		bool IsGameJoin(MessageId id) noexcept;
		bool IsFieldRedirect(MessageId id) noexcept;
		bool IsFieldIdentity(MessageId id) noexcept;
		bool IsSpawn(MessageId id) noexcept;
		bool IsJoinFailure(MessageId id) noexcept;
	}
}
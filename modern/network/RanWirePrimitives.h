#pragma once

// WORLD-ENTRY-001 Phase A: the fixed-layout POD types shared by the world-entry
// packets.
//
// WHY A SHARED HEADER RATHER THAN ONE PER PROTOCOL.
//
// Two of the Phase A packets need the same handful of legacy primitives:
// NET_MSG_LOBBY_CHAR_SEL (2332) carries GLDWDATA, GLLLDATA, SNATIVEID, SCHARSTATS
// and SITEM_LOBY; NET_MSG_LOBBY_CHAR_JOIN (2333) carries SNATIVEID, D3DXVECTOR3
// and GLDWDATA/GLLLDATA. Declaring them once keeps a single definition that both
// static_assert sets can check, which is the property that matters: if the layout
// of GLDWDATA ever drifted, both packets would fail to compile rather than one of
// them silently changing size.
//
// ---------------------------------------------------------------------------
// THE POINT OF THIS FILE: make a layout change a COMPILE ERROR
// ---------------------------------------------------------------------------
//
// Every type below is POD, declared in the order and with the widths legacy
// declares them, and carries a static_assert on its size. That is deliberate: a
// wire protocol whose correctness depends on a compiler's alignment rules should
// be pinned by the compiler rather than by a comment, because a comment does not
// fail.
//
// This is why legacy's own arithmetic is not trusted anywhere in this
// implementation. Two independent readings of SCHARINFO_LOBBY computed 1080 bytes
// by summing SITEM_LOBY's members to 44. It is 48, because LONGLONG forces
// 8-byte alignment and the size rounds up - an 88-byte error in a packet sent once
// per character. ItemLobbyWire below spells the members out precisely so that
// sizeof(ItemLobbyWire) == 48 is a checked consequence rather than an assumption.
//
// ---------------------------------------------------------------------------
// WHAT IS NOT HERE
// ---------------------------------------------------------------------------
//
// No gameplay meaning. ItemLobbyWire is a wire shape, not an equipment system:
// WORLD-ENTRY-001 does not implement equipment, inventory, skills, quickslots,
// quests or guilds, and nothing in this file interprets a single one of those
// fields. They exist because the packets that carry them are fixed-size, and a
// packet that is the wrong size desynchronises a real client. Preserving the
// shape is what keeps the wire honest; understanding the contents is a later
// milestone's job.
//
// Enumeration values are NOT modelled either. EMCHARCLASS, EMTRIBE, EMGAME_JOINTYPE
// and EMCHAR_JOIN_FB are plain 4-byte enums on the wire (legacy/GLCharDefine.h,
// s_NetGlobal.h), and their numeric members are country- and version-dependent.
// These packets carry them as raw 32-bit values and leave interpretation to the
// layer that owns the concept. Inventing a modern enum here would freeze a set of
// values this investigation did not enumerate, which is worse than carrying them
// opaquely.

#include "NetworkTypes.h"
#include "types/Result.h"

#include <cstddef>
#include <string>
#include <vector>

namespace Modern::Network::RanWire
{
	// ---------------------------------------------------------------------------
	// SNATIVEID - legacy Lib_Engine/G-Logic/GLDefine.h:102-121
	//
	//     union { DWORD dwID; struct { WORD wMainID; WORD wSubID; }; };
	//
	// A union of DWORD and two WORDs: 4 bytes either way, and the two views are
	// byte-identical, so one 32-bit field is the whole type. Kept as a named struct
	// rather than a bare WireU32 so that a call site reads as "a native id" and not
	// as "some number".
	// ---------------------------------------------------------------------------
	struct NativeId
	{
		WireU32 value = 0;
	};
	static_assert(sizeof(NativeId) == 4, "SNATIVEID is 4 bytes on every configuration");

	// ---------------------------------------------------------------------------
	// GLDWDATA - GLDefine.h:400-419
	//
	//     union {
	//         struct { DWORD dwData1; DWORD dwData2; };
	//         struct { DWORD dwNow;  DWORD dwMax;  };
	//         ...
	//     };
	//
	// 8 bytes. Legacy fills only the "now" half for HP/MP/SP (s_COdbcGameChaGet.cpp:651
	// and the surrounding lines), leaving "max" at zero; the codec below therefore
	// writes both halves and the callers pass 0 for max where that is what legacy
	// sends. Modelled as now/max because that is the union member the game actually
	// uses, and a codec that named them dwData1/dwData2 would be describing an
	// alias rather than the intent.
	// ---------------------------------------------------------------------------
	struct DwPair
	{
		WireU32 now = 0;
		WireU32 max = 0;
	};
	static_assert(sizeof(DwPair) == 8, "GLDWDATA is 8 bytes");

	// ---------------------------------------------------------------------------
	// GLLLDATA - GLDefine.h:507-526. The 64-bit counterpart of DwPair, used by
	// experience. Reserved for WORLD-ENTRY-001: no implemented field needs it, but
	// it is part of the layout the packets must reproduce, so it is named rather
	// than absorbed into an anonymous pad.
	// ---------------------------------------------------------------------------
	struct LllPair
	{
		WireI64 now  = 0;
		WireI64 max  = 0;
	};
	static_assert(sizeof(LllPair) == 16, "GLLLDATA is 16 bytes");

	// ---------------------------------------------------------------------------
	// SCHARSTATS - GLCharDefine.h:373-380. Six WORDs: pow, str, spi, dex, int, sta.
	// Reserved; the individual members are named only in the comment, because
	// nothing reads them yet and a struct of six anonymous WORDs would be a lie
	// about which is which.
	// ---------------------------------------------------------------------------
	struct StatsSix
	{
		WireU16 values[6] = {};
	};
	static_assert(sizeof(StatsSix) == 12, "SCHARSTATS is six WORDs");

	// ---------------------------------------------------------------------------
	// D3DXVECTOR3 - three floats. RAN never transmits a rotation or an angle
	// anywhere in the post-login protocol (the WORLD-001 investigation searched
	// repo-wide and found no vAngle and no SNETPC_ANGLE), so this type is used for
	// POSITION ONLY. That is a finding worth keeping in the type: if someone later
	// wants a facing direction, the wire has nowhere to put it.
	// ---------------------------------------------------------------------------
	struct Vector3
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	};
	static_assert(sizeof(Vector3) == 12, "D3DXVECTOR3 is three floats");

	// ---------------------------------------------------------------------------
	// SITEM_LOBY - Lib_Client/G-Logic/GLItem.h:1111-1150
	//
	// RESERVED. Equipment is explicitly out of WORLD-ENTRY-001 scope, and nothing
	// here reads a field or writes one. The members are spelled out anyway, for one
	// reason: the size is 48 and that is NOT obvious.
	//
	// The members end at byte 44 (4+4+8+2+1+1+1+7+4+8+2+2). The size is 48 because
	// lnGenNum is a LONGLONG and therefore forces 8-byte alignment, so the compiler
	// rounds 44 up to the next multiple of 8. Summing the members - the obvious way
	// to do this by hand - gives 44 and an 88-byte error across a 22-slot array.
	// The padding is named tailPad below so that the rounding is visible rather
	// than something a future reader has to rediscover.
	// ---------------------------------------------------------------------------
	struct ItemLobbyWire
	{
		NativeId nativeId;     //   0  reserved
		NativeId disguise;     //   4  reserved
		WireI64  genNum;       //   8  reserved - forces 8-byte alignment
		WireU16  turnNum;      //  16  reserved
		WireU8   genType;      //  18  reserved
		WireU8   channelId;    //  19  reserved
		WireU8   fieldId;      //  20  reserved
		WireU8   damage;       //  21  reserved
		WireU8   defense;      //  22  reserved
		WireU8   resistFire;   //  23  reserved
		WireU8   resistIce;    //  24  reserved
		WireU8   resistElec;   //  25  reserved
		WireU8   resistPoison; //  26  reserved
		WireU8   resistSpirit; //  27  reserved
		WireU8   optType[4];   //  28  reserved
		WireI16  optValue[4];  //  32  reserved
		WireU16  color1;       //  40  reserved
		WireU16  color2;       //  42  reserved
		WireU8   tailPad[4];   //  44  alignment padding, NOT a member - see above
	};
	static_assert(sizeof(ItemLobbyWire) == 48,
	              "SITEM_LOBY is 48 bytes, not the 44 its members sum to");
	static_assert(offsetof(ItemLobbyWire, genNum) == 8, "lnGenNum sits at 8");
	static_assert(offsetof(ItemLobbyWire, tailPad) == 44,
	              "SITEM_LOBY's members end at 44; the size is 48 because of alignment");

	// ---------------------------------------------------------------------------
	// CRYPT_KEY - s_NetGlobal.h:2848-2859. Two USHORTs, 4 bytes.
	//
	// IT PROTECTS NOTHING, and that is a measured finding rather than a guess.
	// CClientManager::GetNewCryptKey (s_CClientManager.cpp:93-102) hardcodes both
	// halves to 1 with the random version commented out; the client ignores the
	// transmitted value entirely and hardcodes 1 as well (s_NetClientMsg.cpp:135-144);
	// and the cipher that would have consumed it, m_Bit::buf_encode, is commented out
	// at all four call sites.
	//
	// It is still carried on the wire because NET_GAME_JOIN_FIELD_IDENTITY (2359)
	// has it as a field, and a packet of the wrong size is a malformed packet. The
	// bytes exist to keep framing aligned, and the codec below does not treat them
	// as a secret.
	// ---------------------------------------------------------------------------
	struct CryptKey
	{
		WireU16 keyDirection = 1;
		WireU16 key          = 1;
	};
	static_assert(sizeof(CryptKey) == 4, "CRYPT_KEY is two USHORTs");

	// RAN's own constant, not an assumption. GetNewCryptKey returns 1/1 and the
	// client substitutes 1/1 regardless of what it was sent, so 1/1 is the only
	// value that has ever appeared on this path.
	inline CryptKey DefaultCryptKey() noexcept
	{
		return CryptKey{ 1, 1 };
	}

	// ---------------------------------------------------------------------------
	// Small fixed-width helpers shared by both Phase A codecs.
	// ---------------------------------------------------------------------------

	// Writes a NUL-terminated, zero-padded fixed char field.
	//
	// Rejects a value that would not fit with room for its terminator, rather than
	// truncating it: a silently shortened character name would be a wrong name, and
	// a wrong name is worse than a refused packet. The field is zeroed first so
	// that the padding is deterministic, which is what makes an encoded frame
	// byte-comparable in a test.
	Status PutFixedCharField(std::vector<WireU8>& out, std::size_t offset,
	                         std::size_t fieldSize, const std::string& value);

	// Reads a NUL-terminated fixed char field.
	//
	// A field with no NUL anywhere inside its `fieldSize` bytes is REJECTED. Legacy's
	// clients read these with a C string function and would run past the end; the
	// modern boundary refuses instead, which costs nothing because every field in
	// these packets is zero-filled by construction on the sending side.
	Status ReadFixedCharField(const std::vector<WireU8>& frame, std::size_t offset,
	                          std::size_t fieldSize, std::string& out);

	// True when every byte in [offset, offset+length) is zero.
	//
	// Used by the tests to prove a reserved region is actually reserved rather than
	// merely labelled as such. It is here rather than in the test file because the
	// same check is wanted by both protocol test suites, and duplicating it would
	// let the two drift.
	bool IsRegionZeroed(const std::vector<WireU8>& frame, std::size_t offset,
	                   std::size_t length);
}
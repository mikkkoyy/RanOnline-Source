#pragma once

// WORLD-ENTRY-002K: the ATTACK_DAMAGE result messages, 3043 and 3044.
//
// WHAT THIS IS
//
// 002i implemented the ATTACK REQUEST (3036) and its broadcast (3037): whether an
// attack is valid and in range. This file carries the RESULT - how much damage the
// server actually applied.
//
// The two are deliberately different things and the distinction matters to the
// packets:
//
//   3037 says the attack was ALLOWED. It carries an animation and a target.
//   3043/3044 say the attack LANDED, and carry the number the SERVER applied.
//
// A refused or avoided attack gets 3041/3042 instead and never reaches here, so a
// 3043 is itself the statement that HP changed.
//
// THE IDS AND LAYOUTS, FROM LEGACY
//
// Ids: `Lib_Network/s_NetGlobal.h:1009-1010`
//
//   NET_MSG_GCTRL_ATTACK_DAMAGE     = GCTRL + 151 = 3043
//   NET_MSG_GCTRL_ATTACK_DAMAGE_BRD = GCTRL + 152 = 3044
//
// where NET_MSG_GCTRL is NET_MSG_BASE(992) + 1900 = 2892.
//
// Structs: `Lib_Client/G-Logic/GLContrlPcMsg.h`
//
//   SNETPC_ATTACK_DAMAGE      :853  nmg | emTarCrow | dwTarID | int nDamage | DWORD dwDamageFlag
//   SNETPC_ATTACK_DAMAGE_BRD  :897  SNETPC_BROAD | emTarCrow | dwTarID | int nDamage | DWORD dwDamageFlag
//
// With NET_MSG_GENERIC = 8 bytes (dwSize, nType) and SNETPC_BROAD = 12 bytes
// (nmg, DWORD dwGaeaID) that is 24 and 28 bytes. The 8-byte header is
// cross-checked the same way AttackProtocol.h checks it: legacy's own
// SNETPC_GOTO is 8 + 4 + 12 + 12 = 36, which GotoProtocol already asserts.
//
// TWO SIGNEDNESS FACTS, both load-bearing
//
//   * `nDamage` is a SIGNED `int`. It is sent from DamageProc after at least two
//     truncating multiplications, so a caller could in principle produce a
//     negative; the decoder therefore carries it as int32 rather than
//     reinterpreting the bytes as unsigned.
//   * `dwDamageFlag` is UNSIGNED and is a bitmask of DAMAGE_TYPE_* (GLDefine.h:
//     772-785): SHOCK 0x1, CRITICAL 0x2, CRUSHING_BLOW 0x4, PSY_REDUCE 0x8,
//     MAGIC_REDUCE 0x10, PSY_REFLECTION 0x20, MAGIC_REFLECTION 0x40,
//     DAMAGE_ABSORBED 0x80, IMMUNE 0x100, ILLUSION 0x200. The known bits are
//     named below so a reader need not go back to GLDefine.h; the DECODER still
//     accepts unknown bits rather than refusing them, because the enum is the
//     wire's business and a newer flag must not break an older reader.
//
// WHAT THE APPLIED NUMBER MEANS
//
// The value carried here is the damage the authoritative resource layer APPLIED,
// not the damage the resolution asked for. They differ whenever the hit
// overkills the target's remaining HP: legacy's DECREASE (GLDefine.h:443) floors
// at zero, and GLCHARLOGIC::RECEIVE_DAMAGE (GLogixExPC.cpp:2093) returns the
// DIFFERENCE actually lost for exactly that reason. Reporting the request would
// let a client believe it dealt more damage than the target had left.

#include "AttackProtocol.h"
#include "MessageReader.h"
#include "NetworkCodec.h"
#include "NetworkTypes.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Modern::Network::Attack
{
	using WireU8  = Network::WireU8;
	using WireU32 = Network::WireU32;
	using WireI32 = Network::WireI32;

	// ---- message ids --------------------------------------------------------
	inline constexpr MessageId kAttackDamageId    = Protocol::kGCtrlBase + 151; // 3043
	inline constexpr MessageId kAttackDamageBrdId = Protocol::kGCtrlBase + 152; // 3044

	static_assert(kAttackDamageId == 3043, "ATTACK_DAMAGE is 3043");
	static_assert(kAttackDamageBrdId == 3044, "ATTACK_DAMAGE_BRD is 3044");

	// ---- DAMAGE_TYPE_* (GLDefine.h:772-785) ---------------------------------
	//
	// Named for readability. The codec does NOT validate against them: see the
	// header on why unknown bits are accepted.
	inline constexpr WireU32 kDamageTypeNone           = 0x0000u;
	inline constexpr WireU32 kDamageTypeShock          = 0x0001u;
	inline constexpr WireU32 kDamageTypeCritical       = 0x0002u;
	inline constexpr WireU32 kDamageTypeCrushingBlow   = 0x0004u;
	inline constexpr WireU32 kDamageTypePsyReduce      = 0x0008u;
	inline constexpr WireU32 kDamageTypeMagicReduce    = 0x0010u;
	inline constexpr WireU32 kDamageTypePsyReflection  = 0x0020u;
	inline constexpr WireU32 kDamageTypeMagicReflection = 0x0040u;
	inline constexpr WireU32 kDamageTypeDamageAbsorbed = 0x0080u;
	inline constexpr WireU32 kDamageTypeImmune         = 0x0100u;
	inline constexpr WireU32 kDamageTypeIllusion       = 0x0200u;

	// Every bit this milestone knows about. Not a validation mask - see above.
	inline constexpr WireU32 kKnownDamageFlags =
	    kDamageTypeShock | kDamageTypeCritical | kDamageTypeCrushingBlow |
	    kDamageTypePsyReduce | kDamageTypeMagicReduce | kDamageTypePsyReflection |
	    kDamageTypeMagicReflection | kDamageTypeDamageAbsorbed | kDamageTypeImmune |
	    kDamageTypeIllusion;

	// ---- measured wire sizes ----------------------------------------------
	inline constexpr std::size_t kDamageSize        = 24;
	inline constexpr std::size_t kDamageBroadcastSize = 28;

	// ---- offsets ------------------------------------------------------------
	inline constexpr std::size_t kDamageTargetCrowOffset = 8;
	inline constexpr std::size_t kDamageTargetIdOffset   = 12;
	inline constexpr std::size_t kDamageAmountOffset     = 16;
	inline constexpr std::size_t kDamageFlagOffset       = 20;

	inline constexpr std::size_t kDamageBroadcastGaeaIdOffset     = 8;
	inline constexpr std::size_t kDamageBroadcastTargetCrowOffset = 12;
	inline constexpr std::size_t kDamageBroadcastTargetIdOffset   = 16;
	inline constexpr std::size_t kDamageBroadcastAmountOffset     = 20;
	inline constexpr std::size_t kDamageBroadcastFlagOffset       = 24;

	// `kCrowBytes` already comes from AttackProtocol.h; only the signed width is
	// new here, and it is asserted against sizeof(WireI32) below.
	inline constexpr std::size_t kDamageBytes = 4; // signed int32

	// ---- wire structures, for the compiler to check -------------------------
#pragma pack(push, 1)
	struct AttackDamageWire
	{
		MessageHeader header;      //   0
		WireU32       targetCrow;  //   8
		WireU32       targetId;    //  12
		WireI32       damage;      //  16  SIGNED, unlike every other field
		WireU32       damageFlag;  //  20
	};

	struct AttackDamageBroadcastWire
	{
		MessageHeader header;      //   0
		WireU32       gaeaId;      //   8  from SNETPC_BROAD
		WireU32       targetCrow;  //  12
		WireU32       targetId;    //  16
		WireI32       damage;      //  20  SIGNED
		WireU32       damageFlag;  //  24
	};
#pragma pack(pop)

	static_assert(sizeof(AttackDamageWire) == kDamageSize,
	              "SNETPC_ATTACK_DAMAGE must stay 24 bytes - measured, not summed");
	static_assert(offsetof(AttackDamageWire, targetCrow) == kDamageTargetCrowOffset,
	              "emTarCrow at offset 8");
	static_assert(offsetof(AttackDamageWire, targetId) == kDamageTargetIdOffset,
	              "dwTarID at offset 12");
	static_assert(offsetof(AttackDamageWire, damage) == kDamageAmountOffset,
	              "int nDamage at offset 16");
	static_assert(offsetof(AttackDamageWire, damageFlag) == kDamageFlagOffset,
	              "dwDamageFlag at offset 20");

	static_assert(sizeof(AttackDamageBroadcastWire) == kDamageBroadcastSize,
	              "SNETPC_ATTACK_DAMAGE_BRD must stay 28 bytes");
	static_assert(offsetof(AttackDamageBroadcastWire, gaeaId) == kDamageBroadcastGaeaIdOffset,
	              "SNETPC_BROAD::dwGaeaID at offset 8");
	static_assert(offsetof(AttackDamageBroadcastWire, targetCrow) == kDamageBroadcastTargetCrowOffset,
	              "emTarCrow at offset 12");
	static_assert(offsetof(AttackDamageBroadcastWire, targetId) == kDamageBroadcastTargetIdOffset,
	              "dwTarID at offset 16");
	static_assert(offsetof(AttackDamageBroadcastWire, damage) == kDamageBroadcastAmountOffset,
	              "int nDamage at offset 20");
	static_assert(offsetof(AttackDamageBroadcastWire, damageFlag) == kDamageBroadcastFlagOffset,
	              "dwDamageFlag at offset 24");

	// A signed 32-bit field occupies exactly four bytes, which is the whole reason
	// nDamage can be signed while its neighbours are DWORDs.
	static_assert(sizeof(WireI32) == 4, "nDamage is a 32-bit int on the wire");
	static_assert(sizeof(AttackDamageWire) ==
	                  Network::kMessageHeaderSize + 4 * kCrowBytes,
	              "ATTACK_DAMAGE has no inter-member padding to remove");
	static_assert(sizeof(AttackDamageBroadcastWire) ==
	                  Network::kMessageHeaderSize + 5 * kCrowBytes,
	              "ATTACK_DAMAGE_BRD has no inter-member padding to remove");

	// ---- application model --------------------------------------------------

	// 3043's payload: what this client's own attack did.
	//
	// `damage` is the damage APPLIED, not the damage requested - see the header.
	struct AttackDamage
	{
		WireU32 targetCrow = Attack::kCrowPc;
		WireU32 targetId   = 0;
		WireI32 damage     = 0;
		WireU32 damageFlag = kDamageTypeNone;
	};

	// 3044's payload: the same result, told to everyone who can see it.
	//
	// `gaeaId` is the ATTACKER's id, never the target's - the same convention
	// 3033, 3035, 3037 and 3042 all follow.
	struct AttackDamageBroadcast
	{
		WireU32 gaeaId     = 0;
		WireU32 targetCrow = Attack::kCrowPc;
		WireU32 targetId   = 0;
		WireI32 damage     = 0;
		WireU32 damageFlag = kDamageTypeNone;
	};

	namespace AttackDamageCodec
	{
		// 3043. APPENDED to `out`. Exactly 24 bytes.
		//
		// A negative `damage` IS refused. Legacy cannot produce one - every step
		// that touches nDAMAGE floors it at 1 - so a negative value on the wire is
		// a protocol fault, and encoding one would let a caller emit a frame its
		// own decoder rejects. This is the same encoder/decoder agreement the
		// GotoProtocol comment insists on.
		Status AppendAttackDamage(std::vector<WireU8>& out, const AttackDamage& damage);

		// 3043. Requires exactly 24 bytes carrying id 3043.
		Status DecodeAttackDamage(const std::vector<WireU8>& frame, AttackDamage& out);

		// 3044. APPENDED. Exactly 28 bytes.
		Status AppendAttackDamageBroadcast(std::vector<WireU8>& out,
		                                   const AttackDamageBroadcast& broadcast);

		// 3044. Requires exactly 28 bytes carrying id 3044.
		Status DecodeAttackDamageBroadcast(const std::vector<WireU8>& frame,
		                                   AttackDamageBroadcast& out);

		// ---- predicates ---------------------------------------------------
		bool IsAttackDamage(MessageId id) noexcept;
		bool IsAttackDamageBroadcast(MessageId id) noexcept;

		// Rebuilds the on-wire bytes of a message the framer has already split.
		// Re-exported for the reason AttackProtocol.h gives.
		inline std::vector<WireU8> ReconstructFrame(const Network::Message& message)
		{
			return Network::ReconstructFrame(message);
		}
	}
} // namespace Modern::Network::Attack

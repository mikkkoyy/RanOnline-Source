#pragma once

// WORLD-ENTRY-002i: the ATTACK request family, 3036 / 3037 / 3041 / 3042.
//
// WHAT THIS IS
//
// The first server-authoritative ATTACK messages in the modern stack. Legacy RAN
// models an attack in two clearly separated halves:
//
//   * the REQUEST and its VALIDATION - can this character attack that target at
//     all (GLCharMsg.cpp:327-366);
//   * the RESOLUTION - how much damage, spent SP, 3043/3044 (GLChar::DamageProc,
//     GLChar.cpp:2484-2539).
//
// This file and its service cover the FIRST half only. There is no damage here,
// no HP or SP change, no 3043/3044, and nothing in this family mutates a
// character. That split is deliberate and is recorded in the milestone report:
// the validation block sits BEFORE every PvP and battle-mode rule in legacy, so
// it is the part that can be proven and implemented without dragging in the PK,
// club, school, tyranny and capture-the-flag systems.
//
// THE MESSAGE IDS
//
// From legacy `Lib_Network/s_NetGlobal.h:1003-1008`, where they are expressed
// relative to NET_MSG_GCTRL (which is 2892, the same base as
// NetworkTypes.h's Protocol::kGCtrlBase):
//
//   NET_MSG_GCTRL_ATTACK        = GCTRL + 144 = 3036
//   NET_MSG_GCTRL_ATTACK_BRD    = GCTRL + 145 = 3037
//   NET_MSG_GCTRL_ATTACK_AVOID  = GCTRL + 149 = 3041
//   NET_MSG_GCTRL_ATTACK_AVOID_BRD = GCTRL + 150 = 3042
//
// The IDs are asserted against their decimal values below, because the same
// header contains an UNRELATED `NET_MSG_GCTRL + 3036` (a GM command, line 2570).
// A number alone is not an identity in this file.
//
// THE LAYOUTS
//
// From legacy `Lib_Client/G-Logic/GLContrlPcMsg.h` and `GLContrlBaseMsg.h`,
// with NET_MSG_GENERIC = { DWORD dwSize; EMNET_MSG nType; } (8 bytes) and
// SNETPC_BROAD = { NET_MSG_GENERIC nmg; DWORD dwGaeaID; } (12 bytes):
//
//   SNETPC_ATTACK          :757  24 bytes  nmg | emTarCrow | dwTarID | dwAniSel | dwFlags
//   SNETPC_ATTACK_BRD      :779  24 bytes  SNETPC_BROAD | emTarCrow | dwTarID | dwAniSel
//   SNETPC_ATTACK_AVOID    :818  16 bytes  nmg | emTarCrow | dwTarID
//   SNETPC_ATTACK_AVOID_BRD :825 20 bytes  SNETPC_BROAD | emTarCrow | dwTarID
//
// The 8-byte header is not a guess: legacy's own SNETPC_GOTO is 8 + 4 + 12 + 12
// = 36 bytes, which is exactly the size GotoProtocol.h already asserts for 3034.
//
// WHAT IS NOT HERE
//
// 3038/3039 (ATTACK_CANCEL), 3043/3044 (ATTACK_DAMAGE), and 3049 (UPDATE_SP) are
// deliberately absent. They belong to the resolution half and to a later
// milestone. CROW_MOB targets are accepted by the wire format - the format is
// legacy's and does not get to second-guess it - but no modern target resolves
// to one, so a mob request is refused by the RULE, not by the codec.

#include "MessageReader.h"
#include "NetworkCodec.h"
#include "NetworkTypes.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Modern::Network::Attack
{
	using WireU32 = Network::WireU32;
	using WireU8  = Network::WireU8;

	// ---- message ids -----------------------------------------------------
	inline constexpr MessageId kAttackId       = Protocol::kGCtrlBase + 144; // 3036
	inline constexpr MessageId kAttackBrdId    = Protocol::kGCtrlBase + 145; // 3037
	inline constexpr MessageId kAttackAvoidId  = Protocol::kGCtrlBase + 149; // 3041
	inline constexpr MessageId kAttackAvoidBrdId = Protocol::kGCtrlBase + 150; // 3042

	static_assert(kAttackId == 3036, "ATTACK is 3036");
	static_assert(kAttackBrdId == 3037, "ATTACK_BRD is 3037");
	static_assert(kAttackAvoidId == 3041, "ATTACK_AVOID is 3041");
	static_assert(kAttackAvoidBrdId == 3042, "ATTACK_AVOID_BRD is 3042");

	// ---- EMCROW ----------------------------------------------------------
	//
	// From legacy `Lib_Engine/G-Logic/GLDefine.h:586-588`. Only the three values
	// this milestone can reason about are named; the legacy enum runs further.
	//
	// CROW_PC is the only one the modern world can currently resolve to a live
	// target, because a spawned player character is the only entity type that
	// exists on the Field today.
	inline constexpr WireU32 kCrowPc  = 0;
	inline constexpr WireU32 kCrowNpc = 1;
	inline constexpr WireU32 kCrowMob = 2;

	// ---- measured wire sizes --------------------------------------------
	inline constexpr std::size_t kRequestSize      = 24;
	inline constexpr std::size_t kBroadcastSize    = 24;
	inline constexpr std::size_t kAvoidSize        = 16;
	inline constexpr std::size_t kAvoidBroadcastSize = 20;

	// ---- offsets ---------------------------------------------------------
	//
	// Every one is relative to the start of the frame, and every one is asserted
	// below against the packed layout. `dwSize` at 0 and `nType` at 4 are the
	// NET_MSG_GENERIC header and are the same in all four messages.
	inline constexpr std::size_t kRequestTargetCrowOffset = 8;
	inline constexpr std::size_t kRequestTargetIdOffset   = 12;
	inline constexpr std::size_t kRequestAniSelOffset     = 16;
	inline constexpr std::size_t kRequestFlagsOffset      = 20;

	inline constexpr std::size_t kBroadcastGaeaIdOffset     = 8;
	inline constexpr std::size_t kBroadcastTargetCrowOffset = 12;
	inline constexpr std::size_t kBroadcastTargetIdOffset   = 16;
	inline constexpr std::size_t kBroadcastAniSelOffset     = 20;

	inline constexpr std::size_t kAvoidTargetCrowOffset = 8;
	inline constexpr std::size_t kAvoidTargetIdOffset   = 12;

	inline constexpr std::size_t kAvoidBroadcastGaeaIdOffset     = 8;
	inline constexpr std::size_t kAvoidBroadcastTargetCrowOffset = 12;
	inline constexpr std::size_t kAvoidBroadcastTargetIdOffset   = 16;

	// A legacy `EMCROW` on the wire is one DWORD: the enum's underlying type is
	// the compiler's `int`, which is 4 bytes on every platform this builds for.
	inline constexpr std::size_t kCrowBytes = 4;

	// ---- wire structures, for the compiler to check ----------------------
	//
	// Declared so sizeof and offsetof can be ASSERTED. Never memcpy'd onto the
	// wire: the codec reads and writes little-endian field by field, exactly as
	// GotoProtocol.cpp does.
#pragma pack(push, 1)
	struct AttackRequestWire
	{
		MessageHeader header;  //   0
		WireU32       targetCrow; //   8
		WireU32       targetId;   //  12
		WireU32       aniSel;     //  16
		WireU32       flags;      //  20
	};

	struct AttackBroadcastWire
	{
		MessageHeader header; //   0
		WireU32       gaeaId;     //   8
		WireU32       targetCrow; //  12
		WireU32       targetId;   //  16
		WireU32       aniSel;     //  20
	};

	struct AttackAvoidWire
	{
		MessageHeader header; //   0
		WireU32       targetCrow; //   8
		WireU32       targetId;   //  12
	};

	struct AttackAvoidBroadcastWire
	{
		MessageHeader header; //   0
		WireU32       gaeaId;     //   8
		WireU32       targetCrow; //  12
		WireU32       targetId;   //  16
	};
#pragma pack(pop)

	static_assert(sizeof(AttackRequestWire) == kRequestSize,
	              "SNETPC_ATTACK must stay 24 bytes - measured, not summed");
	static_assert(offsetof(AttackRequestWire, targetCrow) == kRequestTargetCrowOffset,
	              "emTarCrow at offset 8");
	static_assert(offsetof(AttackRequestWire, targetId) == kRequestTargetIdOffset,
	              "dwTarID at offset 12");
	static_assert(offsetof(AttackRequestWire, aniSel) == kRequestAniSelOffset,
	              "dwAniSel at offset 16");
	static_assert(offsetof(AttackRequestWire, flags) == kRequestFlagsOffset,
	              "dwFlags at offset 20");

	static_assert(sizeof(AttackBroadcastWire) == kBroadcastSize,
	              "SNETPC_ATTACK_BRD must stay 24 bytes");
	static_assert(offsetof(AttackBroadcastWire, gaeaId) == kBroadcastGaeaIdOffset,
	              "SNETPC_BROAD::dwGaeaID at offset 8");
	static_assert(offsetof(AttackBroadcastWire, targetCrow) == kBroadcastTargetCrowOffset,
	              "emTarCrow at offset 12");
	static_assert(offsetof(AttackBroadcastWire, targetId) == kBroadcastTargetIdOffset,
	              "dwTarID at offset 16");
	static_assert(offsetof(AttackBroadcastWire, aniSel) == kBroadcastAniSelOffset,
	              "dwAniSel at offset 20");

	static_assert(sizeof(AttackAvoidWire) == kAvoidSize,
	              "SNETPC_ATTACK_AVOID must stay 16 bytes");
	static_assert(offsetof(AttackAvoidWire, targetCrow) == kAvoidTargetCrowOffset,
	              "emTarCrow at offset 8");
	static_assert(offsetof(AttackAvoidWire, targetId) == kAvoidTargetIdOffset,
	              "dwTarID at offset 12");

	static_assert(sizeof(AttackAvoidBroadcastWire) == kAvoidBroadcastSize,
	              "SNETPC_ATTACK_AVOID_BRD must stay 20 bytes");
	static_assert(offsetof(AttackAvoidBroadcastWire, gaeaId) == kAvoidBroadcastGaeaIdOffset,
	              "SNETPC_BROAD::dwGaeaID at offset 8");
	static_assert(offsetof(AttackAvoidBroadcastWire, targetCrow) == kAvoidBroadcastTargetCrowOffset,
	              "emTarCrow at offset 12");
	static_assert(offsetof(AttackAvoidBroadcastWire, targetId) == kAvoidBroadcastTargetIdOffset,
	              "dwTarID at offset 16");

	// The packed layout is not an approximation: every member is 4 bytes behind
	// an 8-byte header, so there is no padding to remove. Asserted so a future
	// "pack it properly" change cannot silently alter the wire.
	static_assert(sizeof(AttackRequestWire) == Network::kMessageHeaderSize + 4 * kCrowBytes,
	              "ATTACK has no inter-member padding to remove");
	static_assert(sizeof(AttackBroadcastWire) == Network::kMessageHeaderSize + 4 * kCrowBytes,
	              "ATTACK_BRD has no inter-member padding to remove");
	static_assert(sizeof(AttackAvoidWire) == Network::kMessageHeaderSize + 2 * kCrowBytes,
	              "ATTACK_AVOID has no inter-member padding to remove");
	static_assert(sizeof(AttackAvoidBroadcastWire) == Network::kMessageHeaderSize + 3 * kCrowBytes,
	              "ATTACK_AVOID_BRD has no inter-member padding to remove");

	// ---- application model ----------------------------------------------

	// 3036's payload. The client's REQUEST: which target, which animation, and
	// a flag word the modern server does not interpret.
	struct AttackRequest
	{
		// `emTarCrow`: a legacy EMCROW. Modelled as the raw wire DWORD rather than
		// a narrow enum, because this codec's job is to carry what legacy sent and
		// let the RULE decide what it means. An unknown value is not a codec error.
		WireU32 targetCrow = Attack::kCrowPc;

		// `dwTarID`. For CROW_PC on the Field this is the target character's
		// gaeaId; legacy reaches it through GLGaeaServer::GetChar (GLCharMsg.cpp:380)
		// and DamageProc reaches the same record the same way (GLChar.cpp:2495).
		WireU32 targetId = 0;

		// `dwAniSel`: an animation selector, copied by legacy into
		// `m_dwANISUBSELECT` and used for the attack animation only
		// (GLCharMsg.cpp:333). Carried, never interpreted.
		WireU32 aniSel = 0;

		// `dwFlags`: unexamined in this milestone. Legacy's constructor default is
		// NULL and no reader of the 3036 path was found that consumes it, so it is
		// carried verbatim and explicitly NOT given meaning here.
		WireU32 flags = 0;
	};

	// 3037's payload: an ATTACK that the server ACCEPTED.
	//
	// Legacy fills `dwGaeaID` with the ATTACKER's own id (GLCharMsg.cpp, and the
	// same convention 3033 and 3035 follow), so a receiving client learns who
	// swung and at whom.
	struct AttackBroadcast
	{
		WireU32 gaeaId     = 0;
		WireU32 targetCrow = Attack::kCrowPc;
		WireU32 targetId   = 0;
		WireU32 aniSel     = 0;
	};

	// 3041's payload: this client's own attack was REFUSED.
	//
	// Legacy sends this on the out-of-range branch (GLCharMsg.cpp:352-355) and
	// carries the target it refused, not a reason code.
	struct AttackAvoid
	{
		WireU32 targetCrow = Attack::kCrowPc;
		WireU32 targetId   = 0;
	};

	// 3042's payload: the same refusal, told to everyone else who can see it.
	struct AttackAvoidBroadcast
	{
		WireU32 gaeaId     = 0;
		WireU32 targetCrow = Attack::kCrowPc;
		WireU32 targetId   = 0;
	};

	namespace AttackCodec
	{
		// 3036. APPENDED to `out`. Exactly 24 bytes.
		//
		// Nothing is filtered: this codec carries what the caller asked to send
		// and decides nothing. `targetId` of 0 is refused by the RULE, not here,
		// for the same reason GotoProtocol refuses a non-finite coordinate but not
		// an unreachable destination - the difference is which layer can answer.
		Status AppendAttackRequest(std::vector<WireU8>& out, const AttackRequest& request);

		// 3036. Requires exactly 24 bytes carrying id 3036.
		Status DecodeAttackRequest(const std::vector<WireU8>& frame, AttackRequest& out);

		// 3037. APPENDED. Exactly 24 bytes.
		Status AppendAttackBroadcast(std::vector<WireU8>& out,
		                             const AttackBroadcast& broadcast);

		// 3037. Requires exactly 24 bytes carrying id 3037.
		Status DecodeAttackBroadcast(const std::vector<WireU8>& frame,
		                             AttackBroadcast& out);

		// 3041. APPENDED. Exactly 16 bytes.
		Status AppendAttackAvoid(std::vector<WireU8>& out, const AttackAvoid& avoid);

		// 3041. Requires exactly 16 bytes carrying id 3041.
		Status DecodeAttackAvoid(const std::vector<WireU8>& frame, AttackAvoid& out);

		// 3042. APPENDED. Exactly 20 bytes.
		Status AppendAttackAvoidBroadcast(std::vector<WireU8>& out,
		                                  const AttackAvoidBroadcast& broadcast);

		// 3042. Requires exactly 20 bytes carrying id 3042.
		Status DecodeAttackAvoidBroadcast(const std::vector<WireU8>& frame,
		                                  AttackAvoidBroadcast& out);

		// ---- predicates ---------------------------------------------------
		bool IsAttack(MessageId id) noexcept;
		bool IsAttackBroadcast(MessageId id) noexcept;
		bool IsAttackAvoid(MessageId id) noexcept;
		bool IsAttackAvoidBroadcast(MessageId id) noexcept;

		// Rebuilds the on-wire bytes of a message the framer has already split.
		//
		// Re-exported rather than reimplemented, for the reason
		// GotoProtocol.h gives: the framing boundary is Network::ReconstructFrame
		// and a second copy would be a second thing to keep correct.
		inline std::vector<WireU8> ReconstructFrame(const Network::Message& message)
		{
			return Network::ReconstructFrame(message);
		}
	}
} // namespace Modern::Network::Attack
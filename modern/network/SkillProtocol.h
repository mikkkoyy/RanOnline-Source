#pragma once

// SKILL-021: the SKILL-CAST protocol - 3303, 3304 and 3305.
//
// This is the first variable-length message in the modern tree, and that fact
// drives almost every decision below.
//
// ---------------------------------------------------------------------------
// THE IDS
// ---------------------------------------------------------------------------
//
//   REQ_SKILL      3303   the client's cast request      variable, 63..159
//   REQ_SKILL_FB   3304   the caster's own feedback       fixed,   45
//   REQ_SKILL_BRD  3305   "this character cast that"      variable, 36..132
//
// s_NetGlobal.h:1111-1113, all NET_MSG_GCTRL + n, and Protocol::kGCtrlBase is
// NET_MSG_BASE + 1900 = 2892.
//
// 3303/3304/3305 are NOT reused from the movement and attack families. 3034
// and 3035 are GOTO and GOTO_BRD (GotoProtocol.h:110-111); 3041 and 3042 are
// ATTACK_AVOID and ATTACK_AVOID_BRD (AttackProtocol.h:76-77). The static_asserts
// at the bottom of this file name all four so a future edit cannot quietly
// merge the two families.
//
// ---------------------------------------------------------------------------
// MEASURED, NOT SUMMED - and the arithmetic that produced it
// ---------------------------------------------------------------------------
//
// GLContrlSkillMsg.h:7 opens a `#pragma pack(1)` region (reset at :428), so the
// struct has no inter-member padding and no tail padding:
//
//   SNETPC_REQ_SKILL     159
//   SNETPC_REQ_SKILL_FB   45
//   SNETPC_REQ_SKILL_BRD 132
//
// The three sizes that are not 4- or 8-aligned in the middle of 3303 come
// straight from that region, and each one is asserted below:
//
//   fSum_TarRange  sits at 29, immediately after the ONE-BYTE `bool
//                  bDefenseSkill` at 28. fAttVelo at 41, fAttVeloItem at 45,
//                  dwAnimationKey at 49, emMType at 53 and emSType at 57 follow.
//                  NONE of them is naturally aligned, and neither is the target
//                  array at 63.
//
// That is why nothing in the codec below ever casts a pointer into the buffer.
// A `reinterpret_cast<const float*>(frame.data() + 29)` is undefined behaviour
// on every target this project builds for, and on x86 it "works" only until a
// compiler assumes the alignment it was told. Every float goes through a
// ReadU32 followed by a memcpy, exactly as GotoProtocol.cpp does for fDelay.
//
// ---------------------------------------------------------------------------
// THE VARIABLE LENGTH IS REAL, AND dwSize IS THE AUTHORITY
// ---------------------------------------------------------------------------
//
// GLContrlSkillMsg.h:203:
//
//     void CALCSIZE () {
//         nmg.dwSize = sizeof(*this) - sizeof(STARID) * (EMTARGET_NET - wTARGET_NUM);
//     }
//
// with `sizeof(STARID) == 4` (GLDefine.h:718-721) and `EMTARGET_NET == 24`
// (GLDefine.h:636). So:
//
//     dwSize = 159 - 4*(24 - n) = 63 + 4n
//
// and the legal lengths are 63, 67, 71, ... 159. The client sends `nmg->dwSize`
// bytes, not `sizeof` - GLCharacterSkill.cpp:39 builds the struct and
// NETSENDTOFIELD passes the length - so this really is variable on the wire.
//
// `RequireFrame`'s exact-size idiom, which every other protocol in this tree
// uses, is WRONG for this message and is not used here.
//
// ---------------------------------------------------------------------------
// TWO BOUNDS, DEFENDING TWO DIFFERENT THINGS
// ---------------------------------------------------------------------------
//
// The target list is bounded twice, and it matters that they are different:
//
//   1. BYTE CAPACITY - (frame.size() - 63) / 4, at most 24.
//
//      This is the memory-safety bound. `wTARGET_NUM` is a peer-controlled
//      WORD: a peer that declares 24 in a 63-byte frame must not cause 24
//      reads. Legacy clamps it at the top of its own loop
//      (`if (i >= EMTARGET_NET) continue;`, GLCharSkillMsg.cpp:415) but only
//      AFTER it has already dereferenced `pNetMsg->sTARGET_IDS[i]` to build the
//      STARGETID - and legacy is reading a struct cast straight onto the packet,
//      so on a short packet that dereference is the bug. Deriving capacity from
//      the validated LENGTH removes the class entirely: there is no index the
//      peer can influence past the end of what was received.
//
//   2. THE DECLARED COUNT - `wTARGET_NUM` itself.
//
//      This is the honesty bound, and it exists because of a real legacy
//      quirk: the SNETPC_REQ_SKILL constructor sets `nmg.dwSize = sizeof(*this)`
//      (GLContrlSkillMsg.h:199) and CALCSIZE is only ever called from ADDTARGET.
//      A cast with zero targets therefore goes out declaring 159 bytes, with
//      96 bytes of never-read array tail after the header.
//
//      Those 159 bytes are accepted - it is normal legacy traffic and refusing
//      it would break the client. But the 96 trailing bytes are NOT 24 targets,
//      and a decoder that reported them as targets would hand a later milestone
//      24 fabricated target ids the client never sent. So the effective list is
//      `min(declared, capacity)` and BOTH numbers stay visible in the decoded
//      struct: a reader can always see what the peer declared and what the
//      bytes actually supported.
//
// Neither bound can exceed 24: capacity is at most (159-63)/4 by construction,
// and the minimum of two numbers neither of which exceeds 24 is at most 24.
// The `wTARGET_NUM > EMTARGET_NET` case, which underflows CALCSIZE's size_t
// subtraction on the send side, is therefore handled without a special check.
//
// ---------------------------------------------------------------------------
// WHAT THIS MILESTONE DELIBERATELY DOES NOT DO
// ---------------------------------------------------------------------------
//
// No damage, no healing, no resource spend, no cooldown, no successful
// broadcast, no range acceptance and no target selection. 3305 is implemented
// as a codec because its LAYOUT is part of this contract, not because anything
// emits it yet. See FieldRoleRuntime::HandleSkillCast for the dispatch side.

#include "AttackProtocol.h"
#include "GotoProtocol.h"
#include "MessageReader.h"
#include "NetworkCodec.h"
#include "NetworkTypes.h"
#include "RanWirePrimitives.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Modern::Network
{
	namespace Skill
	{
		// s_NetGlobal.h:1111-1113.
		constexpr MessageId kSkillId    = Protocol::kGCtrlBase + 411; // 3303
		constexpr MessageId kSkillFbId  = Protocol::kGCtrlBase + 412; // 3304
		constexpr MessageId kSkillBrdId = Protocol::kGCtrlBase + 413; // 3305

		static_assert(kSkillId == 3303, "REQ_SKILL is 3303");
		static_assert(kSkillFbId == 3304, "REQ_SKILL_FB is 3304");
		static_assert(kSkillBrdId == 3305, "REQ_SKILL_BRD is 3305");

		// ---- shared geometry -------------------------------------------------

		// GLDefine.h:636. `EMTARGET_NET`. The compile-time bound on BOTH the
		// request's and the broadcast's target array, and the divisor in the
		// legacy CALCSIZE.
		constexpr std::size_t kMaxTargets = 24;

		// GLDefine.h:718-721:
		//
		//     struct STARID { WORD wCrow; WORD wID; };
		//
		// FOUR bytes, and both halves are 16-bit. `wCrow` is a plain WORD rather
		// than the EMCROW enum the API uses, and `wID` TRUNCATES a 32-bit entity
		// id to 16 bits (GLContrlSkillMsg.h:213 does the truncation explicitly).
		// `wID == 0xFFFF` is the wire sentinel for "no target"; STARID::GETID
		// maps it back to EMTARGET_NULL.
		constexpr std::size_t kTargetEntryBytes = 4;

		// A D3DXVECTOR3 on the wire: three IEEE-754 singles, x, y, z.
		constexpr std::size_t kVector3Bytes = 12;

		// A single IEEE-754 single - fSum_TarRange, fAttVelo, fAttVeloItem.
		constexpr std::size_t kFloatBytes = 4;

		// Every legacy enum on these three messages is a plain unscoped enum, so
		// the compiler sizes it as `int`: FOUR bytes, signed. Forced by
		// EMSKILL_UNKNOWN = 999 (GLDefine.h:660) and AN_TYPE_SIZE = 34
		// (DxAniKeys.h:214) on the wide side, and by EMTARGET_NULL = -1
		// (GLDefine.h:635) on the narrow one. Modelled as WireI32 so no call site
		// depends on this compiler's enum size.
		constexpr std::size_t kEnumBytes = 4;

		// GLCharDefine.h:17 -> s_NetGlobal.h:172. `char[33]`.
		//
		// Exactly 33 and NOT 34. This field is the one place in the RAN protocol
		// where the name buffer is exactly CHR_ID_LENGTH rather than
		// CHR_ID_LENGTH+1, so the constructor's ZeroMemory fills all 33 bytes and
		// nothing ever writes a terminator. A 33-byte all-0xFF name from a peer
		// is a valid packet, and any strlen over it would run off the end.
		constexpr std::size_t kNameBytes = 33;

		// ---- 3303: the request ------------------------------------------------

		// Everything before the target array. CALCSIZE's base term.
		constexpr std::size_t kRequestHeaderSize = 63;

		// One target is 67 bytes: 63 + 4.
		constexpr std::size_t kRequestOneTargetSize = kRequestHeaderSize + kTargetEntryBytes;

		// The full struct: sizeof(SNETPC_REQ_SKILL).
		constexpr std::size_t kRequestMaxSize = kRequestHeaderSize + kMaxTargets * kTargetEntryBytes;

		constexpr std::size_t kRequestSkillIdOffset           = 8;
		constexpr std::size_t kRequestTargetPositionOffset    = 12;
		constexpr std::size_t kRequestFlagsOffset            = 24;
		constexpr std::size_t kRequestDefenseSkillOffset     = 28;
		constexpr std::size_t kRequestSumTarRangeOffset      = 29;
		constexpr std::size_t kRequestBodyRadiusOffset       = 33;
		constexpr std::size_t kRequestAttackRangeOffset      = 35;
		constexpr std::size_t kRequestSkillRangeApplyOffset  = 37;
		constexpr std::size_t kRequestSkillRangeTarOffset    = 39;
		constexpr std::size_t kRequestAttackVelocityOffset      = 41;
		constexpr std::size_t kRequestAttackVelocityItemOffset   = 45;
		constexpr std::size_t kRequestAnimationKeyOffset        = 49;
		constexpr std::size_t kRequestAnimationMainTypeOffset    = 53;
		constexpr std::size_t kRequestAnimationSubTypeOffset    = 57;
		constexpr std::size_t kRequestTargetCountOffset         = 61;
		constexpr std::size_t kRequestTargetsOffset             = 63;

		// ---- 3304: the feedback -----------------------------------------------

		// Fixed. `szName` is never variable, so CALCSIZE does not exist on this
		// struct and `dwSize` is always 45.
		constexpr std::size_t kFeedbackSize = 45;

		constexpr std::size_t kFeedbackCheckOffset = 8;
		constexpr std::size_t kFeedbackNameOffset  = 12;

		// EMSKILLCHECK - GLDefine.h:639-661, verbatim and complete.
		//
		// Named only where the MEANING has been read out of legacy source, because
		// a name that reads plausibly and means something else is worse than no
		// name. In particular EMSKILL_UNKNOWN is the "the server has no such
		// skill data" code - GLogixExPC.cpp:4092 returns it from
		// `if (!pSkill)` - and must never be used as a generic failure.
		namespace SkillCheck
		{
			constexpr WireI32 kOk         = 0;   // EMSKILL_OK
			constexpr WireI32 kNotLearned = 1;   // EMSKILL_NOTLEARN
			constexpr WireI32 kDelayTime  = 2;   // EMSKILL_DELAYTIME
			constexpr WireI32 kNotItem    = 3;   // EMSKILL_NOTITEM
			constexpr WireI32 kNotArrow   = 4;   // EMSKILL_NOTARROW
			constexpr WireI32 kNotCharm   = 5;   // EMSKILL_NOTCHARM
			constexpr WireI32 kNotHp      = 6;   // EMSKILL_NOTHP
			constexpr WireI32 kNotMp      = 7;   // EMSKILL_NOTMP
			constexpr WireI32 kNotSp      = 8;   // EMSKILL_NOTSP
			constexpr WireI32 kNotExp     = 9;   // EMSKILL_NOTEXP
			constexpr WireI32 kNotRebirth = 10;  // EMSKILL_NOTREBIRTH
			constexpr WireI32 kNotCp      = 11;  // EMSKILL_NOTCP
			constexpr WireI32 kNotBullet  = 12;  // EMSKILL_NOTBULLET
			constexpr WireI32 kProhibit   = 13;  // EMSKILL_PROHIBIT
			constexpr WireI32 kUnknown    = 999; // EMSKILL_UNKNOWN
		} // namespace SkillCheck

		// ---- 3305: the broadcast ----------------------------------------------

		// CALCSIZE's base term for the broadcast, which has a shorter header than
		// the request: 36 + 4n, 36..132.
		constexpr std::size_t kBroadcastHeaderSize = 36;
		constexpr std::size_t kBroadcastMaxSize =
		    kBroadcastHeaderSize + kMaxTargets * kTargetEntryBytes;

		constexpr std::size_t kBroadcastCrowOffset           = 8;
		constexpr std::size_t kBroadcastCasterIdOffset       = 12;
		constexpr std::size_t kBroadcastSkillIdOffset        = 16;
		constexpr std::size_t kBroadcastLevelOffset          = 20;
		constexpr std::size_t kBroadcastTargetPositionOffset = 22;
		constexpr std::size_t kBroadcastTargetCountOffset    = 34;
		constexpr std::size_t kBroadcastTargetsOffset        = 36;

		// ---- wire structures, for the compiler to check ----------------------
		//
		// Declared so sizeof and offsetof can be ASSERTED. Never memcpy'd onto the
		// wire: the codec reads and writes little-endian field by field, at named
		// offsets, with no type punning.
#pragma pack(push, 1)
		struct SkillRequestWire
		{
			MessageHeader header;                             //   0
			WireU32       skillId;                            //   8
			WireU8        targetPosition[kVector3Bytes];      //  12
			WireU32       flags;                              //  24
			WireU8        defenseSkill;                       //  28
			WireU8        sumTarRange[kFloatBytes];           //  29
			WireU16       bodyRadius;                         //  33
			WireU16       attackRange;                        //  35
			WireU16       skillRangeApply;                    //  37
			WireU16       skillRangeTar;                      //  39
			WireU8        attackVelocity[kFloatBytes];        //  41
			WireU8        attackVelocityItem[kFloatBytes];    //  45
			WireU32       animationKey;                       //  49
			WireI32       animationMainType;                  //  53
			WireI32       animationSubType;                   //  57
			WireU16       targetCount;                        //  61
			WireU8 targets[kMaxTargets * kTargetEntryBytes]; //  63
		};

		struct SkillFeedbackWire
		{
			MessageHeader header;        //  0
			WireI32       skillCheck;    //  8
			WireU8        name[kNameBytes]; // 12
		};

		struct SkillBroadcastWire
		{
			MessageHeader header;                             //  0
			WireI32       crow;                              //  8
			WireU32       casterId;                          // 12
			WireU32       skillId;                           // 16
			WireU16       level;                             // 20
			WireU8        targetPosition[kVector3Bytes];     // 22
			WireU16       targetCount;                       // 34
			WireU8 targets[kMaxTargets * kTargetEntryBytes]; // 36
		};
#pragma pack(pop)

		static_assert(sizeof(SkillRequestWire) == kRequestMaxSize,
		              "SNETPC_REQ_SKILL must stay 159 bytes - measured, not summed");
		static_assert(offsetof(SkillRequestWire, skillId) == kRequestSkillIdOffset,
		              "skill_id at offset 8");
		static_assert(offsetof(SkillRequestWire, targetPosition) == kRequestTargetPositionOffset,
		              "vTARPOS at offset 12");
		static_assert(offsetof(SkillRequestWire, flags) == kRequestFlagsOffset,
		              "dwFlags at offset 24");
		static_assert(offsetof(SkillRequestWire, defenseSkill) == kRequestDefenseSkillOffset,
		              "bDefenseSkill at offset 28 - the one-byte bool everything after "
		              "is unaligned because of");
		static_assert(offsetof(SkillRequestWire, sumTarRange) == kRequestSumTarRangeOffset,
		              "fSum_TarRange at offset 29, NOT 4-aligned");
		static_assert(offsetof(SkillRequestWire, bodyRadius) == kRequestBodyRadiusOffset,
		              "wBodyRadius at offset 33");
		static_assert(offsetof(SkillRequestWire, attackRange) == kRequestAttackRangeOffset,
		              "wAttRange at offset 35");
		static_assert(offsetof(SkillRequestWire, skillRangeApply) == kRequestSkillRangeApplyOffset,
		              "wSkillRangeApply at offset 37");
		static_assert(offsetof(SkillRequestWire, skillRangeTar) == kRequestSkillRangeTarOffset,
		              "wSkillRangeTar at offset 39");
		static_assert(offsetof(SkillRequestWire, attackVelocity) == kRequestAttackVelocityOffset,
		              "fAttVelo at offset 41, NOT 4-aligned");
		static_assert(offsetof(SkillRequestWire, attackVelocityItem) ==
		                  kRequestAttackVelocityItemOffset,
		              "fAttVeloItem at offset 45, NOT 4-aligned");
		static_assert(offsetof(SkillRequestWire, animationKey) == kRequestAnimationKeyOffset,
		              "dwAnimationKey at offset 49, NOT 4-aligned");
		static_assert(offsetof(SkillRequestWire, animationMainType) ==
		                  kRequestAnimationMainTypeOffset,
		              "emMType at offset 53, NOT 4-aligned");
		static_assert(offsetof(SkillRequestWire, animationSubType) ==
		                  kRequestAnimationSubTypeOffset,
		              "emSType at offset 57, NOT 4-aligned");
		static_assert(offsetof(SkillRequestWire, targetCount) == kRequestTargetCountOffset,
		              "wTARGET_NUM at offset 61");
		static_assert(offsetof(SkillRequestWire, targets) == kRequestTargetsOffset,
		              "sTARGET_IDS at offset 63");

		// The size is the header plus the FULL array, with no padding anywhere.
		// That last clause is the one worth asserting: without `pack(1)` the six
		// unaligned floats alone would have forced the compiler to insert padding
		// and the measured offsets above would all be wrong.
		static_assert(sizeof(SkillRequestWire) == kRequestHeaderSize +
		                  kMaxTargets * kTargetEntryBytes,
		              "REQ_SKILL is 63 bytes of header plus 24 STARIDs and no padding");

		static_assert(sizeof(SkillFeedbackWire) == kFeedbackSize,
		              "SNETPC_REQ_SKILL_FB must stay 45 bytes");
		static_assert(offsetof(SkillFeedbackWire, skillCheck) == kFeedbackCheckOffset,
		              "emSKILL_FB at offset 8, as a FOUR-byte enum");
		static_assert(offsetof(SkillFeedbackWire, name) == kFeedbackNameOffset,
		              "szName at offset 12");
		static_assert(sizeof(SkillFeedbackWire) ==
		                  kMessageHeaderSize + kEnumBytes + kNameBytes,
		              "REQ_SKILL_FB is header + 4-byte enum + 33 raw name bytes");

		static_assert(sizeof(SkillBroadcastWire) == kBroadcastMaxSize,
		              "SNETPC_REQ_SKILL_BRD must stay 132 bytes");
		static_assert(offsetof(SkillBroadcastWire, crow) == kBroadcastCrowOffset,
		              "emCrow at offset 8, as a FOUR-byte enum");
		static_assert(offsetof(SkillBroadcastWire, casterId) == kBroadcastCasterIdOffset,
		              "dwID at offset 12 - the FULL 32-bit caster id");
		static_assert(offsetof(SkillBroadcastWire, skillId) == kBroadcastSkillIdOffset,
		              "skill_id at offset 16");
		static_assert(offsetof(SkillBroadcastWire, level) == kBroadcastLevelOffset,
		              "wLEVEL at offset 20");
		static_assert(offsetof(SkillBroadcastWire, targetPosition) ==
		                  kBroadcastTargetPositionOffset,
		              "vTARPOS at offset 22");
		static_assert(offsetof(SkillBroadcastWire, targetCount) == kBroadcastTargetCountOffset,
		              "wTARNUM at offset 34");
		static_assert(offsetof(SkillBroadcastWire, targets) == kBroadcastTargetsOffset,
		              "sTARIDS at offset 36");

		// The asymmetry that is easiest to get wrong: the CASTER id here is a
		// full 32-bit DWORD, while every TARGET is still a 16-bit STARID. Widening
		// the targets or narrowing the caster would each produce a 132-byte packet
		// that no client could read.
		static_assert(sizeof(SkillBroadcastWire) ==
		                  kMessageHeaderSize + kEnumBytes + sizeof(WireU32) + sizeof(WireU32) +
		                      sizeof(WireU16) + kVector3Bytes + sizeof(WireU16) +
		                      kMaxTargets * kTargetEntryBytes,
		              "REQ_SKILL_BRD mixes a 32-bit caster with 16-bit targets");

		static_assert(kRequestMaxSize == 159, "the full request is 159 bytes");
		static_assert(kRequestOneTargetSize == 67, "a one-target request is 67 bytes");
		static_assert(kBroadcastMaxSize == 132, "the full broadcast is 132 bytes");

		// 3303-3305 must not land on anything another family already owns. Named
		// as the GOTO and ATTACK ids rather than as literals, so a change to
		// either family breaks this build instead of silently colliding with it.
		static_assert(kSkillId != Goto::kGotoId && kSkillId != Goto::kGotoBrdId &&
		                  kSkillId != Attack::kAttackId && kSkillId != Attack::kAttackBrdId &&
		                  kSkillId != Attack::kAttackAvoidId &&
		                  kSkillId != Attack::kAttackAvoidBrdId,
		              "3303 must stay separate from the GOTO and ATTACK families");
		static_assert(kSkillFbId != kSkillId && kSkillBrdId != kSkillId &&
		                  kSkillFbId != kSkillBrdId,
		              "the feedback and the broadcast are different messages");

		// ---- application model ----------------------------------------------

		// One target entry: GLDefine.h:718. Four bytes, both halves 16-bit.
		struct SkillTarget
		{
			// `wCrow`. The peer-controlled half. `GetCROW`/`GetID` in legacy are
			// runtime decisions, and no one decodes this field in SKILL-021.
			WireU16 crow = 0;

			// `wID`. A TRUNCATED entity id - STARID holds a WORD where the runtime
			// holds a DWORD - and 0xFFFF is the wire sentinel for "no target".
			WireU16 id = 0;

			// Whether this entry is the wire's own null marker rather than an id.
			bool IsNull() const noexcept { return id == 0xFFFFu; }
		};

		// 3303's payload: the client's REQUEST to cast a skill.
		//
		// Everything here is what the client CLAIMS. Legacy compares most of it
		// against server-computed values and refuses the cast when they disagree
		// (GLCharSkillMsg.cpp:286-288 for the animation fields, :312-315 for the
		// range fields), and resolves the target list server-side whatever the
		// client sent. Nothing in this struct is authoritative.
		struct SkillRequest
		{
			// `skill_id` - SNATIVEID, 4 bytes, wMainID and wSubID as two halves of
			// the same 32-bit word.
			RanWire::NativeId skillId;

			// `vTARPOS` - where the client says it is aiming. Ground-validated only
			// for TAR_SPECIFIC skills in legacy (GLCharSkillMsg_SetTarget.cpp:777),
			// and echoed verbatim into the broadcast.
			RanWire::Vector3 targetPosition{};

			// `dwFlags`. Never read by legacy's MsgReqSkill at all. Carried.
			WireU32 flags = 0;

			// `bDefenseSkill` - a C++ bool, so ONE byte. Cross-checked against
			// the session's active defense skill twice (GLCharSkillMsg.cpp:248 and
			// :299).
			WireU8 defenseSkill = 0;

			// `fSum_TarRange` - an echoed range bonus.
			float sumTarRange = 0.0f;

			// The four echoed range words. Legacy rejects the cast unless all five
			// match its own getters exactly (GLCharSkillMsg.cpp:312-315).
			WireU16 bodyRadius       = 0;
			WireU16 attackRange      = 0;
			WireU16 skillRangeApply  = 0;
			WireU16 skillRangeTar    = 0;

			// `fAttVelo`, `fAttVeloItem` - echoed attack speeds.
			float attackVelocity     = 0.0f;
			float attackVelocityItem = 0.0f;

			// `dwAnimationKey` - echoed animation start time.
			WireU32 animationKey = 0;

			// `emMType`, `emSType` - EMANI_MAINTYPE and EMANI_SUBTYPE, four-byte
			// enums. Compared against the skill's own m_sEXT_DATA at
			// GLCharSkillMsg.cpp:286-288; the values are NEVER interpreted here.
			WireI32 animationMainType = 0;
			WireI32 animationSubType  = 0;

			// `wTARGET_NUM` exactly as it appeared on the wire.
			//
			// It is reported, not obeyed. `targets` below is
			// `min(this, capacityFromLength)`, and the two can differ - which is
			// the whole reason both are here. A peer that declares 24 in a 63-byte
			// frame gets `targets.size() == 0` and this field still reads 24, so
			// the discrepancy is visible instead of being papered over.
			WireU16 declaredTargetCount = 0;

			// How many STARIDs the validated LENGTH could hold, at most 24.
			//
			// Exposed so a caller can tell "the peer sent 24 targets" from "the
			// packet was long enough for 24 and the peer claimed 3".
			std::size_t targetCapacity = 0;

			// The targets that are both declared AND backed by bytes.
			//
			// Never longer than kMaxTargets, and never longer than what the
			// received bytes hold. This is the list a later milestone would
			// resolve against the world; SKILL-021 resolves none of them.
			std::vector<SkillTarget> targets;
		};

		// 3304's payload: the caster's own answer.
		struct SkillFeedback
		{
			// `emSKILL_FB`, an EMSKILLCHECK. Use SkillCheck:: for the values whose
			// meaning has been verified against legacy.
			WireI32 skillCheck = SkillCheck::kOk;

			// `szName` - EXACTLY kNameBytes raw bytes, carried verbatim.
			//
			// Not a C string and not guaranteed to be NUL-terminated: the legacy
			// field is char[33] for an id length of 33, and nothing writes a
			// terminator. std::string is used precisely because it carries its own
			// length, so nothing here can run off the end of an unterminated name.
			std::string name;
		};

		// 3305's payload: "this character cast that skill".
		struct SkillBroadcast
		{
			// `emCrow` - the CASTER's crow type, a four-byte EMCROW.
			//
			// Not the same width as a target's `wCrow`, which is a two-byte WORD.
			WireI32 crow = 0;

			// `dwID` - the caster's entity id, FULL 32 bits.
			WireU32 casterId = 0;

			RanWire::NativeId skillId;

			// `wLEVEL` - the SERVER's learned level, not a client claim
			// (GLCharSkillMsg.cpp:1005 reads it back from GETLEARNED_SKILL).
			WireU16 level = 0;

			// `vTARPOS` - the client's raw aim point, echoed unchanged.
			RanWire::Vector3 targetPosition{};

			WireU16 declaredTargetCount = 0;
			std::size_t targetCapacity  = 0;
			std::vector<SkillTarget>   targets;
		};

		namespace SkillCodec
		{
			// 3303. APPENDED to `out`. 63 + 4*targets.size() bytes.
			//
			// Refuses a target list longer than kMaxTargets rather than truncating
			// it: a caller that asked for 25 targets has a bug, and silently
			// dropping the 25th would hide it. Refuses a non-finite coordinate or
			// speed - see DecodeSkillRequest for why both directions refuse.
			Status AppendSkillRequest(std::vector<WireU8>& out, const SkillRequest& request);

			// 3303. Accepts any length of the form 63 + 4n for 0 <= n <= 24.
			//
			// Rejects 62, 65, 66 and 164 - the lengths that are either too short
			// for the fixed header or not congruent to 4 bytes over it.
			//
			// On refusal `out` is CLEARED, so a caller that ignores the status
			// cannot act on a half-decoded request.
			Status DecodeSkillRequest(const std::vector<WireU8>& frame, SkillRequest& out);

			// 3304. APPENDED. Exactly 45 bytes.
			//
			// `feedback.name` must be exactly kNameBytes; a shorter or longer name
			// is refused rather than padded or cut, because silently truncating a
			// character name would be a lie about what the server meant to send.
			Status AppendSkillFeedback(std::vector<WireU8>& out, const SkillFeedback& feedback);

			// 3304. Requires exactly 45 bytes with id 3304.
			Status DecodeSkillFeedback(const std::vector<WireU8>& frame, SkillFeedback& out);

			// 3305. APPENDED. 36 + 4*targets.size() bytes.
			Status AppendSkillBroadcast(std::vector<WireU8>& out, const SkillBroadcast& broadcast);

			// 3305. Accepts any length of the form 36 + 4n for 0 <= n <= 24.
			Status DecodeSkillBroadcast(const std::vector<WireU8>& frame, SkillBroadcast& out);

			// ---- predicates -----------------------------------------------------

			bool IsSkill(MessageId id) noexcept;
			bool IsSkillFeedback(MessageId id) noexcept;
			bool IsSkillBroadcast(MessageId id) noexcept;

			// Rebuilds the on-wire bytes of a message the framer has already split.
			//
			// Re-exported rather than reimplemented, for the reason
			// GotoProtocol.h gives: the framing boundary is Network::ReconstructFrame
			// and a second copy would be exactly the duplication this milestone is
			// told to avoid.
			using Network::ReconstructFrame;
		} // namespace SkillCodec
	}     // namespace Skill
} // namespace Modern::Network
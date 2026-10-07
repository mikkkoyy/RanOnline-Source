#pragma once

// WORLD-ENTRY-002H: the UPDATE_STATE protocol - 3046 and 3053.
//
// This is the RESOURCE half of the world's server->client contract.
// MovementStateProtocol.h (WORLD-ENTRY-002a) carries the movement STATE
// word, GotoProtocol.h (WORLD-ENTRY-002f) carries the position, and
// this pair carries the mutable resource pools - the values a client
// must be TOLD because they change for reasons it cannot compute.
//
//     UPDATE_STATE      3046 { sHP, sMP, sSP, sCP,
//                              szCharName, dwCharGaeaID, dwCharID,
//                              bSafeTime }        server -> own client
//     UPDATE_STATE_BRD  3053 { dwGaeaID, sHP, bSafeTime }
//                                                      server -> view-around
//
// ---------------------------------------------------------------------------
// THE FOUR POOLS, AND WHY sCP IS CARRIED
// ---------------------------------------------------------------------------
//
// SNETPC_UPDATE_STATE holds FOUR GLDWDATA pairs (GLContrlPcMsg.h:946):
// sHP, sMP, sSP and sCP. sCP is combat point, added by Juver in 2017,
// and nothing in the modern world model produces it - there is no combat
// in WORLD-ENTRY-002H. It is nevertheless carried, always {0,0}, for the
// same reason GotoProtocol.h carries a dead fDelay: it is four bytes of
// a fixed-size struct, and removing it would change the packet from 82
// bytes to 74 and desynchronise every RAN client. Dead on the wire, not
// absent from it.
//
// ---------------------------------------------------------------------------
// BOTH HALVES OF EVERY POOL CROSS
// ---------------------------------------------------------------------------
//
// A GLDWDATA is a dwNow/dwMax union (GLDefine.h:400), and 3046 carries
// BOTH halves of all four pools. That is what makes the packet a full
// state rather than a delta: the client learns the maxima as well as the
// currents, which is how a status window can show a bar without a second
// message. The modern CharacterSnapshot keeps maxima in `derived` and
// only the current in its pools, so the client-side seam applies the
// currents and exposes the maxima for assertion - see
// ClientCharacterState::ApplyResourceUpdate.
//
// ---------------------------------------------------------------------------
// MEASURED, NOT SUMMED
// ---------------------------------------------------------------------------
//
// Offsets come from the struct definitions under GLContrlPcMsg.h's
// `#pragma pack(1)` region (:355 to :4356) and GLContrlBaseMsg.h's
// (:272 to :392):
//
//     SNETPC_UPDATE_STATE      82   sHP@8  sMP@16  sSP@24  sCP@32
//                                   szCharName@40  dwCharGaeaID@73
//                                   dwCharID@77  bSafeTime@81
//     SNETPC_UPDATE_STATE_BRD  21   dwGaeaID@8  sHP@12  bSafeTime@20
//
// SNETPC_UPDATE_STATE embeds NET_MSG_GENERIC directly; SNETPC_UPDATE_STATE_BRD
// derives from SNETPC_BROAD (GLContrlBaseMsg.h:272), which is why dwGaeaID
// comes FIRST in the broadcast - base class first, exactly as C++
// inheritance lays it out.
//
// Legacy declarations:
//   s_NetGlobal.h:1013, :1021      NET_MSG_GCTRL_UPDATE_STATE, _BRD
//   GLContrlPcMsg.h:946-975        SNETPC_UPDATE_STATE
//   GLContrlPcMsg.h:976-989        SNETPC_UPDATE_STATE_BRD : SNETPC_BROAD
//   GLContrlBaseMsg.h:272-283      SNETPC_BROAD
//   GLDefine.h:400-413             GLDWDATA

#include "MessageReader.h"
#include "NetworkCodec.h"
#include "NetworkTypes.h"
#include "RanWirePrimitives.h"
#include "WorldEntryProtocol.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Modern::Network
{
	namespace UpdateState
	{
		// s_NetGlobal.h:1013 and :1021. Both are NET_MSG_GCTRL + n, and
		// Protocol::kGCtrlBase is NET_MSG_BASE + 1900 = 2892.
		constexpr MessageId kUpdateStateId    = Protocol::kGCtrlBase + 154; // 3046
		constexpr MessageId kUpdateStateBrdId = Protocol::kGCtrlBase + 161; // 3053

		static_assert(kUpdateStateId == 3046, "UPDATE_STATE is 3046");
		static_assert(kUpdateStateBrdId == 3053, "UPDATE_STATE_BRD is 3053");

		// ---- measured sizes and offsets -------------------------------------
		constexpr std::size_t kUpdateStateSize    = 82;
		constexpr std::size_t kUpdateStateBrdSize = 21;

		constexpr std::size_t kUpdateStateHpOffset        = 8;
		constexpr std::size_t kUpdateStateMpOffset        = 16;
		constexpr std::size_t kUpdateStateSpOffset        = 24;
		constexpr std::size_t kUpdateStateCpOffset        = 32;
		constexpr std::size_t kUpdateStateNameOffset      = 40;
		constexpr std::size_t kUpdateStateGaeaIdOffset    = 73;
		constexpr std::size_t kUpdateStateCharIdOffset    = 77;
		constexpr std::size_t kUpdateStateSafeTimeOffset  = 81;

		constexpr std::size_t kUpdateStateBrdGaeaIdOffset   = 8;
		constexpr std::size_t kUpdateStateBrdHpOffset       = 12;
		constexpr std::size_t kUpdateStateBrdSafeTimeOffset = 20;

		// CHAR_SZNAME is CHR_ID_LENGTH, 33 (s_NetGlobal.h:172,
		// GLCharDefine.h:17). The same bound WorldEntry::kNameFieldSize
		// already carries, restated here so this file's asserts cannot
		// drift from the protocol it shares a name field with.
		constexpr std::size_t kNameFieldSize = 33;

		static_assert(kNameFieldSize == WorldEntry::kNameFieldSize,
		              "CHAR_SZNAME is one name field everywhere");

		// A GLDWDATA on the wire is two DWORDs, dwNow then dwMax
		// (GLDefine.h:400-413). RanWire::DwPair is that exact type.
		constexpr std::size_t kPoolBytes = 8;

		// ---- wire structures, for the compiler to check ----------------------
		//
		// Declared so sizeof and offsetof can be ASSERTED. As in
		// GotoProtocol.h these are never memcpy'd onto the wire: the codec
		// reads and writes little-endian field by field.
#pragma pack(push, 1)
		struct StateUpdateWire
		{
			MessageHeader header;                     //   0
			WireU8        hp[kPoolBytes];             //   8
			WireU8        mp[kPoolBytes];             //  16
			WireU8        sp[kPoolBytes];             //  24
			WireU8        cp[kPoolBytes];             //  32
			WireU8        name[kNameFieldSize];       //  40
			WireU32       gaeaId;                     //  73
			WireU32       charId;                     //  77
			WireU8        safeTime;                   //  81
		};

		struct StateBroadcastWire
		{
			MessageHeader header;                     //   0
			WireU32       gaeaId;                     //   8
			WireU8        hp[kPoolBytes];             //  12
			WireU8        safeTime;                   //  20
		};
#pragma pack(pop)

		static_assert(sizeof(StateUpdateWire) == kUpdateStateSize,
		              "SNETPC_UPDATE_STATE must stay 82 bytes - measured, not summed");
		static_assert(offsetof(StateUpdateWire, hp) == kUpdateStateHpOffset,
		              "sHP at offset 8");
		static_assert(offsetof(StateUpdateWire, mp) == kUpdateStateMpOffset,
		              "sMP at offset 16");
		static_assert(offsetof(StateUpdateWire, sp) == kUpdateStateSpOffset,
		              "sSP at offset 24");
		static_assert(offsetof(StateUpdateWire, cp) == kUpdateStateCpOffset,
		              "sCP at offset 32");
		static_assert(offsetof(StateUpdateWire, name) == kUpdateStateNameOffset,
		              "szCharName at offset 40");
		static_assert(offsetof(StateUpdateWire, gaeaId) == kUpdateStateGaeaIdOffset,
		              "dwCharGaeaID at offset 73");
		static_assert(offsetof(StateUpdateWire, charId) == kUpdateStateCharIdOffset,
		              "dwCharID at offset 77");
		static_assert(offsetof(StateUpdateWire, safeTime) == kUpdateStateSafeTimeOffset,
		              "bSafeTime at offset 81");

		static_assert(sizeof(StateBroadcastWire) == kUpdateStateBrdSize,
		              "SNETPC_UPDATE_STATE_BRD must stay 21 bytes");
		static_assert(offsetof(StateBroadcastWire, gaeaId) == kUpdateStateBrdGaeaIdOffset,
		              "dwGaeaID at offset 8");
		static_assert(offsetof(StateBroadcastWire, hp) == kUpdateStateBrdHpOffset,
		              "sHP at offset 12");
		static_assert(offsetof(StateBroadcastWire, safeTime) == kUpdateStateBrdSafeTimeOffset,
		              "bSafeTime at offset 20");

		// The measured packing changes neither size: every member is 1, 4, 8
		// or 33 bytes, the 33-byte run starts at offset 40 and the two
		// DWORDs after it are consecutive. Asserted so a future "let us pack
		// it properly" change cannot silently alter the wire.
		static_assert(sizeof(StateUpdateWire) ==
		                  kMessageHeaderSize + 4 * kPoolBytes + kNameFieldSize +
		                      2 * sizeof(WireU32) + 1,
		              "UPDATE_STATE has no inter-member padding to remove");
		static_assert(sizeof(StateBroadcastWire) ==
		                  kMessageHeaderSize + sizeof(WireU32) + kPoolBytes + 1,
		              "UPDATE_STATE_BRD has no inter-member padding to remove");

		// ---- application model ----------------------------------------------

		// 3046's payload: the server's full authoritative resource state.
		//
		// `hp`, `mp`, `sp` are GLDWDATA pairs - `{dwNow, dwMax}` in
		// RanWire::DwPair's order. `cp` is the dead combat-point pool:
		// carried, always zero, never produced. `name`, `gaeaId` and
		// `charId` are the mismatch check GLCharacterMsg.cpp:57 performs on
		// receipt, so a client that receives a state for somebody else can
		// detect it rather than apply it. `safeTime` is the safe-zone flag.
		struct StateUpdate
		{
			RanWire::DwPair hp{};
			RanWire::DwPair mp{};
			RanWire::DwPair sp{};
			RanWire::DwPair cp{};

			std::string name;  // at most kNameFieldSize-1 characters
			WireU32     gaeaId = 0;
			WireU32     charId = 0;
			bool        safeTime = false;
		};

		// 3053's payload: the HP pair of ONE character, for everyone
		// watching it.
		//
		// `gaeaId` is whose HP this is - the sender's own entity id, read
		// off the server record, never a client-supplied value. The broadcast
		// carries HP only: legacy's SNETPC_UPDATE_STATE_BRD has no MP or SP,
		// because a watcher of another player tracks their health, not their
		// mana.
		struct StateBroadcast
		{
			WireU32         gaeaId = 0;
			RanWire::DwPair hp{};
			bool            safeTime = false;
		};

		namespace UpdateStateCodec
		{
			// 3046. APPENDED to `out`. Exactly 82 bytes.
			//
			// The pool values are written verbatim: a codec that quietly
			// reordered or clamped them would make the bytes differ from the
			// authoritative state, which is the wrong place for a rule - the
			// ResourceSyncService owns clamping, this owns the wire. A name
			// longer than the field is refused rather than silently truncated,
			// because a shortened name would fail the receiver's mismatch
			// check for a reason that is not the character's.
			Status AppendStateUpdate(std::vector<WireU8>& out,
			                         const StateUpdate& update);

			// 3046. Requires exactly 82 bytes with id 3046.
			Status DecodeStateUpdate(const std::vector<WireU8>& frame,
			                         StateUpdate& out);

			// 3053. APPENDED. Exactly 21 bytes.
			Status AppendStateBroadcast(std::vector<WireU8>& out,
			                            const StateBroadcast& broadcast);

			// 3053. Requires exactly 21 bytes with id 3053.
			Status DecodeStateBroadcast(const std::vector<WireU8>& frame,
			                            StateBroadcast& out);

			// ---- predicates ---------------------------------------------------
			bool IsStateUpdate(MessageId id) noexcept;
			bool IsStateBroadcast(MessageId id) noexcept;

			// Rebuilds the on-wire bytes of a message the framer has already
			// split. Re-exported rather than reimplemented, for the reason
			// GotoProtocol.h gives: the framing boundary is
			// Network::ReconstructFrame and a second copy would be exactly the
			// duplication this milestone is told to avoid.
			using Network::ReconstructFrame;
		}
	}
}

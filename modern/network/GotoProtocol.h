#pragma once

// WORLD-ENTRY-002f: the GOTO protocol - 3034 and 3035.
//
// This is the POSITION half of RAN's movement pair. MovementStateProtocol.h
// (WORLD-ENTRY-002a) implemented the STATE half, 3032 and 3033, and left this
// pair explicitly deferred to this milestone. Together:
//
//   MOVESTATE     3032 { dwActState }                          the STATE half
//   MOVESTATE_BRD 3033 { dwGaeaID, dwActState }
//   GOTO          3034 { dwActState, vCurPos, vTarPos }        the POSITION half
//   GOTO_BRD      3035 { dwGaeaID, dwActState, vCurPos,
//                         vTarPos, fDelay }
//
// ---------------------------------------------------------------------------
// 3034 CARRIES NO IDENTITY, AND THAT IS THE WHOLE AUTHORITY STORY
// ---------------------------------------------------------------------------
//
// SNETPC_GOTO has no dwGaeaID, no account, no name and no rotation
// (GLContrlPcMsg.h:636-654). It is 36 bytes of "here is where I think I am and
// here is where I want to be". The server learns whose character it is from the
// Field CONNECTION, which is why the 002a reasoning about 3032 applies unchanged
// here: the request cannot name a character, so it cannot forge one.
//
// The consequence for this layer is narrow and absolute: `GotoRequest` has no id
// field, because the wire has no id field. Adding one would be inventing
// protocol, not implementing it.
//
// ---------------------------------------------------------------------------
// vCurPos IS THE CLIENT'S CLAIM, NOT PERMISSION
// ---------------------------------------------------------------------------
//
// `vCurPos` exists in RAN for exactly one purpose: the desynchronisation check at
// GLCharMsg.cpp:264-288, which compares the client's claimed position against the
// server's authoritative `m_vPos` and refuses the request if they are more than
// 60 units apart. It is an INPUT TO VALIDATION.
//
// It is emphatically not a setter. Nothing in RAN assigns `m_vPos` from
// `pNetMsg->vCurPos`; the authoritative position advances only through
// `Actor::Update` (GLChar.cpp:6099, `m_vPos = m_actorMove.Position()`). A modern
// implementation that let a 3034 write the server position would turn an
// anti-teleport check into a teleport, and the 60-unit rule would become the
// teleport's range limit rather than its prohibition.
//
// ---------------------------------------------------------------------------
// vTarPos IS THE RAW REQUEST, AND THE BROADCAST CARRIES IT RAW TOO
// ---------------------------------------------------------------------------
//
// GLCharMsg.cpp:290 stores the client's target verbatim, and :315 broadcasts
// `NetMsgFB.vTarPos = m_TargetID.vPos` - the RAW value, not the point the ±10
// vertical probe actually resolved to. So peers are told where the mover ASKED to
// go while the server paths to a point that can be up to 10 units away in Y.
//
// That is reproduced. It looks like a bug and it is not this milestone's to fix:
// changing it would change what a real client is told, and a real client
// re-runs its own probe against the target it receives.
//
// ---------------------------------------------------------------------------
// fDelay STAYS ON THE WIRE
// ---------------------------------------------------------------------------
//
// 3034 has no `fDelay` and 3035 does, so the two messages are not a
// prefix-and-suffix pair and cannot be encoded by one function. `fDelay` is dead
// on the server path - `NetMsgFB.fDelay = 0.0f` at GLCharMsg.cpp:316 is the only
// assignment on this route - but it occupies four bytes of a fixed-size struct and
// removing it would change the packet from 44 bytes to 40 and desynchronise every
// RAN client. It is carried, always zero, and named in the layout.
//
// ---------------------------------------------------------------------------
// MEASURED, NOT SUMMED
// ---------------------------------------------------------------------------
//
// Offsets come from the struct definitions under GLContrlPcMsg.h's
// `#pragma pack(1)` region (:355 to :4356):
//
//     SNETPC_GOTO       36   dwActState@8  vCurPos@12  vTarPos@24
//     SNETPC_GOTO_BRD   44   dwGaeaID@8    dwActState@12  vCurPos@16
//                           vTarPos@28  fDelay@40
//
// SNETPC_GOTO does NOT derive from SNETPC_BROAD; it embeds NET_MSG_GENERIC
// directly, so it has no dwGaeaID. SNETPC_GOTO_BRD DOES derive from SNETPC_BROAD
// (GLContrlBaseMsg.h:270-283), which is why dwGaeaID comes FIRST - base class
// first, exactly as C++ inheritance lays it out.
//
// The static_asserts below make that a build-time fact rather than a comment, the
// same mechanism MovementStateProtocol.h uses for 3032/3033.
//
// Legacy declarations:
//   s_NetGlobal.h:1000-1001       NET_MSG_GCTRL_GOTO, NET_MSG_GCTRL_GOTO_BRD
//   GLContrlPcMsg.h:636-654       SNETPC_GOTO
//   GLContrlPcMsg.h:656-676       SNETPC_GOTO_BRD : SNETPC_BROAD
//   GLContrlBaseMsg.h:270-283     SNETPC_BROAD

#include "MessageReader.h"
#include "NetworkCodec.h"
#include "NetworkTypes.h"
#include "RanWirePrimitives.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Modern::Network
{
	namespace Goto
	{
		// s_NetGlobal.h:1000-1001. Both are NET_MSG_GCTRL + n, and
		// Protocol::kGCtrlBase is NET_MSG_BASE + 1900 = 2892.
		constexpr MessageId kGotoId    = Protocol::kGCtrlBase + 142; // 3034
		constexpr MessageId kGotoBrdId = Protocol::kGCtrlBase + 143; // 3035

		static_assert(kGotoId == 3034, "GOTO is 3034");
		static_assert(kGotoBrdId == 3035, "GOTO_BRD is 3035");

		// ---- measured sizes and offsets -------------------------------------
		constexpr std::size_t kRequestSize = 36;
		constexpr std::size_t kRequestActStateOffset      = 8;
		constexpr std::size_t kRequestCurrentPositionOffset = 12;
		constexpr std::size_t kRequestTargetPositionOffset  = 24;

		constexpr std::size_t kBroadcastSize = 44;
		constexpr std::size_t kBroadcastGaeaIdOffset        = 8;
		constexpr std::size_t kBroadcastActStateOffset      = 12;
		constexpr std::size_t kBroadcastCurrentPositionOffset = 16;
		constexpr std::size_t kBroadcastTargetPositionOffset  = 28;
		constexpr std::size_t kBroadcastDelayOffset           = 40;

		// A D3DXVECTOR3 on the wire is three IEEE-754 singles in x, y, z order.
		// Named once so the three places that need the width cannot disagree.
		constexpr std::size_t kVector3Bytes = 12;

		// A single IEEE-754 single, which is what Delay is.
		constexpr std::size_t kFloatBytes = 4;

		// ---- wire structures, for the compiler to check ----------------------
		//
		// Declared so sizeof and offsetof can be ASSERTED. As in
		// MovementStateProtocol.h these are never memcpy'd onto the wire: the codec
		// reads and writes little-endian field by field.
#pragma pack(push, 1)
		struct GotoRequestWire
		{
			MessageHeader header;  //   0
			WireU32       actState;    //   8
			WireU8        currentPosition[kVector3Bytes]; //  12
			WireU8        targetPosition[kVector3Bytes];  //  24
		};

		struct GotoBroadcastWire
		{
			MessageHeader header; //   0
			WireU32       gaeaId;      //   8
			WireU32       actState;    //  12
			WireU8        currentPosition[kVector3Bytes]; //  16
			WireU8        targetPosition[kVector3Bytes];  //  28
			WireU8        delay[kFloatBytes];           //  40
		};
#pragma pack(pop)

		static_assert(sizeof(GotoRequestWire) == kRequestSize,
		              "SNETPC_GOTO must stay 36 bytes - measured, not summed");
		static_assert(offsetof(GotoRequestWire, actState) == kRequestActStateOffset,
		              "dwActState at offset 8");
		static_assert(offsetof(GotoRequestWire, currentPosition) == kRequestCurrentPositionOffset,
		              "vCurPos at offset 12");
		static_assert(offsetof(GotoRequestWire, targetPosition) == kRequestTargetPositionOffset,
		              "vTarPos at offset 24");
		static_assert(sizeof(RanWire::Vector3) == kVector3Bytes,
		              "a wire vector is exactly a D3DXVECTOR3");

		static_assert(sizeof(GotoBroadcastWire) == kBroadcastSize,
		              "SNETPC_GOTO_BRD must stay 44 bytes");
		static_assert(offsetof(GotoBroadcastWire, gaeaId) == kBroadcastGaeaIdOffset,
		              "dwGaeaID at offset 8");
		static_assert(offsetof(GotoBroadcastWire, actState) == kBroadcastActStateOffset,
		              "dwActState at offset 12");
		static_assert(offsetof(GotoBroadcastWire, currentPosition) == kBroadcastCurrentPositionOffset,
		              "vCurPos at offset 16");
		static_assert(offsetof(GotoBroadcastWire, targetPosition) == kBroadcastTargetPositionOffset,
		              "vTarPos at offset 28");
		static_assert(offsetof(GotoBroadcastWire, delay) == kBroadcastDelayOffset,
		              "fDelay at offset 40");

		// The measured packing changes neither size: every member is 1, 4 or 12
		// bytes and the 12-byte runs start on a 4-byte boundary. Asserted so a future
		// "let us pack it properly" change cannot silently alter the wire.
		static_assert(sizeof(GotoRequestWire) ==
		                  kMessageHeaderSize + sizeof(WireU32) + 2 * kVector3Bytes,
		              "GOTO has no inter-member padding to remove");
		static_assert(sizeof(GotoBroadcastWire) ==
		                  kMessageHeaderSize + 2 * sizeof(WireU32) + 2 * kVector3Bytes +
		                      sizeof(float),
		              "GOTO_BRD has no inter-member padding to remove");

		// ---- application model ----------------------------------------------

		// 3034's payload. The client's REQUEST, not a position and not a state.
		struct GotoRequest
		{
			// `pNetMsg->dwActState`. Only EM_ACT_RUN is read out of it - see
			// GotoService, which reproduces the exact single-bit application at
			// GLCharMsg.cpp:254-262.
			WireU32 actState = 0;

			// `pNetMsg->vCurPos`: the client's CLAIM about where it is, used only by the
			// 60-unit desynchronisation check.
			RanWire::Vector3 currentPosition{};

			// `pNetMsg->vTarPos`: where the player clicked.
			RanWire::Vector3 targetPosition{};
		};

		// 3035's payload: the server's authoritative answer for an ACCEPTED GOTO.
		//
		// `actState` is the server's whole word AFTER the EM_ACT_RUN bit has been
		// applied, never the request - the same rule 3033 follows, and for the same
		// reason (a peer that only learned the delta would have to guess the
		// server-owned bits).
		//
		// `currentPosition` is the server's authoritative position at the moment of
		// acceptance, which is how a client that has drifted learns where the server
		// thinks it is.
		struct GotoBroadcast
		{
			WireU32 gaeaId = 0;
			WireU32 actState = 0;
			RanWire::Vector3 currentPosition{};
			RanWire::Vector3 targetPosition{};

			// Dead on this route - always 0.0f - and carried because it is four
			// bytes of a fixed-size struct. See the header.
			float delay = 0.0f;
		};

		namespace GotoCodec
		{
			// 3034. APPENDED to `out`. Exactly 36 bytes.
			//
			// Positions are written as three IEEE-754 singles in x, y, z order and are
			// NOT filtered: a codec that quietly altered a value would make the bytes
			// differ from what the caller asked to send, which is the wrong place for a
			// rule. Whether the destination is reachable is a navigation question, and
			// it is answered after decoding.
			//
			// A non-finite component IS refused, and that is not a rule but a
			// consequence: a NaN in vCurPos would make the 60-unit check compare
			// against NaN and silently pass, and a NaN in vTarPos would be handed to the
			// navigation tree. Both are refused rather than propagated.
			Status AppendGotoRequest(std::vector<WireU8>& out, const GotoRequest& request);

			// 3034. Requires exactly 36 bytes with id 3034.
			Status DecodeGotoRequest(const std::vector<WireU8>& frame, GotoRequest& out);

			// 3035. APPENDED. Exactly 44 bytes.
			Status AppendGotoBroadcast(std::vector<WireU8>& out, const GotoBroadcast& broadcast);

			// 3035. Requires exactly 44 bytes with id 3035.
			Status DecodeGotoBroadcast(const std::vector<WireU8>& frame, GotoBroadcast& out);

			// ---- predicates ---------------------------------------------------
			bool IsGoto(MessageId id) noexcept;
			bool IsGotoBroadcast(MessageId id) noexcept;

			// Rebuilds the on-wire bytes of a message the framer has already split.
			//
			// Re-exported rather than reimplemented, for the reason
			// MovementStateProtocol.h gives: the framing boundary is
			// Network::ReconstructFrame and a second copy would be exactly the
			// duplication this milestone is told to avoid.
			using Network::ReconstructFrame;
		}
	}
}

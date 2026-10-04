#pragma once

// WORLD-ENTRY-002a: the movement-STATE protocol - 3032 and 3033.
//
// THE SMALLEST REAL GAMEPLAY MESSAGE, AND WHY IT IS THE STATE HALF ONLY.
//
// Movement in RAN is two messages with two different jobs:
//
//   MOVESTATE 3032 { dwActState }                     the STATE half  - run vs
//   MOVESTATE_BRD 3033 { dwGaeaID, dwActState }          walk, peace mode, visibility
//   GOTO      3034 { dwActState, vCurPos, vTarPos }   the POSITION half - destination
//   GOTO_BRD  3035 { dwGaeaID, dwActState, vCurPos,
//                     vTarPos, fDelay }
//
// This file implements the first pair ONLY. GOTO, and the position movement behind
// it, are deferred to WORLD-ENTRY-002b: `Actor::Update` returns E_FAIL without a
// NavigationMesh and every position change routes through
// `m_Parent->ResolveMotionOnMesh`, so faithful position movement needs the navmesh
// subsystem and map data that is not in this repository. See
// docs/reference/WORLD-ENTRY-002-MOVEMENT-INVESTIGATION.md.
//
// A MOVESTATE changes NO COORDINATE. x/y/z are untouched by this protocol and by
// this milestone, and saying so here is the point: a 3032 that moved the character
// would be a behaviour RAN does not have.
//
// ---------------------------------------------------------------------------
// MEASURED, NOT SUMMED
// ---------------------------------------------------------------------------
//
// Sizes and offsets come from the WORLD-ENTRY-002 probe, which compiled the real
// struct definitions under a reconstruction of the legacy packing context:
//
//     SNETPC_MOVESTATE     12   actState@8
//     SNETPC_MOVESTATE_BRD 16   gaeaID@8  actState@12
//
// Both live inside GLContrlPcMsg.h's `#pragma pack(1)` region (:355 to :4356), and
// SNETPC_MOVESTATE_BRD DERIVES from SNETPC_BROAD (GLContrlBaseMsg.h:274). The probe
// reproduced that inheritance and found packing changes NEITHER size - every member
// is 4 bytes and naturally aligned - which is recorded because Phase A was burned
// by assuming the opposite for SITEM_LOBY.
//
// The static_asserts below are what make that a build-time fact rather than a
// comment. They are the same mechanism Phase A used.
//
// Legacy declarations:
//   GLContrlPcMsg.h:700  SNETPC_MOVESTATE
//   GLContrlPcMsg.h:717  SNETPC_MOVESTATE_BRD : SNETPC_BROAD
//   GLContrlBaseMsg.h:274  SNETPC_BROAD
//   s_NetGlobal.h:997-998  NET_MSG_GCTRL_MOVESTATE, NET_MSG_GCTRL_MOVESTATE_BRD

#include "MessageReader.h"
#include "NetworkCodec.h"
#include "NetworkTypes.h"
#include "types/Result.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Modern::Network
{
	namespace MovementState
	{
		// s_NetGlobal.h:997-998. Both are NET_MSG_GCTRL + n, and Protocol::kGCtrlBase is
		// NET_MSG_BASE + 1900 = 2892 (s_NetGlobal.h:653 and the country chain).
		constexpr MessageId kMoveStateId   = Protocol::kGCtrlBase + 140; // 3032
		constexpr MessageId kMoveStateBrdId = Protocol::kGCtrlBase + 141; // 3033

		static_assert(kMoveStateId == 3032, "MOVESTATE is 3032");
		static_assert(kMoveStateBrdId == 3033, "MOVESTATE_BRD is 3033");

		// ---- measured sizes and offsets -------------------------------------
		constexpr std::size_t kRequestSize    = 12;
		constexpr std::size_t kRequestActStateOffset = 8;
		constexpr std::size_t kBroadcastSize = 16;
		constexpr std::size_t kBroadcastGaeaIdOffset   = 8;
		constexpr std::size_t kBroadcastActStateOffset = 12;

		// ---------------------------------------------------------------------------
		// EMCHAR_ACTSTATE - GLCharDefine.h:1156-1175, transcribed.
		// ---------------------------------------------------------------------------
		//
		// The wire representation is a BITMASK and it stays one. A readable enum would
		// be convenient and wrong: `dwActState` is a persistent word that other server
		// systems own bits of, and a MOVESTATE applies or clears individual bits rather
		// than assigning a value. Modelling it as an enum would make "the client sent
		// WALK" look like it could clobber EM_ACT_DIE, which it cannot.
		//
		// Which bits a CLIENT may influence:
		//
		//   EM_ACT_RUN, EM_ACT_PEACEMODE              always
		//   EM_REQ_VISIBLENONE, EM_REQ_VISIBLEOFF     only at account level >= USER_GM3
		//
		// Every other bit is SERVER-owned (death, gate-out, logout, view-around) and a
		// MOVESTATE neither sets nor clears it. Legacy simply never looks at those
		// bits, so a client that puts EM_ACT_DIE in its request is IGNORED, not
		// rejected - see MovementStateService, which reproduces that exactly.
		constexpr WireU32 kActRun            = 0x00000001u;
		constexpr WireU32 kActContinueMove   = 0x00000002u;
		constexpr WireU32 kActPeaceMode      = 0x00000004u;
		constexpr WireU32 kGetVaAfter        = 0x00000010u;
		constexpr WireU32 kActWaiting        = 0x00000020u;
		constexpr WireU32 kActDie            = 0x00000040u;
		constexpr WireU32 kReqGateOut        = 0x00000100u;
		constexpr WireU32 kReqLogout         = 0x00000200u;
		constexpr WireU32 kReqVisibleNone    = 0x00001000u;
		constexpr WireU32 kReqVisibleOff     = 0x00002000u;
		constexpr WireU32 kActConftWin       = 0x00010000u;
		constexpr WireU32 kActPkMode         = 0x00100000u;
		constexpr WireU32 kActVehicleBooster = 0x01000000u;

		// Every bit legacy defines. Used to police the AUTHORITATIVE word, which must
		// never contain a bit the server does not know about - that would be
		// corruption, not a client request.
		constexpr WireU32 kKnownFlags =
		    kActRun | kActContinueMove | kActPeaceMode | kGetVaAfter | kActWaiting |
		    kActDie | kReqGateOut | kReqLogout | kReqVisibleNone | kReqVisibleOff |
		    kActConftWin | kActPkMode | kActVehicleBooster;

		// s_NetGlobal.h:313. USER_GM3 = 20, and the ordering is inverted -
		// USER_GM4 is 19, USER_GM1 is 22 - so this is a THRESHOLD, not a rank.
		constexpr WireU32 kUserGm3Level = 20;

		static_assert((kKnownFlags & ~kKnownFlags) == 0, "the flag mask is self-consistent");
		static_assert((kActRun & kActPeaceMode) == 0, "the client-owned flags are distinct");

		// ---- wire structures, for the compiler to check ----------------------
		//
		// Declared so sizeof and offsetof can be ASSERTED. The codec reads and writes
		// fields individually and little-endian, in keeping with the rest of
		// ModernNetwork: no native struct is ever memcpy'd onto the wire, so these
		// exist to be CHECKED rather than to be trusted.
#pragma pack(push, 1)
		struct MoveStateRequestWire
		{
			MessageHeader header; //   0
			WireU32       actState; //   8
		};

		struct MoveStateBroadcastWire
		{
			MessageHeader header;  //   0
			WireU32       gaeaId;   //   8
			WireU32       actState; //  12
		};
#pragma pack(pop)

		static_assert(sizeof(MoveStateRequestWire) == kRequestSize,
		              "SNETPC_MOVESTATE must stay 12 bytes - measured, not summed");
		static_assert(offsetof(MoveStateRequestWire, actState) == kRequestActStateOffset,
		              "dwActState at offset 8");
		static_assert(sizeof(MoveStateBroadcastWire) == kBroadcastSize,
		              "SNETPC_MOVESTATE_BRD must stay 16 bytes");
		static_assert(offsetof(MoveStateBroadcastWire, gaeaId) == kBroadcastGaeaIdOffset,
		              "dwGaeaID at offset 8");
		static_assert(offsetof(MoveStateBroadcastWire, actState) == kBroadcastActStateOffset,
		              "dwActState at offset 12");

		// The measured legacy packing does not change these sizes. Asserted so that a
		// future "let us pack it properly" change cannot silently alter the wire.
		static_assert(sizeof(MoveStateRequestWire) ==
		              sizeof(MessageHeader) + sizeof(WireU32),
		              "MOVESTATE has no inter-member padding to remove");
		static_assert(sizeof(MoveStateBroadcastWire) ==
		              sizeof(MessageHeader) + sizeof(WireU32) * 2,
		              "MOVESTATE_BRD has no inter-member padding to remove");

		// ---- application model ----------------------------------------------

		// 3032's payload: the movement state the client is asking for.
		//
		// It is a REQUEST for a state, not a state. The server derives the authoritative
		// word from this and from the character's existing state, and it is that
		// derived word - not this one - that goes into 3033.
		struct MoveStateRequest
		{
			WireU32 actState = 0;
		};

		// 3033's payload: the server's authoritative answer.
		//
		// Carries the FULL authoritative actState word, exactly as legacy does
		// (`NetMsgFB.dwActState = m_dwActState`, GLCharMsg.cpp:214) - not just the
		// bits this request touched. A client that only learned the delta would have
		// to guess the rest, and the rest includes server-owned bits.
		struct MoveStateBroadcast
		{
			WireU32 gaeaId   = 0;
			WireU32 actState = 0;
		};

		namespace MovementStateCodec
		{
			// 3032. APPENDED to `out`. Exactly 12 bytes.
			//
			// `actState` is NOT filtered here. A request carrying a server-owned bit is
			// refused by the service's rules, not silently rewritten by the codec,
			// because a codec that quietly altered a value would make the bytes on the
			// wire differ from what the caller asked to send.
			Status AppendMoveStateRequest(std::vector<WireU8>& out,
			                             const MoveStateRequest& request);

			// 3032. Requires exactly 12 bytes with id 3032.
			Status DecodeMoveStateRequest(const std::vector<WireU8>& frame,
			                              MoveStateRequest& out);

			// 3033. APPENDED. Exactly 16 bytes.
			Status AppendMoveStateBroadcast(std::vector<WireU8>& out,
			                                const MoveStateBroadcast& broadcast);

			// 3033. Requires exactly 16 bytes with id 3033.
			Status DecodeMoveStateBroadcast(const std::vector<WireU8>& frame,
			                               MoveStateBroadcast& out);

			// ---- predicates ---------------------------------------------------
			bool IsMoveState(MessageId id) noexcept;
			bool IsMoveStateBroadcast(MessageId id) noexcept;

			// Rebuilds the on-wire bytes of a message the framer has already split.
			//
			// Re-exported rather than reimplemented: the Phase C framing boundary is
			// Network::ReconstructFrame, and a second copy of it here would be exactly
			// the duplication this milestone is told to avoid.
			using Network::ReconstructFrame;
		}
	}
}
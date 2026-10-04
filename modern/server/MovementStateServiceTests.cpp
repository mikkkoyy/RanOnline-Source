// WORLD-ENTRY-002a: the authoritative movement-STATE rule, as a pure function.
//
// No socket opens in this file. 3032's meaning is decided by MovementStateService
// before any byte is framed and after any byte is read, so testing it here tests
// the rule itself rather than a transport. WorldEntryTcpTests proves the same rule
// survives a real socket; this file is where a wrong BIT is caught.
//
// The cases are grouped by the property they defend:
//
//   application  a request sets and clears exactly the bits a client owns
//   authority    bits the client does not own are preserved, not cleared
//   privilege    the two visibility flags are gated on ACCOUNT level
//   change       an unchanged word produces no change, which is observable
//   speed        the seam is named, and inventing a number is refused
//   encoding     the wire layout the codec promises is the layout it writes
//
// The authority cases are the security-relevant ones. "A client cannot set EM_ACT_DIE
// by putting it in a 3032" is only a real guarantee if the server's own bits SURVIVE
// the request, so every authority case checks the whole resulting word and not merely
// the bit under test.

#include "TestHarness.h"

#include "MovementStateProtocol.h"
#include "world/MovementStateService.h"
#include "world/WorldCharacter.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace
{
	using namespace Modern;
	using namespace Modern::Network;
	using namespace Modern::Server::World;
	namespace MS = Modern::Network::MovementState;

	// ---- fixtures ----------------------------------------------------------

	WorldCharacter MakeCharacter(WireU32 id, WireU16 level = 10)
	{
		WorldCharacter character;
		character.id             = WorldCharacterId{id};
		character.accountId      = WorldAccountId{1};
		character.userId         = "user_1";
		character.name           = "Alpha";
		character.characterClass = 3;
		character.school         = 1;
		character.level          = level;

		character.hp = {300u * level, 300u * level};
		character.mp = {150u * level, 150u * level};
		character.sp = {80u * level, 80u * level};

		character.saveMapId.value = 7u;
		character.savePosition   = {100.5f, -20.25f, 3.75f};
		character.gaeaId         = id;
		// accountLevel defaults to 0: an ordinary account, below USER_GM3.
		return character;
	}

	// A hand-rolled little-endian read, so the byte assertions are INDEPENDENT of the
	// codec that wrote the frame.
	WireU32 PeekLE32(const std::vector<WireU8>& frame, std::size_t offset)
	{
		WireU32 value = 0;
		for (std::size_t i = 0; i < 4; ++i)
		{
			value |= static_cast<WireU32>(frame[offset + i]) << (8 * i);
		}
		return value;
	}

	// A speed provider that reports a fixed value, so the seam can be shown to be
	// consulted rather than merely declared.
	class FixedSpeedProvider final : public IMovementSpeedProvider
	{
	public:
		float MaxSpeedFor(const WorldCharacter&) const override
		{
			++calls;
			return 4.5f;
		}

		mutable int calls = 0;
	};

	// =========================================================================
	// Message ids and sizes, measured
	// =========================================================================

	// The ids are arithmetic in this file (kGCtrlBase + 140/141), so this is what
	// stops that arithmetic from being quietly wrong.
	MODERN_TEST(MovementState_MessageIdsAre3032And3033)
	{
		CHECK_EQ(MS::kMoveStateId, static_cast<MessageId>(3032));
		CHECK_EQ(MS::kMoveStateBrdId, static_cast<MessageId>(3033));

		// Packed sizes measured from the legacy headers, and the two differ.
		CHECK_EQ(MS::kRequestSize, static_cast<std::size_t>(12));
		CHECK_EQ(MS::kBroadcastSize, static_cast<std::size_t>(16));

		CHECK(MS::MovementStateCodec::IsMoveState(3032));
		CHECK(!MS::MovementStateCodec::IsMoveState(3033));
		CHECK(MS::MovementStateCodec::IsMoveStateBroadcast(3033));
		CHECK(!MS::MovementStateCodec::IsMoveStateBroadcast(3032));

		// The neighbouring ids are WORLD-ENTRY-002b and must not be mistaken for
		// movement state.
		CHECK(!MS::MovementStateCodec::IsMoveState(3034));
		CHECK(!MS::MovementStateCodec::IsMoveStateBroadcast(3035));
	}

	// =========================================================================
	// Encoding
	// =========================================================================

	MODERN_TEST(MovementState_RequestEncodesToTwelveBytesWithActStateAtOffsetEight)
	{
		std::vector<WireU8> frame;
		CHECK(MS::MovementStateCodec::AppendMoveStateRequest(
		          frame, MS::MoveStateRequest{MS::kActRun | MS::kActPeaceMode}).IsOk());

		CHECK_EQ(frame.size(), static_cast<std::size_t>(12));
		// NET_MSG_GENERIC order: dwSize first, then nType. Asserted separately rather
		// than as one comparison so a swap reports which field moved.
		CHECK_EQ(PeekLE32(frame, 0), static_cast<WireU32>(12));
		CHECK_EQ(PeekLE32(frame, 4), static_cast<WireU32>(3032));
		CHECK_EQ(PeekLE32(frame, MS::kRequestActStateOffset),
		         static_cast<WireU32>(MS::kActRun | MS::kActPeaceMode));
	}

	// The request carries NO gaeaId, and this is why: the server already knows whose
	// connection a 3032 arrived on. A codec that added an id field would create a
	// second, client-supplied claim on identity - and the server would then have two
	// ids to choose between.
	//
	// The whole packet is therefore an 8-byte header plus ONE dword, which is the
	// structural form of that claim. sizeof is compared against kRequestSize rather
	// than against kRequestSize minus a header, because sizeof(MessageHeader) is 8
	// only by padding and the wire header is 8 by measurement - two numbers that agree
	// here for unrelated reasons.
	MODERN_TEST(MovementState_RequestCarriesNoIdentity)
	{
		CHECK_EQ(MS::kRequestSize - MS::kRequestActStateOffset, static_cast<std::size_t>(4));
		CHECK_EQ(sizeof(MS::MoveStateRequestWire), MS::kRequestSize);
		CHECK_EQ(sizeof(MS::MoveStateBroadcastWire), MS::kBroadcastSize);
	}

	MODERN_TEST(MovementState_BroadcastRoundTrips)
	{
		const MS::MoveStateBroadcast original{77u, MS::kActRun};

		std::vector<WireU8> frame;
		CHECK(MS::MovementStateCodec::AppendMoveStateBroadcast(frame, original).IsOk());
		CHECK_EQ(frame.size(), static_cast<std::size_t>(16));
		CHECK_EQ(PeekLE32(frame, 0), static_cast<WireU32>(16));
		CHECK_EQ(PeekLE32(frame, 4), static_cast<WireU32>(3033));
		CHECK_EQ(PeekLE32(frame, MS::kBroadcastGaeaIdOffset), static_cast<WireU32>(77));
		CHECK_EQ(PeekLE32(frame, MS::kBroadcastActStateOffset),
		         static_cast<WireU32>(MS::kActRun));

		MS::MoveStateBroadcast decoded;
		CHECK(MS::MovementStateCodec::DecodeMoveStateBroadcast(frame, decoded).IsOk());
		CHECK_EQ(decoded.gaeaId, original.gaeaId);
		CHECK_EQ(decoded.actState, original.actState);
	}

	// A wrong-sized frame is refused at the boundary rather than partially read. A
	// 3033 whose id field is present but whose actState is truncated would otherwise
	// decode as actState = 0 - which is a VALID word, and would silently tell every
	// client that the player stopped.
	MODERN_TEST(MovementState_MalformedFramesAreRefused)
	{
		MS::MoveStateRequest request;
		MS::MoveStateBroadcast broadcast;

		// Too short, and too long.
		CHECK(MS::MovementStateCodec::DecodeMoveStateRequest(
		          std::vector<WireU8>(11, 0), request).IsError());
		CHECK(MS::MovementStateCodec::DecodeMoveStateRequest(
		          std::vector<WireU8>(13, 0), request).IsError());
		CHECK(MS::MovementStateCodec::DecodeMoveStateBroadcast(
		          std::vector<WireU8>(15, 0), broadcast).IsError());
		CHECK(MS::MovementStateCodec::DecodeMoveStateBroadcast(
		          std::vector<WireU8>(17, 0), broadcast).IsError());

		// The right length but the wrong id.
		std::vector<WireU8> wrongId(16, 0);
		wrongId[0] = 4; // low byte of 3033 swapped for 3032
		CHECK(MS::MovementStateCodec::DecodeMoveStateBroadcast(wrongId, broadcast).IsError());
	}

	// =========================================================================
	// Application: the bits a client owns
	// =========================================================================

	MODERN_TEST(MovementState_RunAndPeaceModeAreSetAndCleared)
	{
		const MovementStateService service;

		{
			WorldCharacter character = MakeCharacter(11);
			MovementStateChange change;
			CHECK(service.ApplyMoveState(character, MS::kActRun, change).IsOk());

			CHECK(change.changed);
			CHECK_EQ(character.actState, static_cast<WireU32>(MS::kActRun));
			CHECK_EQ(change.actState, static_cast<WireU32>(MS::kActRun));
			CHECK_EQ(change.previousActState, static_cast<WireU32>(0));
			CHECK_EQ(change.gaeaId, static_cast<WireU32>(11));
		}

		// Clearing: a request with the bit ABSENT clears it, rather than leaving it.
		{
			WorldCharacter character = MakeCharacter(11);
			character.actState        = MS::kActRun | MS::kActPeaceMode;

			MovementStateChange change;
			CHECK(service.ApplyMoveState(character, 0, change).IsOk());

			CHECK(change.changed);
			CHECK_EQ(character.actState, static_cast<WireU32>(0));
			CHECK_EQ(change.previousActState,
			         static_cast<WireU32>(MS::kActRun | MS::kActPeaceMode));
		}

		// And setting both at once.
		{
			WorldCharacter character = MakeCharacter(11);
			MovementStateChange change;
			CHECK(service
			          .ApplyMoveState(character,
			                          MS::kActRun | MS::kActPeaceMode,
			                          change)
			          .IsOk());
			CHECK_EQ(character.actState,
			         static_cast<WireU32>(MS::kActRun | MS::kActPeaceMode));
		}
	}

	// =========================================================================
	// Authority: the bits a client does NOT own
	// =========================================================================

	// The core security property. Every server-owned bit is preserved through a
	// request that does not mention it, so a client cannot clear "dead" or "gate out"
	// by omission - which is the direction a client would actually exploit.
	MODERN_TEST(MovementState_ServerOwnedBitsArePreservedNotCleared)
	{
		const MovementStateService service;

		const WireU32 serverOwned = MS::kActDie | MS::kActWaiting | MS::kReqGateOut |
		                            MS::kReqLogout | MS::kActConftWin | MS::kActPkMode |
		                            MS::kActVehicleBooster | MS::kGetVaAfter |
		                            MS::kActContinueMove;

		WorldCharacter character = MakeCharacter(11);
		character.actState        = serverOwned;

		MovementStateChange change;
		CHECK(service.ApplyMoveState(character, MS::kActRun, change).IsOk());

		// The whole word, not just the bit under test.
		CHECK_EQ(character.actState, static_cast<WireU32>(serverOwned | MS::kActRun));
		CHECK_EQ(change.actState, character.actState);
		CHECK_EQ(change.previousActState, serverOwned);

		// And a request asking for them by name is simply ignored: the server never
		// reads those bits, so asking changes nothing.
		WorldCharacter second = MakeCharacter(11);
		second.actState        = 0;
		MovementStateChange secondChange;
		CHECK(service
		          .ApplyMoveState(second,
		                          MS::kActDie | MS::kReqLogout | MS::kActPkMode,
		                          secondChange)
		          .IsOk());
		CHECK_EQ(second.actState, static_cast<WireU32>(0));
		// Nothing changed, so nothing is broadcast - a client asking for "dead"
		// produces silence, which is the correct answer.
		CHECK(!secondChange.changed);
	}

	// Bits RAN does not define are ignored rather than refused. Legacy has no
	// validation here at all, and a client setting a bit this server has no opinion
	// about has not made a protocol error - it has made a request with no answer.
	MODERN_TEST(MovementState_UndefinedBitsAreIgnoredNotRefused)
	{
		const MovementStateService service;

		WorldCharacter character = MakeCharacter(11);
		character.actState        = MS::kActRun;

		const WireU32 undefined = 0x80000000u | 0x40000000u | 0x00000800u;

		MovementStateChange change;
		CHECK(service.ApplyMoveState(character, undefined, change).IsOk());

		// The client's bits are gone (it asked for none of the four) and the
		// undefined ones were never stored.
		CHECK_EQ(character.actState, static_cast<WireU32>(0));
		CHECK_EQ(change.actState & undefined, static_cast<WireU32>(0));
	}

	// =========================================================================
	// Privilege: the USER_GM3 gate on the visibility flags
	// =========================================================================

	// Below USER_GM3 the two visibility flags are left ALONE - neither set nor
	// cleared. "Ignored" and "cleared" are different, and only the first matches
	// legacy: the tests are inside the level gate precisely so a normal client
	// cannot unset a flag the server set.
	MODERN_TEST(MovementState_VisibilityFlagsAreIgnoredBelowUserGm3)
	{
		const MovementStateService service;

		// Asking to SET them, from clear. Nothing happens.
		{
			WorldCharacter character = MakeCharacter(11);
			character.accountLevel    = MS::kUserGm3Level - 1;

			MovementStateChange change;
			CHECK(service
			          .ApplyMoveState(character,
			                          MS::kReqVisibleNone | MS::kReqVisibleOff,
			                          change)
			          .IsOk());
			CHECK_EQ(character.actState, static_cast<WireU32>(0));
			CHECK(!change.changed);
		}

		// Asking to CLEAR them, from set. Also nothing happens - and this is the
		// direction that matters, because "cannot unset" is the whole point.
		{
			WorldCharacter character = MakeCharacter(11);
			character.accountLevel    = 0;
			character.actState = MS::kReqVisibleNone | MS::kReqVisibleOff | MS::kActRun;

			MovementStateChange change;
			CHECK(service.ApplyMoveState(character, MS::kActRun, change).IsOk());

			CHECK_EQ(character.actState,
			         static_cast<WireU32>(MS::kReqVisibleNone | MS::kReqVisibleOff |
			                               MS::kActRun));
		}
	}

	MODERN_TEST(MovementState_VisibilityFlagsAreClientOwnedAtOrAboveUserGm3)
	{
		const MovementStateService service;

		// Exactly at the gate, which is where an off-by-one would hide.
		{
			WorldCharacter character = MakeCharacter(11);
			character.accountLevel    = MS::kUserGm3Level;

			MovementStateChange change;
			CHECK(service
			          .ApplyMoveState(character,
			                          MS::kReqVisibleNone | MS::kActRun,
			                          change)
			          .IsOk());
			CHECK_EQ(character.actState,
			         static_cast<WireU32>(MS::kReqVisibleNone | MS::kActRun));
		}

		// And they can be cleared again, by a request that omits them.
		{
			WorldCharacter character = MakeCharacter(11);
			character.accountLevel    = MS::kUserGm3Level + 5;
			character.actState = MS::kReqVisibleNone | MS::kReqVisibleOff | MS::kActRun;

			MovementStateChange change;
			CHECK(service.ApplyMoveState(character, MS::kActRun, change).IsOk());
			CHECK_EQ(character.actState, static_cast<WireU32>(MS::kActRun));
		}
	}

	// The gate is on ACCOUNT level, not character level. A level-99 character on an
	// ordinary account must still be refused the GM flags - conflating the two would
	// hand GM visibility to anyone who levelled.
	MODERN_TEST(MovementState_TheGateIsAccountLevelNotCharacterLevel)
	{
		const MovementStateService service;

		WorldCharacter character = MakeCharacter(11);
		character.level          = 99;
		character.accountLevel    = 0;

		MovementStateChange change;
		CHECK(service
		          .ApplyMoveState(character, MS::kReqVisibleNone | MS::kReqVisibleOff, change)
		          .IsOk());

		CHECK_EQ(character.actState, static_cast<WireU32>(0));
	}

	// =========================================================================
	// Change detection
	// =========================================================================

	// Legacy compares the whole word before and after (GLCharMsg.cpp:186,203). A
	// request that asks for the state the character is already in changes nothing,
	// and so produces NO broadcast - observable behaviour, not an optimisation.
	MODERN_TEST(MovementState_AnUnchangedWordReportsNoChange)
	{
		const MovementStateService service;

		WorldCharacter character = MakeCharacter(11);
		character.actState        = MS::kActRun;

		MovementStateChange change;
		CHECK(service.ApplyMoveState(character, MS::kActRun, change).IsOk());

		CHECK(!change.changed);
		CHECK_EQ(change.actState, static_cast<WireU32>(MS::kActRun));
		CHECK_EQ(change.previousActState, static_cast<WireU32>(MS::kActRun));
	}

	// The comparison is of the WHOLE word, so a request that changes a client-owned
	// bit while a server-owned bit sits in the word is still a change.
	MODERN_TEST(MovementState_AnyBitMovingIsAChange)
	{
		const MovementStateService service;

		WorldCharacter character = MakeCharacter(11);
		character.actState        = MS::kActRun | MS::kActDie;

		MovementStateChange change;
		CHECK(service.ApplyMoveState(character, MS::kActRun | MS::kActPeaceMode, change)
		          .IsOk());

		CHECK(change.changed);
		CHECK_EQ(character.actState,
		         static_cast<WireU32>(MS::kActRun | MS::kActPeaceMode | MS::kActDie));
	}

	// =========================================================================
	// The speed seam
	// =========================================================================

	// No provider: the change still applies and the broadcast still goes out, and the
	// reason the speed is missing is RECORDED rather than left to be inferred. A
	// silent 0.0 would be indistinguishable from "this character really is that
	// slow".
	MODERN_TEST(MovementState_WithoutAProviderTheChangeStillAppliesAndTheGapIsReported)
	{
		const MovementStateService service;
		CHECK(!service.HasSpeedProvider());

		WorldCharacter character = MakeCharacter(11);
		MovementStateChange change;
		CHECK(service.ApplyMoveState(character, MS::kActRun, change).IsOk());

		CHECK(change.changed);
		CHECK_EQ(character.actState, static_cast<WireU32>(MS::kActRun));
		CHECK(!change.speedRecomputed);
		CHECK(change.speedUnavailable);
	}

	// With a provider it is consulted exactly once per CHANGE - and not at all for an
	// unchanged word, because legacy only recomputes inside the `if changed` block.
	MODERN_TEST(MovementState_AProviderIsConsultedOnceAndOnlyOnAChange)
	{
		FixedSpeedProvider provider;
		const MovementStateService service(&provider);
		CHECK(service.HasSpeedProvider());

		{
			WorldCharacter character = MakeCharacter(11);
			MovementStateChange change;
			CHECK(service.ApplyMoveState(character, MS::kActRun, change).IsOk());
			CHECK(change.speedRecomputed);
			CHECK(!change.speedUnavailable);
			CHECK_EQ(provider.calls, 1);
		}

		{
			// Same state again: no change, so no recomputation.
			WorldCharacter character = MakeCharacter(11);
			character.actState        = MS::kActRun;

			MovementStateChange change;
			CHECK(service.ApplyMoveState(character, MS::kActRun, change).IsOk());
			CHECK(!change.changed);
			CHECK(!change.speedRecomputed);
			CHECK_EQ(provider.calls, 1);
		}
	}

	// =========================================================================
	// Corrupt server state
	// =========================================================================

	// A character whose EXISTING word contains bits outside EMCHAR_ACTSTATE is
	// refused. It is server state rather than a request, so masking it would hide a
	// bug; and refusing means the operator finds out instead of the character
	// mysteriously losing flags.
	MODERN_TEST(MovementState_ACharacterWithACorruptExistingWordIsRefused)
	{
		const MovementStateService service;

		WorldCharacter character = MakeCharacter(11);
		character.actState        = 0x80000000u;

		MovementStateChange change;
		CHECK(service.ApplyMoveState(character, MS::kActRun, change).IsError());

		// Untouched: a refused apply must not half-modify the record.
		CHECK_EQ(character.actState, static_cast<WireU32>(0x80000000u));
	}
}
#pragma once

// VERTICAL-004: what a codex entry is.
//
// This is the definition half: the immutable data an entry carries, shared by
// every character who has it in progress. A character's own progress and
// completed set live in CodexState; this type is what CodexState is checked
// against and what the contribution aggregator reads.
//
// Legacy provenance: SCODEX_FILE_DATA (GLCodexData.h:26), the entry list
// `GLCodex::Import` reads (GLCodex.cpp:34-93).
//
// What the legacy definition actually drives, and what it does not, is the
// single most important finding in this milestone. RAN declares eleven
// EMCODEX_TYPE values and a per-type progress target for each - a level, a mob
// kill count, a quest, a question box - but every one of those code paths is
// commented out in the shipped build:
//
//   - the per-type `switch` that assigned `dwProgressMax` from
//     `wProgressLevel` / `wProgressMobKill` / ... is commented out in
//     SCODEX_CHAR_DATA::Assign (GLCodexData.cpp:415-480);
//   - the matching correction `switch` is commented out in
//     SCODEX_CHAR_DATA::Correction (GLCodexData.cpp:508-580);
//   - `GLCodex::Import` never even reads those columns (GLCodex.cpp:60-89
//     reads id, title, type, notify, reward point, and five item/quantity/grade
//     triples - nothing else).
//
// What remains live is a uniform item-registration model: an entry names up to
// five items, and registering the required quantity of each completes it. The
// type survives only to select which derived statistic the reward point lands
// in (GLLogixExPC.cpp:5133-5154), which is what the eleven CodexContribution
// fields are for.
//
// So the fields below are the five requirements and the reward. The dead
// per-type targets are deliberately absent rather than carried and ignored;
// see docs/reference/client/VERTICAL-004_CODEX_INVESTIGATION.md §3.

#include "types/Ids.h"

#include <array>
#include <cstdint>
#include <string>

namespace Modern
{
	// The most required items one codex entry can name.
	//
	// RAN hard-codes five item slots: sidProgressItem1..5 / wQuantity1..5 /
	// wItemGrade1..5 in SCODEX_CHAR_DATA (GLCodexData.h:189-212), and
	// GLCodex::Import reads exactly five item/quantity/grade triples
	// (GLCodex.cpp:66-89). The INI file itself declares the row width through
	// `ItemCodexKeySize` (GLCodex.cpp:53), but the loader is written against
	// five, so five is the verified capacity rather than a tunable.
	constexpr uint8_t kCodexMaxRequirements = 5;

	// What kind of thing a codex entry nominally records.
	//
	// RAN's EMCODEX_TYPE (GLCodexDefine.h:29-43). The order and the count are
	// preserved exactly, and the numeric values are meaningful: `SCODEX_FILE_DATA`
	// is serialised with the raw enum (GLCodexData.cpp:62, :207), so a modern
	// entry that claims to be RAN-compatible has to agree on the numbering.
	//
	// The type no longer selects a progress rule - nothing in the legacy build
	// reads it that way any more (§3 of the investigation). It selects which
	// derived statistic the reward point is added to, and that mapping is
	// reproduced verbatim by CodexTypeToField below.
	enum class CodexType : uint8_t
	{
		ReachLevel    = 0,
		KillMob       = 1,
		KillPlayer    = 2,
		ReachMap      = 3,
		TakeItem      = 4,
		UseItem       = 5,
		ReachCodex    = 6,
		CompleteQuest = 7,
		CodexPoint    = 8,
		QuestionBox   = 9,
		Etc           = 10,

		// EMCODEX_TYPE_SIZE. Not a real type; the count.
		Size          = 11,
	};

	constexpr uint8_t kCodexTypeCount = static_cast<uint8_t>(CodexType::Size);

	const char* ToString(CodexType type) noexcept;

	// True for a value in the verified enum range. `Size` and anything above it
	// are rejected, because a serialised type outside the range cannot be
	// mapped to a contribution field and would silently contribute nothing.
	constexpr bool IsValid(CodexType type) noexcept
	{
		return static_cast<uint8_t>(type) < kCodexTypeCount;
	}

	// Which derived statistic a completed entry of this type raises.
	//
	// This is the whole live purpose of the eleven types, and it is transcribed
	// from GLCHARLOGIC::CODEX_STATS (legacy/Lib_Client/G-Logic/GLogixExPC.cpp:5103),
	// whose eleven `if` statements at :5133-5154 read:
	//
	//   EMCODEX_TYPE_REACH_LEVEL    -> m_dwHPIncrease          -> CodexContribution::hp
	//   EMCODEX_TYPE_KILL_MOB       -> m_dwMPIncrease          -> ::mp
	//   EMCODEX_TYPE_KILL_PLAYER    -> m_dwSPIncrease          -> ::sp
	//   EMCODEX_TYPE_REACH_MAP      -> m_dwAttackIncrease      -> ::attack
	//   EMCODEX_TYPE_TAKE_ITEM      -> m_dwDefenseIncrease     -> ::defense
	//   EMCODEX_TYPE_USE_ITEM       -> m_dwShootingIncrease    -> ::shootPower
	//   EMCODEX_TYPE_REACH_CODEX    -> m_dwMeleeIncrease       -> ::meleePower
	//   EMCODEX_TYPE_COMPLETE_QUEST -> m_dwEnergyIncrease      -> ::magicAttack
	//   EMCODEX_TYPE_CODEX_POINT    -> m_dwResistanceIncrease  -> ::resistance
	//   EMCODEX_TYPE_QUESTION_BOX   -> m_dwHitrateIncrease     -> ::hit
	//   EMCODEX_TYPE_ETC            -> m_dwAvoidrateIncrease   -> ::avoid
	//
	// RAN writes a single flat `DWORD` total per field by summing each entry's
	// `dwRewardPoint`, so the contribution is one unsigned accumulation per
	// type and not a per-entry stat block. That is why the aggregator adds a
	// scalar rather than merging a struct.
	enum class CodexRewardField : uint8_t
	{
		None        = 0,
		Hp          = 1,
		Mp          = 2,
		Sp          = 3,
		Attack      = 4,
		Defense     = 5,
		ShootPower  = 6,
		MeleePower  = 7,
		MagicAttack = 8,
		Resistance  = 9,
		Hit         = 10,
		Avoid       = 11,
	};

	// The field a type's reward point is added to. `None` for an out-of-range
	// type, which the aggregator refuses rather than treating as a silent zero.
	constexpr CodexRewardField CodexTypeToField(CodexType type) noexcept
	{
		switch (type)
		{
		case CodexType::ReachLevel:    return CodexRewardField::Hp;
		case CodexType::KillMob:       return CodexRewardField::Mp;
		case CodexType::KillPlayer:    return CodexRewardField::Sp;
		case CodexType::ReachMap:      return CodexRewardField::Attack;
		case CodexType::TakeItem:      return CodexRewardField::Defense;
		case CodexType::UseItem:       return CodexRewardField::ShootPower;
		case CodexType::ReachCodex:    return CodexRewardField::MeleePower;
		case CodexType::CompleteQuest: return CodexRewardField::MagicAttack;
		case CodexType::CodexPoint:    return CodexRewardField::Resistance;
		case CodexType::QuestionBox:   return CodexRewardField::Hit;
		case CodexType::Etc:           return CodexRewardField::Avoid;
		case CodexType::Size:          break;
		}
		return CodexRewardField::None;
	}

	// How loudly a completed entry announces itself.
	//
	// RAN's EMCODEX_NOTIFY (GLCodexDefine.h:21-27). Carried because it is part
	// of the definition and because GLChar::CodexComplete branches on it
	// (GLCharCodex.cpp:29-55) - but every branch is a network broadcast, which
	// this milestone does not build, so nothing reads the value yet. See §11 of
	// the investigation for why it is still here.
	enum class CodexNotify : uint8_t
	{
		None      = 0,
		Around    = 1,
		AllServer = 2,
	};

	constexpr bool IsValid(CodexNotify notify) noexcept
	{
		return static_cast<uint8_t>(notify) <= static_cast<uint8_t>(CodexNotify::AllServer);
	}

	// One required item: what has to be registered, how many, and at what grade.
	//
	// Traces to the (sidProgressItemN, wQuantityN, wItemGradeN) triple that
	// SCODEX_CHAR_DATA::Assign copies out of the file data
	// (GLCodexData.cpp:388-398) and that GLChar::DoCodexRegisterItem tests
	// (GLCharCodex.cpp:75-144).
	struct CodexRequirement
	{
		// The item that must be registered. `sidMobKill` .. `sidItemUse` in
		// RAN; here the five slots keep their meaning only as positions, and the
		// legacy names ("mob kill", "map reach") are dropped because nothing
		// kills a mob or reaches a map any more (§3).
		ItemId item = ItemId::MakeInvalid();

		// How many of the item must be registered in one registration.
		//
		// RAN compares this for equality against the turn number of the stack
		// being registered (GLCharCodex.cpp:81, :95, :109, :123, :137), not
		// with `>=`. That is reproduced rather than relaxed: a relaxed rule
		// would be an invention, and it would let one registration satisfy a
		// five-count requirement.
		uint16_t quantity = 0;

		// The grade the registered item must be.
		//
		// RAN also compares this for equality (GLCharCodex.cpp:79, :93, :107,
		// :121, :135) against the registered instance's grinding grade,
		// `SINVENITEM::sItemCustom.GETGRADE(EMGRINDING_NONE)`
		// (GLCharInvenMsg.cpp:9126).
		//
		// LIMITATION: the modern core cannot enforce this. `ItemInstance`
		// carries no grade, upgrade or option state - it deliberately does not,
		// and its own comment says so (core/item/ItemInstance.h:17-21). The
		// value is kept so the definition stays faithful to the data RAN ships
		// and so enforcement can be added when per-copy state exists, but
		// CodexState::RegisterItem does not read it. See §7 of the investigation.
		uint16_t requiredGrade = 0;

		// Whether this slot names something to be registered.
		//
		// RAN decides "not configured" from the quantity alone: `Assign` tests
		// `wProgressItemUse == 0`, `wProgressItemGet == 0` and so on
		// (GLCodexData.cpp:379-386), and `DoCodexRegisterItem` matches on
		// `sidProgressItemN` (GLCharCodex.cpp:75) which defaults to
		// `NATIVEID_NULL()`. Both are zero/null together in RAN's constructor
		// (GLCodexData.h:80-89). A modern requirement is configured when it
		// names an item and a non-zero quantity, which is the conjunction of the
		// two conditions RAN checks separately.
		constexpr bool IsConfigured() const noexcept
		{
			return item.IsValid() && quantity > 0;
		}

		constexpr bool operator==(const CodexRequirement& other) const noexcept
		{
			return item == other.item && quantity == other.quantity &&
			       requiredGrade == other.requiredGrade;
		}
	};

	// The definition of a codex entry: the shared, immutable description.
	//
	// RAN keeps title, badge string and description as std::string on the file
	// data (GLCodexData.h:31-33) and copies them nowhere near the character, so
	// a modern entry holds them for presentation only. Nothing computes with
	// them.
	struct CodexDefinition
	{
		CodexId      id = CodexId::MakeInvalid();
		std::string  title;
		std::string  description;

		// The badge RAN shows next to a completed entry, and whether to show it
		// at all (GLCodexData.h:32, :39).
		//
		// LIMITATION: presentation only. RAN never grants a codex badge - the
		// award code that copies `strBadgeString` into the character exists only
		// in the Activity system (GLCharActivity.cpp:519-533,
		// GLCharactorReq2.cpp:733-739) and `GLChar::CodexComplete` has no
		// equivalent. Within the codex system these two fields are read only to
		// display a definition's badge in a list. See §8 of the investigation.
		std::string  badge;
		bool         rewardBadge = false;

		CodexType    type = CodexType::ReachLevel;
		CodexNotify  notify = CodexNotify::None;

		// The flat amount a completed entry adds to the one derived statistic
		// `type` selects. `dwRewardPoint` in RAN (GLCodexData.h:38), accumulated
		// as a `DWORD` by CODEX_STATS. Unsigned because that is the legacy type
		// and because a codex bonus can only ever raise a value.
		uint32_t     rewardPoint = 0;

		// The required items, in slot order. Slot order is meaningful: it is the
		// order CODEX_STATS and DoCodexRegisterItem visit, and the order
		// RequiredSlotCount() counts in.
		std::array<CodexRequirement, kCodexMaxRequirements> requirements{};

		bool IsValid() const
		{
			// Qualified: an unqualified IsValid inside this member would resolve to
			// the member itself, not to the enum validators above. The same
			// shadowing is handled this way in Status::IsOk.
			return id.IsValid() && !title.empty() && Modern::IsValid(type) &&
			       Modern::IsValid(notify);
		}

		// How many of the five slots are actually required.
		//
		// This reproduces RAN's cascade in SCODEX_CHAR_DATA::Assign
		// (GLCodexData.cpp:377-386) exactly, including its two quirks. RAN starts
		// at five and lets four sequential `if`s lower it:
		//
		//   dwProgressMax = 5;
		//   if ( wProgressItemUse  == 0 ) dwProgressMax = 4;
		//   if ( wProgressItemGet  == 0 ) dwProgressMax = 3;
		//   if ( wProgressMapReach == 0 ) dwProgressMax = 2;
		//   if ( wProgressMapKill  == 0 ) dwProgressMax = 1;
		//
		// Two things about that are load-bearing and are not tidied up here.
		//
		// The `if`s are sequential assignments, not an early return, so the *last*
		// one that fires wins. For a definition configuring a prefix of the slots
		// that is the same answer as an early return, because the unconfigured
		// slots are the high-numbered ones and the lowest unconfigured slot is
		// tested last. It is written as sequential assignments rather than as a
		// chain of returns so the two cannot silently diverge.
		//
		// The first slot - `sidMobKill` / `wProgressMobKill`, which `Assign` copies
		// into `wQuantity1` and `sidProgressItem1` (GLCodexData.cpp:388, :403) - is
		// never tested. A definition that configures only that slot still reports
		// a required count of 1, because slot 1 being unconfigured is what sets
		// it. That is harmless for the prefix-shaped data RAN's table holds and
		// is reproduced rather than fixed: the required count is the rule that
		// decides when an entry completes, so quietly correcting it would make
		// modern entries complete at a different moment than RAN's. The one shape
		// it mishandles - slots 1..4 configured and slot 0 not - is unreachable
		// from a left-to-right table. §4 of the investigation pins the table.
		constexpr uint8_t RequiredSlotCount() const noexcept
		{
			uint8_t count = 5;
			if (!requirements[4].IsConfigured()) { count = 4; }
			if (!requirements[3].IsConfigured()) { count = 3; }
			if (!requirements[2].IsConfigured()) { count = 2; }
			if (!requirements[1].IsConfigured()) { count = 1; }
			return count;
		}

		// Whether the entry could ever be completed by a registration.
		//
		// Only the counted window matters. A slot past the required count is
		// never consulted when a registration is matched
		// (CodexState::RegisterItem), so a requirement sitting out there cannot
		// make the entry completable even though it is configured.
		//
		// RAN has no such check and will happily seat an entry with nothing in
		// its window in the progress map, where it can only be completed by
		// registering an item that matches nothing - which never happens. §4 of
		// the investigation covers it.
		constexpr bool IsCompletable() const noexcept
		{
			const uint8_t counted = RequiredSlotCount();
			for (uint8_t slot = 0; slot < counted; ++slot)
			{
				if (requirements[slot].IsConfigured())
				{
					return true;
				}
			}
			return false;
		}

		// The item required by one of the counted slots, or an unconfigured
		// requirement for an index past the required count.
		const CodexRequirement& GetRequirement(uint8_t slot) const noexcept
		{
			return requirements[slot < kCodexMaxRequirements ? slot : 0];
		}

		friend bool operator==(const CodexDefinition& lhs, const CodexDefinition& rhs)
		{
			return lhs.id == rhs.id;
		}

		friend bool operator!=(const CodexDefinition& lhs, const CodexDefinition& rhs)
		{
			return !(lhs == rhs);
		}
	};
}

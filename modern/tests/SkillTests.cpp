// VERTICAL-003: passive skill definitions, learned state, and aggregation.
//
// Headless. Links Modern and nothing else: no renderer, no socket, no database,
// no legacy library, no server, no client.
//
// The cases that matter are the transcription ones. The aggregator restates
// GLCHARLOGIC::SUM_PASSIVE, so what is asserted here is that each legacy switch
// arm landed in the contribution field it belongs to, that the per-level value
// is the one for the level actually learned, and that a skill nothing defines is
// a refusal rather than a plausible zero. The formulas that consume the result
// are CORE-002's and are tested there; nothing here recomputes a derived stat.

#include "TestHarness.h"

#include "equipment/EquipmentState.h"
#include "item/ItemInstance.h"
#include "skills/PassiveContributionAggregator.h"
#include "skills/SkillDefinition.h"
#include "skills/SkillDefinitionProvider.h"
#include "skills/SkillState.h"
#include "stats/Contributions.h"

#include <cmath>
#include <limits>

using namespace Modern;

namespace
{
	// A skill id in the first skill class. SkillId is a (classIndex,
	// skillIndex) pair, so most of these cases only care about the second half.
	SkillId Id(uint16_t skillIndex)
	{
		return SkillId{ 1, skillIndex };
	}

	// The same pair written out, for the cases that need both halves. Named
	// differently rather than taking a defaulted second argument: a
	// (skillIndex, classIndex) signature reads naturally and silently swaps the
	// two halves at every call site, which is exactly the mistake a composite
	// key invites.
	SkillId IdIn(uint16_t classIndex, uint16_t skillIndex)
	{
		return SkillId{ classIndex, skillIndex };
	}

	// A definition whose only contribution is a flat HP bonus at one level.
	SkillDefinition MakeHpSkill(uint16_t skillIndex, float hp, uint8_t maxLevel = 1)
	{
		SkillDefinition def;
		def.id       = Id(skillIndex);
		def.name     = "HpPassive" + std::to_string(skillIndex);
		def.maxLevel = maxLevel;
		def.applyType = PassiveApplyType::Hp;
		def.levelData[1].basicVar = hp;
		return def;
	}

	// A provider that answers without going through Add's validation, so the
	// aggregator's own defense-in-depth checks can be exercised in isolation.
	class DirectProvider final : public SkillDefinitionProvider
	{
	public:
		SkillDefinition definition;

		const SkillDefinition* Find(const SkillId& id) const override
		{
			if (id == definition.id)
			{
				return &definition;
			}
			return nullptr;
		}
	};

	ItemInstance MakeInstance(uint32_t defId, uint64_t serial = 1)
	{
		ItemInstance item;
		item.definition = ItemId(defId);
		item.serial     = serial;
		item.count      = 1;
		return item;
	}
}

// ---------------------------------------------------------------------------
// SkillId
// ---------------------------------------------------------------------------

MODERN_TEST(SkillId_ValidityIsASentinelNotAZero)
{
	// SkillId is valid unless one of its halves is 0xFFFF, the value the legacy
	// code carries as ID_NULL (SNATIVEID). So SkillId{0, 0} is a real id - class
	// 0, skill 0 - and not a "no skill" value.
	//
	// Worth pinning because it differs from ItemId, which has an explicit
	// MakeInvalid(): there is no way to spell an invalid SkillId other than the
	// literal 0xFFFF, and a zero-initialised one is learnable. What actually
	// keeps that safe is ServerCharacter::LearnSkill checking the provider
	// before it touches the state (ServerCharacter.cpp:201), so a zeroed id
	// finds no definition and is refused as NotFound. The value test in
	// SkillState is the weaker of the two.
	const SkillId zero;
	CHECK(zero.IsValid());
	CHECK(zero == IdIn(0, 0));

	CHECK(!IdIn(0xFFFF, 3).IsValid());
	CHECK(!IdIn(3, 0xFFFF).IsValid());
	CHECK(IdIn(1, 1).IsValid());
}

MODERN_TEST(SkillId_EqualityAndOrdering)
{
	CHECK(Id(5) == Id(5));
	CHECK(Id(5) != Id(6));
	// Class index dominates, matching a composite (wMainID, wSubID) key: this is
	// what makes iteration order stable and SkillState usable as a sorted value.
	CHECK(IdIn(1, 1) < IdIn(2, 1));
	CHECK(IdIn(1, 9) < IdIn(2, 1));
	CHECK(IdIn(1, 1) < IdIn(1, 2));
	CHECK(!(IdIn(1, 2) < IdIn(1, 1)));
	CHECK(!(IdIn(1, 1) < IdIn(1, 1)));
}

MODERN_TEST(SkillId_ConstantsMatchRan)
{
	// SKILL::MAX_LEVEL / MAX_IMPACT / MAX_SPEC in GLSkillDefine.h:16-18.
	CHECK_EQ(static_cast<int>(kMaxSkillLevel), 9);
	CHECK_EQ(static_cast<int>(kMaxSkillImpacts), 5);
	CHECK_EQ(static_cast<int>(kMaxSkillSpecs), 5);
}

// ---------------------------------------------------------------------------
// SkillDefinition
// ---------------------------------------------------------------------------

MODERN_TEST(SkillDefinition_ValidityRequiresIdentityNameAndAValue)
{
	SkillDefinition def = MakeHpSkill(1, 100.0f);
	CHECK(def.IsValid());

	// No name: RAN's skill tables always carry one, and a nameless passive
	// could not be presented by a UI.
	def.name.clear();
	CHECK(!def.IsValid());

	def = MakeHpSkill(1, 100.0f);
	def.maxLevel = 0;
	CHECK(!def.IsValid());

	def = MakeHpSkill(1, 100.0f);
	def.maxLevel = static_cast<uint8_t>(kMaxSkillLevel + 1);
	CHECK(!def.IsValid());

	// Every level contributes nothing: there is no such passive in RAN's data.
	def = MakeHpSkill(1, 0.0f);
	CHECK(!def.IsValid());
}

MODERN_TEST(SkillDefinition_WeaponRequirementPredicates)
{
	// NoCare is the default and means "no requirement", not "requires nothing
	// to be held" — the distinction is the whole point of the check.
	SkillDefinition def = MakeHpSkill(1, 100.0f);
	CHECK(!def.RequiresWeapon(SkillWeaponSlot::RightHand));
	CHECK(def.GetRequiredWeapon(SkillWeaponSlot::RightHand) == SkillWeaponType::NoCare);

	def.rightWeapon = SkillWeaponType::Sword;
	CHECK(def.RequiresWeapon(SkillWeaponSlot::RightHand));
	CHECK(!def.RequiresWeapon(SkillWeaponSlot::LeftHand));
	CHECK(def.GetRequiredWeapon(SkillWeaponSlot::RightHand) == SkillWeaponType::Sword);
}

MODERN_TEST(SkillDefinition_LegacyEnumConversionsRoundTrip)
{
	// The weapon enum is a 1:1 transcription of SKILL::GLSKILL_ATT
	// (GLSkillBasic.h:97-121), including SKILLATT_NOCARE == 22.
	for (int legacy = 0; legacy <= 25; ++legacy)
	{
		const SkillWeaponType type = LegacyWeaponTypeToModern(legacy);
		CHECK_EQ(ModernWeaponTypeToLegacy(type), legacy);
	}
	CHECK(LegacyWeaponTypeToModern(22) == SkillWeaponType::NoCare);
	// An unknown legacy value degrades to "no requirement" rather than to a
	// requirement the aggregator would then refuse to satisfy.
	CHECK(LegacyWeaponTypeToModern(999) == SkillWeaponType::NoCare);

	// EMIMPACT_ADDON is not contiguous in the legacy enum, so it is mapped
	// case by case rather than cast.
	CHECK(LegacyImpactTypeToModern(1)  == PassiveImpactType::HitRate);
	CHECK(LegacyImpactTypeToModern(5)  == PassiveImpactType::VarHp);
	CHECK(LegacyImpactTypeToModern(14) == PassiveImpactType::HpRate);
	CHECK(LegacyImpactTypeToModern(17) == PassiveImpactType::Resist);
	CHECK(LegacyImpactTypeToModern(18) == PassiveImpactType::None);
	CHECK(LegacyImpactTypeToModern(-1) == PassiveImpactType::None);

	// No spec in the legacy table feeds the stat pipeline.
	CHECK(LegacySpecTypeToModern(0) == PassiveSpecType::None);
	CHECK(LegacySpecTypeToModern(3) == PassiveSpecType::None);
}

MODERN_TEST(SkillDefinition_EveryEnumHasAName)
{
	CHECK(std::string(ToString(SkillWeaponType::NoCare)) == "NoCare");
	CHECK(std::string(ToString(PassiveApplyType::HpRate)) == "HpRate");
	CHECK(std::string(ToString(PassiveImpactType::VarHp)) == "VarHp");
	CHECK(std::string(ToString(PassiveSpecType::None)) == "None");
}

// ---------------------------------------------------------------------------
// InMemorySkillDefinitions
// ---------------------------------------------------------------------------

MODERN_TEST(SkillDefinitions_AddRegistersValidDefinition)
{
	InMemorySkillDefinitions provider;
	CHECK_EQ(provider.GetCount(), static_cast<size_t>(0));

	const SkillDefinition def = MakeHpSkill(1, 100.0f);
	CHECK(provider.Add(def).IsOk());
	CHECK_EQ(provider.GetCount(), static_cast<size_t>(1));
	CHECK(provider.Find(Id(1)) != nullptr);
}

MODERN_TEST(SkillDefinitions_AddRejectsInvalidDefinition)
{
	InMemorySkillDefinitions provider;
	SkillDefinition def = MakeHpSkill(1, 0.0f);
	CHECK(provider.Add(def).IsError());
	CHECK_EQ(provider.GetCount(), static_cast<size_t>(0));
	CHECK(provider.Find(Id(1)) == nullptr);
}

MODERN_TEST(SkillDefinitions_AddReplacesExisting)
{
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeHpSkill(1, 100.0f)).IsOk());
	CHECK(provider.Add(MakeHpSkill(1, 250.0f)).IsOk());

	CHECK_EQ(provider.GetCount(), static_cast<size_t>(1));
	const SkillDefinition* found = provider.Find(Id(1));
	CHECK(found != nullptr);
	if (found != nullptr)
	{
		CHECK_EQ(found->levelData[1].basicVar, 250.0f);
	}
}

MODERN_TEST(SkillDefinitions_FindIsIndependentOfRegistrationOrder)
{
	// The container is sorted, so a lookup cannot depend on the order the
	// definitions were added in. Registration order is not a property RAN's
	// data has, and a provider that leaked it would make results
	// unreproducible between runs.
	InMemorySkillDefinitions provider;
	InMemorySkillDefinitions reversed;

	const SkillDefinition a = MakeHpSkill(10, 100.0f);
	const SkillDefinition b = MakeHpSkill(20, 200.0f);
	const SkillDefinition c = MakeHpSkill(30, 300.0f);

	CHECK(provider.Add(a).IsOk());
	CHECK(provider.Add(b).IsOk());
	CHECK(provider.Add(c).IsOk());

	CHECK(reversed.Add(c).IsOk());
	CHECK(reversed.Add(b).IsOk());
	CHECK(reversed.Add(a).IsOk());

	CHECK(provider.Find(Id(10)) != nullptr);
	CHECK(provider.Find(Id(20)) != nullptr);
	CHECK(provider.Find(Id(30)) != nullptr);
	CHECK_EQ(provider.GetCount(), reversed.GetCount());
}

MODERN_TEST(SkillDefinitions_RemoveAndClear)
{
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeHpSkill(1, 100.0f)).IsOk());
	CHECK(provider.Add(MakeHpSkill(2, 100.0f)).IsOk());

	CHECK(provider.Remove(Id(1)).IsOk());
	CHECK(provider.Find(Id(1)) == nullptr);
	CHECK_EQ(provider.GetCount(), static_cast<size_t>(1));

	const Status missing = provider.Remove(Id(1));
	CHECK(missing.IsError());
	CHECK_EQ(missing.GetCode(), ErrorCode::NotFound);

	provider.Clear();
	CHECK_EQ(provider.GetCount(), static_cast<size_t>(0));
	CHECK(provider.Find(Id(2)) == nullptr);
}

MODERN_TEST(SkillDefinitions_FindInvalidIdReturnsNull)
{
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeHpSkill(1, 100.0f)).IsOk());

	const SkillId invalid{ 0xFFFFu, 0xFFFFu };
	CHECK(provider.Find(invalid) == nullptr);
}

// ---------------------------------------------------------------------------
// SkillState
// ---------------------------------------------------------------------------

MODERN_TEST(SkillState_StartsEmpty)
{
	SkillState state;
	CHECK_EQ(state.GetLearnedCount(), static_cast<size_t>(0));
	CHECK(!state.HasSkill(Id(1)));
	CHECK_EQ(state.GetSkillLevel(Id(1)), static_cast<uint8_t>(0));
	CHECK(!state.GetSkill(Id(1)).IsLearned());
}

MODERN_TEST(SkillState_LearnAddsAtLevelOne)
{
	SkillState state;
	CHECK(state.LearnSkill(Id(1)).IsOk());
	CHECK(state.HasSkill(Id(1)));
	CHECK_EQ(state.GetSkillLevel(Id(1)), static_cast<uint8_t>(1));
	CHECK(state.GetSkill(Id(1)).IsLearned());
	CHECK_EQ(state.GetLearnedCount(), static_cast<size_t>(1));
}

MODERN_TEST(SkillState_LearnRejectsDuplicateAndInvalid)
{
	SkillState state;
	CHECK(state.LearnSkill(Id(1)).IsOk());

	const Status again = state.LearnSkill(Id(1));
	CHECK(again.IsError());
	CHECK_EQ(again.GetCode(), ErrorCode::AlreadyExists);
	// The refused call must not have disturbed the learned level.
	CHECK_EQ(state.GetSkillLevel(Id(1)), static_cast<uint8_t>(1));

	const Status invalid = state.LearnSkill(SkillId{ 0xFFFFu, 0xFFFFu });
	CHECK(invalid.IsError());
	CHECK_EQ(invalid.GetCode(), ErrorCode::InvalidArgument);
	CHECK_EQ(state.GetLearnedCount(), static_cast<size_t>(1));
}

MODERN_TEST(SkillState_UnlearnRemoves)
{
	SkillState state;
	CHECK(state.LearnSkill(Id(1)).IsOk());
	CHECK(state.UnlearnSkill(Id(1)).IsOk());
	CHECK(!state.HasSkill(Id(1)));
	CHECK_EQ(state.GetLearnedCount(), static_cast<size_t>(0));

	// Unlearning something not learned is NotFound, not a silent success:
	// a caller that believed it had removed a skill would otherwise go on
	// believing a contribution was gone when nothing changed.
	const Status missing = state.UnlearnSkill(Id(1));
	CHECK(missing.IsError());
	CHECK_EQ(missing.GetCode(), ErrorCode::NotFound);
}

MODERN_TEST(SkillState_SetLevelRequiresTheSkillToBeLearned)
{
	SkillState state;
	const Status notLearned = state.SetSkillLevel(Id(1), 3);
	CHECK(notLearned.IsError());
	CHECK_EQ(notLearned.GetCode(), ErrorCode::NotFound);

	CHECK(state.LearnSkill(Id(1)).IsOk());
	CHECK(state.SetSkillLevel(Id(1), 3).IsOk());
	CHECK_EQ(state.GetSkillLevel(Id(1)), static_cast<uint8_t>(3));
}

MODERN_TEST(SkillState_SetLevelRejectsOutOfRange)
{
	SkillState state;
	CHECK(state.LearnSkill(Id(1)).IsOk());

	// Level zero is "not learned", which is what UnlearnSkill is for; letting
	// SetSkillLevel express it would give one skill two ways to be absent.
	CHECK(state.SetSkillLevel(Id(1), 0).IsError());
	CHECK(state.SetSkillLevel(Id(1), static_cast<uint8_t>(kMaxSkillLevel + 1)).IsError());
	CHECK_EQ(state.GetSkillLevel(Id(1)), static_cast<uint8_t>(1));
}

MODERN_TEST(SkillState_IteratesInSkillIdOrder)
{
	SkillState state;
	CHECK(state.LearnSkill(Id(30)).IsOk());
	CHECK(state.LearnSkill(Id(10)).IsOk());
	CHECK(state.LearnSkill(IdIn(2, 20)).IsOk());

	// SKILL_MAP is std::map<DWORD, SCHARSKILL> keyed on a composite id, so
	// the legacy iteration order was the key order too.
	size_t previous = 0;
	size_t seen     = 0;
	for (const auto& [id, learned] : state.GetAllSkills())
	{
		(void)learned;
		const size_t current = (static_cast<size_t>(id.classIndex) << 16) | id.skillIndex;
		CHECK(current > previous);
		previous = current;
		++seen;
	}
	CHECK_EQ(seen, static_cast<size_t>(3));
}

MODERN_TEST(SkillState_EqualityComparesLevels)
{
	SkillState a;
	SkillState b;
	CHECK(a == b);

	CHECK(a.LearnSkill(Id(1)).IsOk());
	CHECK(a != b);

	CHECK(b.LearnSkill(Id(1)).IsOk());
	CHECK(a == b);

	CHECK(b.SetSkillLevel(Id(1), 5).IsOk());
	CHECK(a != b);

	a.Clear();
	CHECK_EQ(a.GetLearnedCount(), static_cast<size_t>(0));
}

MODERN_TEST(SkillState_IsAValueType)
{
	// Copying copies the skill set, because a snapshot-building or a staged
	// mutation both rely on it.
	SkillState original;
	CHECK(original.LearnSkill(Id(1)).IsOk());
	CHECK(original.SetSkillLevel(Id(1), 4).IsOk());

	const SkillState copy = original;
	CHECK(copy == original);
	CHECK_EQ(copy.GetSkillLevel(Id(1)), static_cast<uint8_t>(4));
}

// ---------------------------------------------------------------------------
// PassiveContributionAggregator
// ---------------------------------------------------------------------------

MODERN_TEST(PassiveAggregator_EmptyStateYieldsZero)
{
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeHpSkill(1, 100.0f)).IsOk());

	const SkillState skills;
	EquipmentState equipment;

	const Result<PassiveContributionResult> aggregated =
		PassiveContributionAggregator::Aggregate(skills, provider, equipment);

	CHECK(aggregated.IsOk());
	if (aggregated.IsError())
	{
		return;
	}
	CHECK(aggregated.GetValue().IsOk());
	CHECK(aggregated.GetValue().contribution == Stats::PassiveContribution());
	CHECK_EQ(aggregated.GetValue().contributingSkills, static_cast<size_t>(0));
}

MODERN_TEST(PassiveAggregator_SingleSkillBasicValue)
{
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeHpSkill(1, 250.0f)).IsOk());

	SkillState skills;
	CHECK(skills.LearnSkill(Id(1)).IsOk());
	EquipmentState equipment;

	const Result<PassiveContributionResult> aggregated =
		PassiveContributionAggregator::Aggregate(skills, provider, equipment);

	CHECK(aggregated.IsOk());
	if (aggregated.IsError())
	{
		return;
	}
	CHECK_EQ(aggregated.GetValue().contribution.hp, 250);
	CHECK_EQ(aggregated.GetValue().contributingSkills, static_cast<size_t>(1));
}

MODERN_TEST(PassiveAggregator_UsesTheValueForTheLearnedLevel)
{
	// The per-level value is the one thing a level change must move: RAN reads
	// sDATA_LVL[sCharSkill.wLevel] (GLogixExPC.cpp:916), so a skill learned at
	// level 3 contributes row 3 and nothing else.
	SkillDefinition def = MakeHpSkill(1, 100.0f, 3);
	def.levelData[2].basicVar = 175.0f;
	def.levelData[3].basicVar = 260.0f;

	InMemorySkillDefinitions provider;
	CHECK(provider.Add(def).IsOk());

	SkillState skills;
	CHECK(skills.LearnSkill(Id(1)).IsOk());
	EquipmentState equipment;

	auto contribution = [&]()
	{
		return PassiveContributionAggregator::Aggregate(skills, provider, equipment)
			.GetValue().contribution;
	};

	CHECK_EQ(contribution().hp, 100);
	CHECK(skills.SetSkillLevel(Id(1), 2).IsOk());
	CHECK_EQ(contribution().hp, 175);
	CHECK(skills.SetSkillLevel(Id(1), 3).IsOk());
	CHECK_EQ(contribution().hp, 260);
}

MODERN_TEST(PassiveAggregator_StacksAdditivelyAndDeterministically)
{
	// SUM_PASSIVE is a plain accumulation into one SPASSIVE_SKILL_DATA with no
	// ordering or priority rule, so A+B and B+A must be the same number.
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeHpSkill(10, 100.0f)).IsOk());
	CHECK(provider.Add(MakeHpSkill(20, 40.0f)).IsOk());
	CHECK(provider.Add(MakeHpSkill(30, 7.0f)).IsOk());

	EquipmentState equipment;

	SkillState forward;
	CHECK(forward.LearnSkill(Id(10)).IsOk());
	CHECK(forward.LearnSkill(Id(20)).IsOk());
	CHECK(forward.LearnSkill(Id(30)).IsOk());

	SkillState reversed;
	CHECK(reversed.LearnSkill(Id(30)).IsOk());
	CHECK(reversed.LearnSkill(Id(20)).IsOk());
	CHECK(reversed.LearnSkill(Id(10)).IsOk());

	const Stats::PassiveContribution a =
		PassiveContributionAggregator::Aggregate(forward, provider, equipment)
			.GetValue().contribution;
	const Stats::PassiveContribution b =
		PassiveContributionAggregator::Aggregate(reversed, provider, equipment)
			.GetValue().contribution;

	CHECK_EQ(a.hp, 147);
	CHECK(a == b);

	// And removing one removes exactly its share.
	CHECK(forward.UnlearnSkill(Id(20)).IsOk());
	const Stats::PassiveContribution reduced =
		PassiveContributionAggregator::Aggregate(forward, provider, equipment)
			.GetValue().contribution;
	CHECK_EQ(reduced.hp, 107);
}

MODERN_TEST(PassiveAggregator_ImpactsAddToTheirOwnFields)
{
	SkillDefinition def = MakeHpSkill(1, 200.0f);
	def.impacts[0].type            = PassiveImpactType::Defense;
	def.impacts[0].values[1]       = 12.0f;
	def.impacts[1].type            = PassiveImpactType::HpRate;
	def.impacts[1].values[1]       = 0.5f;
	def.impacts[2].type            = PassiveImpactType::Resist;
	def.impacts[2].values[1]       = 3.0f;

	InMemorySkillDefinitions provider;
	CHECK(provider.Add(def).IsOk());

	SkillState skills;
	CHECK(skills.LearnSkill(Id(1)).IsOk());
	EquipmentState equipment;

	const Result<PassiveContributionResult> aggregated =
		PassiveContributionAggregator::Aggregate(skills, provider, equipment);

	CHECK(aggregated.IsOk());
	if (aggregated.IsError())
	{
		return;
	}
	const Stats::PassiveContribution& contribution = aggregated.GetValue().contribution;
	CHECK_EQ(contribution.hp, 200);
	CHECK_EQ(contribution.defense, 12);
	CHECK_EQ(contribution.hpRate, 0.5f);
	// SUM_PASSIVE adds the resist value to all five elements
	// (GLogixExPC.cpp:1074), matching SRESIST::operator+=(int).
	CHECK_EQ(contribution.resistances.fire, 3);
	CHECK_EQ(contribution.resistances.ice, 3);
	CHECK_EQ(contribution.resistances.electric, 3);
	CHECK_EQ(contribution.resistances.poison, 3);
	CHECK_EQ(contribution.resistances.spirit, 3);
	// Impacts belong to the skill that carries them, so they do not make it a
	// second contributing skill.
	CHECK_EQ(aggregated.GetValue().contributingSkills, static_cast<size_t>(1));
}

MODERN_TEST(PassiveAggregator_ImpactValueIsReadForTheLearnedLevel)
{
	SkillDefinition def = MakeHpSkill(1, 0.0f, 2);
	def.levelData[1].basicVar    = 10.0f;
	def.impacts[0].type          = PassiveImpactType::Pa;
	def.impacts[0].values[1]     = 5.0f;
	def.impacts[0].values[2]     = 40.0f;

	InMemorySkillDefinitions provider;
	CHECK(provider.Add(def).IsOk());

	SkillState skills;
	CHECK(skills.LearnSkill(Id(1)).IsOk());
	EquipmentState equipment;

	CHECK_EQ(PassiveContributionAggregator::Aggregate(skills, provider, equipment)
		.GetValue().contribution.meleePower, 5);
	CHECK(skills.SetSkillLevel(Id(1), 2).IsOk());
	CHECK_EQ(PassiveContributionAggregator::Aggregate(skills, provider, equipment)
		.GetValue().contribution.meleePower, 40);
}

MODERN_TEST(PassiveAggregator_MissingDefinitionIsAnError)
{
	// A learned skill nothing defines is a data fault. A silent zero would let
	// it hide behind a plausible number.
	InMemorySkillDefinitions provider;

	SkillState skills;
	CHECK(skills.LearnSkill(Id(1)).IsOk());
	EquipmentState equipment;

	const Result<PassiveContributionResult> aggregated =
		PassiveContributionAggregator::Aggregate(skills, provider, equipment);

	CHECK(aggregated.IsOk());
	if (aggregated.IsError())
	{
		return;
	}
	CHECK(!aggregated.GetValue().IsOk());
	CHECK(aggregated.GetValue().error == PassiveAggregationError::MissingDefinition);
}

MODERN_TEST(PassiveAggregator_NonFiniteDefinitionIsAnError)
{
	// The aggregator re-checks what Add validated, because a future provider
	// need not be InMemorySkillDefinitions.
	DirectProvider provider;
	provider.definition       = MakeHpSkill(1, 100.0f);
	provider.definition.levelData[1].basicVar =
		std::numeric_limits<float>::quiet_NaN();

	SkillState skills;
	CHECK(skills.LearnSkill(Id(1)).IsOk());
	EquipmentState equipment;

	const Result<PassiveContributionResult> aggregated =
		PassiveContributionAggregator::Aggregate(skills, provider, equipment);

	CHECK(aggregated.IsOk());
	if (aggregated.IsError())
	{
		return;
	}
	CHECK(!aggregated.GetValue().IsOk());
	CHECK(aggregated.GetValue().error == PassiveAggregationError::NonFinite);
}

MODERN_TEST(PassiveAggregator_NonFiniteImpactIsAnError)
{
	DirectProvider provider;
	provider.definition                 = MakeHpSkill(1, 100.0f);
	provider.definition.impacts[0].type = PassiveImpactType::HpRate;
	provider.definition.impacts[0].values[1] =
		std::numeric_limits<float>::infinity();

	SkillState skills;
	CHECK(skills.LearnSkill(Id(1)).IsOk());
	EquipmentState equipment;

	const Result<PassiveContributionResult> aggregated =
		PassiveContributionAggregator::Aggregate(skills, provider, equipment);

	CHECK(aggregated.IsOk());
	if (aggregated.IsError())
	{
		return;
	}
	CHECK(aggregated.GetValue().error == PassiveAggregationError::NonFinite);
}

MODERN_TEST(PassiveAggregator_WeaponRequirementGatesTheContribution)
{
	// SUM_PASSIVE skips a passive whose weapon requirement the worn set does not
	// satisfy (GLogixExPC.cpp:882-914), and skipping is not an error: the
	// character still has the skill, it is simply inactive.
	SkillDefinition def = MakeHpSkill(1, 300.0f);
	def.rightWeapon = SkillWeaponType::Sword;

	InMemorySkillDefinitions provider;
	CHECK(provider.Add(def).IsOk());

	SkillState skills;
	CHECK(skills.LearnSkill(Id(1)).IsOk());
	EquipmentState equipment;

	// Nothing in the right hand: inactive, and no error.
	Result<PassiveContributionResult> aggregated =
		PassiveContributionAggregator::Aggregate(skills, provider, equipment);
	CHECK(aggregated.IsOk());
	if (aggregated.IsError())
	{
		return;
	}
	CHECK(aggregated.GetValue().IsOk());
	CHECK_EQ(aggregated.GetValue().contribution.hp, 0);
	CHECK_EQ(aggregated.GetValue().contributingSkills, static_cast<size_t>(0));

	// Something in the right hand: active.
	CHECK(equipment.Equip(EquipmentSlot::RightHand, MakeInstance(1)).IsOk());
	aggregated = PassiveContributionAggregator::Aggregate(skills, provider, equipment);
	CHECK(aggregated.IsOk());
	if (aggregated.IsError())
	{
		return;
	}
	CHECK_EQ(aggregated.GetValue().contribution.hp, 300);

	// A weapon in the other hand does not satisfy a right-hand requirement.
	EquipmentState wrongHand;
	CHECK(wrongHand.Equip(EquipmentSlot::LeftHand, MakeInstance(1)).IsOk());
	const Result<PassiveContributionResult> other =
		PassiveContributionAggregator::Aggregate(skills, provider, wrongHand);
	CHECK(other.IsOk());
	if (other.IsError())
	{
		return;
	}
	CHECK_EQ(other.GetValue().contribution.hp, 0);
}

MODERN_TEST(PassiveAggregator_LeftAndRightRequirementsAreIndependent)
{
	SkillDefinition def = MakeHpSkill(1, 300.0f);
	def.leftWeapon  = SkillWeaponType::Shield;
	def.rightWeapon = SkillWeaponType::Sword;

	InMemorySkillDefinitions provider;
	CHECK(provider.Add(def).IsOk());

	SkillState skills;
	CHECK(skills.LearnSkill(Id(1)).IsOk());

	EquipmentState onlyLeft;
	CHECK(onlyLeft.Equip(EquipmentSlot::LeftHand, MakeInstance(1)).IsOk());
	CHECK_EQ(PassiveContributionAggregator::Aggregate(skills, provider, onlyLeft)
		.GetValue().contribution.hp, 0);

	EquipmentState both;
	CHECK(both.Equip(EquipmentSlot::LeftHand, MakeInstance(1)).IsOk());
	CHECK(both.Equip(EquipmentSlot::RightHand, MakeInstance(2)).IsOk());
	CHECK_EQ(PassiveContributionAggregator::Aggregate(skills, provider, both)
		.GetValue().contribution.hp, 300);
}

MODERN_TEST(PassiveAggregator_LevelAboveTheDefinitionMaxIsSkipped)
{
	// RAN indexes sDATA_LVL by the learned level, and a level past the
	// definition's own maximum has no row. Skipping rather than reading
	// out of range is what keeps this from being undefined behaviour.
	SkillDefinition def = MakeHpSkill(1, 100.0f, 2);
	def.levelData[3].basicVar = 999.0f;  // beyond maxLevel; must not be read

	InMemorySkillDefinitions provider;
	CHECK(provider.Add(def).IsOk());

	SkillState skills;
	CHECK(skills.LearnSkill(Id(1)).IsOk());
	CHECK(skills.SetSkillLevel(Id(1), 3).IsOk());
	EquipmentState equipment;

	const Result<PassiveContributionResult> aggregated =
		PassiveContributionAggregator::Aggregate(skills, provider, equipment);

	CHECK(aggregated.IsOk());
	if (aggregated.IsError())
	{
		return;
	}
	// Skipped silently: the skill is known, so this is not a data fault.
	CHECK(aggregated.GetValue().IsOk());
	CHECK_EQ(aggregated.GetValue().contribution.hp, 0);
}

MODERN_TEST(PassiveAggregator_IsRepeatable)
{
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeHpSkill(10, 100.0f)).IsOk());
	CHECK(provider.Add(MakeHpSkill(20, 40.0f)).IsOk());

	SkillState skills;
	CHECK(skills.LearnSkill(Id(10)).IsOk());
	CHECK(skills.LearnSkill(Id(20)).IsOk());
	EquipmentState equipment;

	const Stats::PassiveContribution first =
		PassiveContributionAggregator::Aggregate(skills, provider, equipment)
			.GetValue().contribution;
	for (int repeat = 0; repeat < 16; ++repeat)
	{
		CHECK(PassiveContributionAggregator::Aggregate(skills, provider, equipment)
			.GetValue().contribution == first);
	}
}

MODERN_TEST(PassiveAggregator_DoesNotMutateItsInputs)
{
	InMemorySkillDefinitions provider;
	CHECK(provider.Add(MakeHpSkill(1, 100.0f)).IsOk());

	SkillState skills;
	CHECK(skills.LearnSkill(Id(1)).IsOk());
	const SkillState skillsBefore = skills;

	EquipmentState equipment;
	CHECK(equipment.Equip(EquipmentSlot::RightHand, MakeInstance(1)).IsOk());
	const EquipmentEntry rightHandBefore = equipment.Get(EquipmentSlot::RightHand);

	(void) PassiveContributionAggregator::Aggregate(skills, provider, equipment);

	CHECK(skills == skillsBefore);
	CHECK(equipment.Get(EquipmentSlot::RightHand).item == rightHandBefore.item);
	CHECK_EQ(equipment.GetOccupiedCount(), static_cast<size_t>(1));
}

#include "movement/MovementSpeed.h"

namespace Modern::Movement
{
	namespace
	{
		// WORLD-ENTRY-002c §2.3, one row per `EMCHARINDEX`, in the order
		// GLCharDefine.h:237-252 declares them.
		//
		//     index  class              SETFILE key          WALK  RUN
		//        0   BrawlerMale         BRAWLER_M            14.0  37.0
		//        1   SwordsmanMale       SWORDSMAN_M          12.0  36.0
		//        2   ArcherFemale        ARCHER_W             16.0  42.0
		//        3   ShamanFemale        SHAMAN_W             13.0  40.0
		//        4   ExtremeMale         EXTREME_M            14.0  39.0
		//        5   ExtremeFemale       EXTREME_W            14.0  39.0
		//        6   BrawlerFemale       BRAWLER_W            14.0  37.0
		//        7   SwordsmanFemale     SWORDSMAN_W          12.0  36.0
		//        8   ArcherMale          ARCHER_M             16.0  42.0
		//        9   ShamanMale          SHAMAN_M             12.0  39.0
		//       10   GunnerMale          GUNNER_M             15.0  40.0
		//       11   GunnerFemale        GUNNER_W             15.0  40.0
		//       12   AssassinMale        ASSASSIN_M           12.0  44.0
		//       13   AssassinFemale      ASSASSIN_W           12.0  44.0
		//       14   TrickerMale         TRICKER_M            15.0  41.0
		//       15   TrickerFemale       TRICKER_W            15.0  41.0
		//
		// Transcribed, not derived. The 12/34 constructor defaults that 002b used are
		// wrong for all sixteen rows: no class walks at 12 and runs at 34, and 34 is
		// not in the run column at all.
		constexpr ClassMoveSpeed kClassSpeeds[Stats::kClassCount] = {
		    { 14.0f, 37.0f }, // 0  BrawlerMale
		    { 12.0f, 36.0f }, // 1  SwordsmanMale
		    { 16.0f, 42.0f }, // 2  ArcherFemale
		    { 13.0f, 40.0f }, // 3  ShamanFemale
		    { 14.0f, 39.0f }, // 4  ExtremeMale
		    { 14.0f, 39.0f }, // 5  ExtremeFemale
		    { 14.0f, 37.0f }, // 6  BrawlerFemale
		    { 12.0f, 36.0f }, // 7  SwordsmanFemale
		    { 16.0f, 42.0f }, // 8  ArcherMale
		    { 12.0f, 39.0f }, // 9  ShamanMale
		    { 15.0f, 40.0f }, // 10 GunnerMale
		    { 15.0f, 40.0f }, // 11 GunnerFemale
		    { 12.0f, 44.0f }, // 12 AssassinMale
		    { 12.0f, 44.0f }, // 13 AssassinFemale
		    { 15.0f, 41.0f }, // 14 TrickerMale
		    { 15.0f, 41.0f }, // 15 TrickerFemale
		};

		static_assert(sizeof(kClassSpeeds) / sizeof(kClassSpeeds[0]) == Stats::kClassCount,
		              "the speed table must have exactly one row per character class");
	}

	const ClassMoveSpeed* ClassSpeedTable() noexcept
	{
		return kClassSpeeds;
	}

	std::size_t ClassSpeedCount() noexcept
	{
		return sizeof(kClassSpeeds) / sizeof(kClassSpeeds[0]);
	}

	const ClassMoveSpeed* ClassSpeedFor(Stats::CharClassIndex index) noexcept
	{
		if (!Stats::IsValidClass(index))
		{
			return nullptr;
		}
		return &kClassSpeeds[static_cast<std::size_t>(index)];
	}

	bool TryGetBaseVelocity(Stats::CharClassIndex index, bool running, float& out) noexcept
	{
		const ClassMoveSpeed* speed = ClassSpeedFor(index);
		if (speed == nullptr)
		{
			out = 0.0f;
			return false;
		}

		out = running ? speed->runVelocity : speed->walkVelocity;
		return true;
	}

	float MoveVelocity(float baseVelocity, const MoveVelocityTerms& terms, bool running)
	{
		// The run/walk branch already happened upstream, in the choice of
		// `baseVelocity`. `running` is kept in the signature because RAN's
		// `MoveVelocity` takes it (GLChar.cpp:4973) and because it documents that
		// NOTHING in the formula branches on it: the divisor for the item term is
		// `fRUNVELO` either way (002c §3.1), and switching it with the run flag is the
		// easiest way to disagree with RAN for a walking character.
		(void)running;

		float effective = terms.stateMultiplier;

		// `itemDivisor` is zero in the default terms, because there is no item term
		// to divide. Dividing anyway would produce an infinity that turns any speed
		// into NaN one tick later, so the guarded form is used and the no-item case is
		// the arithmetic identity it should be.
		if (terms.moveItem != 0.0f && terms.itemDivisor != 0.0f)
		{
			effective += terms.moveItem / terms.itemDivisor;
		}

		return baseVelocity * effective;
	}
}

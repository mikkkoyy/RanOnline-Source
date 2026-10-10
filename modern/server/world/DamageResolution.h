#pragma once

// WORLD-ENTRY-002K: the damage resolution boundary.
//
// WHAT THIS IS
//
// A PURE function turning a validated attack plus two rolled values into an
// outcome: avoided, or a damage figure. It performs no I/O, mutates no global
// state, and owns no clock or RNG - every random input arrives as a parameter.
//
// It is deliberately NOT the thing that applies the damage. `FieldRoleRuntime`
// does that, through `ResourceSyncService::ApplyDamage`, and THIS milestone sends
// the number that service actually applied. Keeping resolution and application
// apart is what lets a test pin the resolution's arithmetic without a socket, and
// what lets the resource layer stay the single owner of HP.
//
// THE ARITHMETIC IS NOT INVENTED HERE
//
// Resolution delegates to `HitCalculator::CalculateHit` and
// `PhysicalDamageCalculator::CalculatePhysicalDamage`, both of which are already
// implemented and tested in core and both of which were derived from legacy:
//
//   * HitRate       GameCharacterCalculations.cpp:68  - `100 + hit - avoid +
//                                                      brightMod`, clamped [20,99]
//   * RandomDamageRange  :602  - `low + (high-low) * roll`, truncating
//   * Defense / critical / crushing / reflection / state damage, same file
//
// The legacy entry points are `GLCHARLOGIC::CALCDAMAGE` (GLogixExPC.cpp:1329),
// which dispatches to `CALCDAMAGE_20060328` (:1363) in released builds, and the
// hit roll in `GLCHARLOGIC::CHECKHIT` (:1291). Their reference is the modern
// core calculator, not a re-derivation.
//
// THE PROTOTYPE GAP - READ THIS BEFORE TRUSTING A NUMBER
//
// Legacy's inputs are item-derived and the modern Field path has none of them:
//
//   * `m_gdDAMAGE` comes from the equipped weapon (item.csv, undecoded);
//   * `GETBODYRADIUS()`, `GETATTACKRANGE()` and `m_wSUM_TARRANGE()` likewise;
//   * `GETDEFENSE`, `m_nSUM_HIT`, `m_nSUM_AVOID` are aggregated from equipment,
//     passives and codex, none of which the live Field role computes.
//
// `WorldCharacter` carries id, name, class, school, level, gender and gaeaId -
// and no combat statistics at all. `ServerCharacter` HAS the full derived stat
// pipeline and is entirely test-only; integrating it is explicitly a later
// milestone.
//
// So the stat inputs below are PROTOTYPE values, named as such, supplied by the
// caller in `DamageInput`. This milestone therefore proves the resolution
// BOUNDARY and the wire, not legacy numerical parity. Nothing here claims that a
// modern 3036 lands the same number a RAN 3036 would. Replacing these inputs
// with recovered item data must not change this file's shape - only the values a
// caller passes.

#include "AttackDamageProtocol.h"
#include "AttackProtocol.h"
#include "types/Result.h"

#include <cstdint>
#include <string>

namespace Modern::Server::World
{
	using Network::WireU32;

	// ---- THE PROTOTYPE STATISTICS, named where a caller can see them ---------
	//
	// These are the values the Field role passes until item-derived attack and
	// defence aggregates exist. They are NOT recovered RAN numbers and no parity is
	// claimed for them; they exist so the damage boundary can be exercised
	// end to end while the inputs are still unavailable.
	//
	// The hit/avoid pair is RAN's own documented placeholder: `m_nSUM_HIT =
	// m_nSUM_AVOID = 30` appears at GLogicExNPC.cpp:722, GLogixExPC.cpp:2525 and
	// GLSummon.cpp:871 as a stand-in before equipment is aggregated. Carrying the
	// same 30/30 means the hit rate starts at the legacy midpoint of 50 rather
	// than at an arbitrary point, and it is the one prototype input that is
	// actually lifted from the source.
	inline constexpr std::int32_t kPrototypeHit   = 30;
	inline constexpr std::int32_t kPrototypeAvoid = 30;

	// The damage range, in HP units. RAN's `m_gdDAMAGE` comes from the equipped
	// weapon (item.csv, undecoded), so these are placeholders chosen to be small
	// and non-zero: zero would make every attack a no-op and prove nothing, and
	// large would trivially overkill any test target.
	inline constexpr Network::WireU32 kPrototypeLowDamage  = 10;
	inline constexpr Network::WireU32 kPrototypeHighDamage = 20;

	// What happened to one attack.
	enum class DamageOutcome : std::uint8_t
	{
		// The hit roll failed. Legacy: `CHECKHIT` false -> `GLChar::AvoidProc`
		// (GLChar.cpp:2852), which sends 3041/3042. 002i already routes those.
		Avoided,

		// The roll succeeded and produced a figure. NOT YET APPLIED - the caller
		// applies it and reports back what was really applied.
		Hit,

		// The resolution refused: no attacker, no target, or a roll outside [0,1].
		// Nothing goes on the wire.
		Refused,
	};

	const char* ToString(DamageOutcome outcome) noexcept;

	// ---------------------------------------------------------------------------
	// THE RUNTIME CONTEXT: the two characters' levels and HP
	// ---------------------------------------------------------------------------
	//
	// Legacy's CALCDAMAGE_20060328 (GLogixExPC.cpp:1363-1723) reads the attacker's
	// level and HP and the target's level: `nLEVEL = pActor->GetLevel()`,
	// `GETHP()`, `GETMAXHP()`, `GETLEVEL()`. Those are not statistics and they
	// are not recoverable from `class<N>.classconst` - they are live per-attack
	// state, so they belong to the runtime, and 002K could not supply them.
	//
	// 002K therefore hardcoded them (attacker/target level 1, HP 100/100), which
	// was labelled a prototype. This struct is what removes that: the caller
	// states the values AND says where they came from.
	//
	// `authoritative` is the load-bearing field. When it is false, every other
	// field is a placeholder and the resolver REFUSES the attack rather than
	// computing with a level it substituted. Legacy's own level term is the
	// single largest lever in the damage formula (`CriticalBaseRate` takes a
	// ±5 level delta, and the final defence reduction is scaled by the target's
	// level), so a silently-wrong level produces a confidently wrong number.
	//
	// The values it carries are:
	//
	//   attackerLevel / targetLevel   `WorldCharacter::level`, authoritative
	//   attackerCurrentHP             `ResourceSyncService`'s live HP
	//   attackerMaxHP                 `ResourceSyncService`'s maximum
	//
	// HP is READ from the owner, never duplicated: `FieldRoleRuntime` borrows
	// `ResourceSyncService`, which stays the single owner. `targetCurrentHP` is
	// deliberately absent - the calculator does not read the target's current HP,
	// only its level, and carrying a value nothing consumes invites a caller to
	// think it mattered.
	struct DamageContext
	{
		std::int32_t  attackerLevel = 1;
		std::int32_t  targetLevel   = 1;
		std::uint32_t attackerMaxHP     = 100;
		std::uint32_t attackerCurrentHP = 100;

		// True only when every field above was read from authoritative runtime
		// state for THIS attack. False means the resolver must refuse.
		bool authoritative = false;
	};

	// Everything the resolution is allowed to know.
	struct DamageInput
	{
		// ---- identity, already validated by the caller --------------------
		//
		// The caller has already proved both exist and are in the world; this
		// struct only carries their ids so the rule can report them.
		WireU32 attackerGaeaId = 0;
		WireU32 targetGaeaId   = 0;
		WireU32 targetCrow     = Network::Attack::kCrowPc;

		// ---- rolls, all in [0,1] and all injected ---------------------------
		//
		// Legacy's RANDOM_POS is `rand()/RAND_MAX` (GLDefine.h:11) and is rolled
		// per strike in `PreStrikeProc` (GLChar.cpp:2416). Passing the values in
		// rather than generating them here is what makes every case in
		// DamageResolutionTests deterministic: no seed, no clock, no retry.
		float hitRoll       = 0.0f;
		float damageRoll    = 0.0f;
		float criticalRoll  = 0.0f;
		float crushingRoll  = 0.0f;
		float reflectionRoll = 0.0f;

		// ---- PROTOTYPE combat statistics -----------------------------------
		//
		// These stand in for legacy's item-derived aggregates. See the header for
		// why no value here is a recovered RAN number.
		struct PrototypeStats
		{
			// Attacker's damage range, in HP units.
			WireU32 lowDamage  = 10;
			WireU32 highDamage = 20;

			// `m_nSUM_HIT` / `m_nSUM_AVOID`.
			std::int32_t hit  = 30;
			std::int32_t avoid = 30;

			// `meleePower` / `shootPower`, the attack power applied to the range.
			std::uint16_t meleePower = 5;
			std::uint16_t shootPower = 5;

			// Target's aggregates. All zero means "no mitigation at all", which is
			// the honest value when the modern path has none: damage then passes
			// through unreduced, and the flag says so.
			std::int32_t defense      = 0;
			std::int32_t defenseBody  = 0;

			// The TARGET's item defence, and why zero is the correct value here
			// rather than a placeholder.
			//
			// Legacy's CALCDAMAGE reads `nITEM_DEFENSE = pActor->GetItemDefense()`
			// (GLogixExPC.cpp:1385), and the base `GLACTOR::GetItemDefense()`
			// returns 0 (GLogicEx.h:49) - only the PC override returns
			// `m_sSUMITEM.nDefense` (GLogicEx.h:495). The live world server has no
			// equipped items, so the honest value for an unarmoured target is 0,
			// and the final body-by-item defence reduction is skipped by the
			// calculator's own `defenseItem > 0` guard
			// (PhysicalDamageCalculator.h:227). This field exists so that when
			// equipment lands it has somewhere to go; it is NOT a missing input.
			std::int32_t defenseItem  = 0;

			// The TARGET's level, for the caller's bookkeeping only. The level the
			// calculator uses is `DamageContext::targetLevel` - see the struct
			// above. This one is left at its default so a caller cannot mistake it
			// for an input that was wired up.
			std::int32_t level        = 1;

			std::int32_t resistElement = 0;

			// Multipliers, all neutral by default so the prototype formula reduces
			// to the rolled range.
			float damageRate  = 1.0f;
			float stateDamage = 1.0f;
			float damageReduce = 0.0f;
		};

		PrototypeStats stats{};

		// The live per-attack context: both characters' levels and the
		// attacker's HP. 002L-C. Not authoritative by default, and a
		// non-authoritative context refuses the attack.
		DamageContext context{};
	};

	// What the resolution decided.
	struct DamageResult
	{
		DamageOutcome outcome = DamageOutcome::Refused;

		// The damage the RESOLUTION asked for. This is NOT what the caller must
		// report on the wire - see `appliedDamage` and the header.
		WireU32 requestedDamage = 0;

		// The damage actually removed from the target's HP.
		//
		// Set by the CALLER, from `ResourceSyncService::ApplyDamage`'s return,
		// which is GLCHARLOGIC::RECEIVE_DAMAGE's "difference actually lost"
		// (GLogixExPC.cpp:2093). It differs from `requestedDamage` whenever the
		// hit overkills, and it is the value 3043/3044 must carry.
		//
		// Defaults to `kUnapplied`, so a caller that forgets to report cannot
		// accidentally ship the requested figure.
		inline static constexpr WireU32 kUnapplied = 0xFFFFFFFFu;
		WireU32                      appliedDamage = kUnapplied;

		// DAMAGE_TYPE_* flags, as built by the calculator.
		WireU32 damageFlag = Network::Attack::kDamageTypeNone;

		// The final hit rate, 0-100. Reported so a test can assert the roll rather
		// than infer it from whether the outcome happened to be Avoided.
		WireU32 hitRate = 0;

		// Why it was refused, in one line. Empty otherwise.
		std::string detail;
	};

	// The rule. Stateless, pure, and safe to call from a worker thread.
	class DamageResolution
	{
	public:
		// Resolves one attack.
		//
		// `attackerPresent` / `targetPresent` are the caller's already-verified
		// world-membership facts. They are parameters rather than something the
		// rule looks up, so this stays a pure function and the Field role keeps
		// ownership of who is connected.
		static DamageResult Resolve(bool attackerPresent, bool targetPresent,
		                            const DamageInput& input);
	};
} // namespace Modern::Server::World

#pragma once

// VERTICAL-005: HP / MP / SP as a domain of its own.
//
// CORE-002 already computes the three maxima and the three recovery rates, and
// those values are the *only* place they come from. This header adds what a
// stat result cannot hold: the mutable part. A maximum is a property of a
// character; a current value is a property of a moment, and it changes for
// reasons that have nothing to do with statistics - a potion, a hit, a tick of
// regeneration. Putting that in `DerivedStats` would make a stat result
// time-varying, which is what the stat chain is not.
//
// The split follows the legacy one exactly. RAN holds a `GLDWDATA` per resource
// (GLDefine.h:400) - a `dwNow` / `dwMax` pair - next to the maxima the stat
// pass produced, and mutates only the `dwNow` half afterwards. So:
//
//   - `Stats::Calculate` produces the maxima and the recovery terms. Unchanged.
//   - `ResourceState` owns the current values and the rules that move them.
//
// Nothing here recomputes a maximum. `SyncFrom` copies the numbers the stat pass
// produced; it contains no formula. That is the property that keeps a second
// stat path from appearing.
//
// The three operations RAN has, and their modern equivalents:
//
//   GLDWDATA::LIMIT      -> ClampToMaximum, run after every operation
//   GLDWDATA::TO_FULL   -> FullRestore
//   GLDWDATA::DECREASE  -> Spend / ApplyDamage, saturating at zero
//   GLDWDATA::INCREASE  -> Restore, clamping at the maximum
//   GLOGICEX::UPDATE_POINT -> Recover, including the carried remainder
//   GLCHARLOGIC::INIT_RECOVER -> RespawnRestore
//
// What is deliberately absent: damage types, critical hits, absorption, life
// steal, death, resurrection, potions, and any notion of a game loop. A caller
// that wants those calls the operations here and owns the rules.

#include "stats/DerivedStats.h"
#include "types/Result.h"

#include <cstdint>

namespace Modern::Resources
{
	// Which of the three pools an operation names.
	//
	// RAN has no equivalent: it reaches for `m_sHP` / `m_sMP` / `m_sSP` by name
	// at every call site, which is why the same recovery loop appears three
	// times in UPDATE_POINT and three more in `GLCHARLOGIC::UpdateCharacter`.
	// Naming the pool once is what lets the operations below be written once
	// instead of three times, and it is what makes a table-driven test
	// possible.
	enum class ResourceKind : uint8_t
	{
		Hp,
		Mp,
		Sp,
	};

	inline constexpr size_t kResourceKindCount = 3;

	constexpr bool IsValid(ResourceKind kind) noexcept
	{
		return kind == ResourceKind::Hp || kind == ResourceKind::Mp ||
		       kind == ResourceKind::Sp;
	}

	// The floor each pool's *regeneration* will not leave the character below.
	//
	// legacy/Lib_Client/G-Logic/GLogicEx.h:270, the `dwLOW_LMT` parameter of
	// UPDATE_POINT, as called at GLogixExPC.cpp:3024-3026:
	//
	//   UPDATE_POINT ( m_sHP, m_fIncHP, fINC_HP, 1 );
	//   UPDATE_POINT ( m_sMP, m_fIncMP, fINC_MP, 0 );
	//   UPDATE_POINT ( m_sSP, m_fIncSP, fINC_SP, 0 );
	//
	// So HP regenerates to 1 and never to 0, while MP and SP regenerate to
	// whatever they can reach, including nothing at all. This is a floor on one
	// operation only: damage still takes HP to 0, because DECREASE has no floor
	// (GLDefine.h:443).
	//
	// It is reproduced rather than tidied to 0, and the consequence is
	// documented rather than hidden: recovering a character that is at 0 HP
	// leaves it at 1, so this system does not keep a dead character dead. RAN
	// relies on the death system to stop recovery before it matters. Death is
	// out of scope here, so a caller driving `Recover` on a character it
	// believes is dead will see it reach 1 HP. That is the legacy behaviour and
	// is the one thing a future death milestone must gate.
	namespace RecoveryFloor
	{
		inline constexpr uint32_t kHp = 1;
		inline constexpr uint32_t kMp = 0;
		inline constexpr uint32_t kSp = 0;
	}

	// One resource: a maximum and a current value.
	//
	// RAN's `GLDWDATA` (GLDefine.h:400) is a `union` of three name pairs over
	// two `DWORD`s - `(dwNow, dwMax)`, `(dwData1, dwData2)` and
	// `(dwLow, dwHigh)`. The union is why damage ranges and resource pools are
	// the same type in RAN, and it is why the same struct is a legal thing to
	// find in a damage field.
	//
	// Here the two are only ever one thing, and the damage range is its own type
	// (`Stats::DamageRange`). A union that can be read as either a pool or a
	// damage range is a way to pass a pool where a range belongs, and there is
	// nothing the modern tree gains from being able to.
	struct Pool
	{
		uint32_t maximum = 0;
		uint32_t current = 0;

		constexpr bool operator==(const Pool& other) const noexcept
		{
			return maximum == other.maximum && current == other.current;
		}
	};

	// How one pool regenerates, in the two units RAN's formula actually uses.
	//
	// GLogixExPC.cpp:3020:
	//
	//   fINC_HP = fElap * ( m_sHP.dwMax * fINCR_HP
	//                      + GLCONST_CHAR::fHP_INC
	//                      + m_sSUMITEM.fInc_HP );
	//
	// `rate` is the fraction-of-maximum term (`fINCR_HP`) and `flat` is the
	// absolute one. They are different units, so they cannot be one number.
	//
	// `remainder` is the sub-unit carry. UPDATE_POINT accumulates a float,
	// consumes only its whole part, and keeps the fraction for the next call:
	//
	//   fELP_VAR += fVAR;
	//   int nNEWP = int(sPOINT.dwNow) + int(fELP_VAR);
	//   fELP_VAR -= int(fELP_VAR);
	//
	// Without it, a pool regenerating 0.3 HP per tick would gain nothing at all,
	// ever. This is the single most important detail in the recovery rule, and
	// it is state, not a local, so it lives here and is carried across calls.
	struct PoolRecovery
	{
		float rate     = 0.0f;
		float flat     = 0.0f;
		float remainder = 0.0f;

		constexpr bool operator==(const PoolRecovery& other) const noexcept
		{
			return rate == other.rate && flat == other.flat && remainder == other.remainder;
		}
	};

	// The whole of a character's resources: three pools, three recovery
	// configurations, and the timing divisor.
	//
	// Default-constructed it is the *empty* resource set - every maximum zero -
	// which is a legal and self-consistent state, not a broken one. A character
	// with a zero maximum can spend nothing and regenerate nothing, and every
	// operation below says so rather than refusing.
	class ResourceState
	{
	public:
		// A character that has no maximum resource yet.
		ResourceState() = default;

		// The authoritative pools of a character whose record already holds
	// them, plus the recovery configuration to run them with.
	//
	// The world-entry path is the producer: the DB's ChaHP/ChaMP/ChaSP
	// (s_COdbcGameChaGet.cpp:302-303,317) are the authority for BOTH
	// halves of every pool, and no stat pass has run to build a
	// `DerivedStats` from - which is exactly why `CreateFull` and
	// `SyncFrom` cannot serve this path: they take maxima from a stat
	// result, and `CreateFull` additionally starts every pool FULL,
	// which would resurrect a character that saved at 47 HP.
	//
	// The recovery terms are the caller's, and the world-entry caller
	// supplies the legacy globals - with no items, passives or facts
	// in the world model, those are the only terms the legacy formula
	// has left (GLogixExPC.cpp:3021-3023).
	struct ResourceRecord
	{
		Pool         hp;
		Pool         mp;
		Pool         sp;

		PoolRecovery hpRecovery;
		PoolRecovery mpRecovery;
		PoolRecovery spRecovery;
	};

	// Adopts a record verbatim - both halves of every pool, and the
	// recovery configuration.
	//
	// A current value above its maximum is CLAMPED, not refused: the
	// record is authoritative data, not a request, and `LIMIT`
	// (GLDefine.h:439) is what legacy does with an over-maximum load
	// rather than discarding the character. A non-finite recovery term
	// is refused, for the same reason `SyncFrom` refuses one.
	//
	// The remainders start at zero: a fresh session is not owed
	// anything, and the DB stores no sub-unit carry.
	static Result<ResourceState> Adopt(const ResourceRecord& record);

	// Builds the state from one stat result. The pools start *full*.
		//
		// Full is RAN's behaviour for a new character: `INIT_DATA` calls
		// `TO_FULL()` on all three when `bNEW` (GLogixExPC.cpp:1255-1264). It is
		// the right default here too, because a character whose maxima arrive
		// from a calculation the server performed has no earlier current value to
		// carry, and starting at zero would publish a character that is
		// indistinguishable from one at 1 HP.
		//
		// Fails with InvalidArgument if `derived` holds a non-finite recovery
		// term. `Stats::Calculate` already refuses those, so reaching this is a
		// programming error rather than a data condition, and refusing it here
		// means no `ResourceState` can exist that would propagate a NaN into a
		// current value.
		static Result<ResourceState> CreateFull(const Stats::DerivedStats& derived);

		// Adopts a stat result's maxima and recovery terms.
		//
		// Current values are *preserved* when the maximum did not fall, and
		// clamped down when it did. That asymmetry is the whole policy for a
		// changing maximum, and it is described in full on the implementation.
		//
		// This contains no formula. It is a copy, which is what stops the
		// resource domain from becoming a second stat calculation.
		Status SyncFrom(const Stats::DerivedStats& derived);

		// ---- Reading ----

		const Pool& GetPool(ResourceKind kind) const noexcept;

		uint32_t GetMaximum(ResourceKind kind) const noexcept;
		uint32_t GetCurrent(ResourceKind kind) const noexcept;

		// The fraction of the maximum remaining, for a bar. Zero when the maximum
		// is zero rather than a division by it, and clamped to [0, 1] so a
		// current value that somehow exceeds its maximum cannot render a bar
		// past full.
		float GetFraction(ResourceKind kind) const noexcept;

		const PoolRecovery& GetRecovery(ResourceKind kind) const noexcept;

		// The recovery rate as RAN's status window shows it: the stored fraction
		// times 100, so the constant 0.003 reads as "0.30".
		//
		// legacy/Lib_ClientUI/Interface/CharacterWindow/
		// CharacterWindowCharAdditionalInfo.cpp:434, which formats
		// `m_fINCR_HP * 100.0f` as a percentage. Confirming the stored value is
		// a fraction and not a percentage.
		float GetRecoveryPercent(ResourceKind kind) const noexcept;

		// The carried sub-unit accumulator, exposed for tests and for a future
		// scheduler that needs to persist it. Not a presentation value.
		float GetRecoveryRemainder(ResourceKind kind) const noexcept;

		// `GLCONST_CHAR::fUNIT_TIME`, the divisor turning elapsed time into
		// recovery units. Defaults to RAN's shipped 1.0f.
		float GetUnitTime() const noexcept { return m_unitTime; }

		// ---- Setting the current value directly ----
		//
		// Refuses a value above the maximum rather than clamping it, which is a
		// deliberate divergence from `LIMIT()`: an out-of-range assignment is
		// then visible as a failure instead of silently becoming full. The
		// *automatic* paths - a maximum change, a restore - do clamp, because
		// those are not requests and have nothing to report.
		Status SetCurrent(ResourceKind kind, uint32_t value);

		// ---- Full restore ----
		//
		// `GLDWDATA::TO_FULL` (GLDefine.h:441): every pool to its maximum. RAN
		// uses it on character creation, and `INIT_RECOVER` uses `CHECKMIN`
		// instead for the partial case.
		//
		// Does not clear the recovery remainders. Regeneration state is not part
		// of "full"; a character that is manually topped up keeps whatever
		// fraction it was owed, which is what the legacy code does because
		// `TO_FULL` does not touch `m_fIncHP` either.
		void FullRestore();

		// ---- Restoring ----
		//
		// `GLDWDATA::INCREASE(value, bRate=FALSE)` (GLDefine.h:449): adds a flat
		// amount, then clamps to the maximum.
		//
		// Returns InvalidArgument for a zero amount or an invalid kind. Zero is
		// refused rather than accepted as a no-op so a caller passing an unset
		// potion value finds out.
		Status Restore(ResourceKind kind, uint32_t amount);

		// Restores a fraction of the maximum, `GLDWDATA::INCREASE(value,
		// bRate=TRUE)`. The amount is `(maximum * percent) / 100` in unsigned
		// arithmetic, exactly as RAN computes it, so a percent that does not
		// divide evenly rounds *down* and cannot overflow the multiplication.
		//
		// A percent of 100 or more is refused. RAN would compute more than a
		// full pool and let `LIMIT` clip it, which is the same end state by way
		// of a value that briefly exceeded the maximum.
		Status RestorePercent(ResourceKind kind, uint32_t percent);

		// ---- Spending ----
		//
		// `GLDWDATA::DECREASE` (GLDefine.h:443):
		//
		//   if ( dwNow >= dwValue ) dwNow -= dwValue;  else dwNow = 0;
		//
		// Saturating, never wrapping, and never refused for being larger than
		// the pool: a spell that costs more MP than the character has spends the
		// MP it has. Returns the amount actually spent, so a caller can tell a
		// full cost from a partial one.
		uint32_t Spend(ResourceKind kind, uint32_t amount);

		// ---- Damage ----
		//
		// `GLCHARLOGIC::RECEIVE_DAMAGE` (GLogixExPC.cpp:2093), which is
		// `DECREASE` plus a report of the difference:
		//
		//   DWORD dwOLD = m_sHP.dwNow;
		//   m_sHP.DECREASE ( dwDamage );
		//   return (dwOLD>m_sHP.dwNow) ? (dwOLD-m_sHP.dwNow) : 0;
		//
		// Returns the damage actually applied, which is less than the request
		// when the request overkills. An overkill of 100 against 10 HP returns
		// 10.
		//
		// HP only, because that is the pool RAN damages. MP and SP are spent,
		// not damaged, and the distinction is the caller's rather than this
		// function's.
		uint32_t ApplyDamage(uint32_t amount);

		// ---- Regeneration ----
		//
		// `GLOGICEX::UPDATE_POINT` applied to all three pools, in one call, as
		// `GLCHARLOGIC::UpdateCharacter` does at GLogixExPC.cpp:3024-3026.
		//
		// For each pool:
		//
		//   units   = elapsedTime / unitTime
		//   amount  = units * ( maximum * rate + flat )
		//   carried = remainder + amount
		//   whole   = trunc(carried)          // toward zero, as int() does
		//   current = clamp(current + whole, floor, maximum)
		//   remainder = carried - whole
		//
		// Three deliberate departures from the legacy text, all of them the same
		// departure - RAN's version of this arithmetic is undefined for large
		// values, and undefined is not something to reproduce:
		//
		//   1. `int(fELP_VAR)` becomes a truncating double conversion with a
		//      saturating range check. RAN's `int` cast of a float outside
		//      `int` range is undefined, and `int(sPOINT.dwNow)` is undefined
		//      for any pool above `INT_MAX`, which a codex bonus can produce.
		//   2. The addition is done in unsigned 64-bit and then narrowed, so
		//      `current + whole` cannot wrap the way RAN's `int` can.
		//   3. `if (nNEWP > UINT_MAX) nNEWP = UINT_MAX;` is removed. In RAN that
		//      comparison converts a negative `int` to `unsigned` and is
		//      therefore never true in the way it reads; the clamp that actually
		//      does the work is `sPOINT.LIMIT()`. Here the maximum *is* the
		//      clamp, and a pool's maximum is at most `UINT32_MAX` by
		//      construction, so the guard is redundant rather than reproduced.
		//
		// Fails with InvalidArgument for a negative, non-finite, or zero elapsed
		// time, or a non-positive unit time. Recovery is driven forward by the
		// caller, so a caller with no clock has no recovery; nothing here reads
		// one.
		Status Recover(float elapsedTime);

		// ---- The partial restore on respawn ----
		//
		// `GLCHARLOGIC::INIT_RECOVER` (GLogixExPC.cpp:1156), which is the
		// *other* half of RAN's recovery rules and is not `Recover`:
		//
		//   m_sHP.CHECKMIN ( (m_sHP.dwMax*nRECOVER)/100 + 1 );
		//   m_sMP.CHECKMIN ( (m_sMP.dwMax*nRECOVER)/100 + 1 );
		//   m_sSP.CHECKMIN ( (m_sSP.dwMax*nRECOVER)/100 + 1 );
		//
		// `CHECKMIN` only raises, so this is a floor and not a set. The `+ 1`
		// is present, which means a 30% restore of a maximum below 3 lands on 1
		// rather than 0.
		//
		// Defaults to 30, RAN's value at GLogicEx.h:737. Percent is clamped to
		// 100 rather than refused: a caller asking for 200 wants "as much as
		// possible", and the ceiling is the maximum either way.
		void RespawnRestore(uint32_t percent = 30);

		// ---- Validity ----
		//
		// `0 <= current <= maximum` for all three, and a finite rate, flat and
		// remainder for all three. The second half is the part that catches a
		// state built by a caller rather than by `SyncFrom`.
		//
		// A remainder of any magnitude passes: the carry is not a resource
		// value and is bounded only by the code that produces it.
		bool IsValid() const noexcept;

	private:
		void ClampToMaximum(ResourceKind kind) noexcept;

		Pool           m_pools[kResourceKindCount];
		PoolRecovery   m_recovery[kResourceKindCount];

		// GLCONST_CHAR::fUNIT_TIME. Stored per character rather than read from a
		// global, so the whole state stays a value with no ambient input.
		float m_unitTime = 1.0f;
	};
}

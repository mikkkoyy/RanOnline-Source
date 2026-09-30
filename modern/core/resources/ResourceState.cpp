// VERTICAL-005: the HP / MP / SP implementation.
//
// Every rule here has a legacy line in the comment beside it. The file is
// deliberately boring: no globals, no clock, no I/O, no randomness, and no
// formula for a maximum. The same input always produces the same output.

#include "resources/ResourceState.h"

#include <cmath>
#include <cstdint>

namespace Modern::Resources
{
	namespace
	{
		size_t IndexOf(ResourceKind kind) noexcept
		{
			return static_cast<size_t>(kind);
		}

		// `int(fELP_VAR)` in UPDATE_POINT, defined.
		//
		// RAN's cast is undefined outside the range of `int`, and the accumulator
		// is a float that has already been through a `DWORD * float` product. The
		// range here is the one where the conversion is defined and where a
		// `double` still represents every integer exactly, so the truncation
		// below and RAN's agree on every input RAN actually produces.
		constexpr double kMaxWholeUnits = 4294967296.0; // 2^32

		// Applies a whole-unit delta to a pool, saturating rather than wrapping.
		//
		// RAN computes `int nNEWP = int(dwNow) + int(fELP_VAR)` and then repairs
		// it with a sequence of comparisons. The addition itself is the problem:
		// two ints that overflow is undefined, and it can happen with a pool
		// above `INT_MAX` and a full tick of regeneration. Done in `uint64_t` it
		// is merely a large number, which is then narrowed once, deliberately,
		// before the range is checked.
		uint32_t ApplyDelta(uint32_t current, double whole) noexcept
		{
			if (whole >= 0.0)
			{
				const uint64_t sum = static_cast<uint64_t>(current) +
					static_cast<uint64_t>(whole);
				return sum > UINT32_MAX ? UINT32_MAX : static_cast<uint32_t>(sum);
			}

			// A negative delta larger in magnitude than the pool leaves zero,
			// which is the saturating behaviour DECREASE has.
			const uint64_t magnitude = static_cast<uint64_t>(-whole);
			return magnitude > current ? 0u : static_cast<uint32_t>(current - magnitude);
		}

		// The floor for a pool, never above the pool's own maximum.
		uint32_t FloorFor(ResourceKind kind, uint32_t maximum) noexcept
		{
			uint32_t floor = 0;
			switch (kind)
			{
			case ResourceKind::Hp: floor = RecoveryFloor::kHp; break;
			case ResourceKind::Mp: floor = RecoveryFloor::kMp; break;
			case ResourceKind::Sp: floor = RecoveryFloor::kSp; break;
			}

			return floor > maximum ? maximum : floor;
		}

		// Is every float in a recovery configuration usable?
		bool IsRecoveryUsable(const PoolRecovery& recovery) noexcept
		{
			return std::isfinite(recovery.rate) && std::isfinite(recovery.flat) &&
			       std::isfinite(recovery.remainder);
		}
	}

	Result<ResourceState> ResourceState::CreateFull(const Stats::DerivedStats& derived)
	{
		ResourceState state;
if (state.SyncFrom(derived).IsError())
	{
		return Result<ResourceState>(Status(ErrorCode::InvalidArgument));
	}

		// GLogixExPC.cpp:1255-1264, the `bNEW` branch of INIT_DATA.
		state.FullRestore();
		return Result<ResourceState>(std::move(state));
	}

	Status ResourceState::SyncFrom(const Stats::DerivedStats& derived)
	{
		// Refuse a non-finite recovery term rather than storing it. A NaN rate
		// would turn the first `Recover` into a NaN current value, and the
		// invariant would be unrecoverable afterwards.
		const PoolRecovery candidates[kResourceKindCount] = {
			{ derived.hpRecoveryRate, derived.hpRecoveryFlat, m_recovery[0].remainder },
			{ derived.mpRecoveryRate, derived.mpRecoveryFlat, m_recovery[1].remainder },
			{ derived.spRecoveryRate, derived.spRecoveryFlat, m_recovery[2].remainder },
		};

		for (size_t index = 0; index < kResourceKindCount; ++index)
		{
			if (!IsRecoveryUsable(candidates[index]))
			{
				return Status(ErrorCode::InvalidArgument);
			}
		}

		// Commit. The maxima are copied, never computed - there is no formula
		// here, which is the property that keeps a second stat path from
		// appearing in the resource domain.
		m_pools[IndexOf(ResourceKind::Hp)].maximum = derived.maxHp;
		m_pools[IndexOf(ResourceKind::Mp)].maximum = derived.maxMp;
		m_pools[IndexOf(ResourceKind::Sp)].maximum = derived.maxSp;

		for (size_t index = 0; index < kResourceKindCount; ++index)
		{
			m_recovery[index].rate     = candidates[index].rate;
			m_recovery[index].flat     = candidates[index].flat;
			m_recovery[index].remainder = candidates[index].remainder;
		}

		// A maximum that fell takes the current value down with it, and a
		// maximum that rose leaves the current value alone.
		//
		// The asymmetry is the policy, and it is the only one available. Raising
		// the current value to the new maximum would be a heal - the character
		// would gain the whole of every equipment swap's HP bonus for free - and
		// RAN does not do it, because `LIMIT` only ever moves `dwNow` down. So a
		// stat drop costs the character the part of its pool above the new
		// maximum, and a stat gain costs it nothing. That is RAN's behaviour, it
		// is the conservative direction, and it is what
		// `GLDWDATA::LIMIT` (GLDefine.h:439) does: `if (dwNow > dwMax) dwNow = dwMax`.
		//
		// The remainder deliberately survives a retune. It is a fraction of a
		// unit the character is owed, and a maximum change does not un-owe it.
		for (size_t index = 0; index < kResourceKindCount; ++index)
		{
			ClampToMaximum(static_cast<ResourceKind>(index));
		}

		return Ok();
	}

	const Pool& ResourceState::GetPool(ResourceKind kind) const noexcept
	{
		return m_pools[IndexOf(kind)];
	}

	uint32_t ResourceState::GetMaximum(ResourceKind kind) const noexcept
	{
		return m_pools[IndexOf(kind)].maximum;
	}

	uint32_t ResourceState::GetCurrent(ResourceKind kind) const noexcept
	{
		return m_pools[IndexOf(kind)].current;
	}

	float ResourceState::GetFraction(ResourceKind kind) const noexcept
	{
		const Pool& pool = m_pools[IndexOf(kind)];
		if (pool.maximum == 0)
		{
			// No maximum means no fraction to report, and dividing here is how a
			// status bar fills with NaN. Zero is also the answer that composes:
			// a pool that cannot be spent is an empty pool, not a full one.
			return 0.0f;
		}

		const float fraction = static_cast<float>(pool.current) /
			static_cast<float>(pool.maximum);

		if (fraction < 0.0f)
		{
			return 0.0f;
		}
		return fraction > 1.0f ? 1.0f : fraction;
	}

	const PoolRecovery& ResourceState::GetRecovery(ResourceKind kind) const noexcept
	{
		return m_recovery[IndexOf(kind)];
	}

	float ResourceState::GetRecoveryPercent(ResourceKind kind) const noexcept
	{
		// CharacterWindowCharAdditionalInfo.cpp:434 formats the stored fraction
		// times 100, so the stored value is a fraction.
		return m_recovery[IndexOf(kind)].rate * 100.0f;
	}

	float ResourceState::GetRecoveryRemainder(ResourceKind kind) const noexcept
	{
		return m_recovery[IndexOf(kind)].remainder;
	}

	Status ResourceState::SetCurrent(ResourceKind kind, uint32_t value)
	{
		if (static_cast<uint8_t>(kind) >= kResourceKindCount)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// Refused rather than clamped. This is the one place the modern rule
		// differs from `LIMIT`, and it differs deliberately: an explicit
		// "set this to a value the character cannot have" is a request that
		// should be refused, so the caller learns, instead of being handed a full
		// pool and no indication that the number was wrong. Every automatic path
		// - a maximum change, a restore, regeneration - clamps instead, because
		// those are not requests and have nothing to report.
		if (value > m_pools[IndexOf(kind)].maximum)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		m_pools[IndexOf(kind)].current = value;
		return Ok();
	}

	void ResourceState::FullRestore()
	{
		// GLDWDATA::TO_FULL (GLDefine.h:441).
		for (size_t index = 0; index < kResourceKindCount; ++index)
		{
			m_pools[index].current = m_pools[index].maximum;
		}
	}

	Status ResourceState::Restore(ResourceKind kind, uint32_t amount)
	{
		if (static_cast<uint8_t>(kind) >= kResourceKindCount || amount == 0)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		Pool& pool = m_pools[IndexOf(kind)];

		// GLDWDATA::INCREASE(value, bRate=FALSE) then LIMIT (GLDefine.h:449-455).
		// Saturating in unsigned arithmetic; `LIMIT` is then still applied, so a
		// restore into a pool that is somehow already over its maximum is
		// corrected rather than amplified.
		const uint64_t sum = static_cast<uint64_t>(pool.current) + amount;
		pool.current = sum > UINT32_MAX ? UINT32_MAX : static_cast<uint32_t>(sum);

		ClampToMaximum(kind);
		return Ok();
	}

	Status ResourceState::RestorePercent(ResourceKind kind, uint32_t percent)
	{
		if (static_cast<uint8_t>(kind) >= kResourceKindCount || percent == 0 || percent >= 100)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// GLDWDATA::INCREASE(value, bRate=TRUE) computes `(dwMax*dwValue)/100`
		// in DWORD arithmetic (GLDefine.h:451). Done the same way here, the
		// product cannot overflow and an inexact division rounds down, so a 30%
		// restore of a maximum of 7 is 2 and not 3.
		return Restore(kind, static_cast<uint32_t>(
			(static_cast<uint64_t>(m_pools[IndexOf(kind)].maximum) * percent) / 100));
	}

	uint32_t ResourceState::Spend(ResourceKind kind, uint32_t amount)
	{
		if (static_cast<uint8_t>(kind) >= kResourceKindCount)
		{
			return 0;
		}

		// GLDWDATA::DECREASE (GLDefine.h:443-447): saturating at zero, never
		// refused for exceeding the pool. A spell costing more MP than the
		// character has spends what it has.
		Pool& pool = m_pools[IndexOf(kind)];
		if (amount >= pool.current)
		{
			// `>=` rather than `>`, so spending exactly the pool empties it -
			// RAN's first branch takes that case too, because `dwNow >= dwValue`
			// holds at equality and the subtraction then yields zero.
			const uint32_t spent = pool.current;
			pool.current = 0;
			return spent;
		}

		pool.current -= amount;
		return amount;
	}

	uint32_t ResourceState::ApplyDamage(uint32_t amount)
	{
		// GLCHARLOGIC::RECEIVE_DAMAGE (GLogixExPC.cpp:2093-2099): the decrease,
		// then the difference. Reporting the difference rather than the request
		// is what lets a caller tell an overkill from a hit, and it is why the
		// return type is the damage *applied*.
		return Spend(ResourceKind::Hp, amount);
	}

	Status ResourceState::Recover(float elapsedTime)
	{
		// A negative or non-finite elapsed time is a caller fault, not a state to
		// absorb. Regeneration is driven by whoever owns the clock, and nothing
		// here reads one, so a caller that has no tick has no recovery rather
		// than an accidental one.
		if (!std::isfinite(elapsedTime) || elapsedTime <= 0.0f)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		if (!std::isfinite(m_unitTime) || m_unitTime <= 0.0f)
		{
			return Status(ErrorCode::InvalidArgument);
		}

		// GLogixExPC.cpp:3019: `fElap = (fElapsedTime/fUNIT_TIME)`. The divisor
		// is applied once, to the whole elapsed time, not per pool - the three
		// pools therefore share one elapsed interval, as they do in RAN.
		const float units = elapsedTime / m_unitTime;

		for (size_t index = 0; index < kResourceKindCount; ++index)
		{
			const auto kind = static_cast<ResourceKind>(index);
			Pool&       pool = m_pools[index];
			PoolRecovery& recovery = m_recovery[index];

			if (!IsRecoveryUsable(recovery))
			{
				return Status(ErrorCode::InvalidArgument);
			}

			// GLogixExPC.cpp:3020-3022, the amount:
			//
			//   fINC_x = fElap * ( maxX * fINCR_x + fX_INC + m_sSUMITEM.fInc_X )
			//
			// `rate` is the class constant plus the item rate plus the passive,
			// and `flat` is the class constant plus the item flat. The two are
			// summed in the units RAN sums them, which are different units, and
			// which is why they were not collapsed into one number upstream.
			const double amount = static_cast<double>(units) *
				(static_cast<double>(pool.maximum) * static_cast<double>(recovery.rate) +
					static_cast<double>(recovery.flat));

			// GLOGICEX::UPDATE_POINT (GLogicEx.h:270-284), the accumulator:
			//
			//   fELP_VAR += fVAR;
			//   int nNEWP = int(sPOINT.dwNow) + int(fELP_VAR);
			//   fELP_VAR -= int(fELP_VAR);
			//
			// The whole part is consumed and the fraction is kept, so a pool
			// regenerating less than one unit per tick still gains the point. A
			// tick where nothing is owed changes nothing.
			double carried = static_cast<double>(recovery.remainder) + amount;
			if (!std::isfinite(carried))
			{
				return Status(ErrorCode::InvalidArgument);
			}

			if (carried > kMaxWholeUnits)
			{
				carried = kMaxWholeUnits;
			}
			else if (carried < -kMaxWholeUnits)
			{
				carried = -kMaxWholeUnits;
			}

			// `int()` truncates toward zero, and `std::trunc` is defined to do
			// exactly that, so the two agree including on negatives.
			const double whole = std::trunc(carried);
			recovery.remainder = static_cast<float>(carried - whole);

			const uint32_t applied = ApplyDelta(pool.current, whole);
			const uint32_t floor   = FloorFor(kind, pool.maximum);

			// The order of the two clamps in UPDATE_POINT is preserved: the
			// low limit first, then `LIMIT` against the maximum. When the floor
			// is below the maximum - always, and by construction - the order is
			// not observable, but reproducing it costs nothing and removes a
			// question about what happens if they ever disagree.
			pool.current = applied < floor ? floor : applied;
			ClampToMaximum(kind);
		}

		return Ok();
	}

	void ResourceState::RespawnRestore(uint32_t percent)
	{
		// GLCHARLOGIC::INIT_RECOVER (GLogixExPC.cpp:1156-1160):
		//
		//   m_sHP.CHECKMIN ( (m_sHP.dwMax*nRECOVER)/100 + 1 );
		//
		// CHECKMIN only raises (GLDefine.h:440), so this is a floor. The `+ 1`
		// is in RAN and is kept: a 30% restore of a maximum of 2 is 1, not 0.
		// The division is unsigned, so it rounds down, as RAN's does.
		//
		// A percent above 100 is clamped rather than refused. RAN has no such
		// guard at all, so the value would be whatever the caller passed; the
		// ceiling is the maximum in every case above 100, and clamping is the
		// only difference worth having.
		const uint32_t bounded = percent > 100 ? 100 : percent;

		for (size_t index = 0; index < kResourceKindCount; ++index)
		{
			const uint32_t threshold = static_cast<uint32_t>(
				(static_cast<uint64_t>(m_pools[index].maximum) * bounded) / 100) + 1u;

			if (m_pools[index].current < threshold)
			{
				m_pools[index].current = threshold;
			}
		}

		// CHECKMIN can push a pool above its own maximum when the maximum is
		// small: at 0 maximum the threshold is 1. RAN would publish a pool of 1
		// with a maximum of 0, which violates the invariant this whole file
		// exists to keep, so the maximum wins. For every maximum of 1 or more
		// the threshold is at most the maximum and the clamp does nothing.
		for (size_t index = 0; index < kResourceKindCount; ++index)
		{
			ClampToMaximum(static_cast<ResourceKind>(index));
		}
	}

	bool ResourceState::IsValid() const noexcept
	{
		for (size_t index = 0; index < kResourceKindCount; ++index)
		{
			if (m_pools[index].current > m_pools[index].maximum)
			{
				return false;
			}

			if (!IsRecoveryUsable(m_recovery[index]))
			{
				return false;
			}
		}

		return std::isfinite(m_unitTime) && m_unitTime > 0.0f;
	}

	void ResourceState::ClampToMaximum(ResourceKind kind) noexcept
	{
		Pool& pool = m_pools[IndexOf(kind)];
		if (pool.current > pool.maximum)
		{
			// GLDWDATA::LIMIT (GLDefine.h:439), which is the whole of it.
			pool.current = pool.maximum;
		}
	}
}


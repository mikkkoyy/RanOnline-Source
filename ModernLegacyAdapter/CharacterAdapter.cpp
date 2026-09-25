#include "CharacterAdapter.h"

#include "Character/Character.h"
#include "Math/Vector3.h"
#include "Core/Types.h"

#include "GLChar.h"

#include <cstdio>

namespace Modern
{
	Character CharacterAdapter::FromLegacy(Legacy::GLChar& src)
	{
		Character c;
		c.SetName(src.m_szName);
		c.SetClass(static_cast<uint32_t>(src.m_emClass),
			CharClassToGender(src.m_emClass) == GLGENDER_M ? Gender::Male : Gender::Female);
		c.SetSchool(src.m_wSchool);
		c.SetLevel(src.m_wLevel);
		c.SetExperience(static_cast<uint64_t>(src.m_sExperience.lnNow),
						static_cast<uint64_t>(src.m_sExperience.lnMax));

		Stats stats;
		stats.hpNow = src.m_sHP.dwNow; stats.hpMax = src.m_sHP.dwMax;
		stats.mpNow = src.m_sMP.dwNow; stats.mpMax = src.m_sMP.dwMax;
		stats.spNow = src.m_sSP.dwNow; stats.spMax = src.m_sSP.dwMax;
		c.SetStats(stats);

		const D3DXVECTOR3& pos = src.GetPosition();
		c.SetPosition(Vector3(pos.x, pos.y, pos.z));
		c.SetDirection(Vector3(src.m_vDir.x, src.m_vDir.y, src.m_vDir.z));

		if (src.IsDie())
			c.Die();
		else if (src.IsRunning())
			c.SetAction(ActionType::Move);
		else
			c.SetAction(ActionType::Idle);

		c.SetActState(src.GetSTATE());
		return c;
	}

	void CharacterAdapter::ToLegacy(const Character& src, Legacy::GLChar& dst)
	{
		std::snprintf(dst.m_szName, 32, "%s", src.GetName().c_str());
		dst.m_emClass = static_cast<EMCHARCLASS>(src.GetClassId());
		dst.m_wSchool = src.GetSchool();
		dst.m_wLevel = src.GetLevel();

		const CharacterInfo info = src.Inspect();
		dst.m_sExperience.lnNow = static_cast<LONGLONG>(info.expNow);
		dst.m_sExperience.lnMax = static_cast<LONGLONG>(info.expMax);

		const Stats s = src.GetStats();
		dst.m_sHP.dwNow = s.hpNow; dst.m_sHP.dwMax = s.hpMax;
		dst.m_sMP.dwNow = s.mpNow; dst.m_sMP.dwMax = s.mpMax;
		dst.m_sSP.dwNow = s.spNow; dst.m_sSP.dwMax = s.spMax;

		const Vector3& pos = src.GetPosition();
		const Vector3& dir = src.GetDirection();
		dst.m_vPos = D3DXVECTOR3(pos.x, pos.y, pos.z);
		dst.m_vDir = D3DXVECTOR3(dir.x, dir.y, dir.z);

		dst.ReSetSTATE(EM_ACT_DIE);
		if (src.IsDead())
		{
			dst.SetSTATE(EM_ACT_DIE);
		}
		else
		{
			if ((src.GetActState() & static_cast<uint32_t>(ActState::Run)) != 0)
				dst.SetSTATE(EM_ACT_RUN);
			else
				dst.ReSetSTATE(EM_ACT_RUN);
		}
	}
}

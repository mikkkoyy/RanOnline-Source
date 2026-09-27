#include "CharacterAdapter.h"

#include "character/Character.h"
#include "math/Vector3.h"
#include "types/Types.h"

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
		
		// Set experience by adding the current amount (starts at 0 after Spawn)
		if (src.m_sExperience.lnNow > 0)
			c.AddExperience(static_cast<int64_t>(src.m_sExperience.lnNow));

		c.SetMaxHP(src.m_sHP.dwMax);
		c.SetMaxMP(src.m_sMP.dwMax);
		c.SetMaxSP(src.m_sSP.dwMax);
		c.SetHP(src.m_sHP.dwNow);
		c.SetMP(src.m_sMP.dwNow);
		c.SetSP(src.m_sSP.dwNow);

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

		dst.m_sHP.dwNow = info.hpNow; dst.m_sHP.dwMax = info.hpMax;
		dst.m_sMP.dwNow = info.mpNow; dst.m_sMP.dwMax = info.mpMax;
		dst.m_sSP.dwNow = info.spNow; dst.m_sSP.dwMax = info.spMax;

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

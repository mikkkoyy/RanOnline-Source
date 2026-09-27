#include "stdafx.h"
#include "GLSkillDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace COMMENT
{
	std::string SKILL_LEVEL[SKILL::MAX_LEVEL] =
	{
		"Level01",
		"Level02",
		"Level03",
		"Level04",
		"Level05",
		"Level06",
		"Level07",
		"Level08",
		"Level09",
	};

	std::string SKILL_EFFECT_TYPE[SKILL::EMEFFECT_TYPE_SIZE] = 
	{
		"None",
		"Side Effect",
		"Special Effect",
		"Special Ability",
	};

	std::string SPEC_STIGMA_TYPE[SKILL::EMSPEC_STIGMA_TYPE_SIZE] = 
	{
		"Normal Attack",
		"Skill Attack",
		"Buff",
		"Debuff",
		"Limit",
	};

	std::string SPEC_HALLUCINATE_TYPE[SKILL::EMSPEC_HALLUCINATE_TYPE_SIZE] = 
	{
		"Transform",
		"Force Action",
	};

	std::string SPEC_DOMINATE_TYPE[SKILL::EMSPEC_DOMINATE_TYPE_SIZE] = 
	{
		"Myself",
		"Caster",
		"Random",
		"Fixed",
	};

	std::string SPEC_ASIGN_TYPE[SKILL::EMSPEC_ASIGN_TYPE_SIZE] = 
	{
		"Before",
		"After",
	};

	std::string SPEC_INVISIBLE_USE_TYPE[SKILL::EMSPEC_INVISIBLE_USE_TYPE_SIZE] = 
	{
		"Use during combat status",
	};

	std::string SPEC_INVISIBLE_ANI_TYPE[SKILL::EMSPEC_INVISIBLE_ANI_TYPE_SIZE] = 
	{
		"None",
		"Walking",
		"Running",
	};

	std::string SPEC_SWAPPOS_DIR_TYPE[SKILL::EMSPEC_SWAPPOS_DIR_TYPE_SIZE] = 
	{
		"All",
		"Self",
		"Target",
	};

};
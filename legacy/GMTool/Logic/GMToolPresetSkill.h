#ifndef GMTOOL_PRESET_SKILL
#define GMTOOL_PRESET_SKILL

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Logic/GMToolData.h"

class CGMToolPresetSkill
{

public:
	std::string					m_strPATH;
	SGMTOOL_PRESET_SKILL_MAP	m_MapPresetSkill;

public:
	void	SetPath( std::string strPATH )	{	m_strPATH = strPATH.c_str();	}
	BOOL	PresetLoad();
	BOOL	PresetSave();

public:
	SGMTOOL_PRESET_SKILL*	PresetSkillGet( WORD wID );
	BOOL PresetSkillAdd( SGMTOOL_PRESET_SKILL sPRESET );
	void PresetSkillDel( WORD wID );
	void PresetSkillDelAll();
	WORD PresetSkillNewID();

public:
	CGMToolPresetSkill();
	~CGMToolPresetSkill();

public:
	static CGMToolPresetSkill& GetInstance ();

};

#endif // GMTOOL_PRESET_SKILL
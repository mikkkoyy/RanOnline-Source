/////////////////////////////////////////////////////////////////////////////
//	FileName	: GLAutoSystem.h
//	Project		: Lib -- RanClient
//	Coder		: LG-7

#ifndef GLAUTOSYSTEM_H_
#define GLAUTOSYSTEM_H_

#pragma once

#include "../Lib_Engine/G-Logic/GLDefine.h"
#include "../GLCharDefine.h"

enum
{
	nMAXSKILL_SLOT	= 10,
	nMAXPOTION_SLOT	= 3,
	nMAXPOTION_TYPE = 4
};

struct SAUTO_SKILL
{
	BOOL			m_bActive;
	BOOL			m_bFound;
	int				m_nTab;
	int				m_nLevMin;
	int				m_nLevMax;
	int				m_nRange;
	STARGETID		m_sTargetID;
	SNATIVEID		m_sSkillID;

	SAUTO_SKILL();
	void RESET();
};

struct SAUTO_POTION
{
	BOOL			m_bActive;
	int				m_nTab;
	int				m_nHPSet;
	int				m_nMPSet;
	int				m_nSPSet;

	SAUTO_POTION();
	void RESET();
};

struct SAUTO_FOLLOW
{
	char			m_szName[CHAR_SZNAME];
	STARGETID		m_sTarID;
	BOOL			m_bFound;
	BOOL			m_bActive;
	BOOL			m_bMoving;
	float			m_fReqTime;
	int				m_nRange;

	SAUTO_FOLLOW();
	void RESET();
};

#endif // GLAUTOSYSTEM_H_
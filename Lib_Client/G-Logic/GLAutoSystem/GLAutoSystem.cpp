/////////////////////////////////////////////////////////////////////////////
//	FileName	: GLAutoSystem.cpp
//	Project		: Lib -- RanClient
//	Coder		: LG-7

#include "stdafx.h"
#include "GLAutoSystem/GLAutoSystem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SAUTO_SKILL::SAUTO_SKILL()
	: m_bActive(FALSE)
	, m_bFound(FALSE)
	, m_nTab(-1)
	, m_nLevMin(-1)
	, m_nLevMax(-1)
	, m_nRange(-1)
	, m_sSkillID(NATIVEID_NULL())
{
	m_sTargetID.RESET();
}

void SAUTO_SKILL::RESET()
{
	m_bActive	= FALSE;
	m_bFound	= FALSE;
	m_nTab		= -1;
	m_nLevMin	= -1;
	m_nLevMax	= -1;
	m_nRange	= -1;
	m_sTargetID.RESET();
	m_sSkillID = NATIVEID_NULL();
}

SAUTO_POTION::SAUTO_POTION()
	: m_bActive(FALSE)
	, m_nTab(-1)
	, m_nHPSet(-1)
	, m_nMPSet(-1)
	, m_nSPSet(-1)
{
}

void SAUTO_POTION::RESET()
{
	m_bActive	= FALSE;
	m_nTab		= -1;
	m_nHPSet	= -1;
	m_nMPSet	= -1;
	m_nSPSet	= -1;
}

SAUTO_FOLLOW::SAUTO_FOLLOW()
	: m_bFound(FALSE)
	, m_bActive(FALSE)
	, m_bMoving(FALSE)
	, m_fReqTime(0.0f)
	, m_nRange(-1)
{
	m_sTarID.RESET();

	memset(m_szName, NULL, sizeof(char) * CHAR_SZNAME);
}

void SAUTO_FOLLOW::RESET()
{
	m_sTarID.RESET();
	m_bFound	= FALSE;
	m_bActive	= FALSE;
	m_bMoving	= FALSE;
	m_fReqTime	= 0.0f;
	m_nRange	= -1;

	memset(m_szName, NULL, sizeof(char) * CHAR_SZNAME);
}
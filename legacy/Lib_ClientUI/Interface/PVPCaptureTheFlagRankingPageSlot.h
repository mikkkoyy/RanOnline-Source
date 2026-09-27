/*!
 * \file PVPCaptureTheFlagRankingPageSlot.h
 *
 * \author Juver
 * \date February 2018
 *
 * 
 */

#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "GLPVPCaptureTheFlagData.h"
#include "GLPVPCaptureTheFlagDefine.h"
#include "GLCharDefine.h"

class CBasicTextBox;

class CPVPCaptureTheFlagRankingPageSlot : public CUIGroup
{
private:
	enum
	{
	};

public:
	CPVPCaptureTheFlagRankingPageSlot ();
	virtual	~CPVPCaptureTheFlagRankingPageSlot ();

public:
	void	CreateSubControl( BOOL bSelf );

public:
	CBasicTextBox*	m_pTextRank;
	CBasicTextBox*	m_pTextName;
	CBasicTextBox*	m_pTextFlag;
	CBasicTextBox*	m_pTextKill;
	CBasicTextBox*	m_pTextDeath;
	CBasicTextBox*	m_pTextResu;
	CBasicTextBox*	m_pTextHeal;
	CBasicTextBox*	m_pTextScore;
	CBasicTextBox*	m_pTextPoint;

	CUIControl*		m_pImageWin;
	CUIControl*		m_pImageLose;
	CUIControl*		m_pTeamImage[CAPTURE_THE_FLAG_TEAM_SIZE];
	CUIControl*		m_pSchoolImage[CAPTURE_THE_FLAG_INFO_SCHOOL_SIZE];
	CUIControl*		m_pClassImage[GLCI_NUM_8CLASS];

public:
	BOOL	m_bSelf;
	void	ResetData();
	void	SetData( CAPTURE_THE_FLAG_PLAYER_DATA* pData, BOOL bTotal, WORD wTeamWinner );

};
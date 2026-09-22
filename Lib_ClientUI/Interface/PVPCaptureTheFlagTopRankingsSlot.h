/*!
 * \file PVPCaptureTheFlagTopRankingsSlot.h
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

class CPVPCaptureTheFlagTopRankingsSlot : public CUIGroup
{
private:
	enum
	{
	};

public:
	CPVPCaptureTheFlagTopRankingsSlot ();
	virtual	~CPVPCaptureTheFlagTopRankingsSlot ();

public:
	void	CreateSubControl ();

public:
	CBasicTextBox*	m_pTextRank;
	CBasicTextBox*	m_pTextName;

	CUIControl*		m_pImageWin;
	CUIControl*		m_pImageLose;

	CUIControl*		m_pTeamImage[CAPTURE_THE_FLAG_TEAM_SIZE];
	CUIControl*		m_pSchoolImage[CAPTURE_THE_FLAG_INFO_SCHOOL_SIZE];
	CUIControl*		m_pClassImage[GLCI_NUM_8CLASS];

public:
	void	ResetData();
	void	SetData( CAPTURE_THE_FLAG_PLAYER_DATA* pData, WORD wTeamWinner );

};
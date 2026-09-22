/*!
 * \file PVPCaptureTheFlagTopRankings.h
 *
 * \author Juver
 * \date February 2018
 *
 * 
 */

#pragma	once

#include "UIWindowEx.h"

#include "GLPVPCaptureTheFlagDefine.h"

class CPVPCaptureTheFlagTopRankingsSlot;
class CBasicTextButton;
class CBasicTextBox;

class	CPVPCaptureTheFlagTopRankings : public CUIWindowEx
{
	enum
	{
		CAPTURE_THE_FLAG_TOPRANK_BUTTON_CLOSE = ET_CONTROL_NEXT,
		CAPTURE_THE_FLAG_TOPRANK_RANKING_SLOT_0,
		CAPTURE_THE_FLAG_TOPRANK_RANKING_SLOT_1,
		CAPTURE_THE_FLAG_TOPRANK_RANKING_SLOT_2,
		CAPTURE_THE_FLAG_TOPRANK_RANKING_SLOT_3,
		CAPTURE_THE_FLAG_TOPRANK_RANKING_SLOT_4,
		CAPTURE_THE_FLAG_TOPRANK_RANKING_SLOT_5,
		CAPTURE_THE_FLAG_TOPRANK_RANKING_SLOT_6,
		CAPTURE_THE_FLAG_TOPRANK_RANKING_SLOT_7,
		CAPTURE_THE_FLAG_TOPRANK_RANKING_SLOT_8,
		CAPTURE_THE_FLAG_TOPRANK_RANKING_SLOT_9,
	};

public:
	CPVPCaptureTheFlagTopRankings ();
	virtual ~CPVPCaptureTheFlagTopRankings ();

public:
	void	CreateSubControl ();
	CBasicTextBox*	CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign );

public:
	CBasicTextBox*		m_pRankText;
	CBasicTextBox*		m_pResultText;
	CBasicTextBox*		m_pTeamText;
	CBasicTextBox*		m_pSchoolText;
	CBasicTextBox*		m_pClassText;
	CBasicTextBox*		m_pNameText;

	CPVPCaptureTheFlagTopRankingsSlot*	m_pSlot[CAPTURE_THE_FLAG_MINI_RANKING_NUM];

	CBasicTextButton*	m_pButtonClose;

public:
	virtual	void SetVisibleSingle ( BOOL bVisible );
	virtual	void TranslateUIMessage ( UIGUID cID, DWORD dwMsg );

public:
	void	SetVisiblePage( int nPage );
	void	UpdateRankings();
};
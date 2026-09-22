/*!
 * \file PVPTyrannyTopRankings.h
 *
 * \author Juver
 * \date July 2017
 *
 * 
 */

#pragma	once

#include "UIWindowEx.h"

#include "GLPVPTyrannyDefine.h"

class CPVPTyrannyTopRankingsSlot;
class CBasicTextButton;
class CBasicTextBox;

class	CPVPTyrannyTopRankings : public CUIWindowEx
{
	enum
	{
		TYRANNY_TOPRANK_BUTTON_CLOSE = ET_CONTROL_NEXT,
		TYRANNY_TOPRANK_RANKING_SLOT_0,
		TYRANNY_TOPRANK_RANKING_SLOT_1,
		TYRANNY_TOPRANK_RANKING_SLOT_2,
		TYRANNY_TOPRANK_RANKING_SLOT_3,
		TYRANNY_TOPRANK_RANKING_SLOT_4,
		TYRANNY_TOPRANK_RANKING_SLOT_5,
		TYRANNY_TOPRANK_RANKING_SLOT_6,
		TYRANNY_TOPRANK_RANKING_SLOT_7,
		TYRANNY_TOPRANK_RANKING_SLOT_8,
		TYRANNY_TOPRANK_RANKING_SLOT_9,
	};

public:
	CPVPTyrannyTopRankings ();
	virtual ~CPVPTyrannyTopRankings ();

public:
	void	CreateSubControl ();
	CBasicTextBox*	CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign );

public:
	CBasicTextBox*		m_pRankText;
	CBasicTextBox*		m_pSchoolText;
	CBasicTextBox*		m_pClassText;
	CBasicTextBox*		m_pNameText;

	CPVPTyrannyTopRankingsSlot*	m_pSlot[TYRANNY_MINI_RANKING_NUM];

	CBasicTextButton*	m_pButtonClose;

public:
	virtual	void SetVisibleSingle ( BOOL bVisible );
	virtual	void TranslateUIMessage ( UIGUID cID, DWORD dwMsg );

public:
	void	SetVisiblePage( int nPage );
	void	UpdateRankings();
};
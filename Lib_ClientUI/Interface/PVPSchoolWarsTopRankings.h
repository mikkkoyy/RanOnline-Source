/*!
 * \file PVPSchoolWarsTopRankings.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */

#pragma	once

#include "UIWindowEx.h"

#include "GLPVPSchoolWarsDefine.h"

class CPVPSchoolWarsTopRankingsSlot;
class CBasicTextButton;
class CBasicTextBox;

class	CPVPSchoolWarsTopRankings : public CUIWindowEx
{
	enum
	{
		SCHOOLWARS_TOPRANK_BUTTON_CLOSE = ET_CONTROL_NEXT,
		SCHOOLWARS_TOPRANK_RANKING_SLOT_0,
		SCHOOLWARS_TOPRANK_RANKING_SLOT_1,
		SCHOOLWARS_TOPRANK_RANKING_SLOT_2,
		SCHOOLWARS_TOPRANK_RANKING_SLOT_3,
		SCHOOLWARS_TOPRANK_RANKING_SLOT_4,
		SCHOOLWARS_TOPRANK_RANKING_SLOT_5,
		SCHOOLWARS_TOPRANK_RANKING_SLOT_6,
		SCHOOLWARS_TOPRANK_RANKING_SLOT_7,
		SCHOOLWARS_TOPRANK_RANKING_SLOT_8,
		SCHOOLWARS_TOPRANK_RANKING_SLOT_9,
	};

public:
	CPVPSchoolWarsTopRankings ();
	virtual ~CPVPSchoolWarsTopRankings ();

public:
	void	CreateSubControl ();
	CBasicTextBox*	CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign );

public:
	CBasicTextBox*		m_pRankText;
	CBasicTextBox*		m_pSchoolText;
	CBasicTextBox*		m_pClassText;
	CBasicTextBox*		m_pNameText;

	CPVPSchoolWarsTopRankingsSlot*	m_pSlot[SCHOOLWARS_MINI_RANKING_NUM];

	CBasicTextButton*	m_pButtonClose;

public:
	virtual	void SetVisibleSingle ( BOOL bVisible );
	virtual	void TranslateUIMessage ( UIGUID cID, DWORD dwMsg );

public:
	void	SetVisiblePage( int nPage );
	void	UpdateRankings();
};
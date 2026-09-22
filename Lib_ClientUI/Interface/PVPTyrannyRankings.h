/*!
 * \file PVPTyrannyRankings.h
 *
 * \author Juver
 * \date July 2017
 *
 * 
 */

#pragma	once

#include "UIWindowEx.h"

class CMultiModeButton;
class CBasicTextButton;
class CPVPTyrannyRankingPage;
class CBasicTextBox;

class	CPVPTyrannyRankings : public CUIWindowEx
{

	enum
	{
		TYRANNY_RANK_BUTTON_ALL = ET_CONTROL_NEXT,
		TYRANNY_RANK_BUTTON_SG,
		TYRANNY_RANK_BUTTON_MP,
		TYRANNY_RANK_BUTTON_PHX,

		TYRANNY_RANK_PAGE_ALL,
		TYRANNY_RANK_PAGE_SG,
		TYRANNY_RANK_PAGE_MP,
		TYRANNY_RANK_PAGE_PHX,

		TYRANNY_RANK_BUTTON_CLOSE,
	};

	enum
	{
		TYRANNY_RANK_ALL,
		TYRANNY_RANK_SG,
		TYRANNY_RANK_MP,
		TYRANNY_RANK_PHX,
		TYRANNY_RANK_SIZE,
	};

public:
	CPVPTyrannyRankings ();
	virtual ~CPVPTyrannyRankings ();

public:
	void	CreateSubControl ();
	CMultiModeButton*	CreateTabButton( const char* szButtonBox, const char* szButtonText, UIGUID ControlID );

public:
	virtual	void SetVisibleSingle ( BOOL bVisible );
	virtual	void TranslateUIMessage ( UIGUID cID, DWORD dwMsg );
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
public:
	CMultiModeButton*		m_pButtonRank[TYRANNY_RANK_SIZE];
	CPVPTyrannyRankingPage*	m_pPageRanking[TYRANNY_RANK_SIZE];
	CBasicTextButton*		m_pButtonClose;
	CBasicTextBox*			m_pInfoExit;
	float		m_fUpdateTime;

public:
	int m_nPage;

public:
	void	SetVisiblePage( int nPage );
	void	UpdateRankings();
};
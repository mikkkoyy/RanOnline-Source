/*!
 * \file PVPSchoolWarsRankings.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */
#pragma	once

#include "UIWindowEx.h"

class CMultiModeButton;
class CBasicTextButton;
class CPVPSchoolWarsRankingPage;
class CBasicTextBox;

class	CPVPSchoolWarsRankings : public CUIWindowEx
{

	enum
	{
		SCHOOLWARS_RANK_BUTTON_ALL = ET_CONTROL_NEXT,
		SCHOOLWARS_RANK_BUTTON_SG,
		SCHOOLWARS_RANK_BUTTON_MP,
		SCHOOLWARS_RANK_BUTTON_PHX,

		SCHOOLWARS_RANK_PAGE_ALL,
		SCHOOLWARS_RANK_PAGE_SG,
		SCHOOLWARS_RANK_PAGE_MP,
		SCHOOLWARS_RANK_PAGE_PHX,

		SCHOOLWARS_RANK_BUTTON_CLOSE,
	};

	enum
	{
		SCHOOLWARS_RANK_ALL,
		SCHOOLWARS_RANK_SG,
		SCHOOLWARS_RANK_MP,
		SCHOOLWARS_RANK_PHX,
		SCHOOLWARS_RANK_SIZE,
	};

public:
	CPVPSchoolWarsRankings ();
	virtual ~CPVPSchoolWarsRankings ();

public:
	void	CreateSubControl ();
	CMultiModeButton*	CreateTabButton( const char* szButtonBox, const char* szButtonText, UIGUID ControlID );

public:
	virtual	void SetVisibleSingle ( BOOL bVisible );
	virtual	void TranslateUIMessage ( UIGUID cID, DWORD dwMsg );
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
public:
	CMultiModeButton*		m_pButtonRank[SCHOOLWARS_RANK_SIZE];
	CPVPSchoolWarsRankingPage*	m_pPageRanking[SCHOOLWARS_RANK_SIZE];
	CBasicTextButton*		m_pButtonClose;
	CBasicTextBox*			m_pInfoExit;
	float					m_fUpdateTime;

public:
	int m_nPage;

public:
	void	SetVisiblePage( int nPage );
	void	UpdateRankings();
};
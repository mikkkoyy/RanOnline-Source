/*!
 * \file PVPCaptureTheFlagRankings.h
 *
 * \author Juver
 * \date February 2018
 *
 * 
 */
#pragma	once

#include "UIWindowEx.h"

class CMultiModeButton;
class CBasicTextButton;
class CPVPCaptureTheFlagRankingPage;
class CBasicTextBox;

class	CPVPCaptureTheFlagRankings : public CUIWindowEx
{

	enum
	{
		CAPTURE_THE_FLAG_RANK_BUTTON_ALL = ET_CONTROL_NEXT,
		CAPTURE_THE_FLAG_RANK_BUTTON_TEAM_A,
		CAPTURE_THE_FLAG_RANK_BUTTON_TEAM_B,

		CAPTURE_THE_FLAG_RANK_PAGE_ALL,
		CAPTURE_THE_FLAG_RANK_PAGE_TEAM_A,
		CAPTURE_THE_FLAG_RANK_PAGE_TEAM_B,

		CAPTURE_THE_FLAG_RANK_BUTTON_CLOSE,
	};

	enum
	{
		CAPTURE_THE_FLAG_RANK_ALL,
		CAPTURE_THE_FLAG_RANK_TEAM_A,
		CAPTURE_THE_FLAG_RANK_TEAM_B,
		CAPTURE_THE_FLAG_RANK_SIZE,
	};

public:
	CPVPCaptureTheFlagRankings ();
	virtual ~CPVPCaptureTheFlagRankings ();

public:
	void	CreateSubControl ();
	CMultiModeButton*	CreateTabButton( const char* szButtonBox, const char* szButtonText, UIGUID ControlID );

public:
	virtual	void SetVisibleSingle ( BOOL bVisible );
	virtual	void TranslateUIMessage ( UIGUID cID, DWORD dwMsg );
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
public:
	CMultiModeButton*		m_pButtonRank[CAPTURE_THE_FLAG_RANK_SIZE];
	CPVPCaptureTheFlagRankingPage*	m_pPageRanking[CAPTURE_THE_FLAG_RANK_SIZE];
	CBasicTextButton*		m_pButtonClose;
	CBasicTextBox*			m_pInfoExit;
	float					m_fUpdateTime;

public:
	int m_nPage;

public:
	void	SetVisiblePage( int nPage );
	void	UpdateRankings();
};
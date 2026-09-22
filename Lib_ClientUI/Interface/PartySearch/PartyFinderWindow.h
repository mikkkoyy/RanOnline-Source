/////////////////////////////////////////////////////////////////////////////
//	FileName	: PartyFinderWindow.h
//	Project		: RanClientUILib
//	Coder		: KHALEL

#pragma once

#include "UIWindowEx.h"
#include "GLDefine.h"
#include "../Lib_Client/G-Logic/GLCharData.h"
class CBasicLineBox;
class CBasicTextBox;
class CBasicTextButton;

class	CBasicTextBoxEx;
class	CBasicScrollBarEx;

class CPartyFinderSlot;

class CPartyFinderWindow : public CUIWindowEx
{
	enum
	{
		PARTYFINDER_SEARCH_PARTY_BUTTON = ET_CONTROL_NEXT,
		PARTYFINDER_SEARCH_PARTY_LOCATION,
		PARTYFINDER_SEARCH_PARTY_LOCATION_SCROLLBAR,
		PARTYFINDER_PREV_BUTTON,
		PARTYFINDER_NEXT_BUTTON,
		PARTYFINDER_SLOT00,
		PARTYFINDER_SLOT01,
		PARTYFINDER_SLOT02,
		PARTYFINDER_SLOT03,
		PARTYFINDER_SLOT04,
		PARTYFINDER_SLOT05,
		PARTYFINDER_SLOT06,
		PARTYFINDER_SLOT07,		
	};
	enum
	{
		PARTYFINDER_MAX_SLOT = 8,
	};

public:
	CPartyFinderWindow();
	virtual	~CPartyFinderWindow ();
	void CreateSubControl();
	virtual	void TranslateUIMessage(UIGUID ControlID, DWORD dwMsg);
	virtual	void SetVisibleSingle(BOOL bVisible);
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

private:
	CBasicLineBox*	CreateUILineBoxQuestList( char* szBaseControl );
	CBasicTextBox*	CreateStaticControl ( char* szControlKeyword, CD3DFontPar* pFont, D3DCOLOR D3DCOLOR, int nAlign );
	void SetTextInfo();
	void SelectParty( int nSelect );
	void PartyResetInfo();
	void ShowParty( int nPage );
	void SelectPartyArea ( int nIndex );

private:
	CBasicTextBoxEx*	m_pListTextBox;
	CBasicScrollBarEx*	m_pListScrollBar;
	int					m_nSelectIndexType;

	int					m_nCurrentPage;
	int					m_nMaxPage;

	float				m_fUpdateTime;
	float				m_fCooldown;

	CBasicTextBox*		m_pPartySearch;
	CBasicTextBox*		m_pPartyResult;
	CBasicTextButton*	m_pButtonSearch;

	CBasicButton*		m_pButtonPrev;
	CBasicButton*		m_pButtonNext;
	CBasicTextBox*		m_pTextPage;
	
	CPartyFinderSlot*	m_pPartySlot[PARTYFINDER_MAX_SLOT];
private:
	MAPPARTYSEARCH			m_mapPartySearch; 
	
	VECPARTYSEARCH			m_vecPartySearch; 	// first container
	VECPARTYSEARCH			m_vecPartySearchView; // view container 

	PARTYLINKDATA			m_sPartyData;
	
	WORD					GetSchool();
public:
	void	InitPartySearch();
	BOOL	m_bCanJoin;
	void	SetRequest(BOOL bRequest);


};
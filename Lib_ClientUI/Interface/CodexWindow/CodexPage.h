/*!
 * \file CodexPage.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */
#pragma once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include  "GLCodexDefine.h"

class CBasicTextBoxEx;
class CBasicScrollBarEx;
class CBasicTextBox;
class CBasicButton;
class CCodexPageMain;
class	CBasicComboBox;
class	CBasicComboBoxRollOver;

class	CCodexPage : public CUIGroup
{
private:
	enum
	{
		CODEX_PAGE_LIST_TEXTBOX = NO_ID + 1,
		CODEX_PAGE_LIST_SCROLLBAR,
		CODEX_PAGE_MAIN,
		CODEX_PAGE_ITEM_BUTTON,
		
		
		ITEMMALL_COMBO_ROLLOVER,
		ITEMMALL_COMBO_OPEN,
	};

public:
	CCodexPage ();
	virtual	~CCodexPage ();

public:
	void	CreateSubControl ();

public:
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void SetVisibleSingle ( BOOL bVisible );

private:
	CBasicTextBoxEx*	m_pListTextBox;
	CBasicScrollBarEx*	m_pListScrollBar;
	CBasicTextBox*		m_pTextTitle;
	CBasicTextBox*		m_pTextReportTitle;
	CBasicTextBox*		m_pTextCompleted;
	CBasicTextBox*		m_pTextPoint;
	CBasicButton*		m_pButtonItem;


	CCodexPageMain*	m_pMainPage;

	int					m_nSelectIndex;

	float				m_fActPointUpdate;
	
	CBasicComboBox*				m_pComboBoxTypeOpen;
	CBasicComboBoxRollOver*		m_pComboBoxTypeRollOver;

	DWORD	m_dwHPIncr;
	DWORD	m_dwMPIncr;
	DWORD	m_dwSPIncr;
	DWORD	m_dwAttIncr;
	DWORD	m_dwDefIncr;
	DWORD	m_dwShootIncr;
	DWORD	m_dwMeleeIncr;
	DWORD	m_dwEnerIncr;
	DWORD	m_dwResistIncr;
	DWORD	m_dwHRIncr;
	DWORD	m_dwARIncr;
	
	CBasicTextBox*		m_pTextList[11];


private:
	void	UpdateActPoint();
	void	LoadData();
	void	SelectType( DWORD dwSelect );

};
/////////////////////////////////////////////////////////////////////////////
//	FileName	: PartyFinderWindow.h
//	Project		: RanClientUILib
//	Coder		: KHALEL

#pragma once

#include "UIWindowEx.h"
#include "GLDefine.h"

#include "GLCharDefine.h"

class CBasicLineBox;
class CBasicTextBox;
class CBasicTextButton;

class	CBasicTextBoxEx;
class	CBasicScrollBarEx;

class CPartyFinderSlot;

class CPartyRequestModalWindow : public CUIWindowEx
{
	enum
	{
		PARTYREQUEST_MODAL_OK = ET_CONTROL_NEXT,
		PARTYREQUEST_MODAL_CANCEL ,
	};

public:
	CPartyRequestModalWindow();
	virtual	~CPartyRequestModalWindow ();
	void CreateSubControl();
	virtual	void TranslateUIMessage(UIGUID ControlID, DWORD dwMsg);
	virtual	void SetVisibleSingle ( BOOL bVisible );

private:
	CBasicLineBox*	CreateUILineBoxQuestList( char* szBaseControl );
	CBasicTextBox*	CreateStaticControl ( char* szControlKeyword, CD3DFontPar* pFont, D3DCOLOR D3DCOLOR, int nAlign );
	CUIControl*		CreateControl ( const char* szControl, const UIGUID& cID = NO_ID );

private:
	static const int nSCHOOL_MARK = 3;
	CUIControl*	m_pSchoolMark[nSCHOOL_MARK];
	CUIControl*	m_pClassImg[GLCI_NUM_7CLASS];

private:
	CBasicTextButton*	m_pCANCEL;
	CBasicTextButton*	m_pOK;
	CBasicTextBox*		m_pText;
	CBasicTextBox*		m_pName;
	CBasicTextBox*		m_pLevel;
	CBasicTextBox*		m_pInfo;
private:
	/* Party Search Info of Requestor*/
	DWORD			m_dwRequestorGaeaID;
	CString			m_strRequestorName;
	DWORD			m_dwRequestorLevel;
	DWORD			m_dwRequestorSchool;
	EMCHARCLASS		m_emRequestorClass;

public:
	void PartyRequestSet( const CString& strName, DWORD dwGaeaID, DWORD dwLevel, DWORD dwSchool, EMCHARCLASS emRequestorClass  );
	void PartyRequestReSet();
};
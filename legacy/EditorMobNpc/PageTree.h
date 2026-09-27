#pragma once

#include "GLCrowData.h"

class	CsheetWithTab;
// CPageTree dialog

class CPageTree : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageTree)

public:
	CPageTree(LOGFONT logfont);
	virtual ~CPageTree();

// Dialog Data
	enum { IDD = IDD_TREE };

private:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	CTreeCtrl		m_ctrlTree;	
	HTREEITEM		m_TreeList[GLCrowDataMan::MAX_CROW_MID][GLCrowDataMan::MAX_CROW_SID];	
	HTREEITEM		m_TreeRoot;
	PCROWDATA		m_pCrow;

	BOOL			m_bDummyHasCrow;
	PCROWDATA		m_pDummyCrow;

public:
	BOOL	UpdateTree();
	const char*	GetFormatName ( PCROWDATA pCROW );
	void	ForceSelectOnRightClick ( POINT pt );
	void	SetSelectCrow( PCROWDATA pCrow )	{	m_pCrow = pCrow;	}
	PCROWDATA	GetSelectCrow()	{	return m_pCrow;	}
	int		GetSelectMenu ( POINT pt );
	void	RunSelectMenu ( int nSelect );
	void	SelectTree( WORD wMID, WORD wSID );
	BOOL	UpdateItem ( PCROWDATA pCrow, HTREEITEM hMainItem );
	void	UpdateName();

public:
	BOOL	AddCrowMain( HTREEITEM hMainItem );
	BOOL	AddCrowSub();  
	BOOL	AddCrow();
	BOOL	DelCrow();
	BOOL	CopyCrow();
	BOOL	PasteCrowNew();
	BOOL	PasteCrowValue();

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnNMRClickTree(NMHDR *pNMHDR, LRESULT *pResult);
};

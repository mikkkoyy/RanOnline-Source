#pragma once


#include "GLItemMan.h"
#include "GLMapList.h"

// CPageTree dialog
class	CsheetWithTab;
struct	SITEM;

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
	HTREEITEM		m_TreeList[GLItemMan::MAX_MID][GLItemMan::MAX_SID];	
	HTREEITEM		m_TreeRoot;
	PITEMNODE		m_pItemNode;
	BOOL			m_bDummyHasItem;
	PITEMNODE		m_pDummyItemNode;
	GLMapList		m_MapsList;

public:
	BOOL	UpdateTree();
	const char*	GetFormatName ( SITEM* pItem );
	void	ForceSelectOnRightClick ( POINT pt );
	void	SetSelectItem ( PITEMNODE pItemNode )	{	m_pItemNode = pItemNode;	}
	PITEMNODE	GetSelectItem ()	{	return m_pItemNode;	}
	int		GetSelectMenu ( POINT pt );
	void	RunSelectMenu ( int nSelect );
	void	SelectTree( WORD wMID, WORD wSID );
	BOOL	UpdateItem ( SITEM* pItem, HTREEITEM hMainItem );
	void	UpdateName();

public:
	BOOL	AddItemMain( HTREEITEM hMainItem );
	BOOL	AddItemSub ();  
	BOOL	AddItem ();
	BOOL	DelItem ();
	BOOL	CopyItem ();
	BOOL	PasteItemNew ();
	BOOL	PasteItemValue ();

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

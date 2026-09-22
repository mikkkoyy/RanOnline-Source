#pragma once

#include "GLSkill.h"
#include "GLCharDefine.h"
// CPageTree dialog
class	CsheetWithTab;
struct	GLSKILL;

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
	HTREEITEM		m_TreeList[EMSKILLCLASS_NSIZE][GLSkillMan::MAX_CLASSSKILL];	
	PGLSKILL		m_pSkill;
	BOOL			m_bDummyHasSkill;
	PGLSKILL		m_pDummySkill;

public:
	BOOL	UpdateTree();
	const char*	GetFormatName ( PGLSKILL pSKILL );
	void	SetSelectSkill ( PGLSKILL pSkill )	{	m_pSkill = pSkill;	}
	PGLSKILL	GetSelectSkill()	{	return m_pSkill;	}
	void	ForceSelectOnRightClick ( POINT pt );
	int		GetSelectMenu ( POINT pt );
	void	RunSelectMenu ( int nSelect );
	BOOL	IsParent( HTREEITEM hITEM );
	WORD	GetParentID( HTREEITEM hITEM );
	BOOL	UpdateItem ( PGLSKILL pSkill, HTREEITEM hITEM );
	void	UpdateName();
	void	SelectTree( WORD wMID, WORD wSID );

public:
	BOOL	AddSkill();
	BOOL	DelSkill();
	BOOL	CopySkill();
	BOOL	PasteSkillNew();
	BOOL	PasteSkillValue();

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

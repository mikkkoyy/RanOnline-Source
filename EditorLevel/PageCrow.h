#pragma once

#include "GLLandMan.h"

class	CsheetWithTab;
// CPageCrow dialog

enum EMCROWSHOW_TYPE
{
	EMCROWSHOW_TYPE_ALL		= 0,
	EMCROWSHOW_TYPE_NPC		= 1,
	EMCROWSHOW_TYPE_ERROR	= 2,
};

class CPageCrow : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageCrow)

public:
	CPageCrow(LOGFONT logfont);
	virtual ~CPageCrow();

// Dialog Data
	enum { IDD = IDD_PAGE_CROW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

protected:
	GLLandMan*		m_pGLLand;

private:
	CListCtrl	m_List;
	EMCROWSHOW_TYPE	m_emSHOW;

public:
	void	DataSet( GLLandMan* pLand );
	void	ResetEditor();

public:
	void	JumpTo( D3DXVECTOR3 vPos );
	void	SelectCrow( std::string strName );

public:
	void	DataShow( EMCROWSHOW_TYPE emSHOW, BOOL bRESETFILTER = FALSE );

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonShowAll();
	afx_msg void OnBnClickedButtonShowNpc();
	afx_msg void OnBnClickedButtonShowInvalid();
	afx_msg void OnBnClickedButtonShowfind();
	afx_msg void OnBnClickedButtonShowfindClear();
	afx_msg void OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonNew();
	afx_msg void OnBnClickedButtonEdit();
	afx_msg void OnBnClickedButtonDelete();
	afx_msg void OnBnClickedButtonCopy();
	afx_msg void OnBnClickedButtonPastenew();
	afx_msg void OnBnClickedButtonRebuildname();
	afx_msg void OnBnClickedButtonReplaceid();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClear();
	afx_msg void OnBnClickedButtonMobMember();
	afx_msg void OnBnClickedButtonMobLeader();
	afx_msg void OnBnClickedButtonFsched();
};

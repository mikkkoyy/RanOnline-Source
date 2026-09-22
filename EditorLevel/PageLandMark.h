#pragma once

#include "GLLandMan.h"

class	CsheetWithTab;
// CPageLandMark dialog

class CPageLandMark : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageLandMark)

public:
	CPageLandMark(LOGFONT logfont);
	virtual ~CPageLandMark();

// Dialog Data
	enum { IDD = IDD_PAGE_LM };

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

public:
	void	DataSet( GLLandMan* pLand );
	void	ResetEditor();

public:
	void	DataShow();
	void	JumpTo( D3DXVECTOR3 vPOS );

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonNew();
	afx_msg void OnBnClickedButtonEdit();
	afx_msg void OnBnClickedButtonDelete();
	afx_msg void OnBnClickedButtonRebuildname();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult);
};

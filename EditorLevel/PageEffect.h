#pragma once


#include "GLLandMan.h"

class	CsheetWithTab;
// CPageEffect dialog

class CPageEffect : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEffect)

public:
	CPageEffect(LOGFONT logfont);
	virtual ~CPageEffect();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF };

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
	void	DataShow();
	void	JumpTo( D3DXMATRIX sMat );

public:
	void	DataSet( GLLandMan* pLand );
	void	ResetEditor();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonNew();
	afx_msg void OnBnClickedButtonEdit();
	afx_msg void OnBnClickedButtonDelete();
	afx_msg void OnBnClickedButtonRebuildname();
	afx_msg void OnBnClickedButtonSave();
};

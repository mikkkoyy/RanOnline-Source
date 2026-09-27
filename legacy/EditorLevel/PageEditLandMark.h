#pragma once

#include "GLLandMan.h"

class	CsheetWithTab;
// CPageEditLandMark dialog

class CPageEditLandMark : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEditLandMark)

public:
	CPageEditLandMark(LOGFONT logfont);
	virtual ~CPageEditLandMark();

// Dialog Data
	enum { IDD = IDD_PAGE_EDIT_LANDMARK };

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
	PLANDMARK		m_pEdit;

public:
	void	SetData( GLLandMan* pLand, PLANDMARK pEdit );
	void	ResetData();

public:
	void	JumpTo( D3DXVECTOR3 vPOS );
	void	InitUI( BOOL bENABLE );
	void	DataShow();
	BOOL	DataSave();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonRegnew();
	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonCheck();
	afx_msg void OnBnClickedButtonNew();
};

#pragma once

#include "GLLandMan.h"

class	CsheetWithTab;
// CPageEditEffect dialog

class CPageEditEffect : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEditEffect)

public:
	CPageEditEffect(LOGFONT logfont);
	virtual ~CPageEditEffect();

// Dialog Data
	enum { IDD = IDD_PAGE_EDIT_EFFECT };

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
	PLANDEFF		m_pEdit;
	D3DXMATRIX		m_matWorld_OLD;	

public:
	void	SetData( GLLandMan* pLand, PLANDEFF pEdit );
	void	ResetData();

public:
	void	JumpTo( D3DXMATRIX sMat );
	void	InitUI( BOOL bENABLE );
	void	DataShow();
	BOOL	DataSave();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonRegnew();
	afx_msg void OnBnClickedButtonRestore();
	afx_msg void OnBnClickedButtonOpeneffect();
	afx_msg void OnBnClickedButtonCheck();
	afx_msg void OnBnClickedButtonNew();
	afx_msg void OnBnClickedButtonApply();
};

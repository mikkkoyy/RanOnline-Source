#pragma once

#include "GLLandMan.h"

class	CsheetWithTab;
// CPageEditCrow dialog

class CPageEditCrow : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEditCrow)

public:
	CPageEditCrow(LOGFONT logfont);
	virtual ~CPageEditCrow();

// Dialog Data
	enum { IDD = IDD_PAGE_EDIT_CROW };

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
	GLMobSchedule*	m_pEdit;

public:
	CListCtrl		m_List;

public:
	void	SetData( GLLandMan* pLand, GLMobSchedule* pEdit );
	void	ResetData();

public:
	void	InitUI( BOOL bENABLE );
	void	JumpTo( D3DXVECTOR3 vPos );

	void	DataShow();
	BOOL	DataSave();

	void	ShowActions();
	void	AdjustAction();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonCheck();
	afx_msg void OnBnClickedButtonNew();
	afx_msg void OnBnClickedButtonCrow();
	afx_msg void OnEnChangeEditCrowMid();
	afx_msg void OnEnChangeEditCrowSid();
	afx_msg void OnBnClickedButtonRegnew();
	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonBus();
	afx_msg void OnBnClickedButtonActionAdd();
	afx_msg void OnBnClickedButtonActionEdit();
	afx_msg void OnBnClickedButtonActionDel();
	afx_msg void OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonActionClose();
	afx_msg void OnCbnSelchangeComboAction();
	afx_msg void OnBnClickedButtonRebuild();
	afx_msg void OnBnClickedButtonActionPos();
	afx_msg void OnBnClickedButtonActionSave();
};

#pragma once

#include "DxEffChar.h"
#include "DxEffCharSingle.h"

class	CsheetWithTab;
// CPageEff_00_Single dialog

class CPageEff_00_Single : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_00_Single)

public:
	CPageEff_00_Single(LOGFONT logfont);
	virtual ~CPageEff_00_Single();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_00_SINGLE };

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
	DxSkinPiece*		m_pPiece;
	DxEffCharSingle*	m_pEff;

private:
	BOOL	m_bADD;


public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharSingle* pEff, BOOL bADD );
	void ResetTool();
	void DataShow();
	BOOL DataSave();

public:
	CListCtrl		m_List_Dot1;
	CListCtrl		m_List_Dot2;

public:
	void	TraceShow();
	void	MatrixEdit( BOOL bEDIT );

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonEditApply();
	afx_msg void OnBnClickedButtonEditSave();
	afx_msg void OnBnClickedButtonEditClose();
	afx_msg void OnBnClickedButtonEffect();
	afx_msg void OnNMDblclkListTrace1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListTrace2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCheckDot1();
	afx_msg void OnBnClickedCheckDot2();
	afx_msg void OnBnClickedCheckDotMatrix();
	afx_msg void OnBnClickedCheckDotMatrix2();
};

#pragma once

#include "DxEffChar.h"
#include "DxEffCharBlur.h"

class	CsheetWithTab;
// CPageEff_01_Blur dialog

class CPageEff_01_Blur : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_01_Blur)

public:
	CPageEff_01_Blur(LOGFONT logfont);
	virtual ~CPageEff_01_Blur();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_01_BLUR };

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
	DxEffCharBlur*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharBlur* pEff, BOOL bADD );
	void ResetTool();
	void DataShow();
	BOOL DataSave();

public:
	CListCtrl		m_List_TraceUp;
	CListCtrl		m_List_TraceDown;

public:
	void	TraceShow();

public:
	virtual BOOL OnInitDialog();


	afx_msg void OnBnClickedButtonEditApply();
	afx_msg void OnBnClickedButtonEditSave();
	afx_msg void OnBnClickedButtonEditClose();
	afx_msg void OnBnClickedButtonTexture();
	afx_msg void OnBnClickedButtonColorUp();
	afx_msg void OnBnClickedButtonColorDown();
	afx_msg void OnNMDblclkListTraceUp(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListTraceDown(NMHDR *pNMHDR, LRESULT *pResult);
};

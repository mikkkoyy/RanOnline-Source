#pragma once

#include "DxEffChar.h"
#include "DxEffCharCloneBlur.h"

class	CsheetWithTab;
// CPageEff_02_CloneBlur dialog

class CPageEff_02_CloneBlur : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_02_CloneBlur)

public:
	CPageEff_02_CloneBlur(LOGFONT logfont);
	virtual ~CPageEff_02_CloneBlur();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_02_CLONEBLUR };

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
	DxEffCharCloneBlur*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharCloneBlur* pEff, BOOL bADD );
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


	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonTextureCb();
	afx_msg void OnNMDblclkListTraceUpCb(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListTraceDownCb(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonColorUpCb();
	afx_msg void OnBnClickedButtonColorDownCb();
};

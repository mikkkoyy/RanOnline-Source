#pragma once

#include "DxEffChar.h"
#include "DxEffCharNoAlpha.h"

class	CsheetWithTab;
// CPageEff_13_NoAlpha dialog

class CPageEff_13_NoAlpha : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_13_NoAlpha)

public:
	CPageEff_13_NoAlpha(LOGFONT logfont);
	virtual ~CPageEff_13_NoAlpha();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_13_NOALPHA };

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
	DxEffCharNoAlpha*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharNoAlpha* pEff, BOOL bADD );
	void ResetTool();
	void DataShow();
	BOOL DataSave();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
};

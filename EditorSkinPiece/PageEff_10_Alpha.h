#pragma once

#include "DxEffChar.h"
#include "DxEffCharAlpha.h"

class	CsheetWithTab;
// CPageEff_10_Alpha dialog

class CPageEff_10_Alpha : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_10_Alpha)

public:
	CPageEff_10_Alpha(LOGFONT logfont);
	virtual ~CPageEff_10_Alpha();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_10_ALPHA };

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
	DxEffCharAlpha*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharAlpha* pEff, BOOL bADD );
	void ResetTool();
	void DataShow();
	BOOL DataSave();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
};

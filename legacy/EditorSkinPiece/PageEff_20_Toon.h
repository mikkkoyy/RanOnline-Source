#pragma once

#include "DxEffChar.h"
#include "DxEffCharToon.h"

class	CsheetWithTab;
// CPageEff_20_Toon dialog

class CPageEff_20_Toon : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_20_Toon)

public:
	CPageEff_20_Toon(LOGFONT logfont);
	virtual ~CPageEff_20_Toon();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_20_TOON };

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
	DxEffCharToon*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharToon* pEff, BOOL bADD );
	void ResetTool();
	void DataShow();
	BOOL DataSave();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonCtnTex();
	afx_msg void OnBnClickedButtonCtnOblique();
};

#pragma once

#include "DxEffChar.h"
#include "DxEffCharNormal.h"

class	CsheetWithTab;
// CPageEff_26_Normal dialog

class CPageEff_26_Normal : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_26_Normal)

public:
	CPageEff_26_Normal(LOGFONT logfont);
	virtual ~CPageEff_26_Normal();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_26_NORMALMAP };

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
	DxEffCharNormal*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharNormal* pEff, BOOL bADD );
	void ResetTool();
	void DataShow();
	BOOL DataSave();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
};

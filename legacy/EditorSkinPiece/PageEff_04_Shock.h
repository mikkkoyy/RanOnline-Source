#pragma once

#include "DxEffChar.h"
#include "DxEffCharShock.h"

class	CsheetWithTab;
// CPageEff_04_Shock dialog

class CPageEff_04_Shock : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_04_Shock)

public:
	CPageEff_04_Shock(LOGFONT logfont);
	virtual ~CPageEff_04_Shock();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_04_SHOCK };

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
	DxEffCharShock*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharShock* pEff, BOOL bADD );
	void ResetTool();
	void DataShow();
	BOOL DataSave();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonShColor();
};

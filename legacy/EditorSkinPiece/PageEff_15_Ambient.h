#pragma once

#include "DxEffChar.h"
#include "DxEffCharAmbient.h"

class	CsheetWithTab;
// CPageEff_15_Ambient dialog

class CPageEff_15_Ambient : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_15_Ambient)

public:
	CPageEff_15_Ambient(LOGFONT logfont);
	virtual ~CPageEff_15_Ambient();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_15_AMBIENT };

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
	DxEffCharAmbient*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharAmbient* pEff, BOOL bADD );
	void ResetTool();
	void DataShow();
	BOOL DataSave();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonCaColor();
};

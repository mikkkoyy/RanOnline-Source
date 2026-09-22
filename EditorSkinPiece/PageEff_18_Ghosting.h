#pragma once

#include "DxEffChar.h"
#include "DxEffCharGhosting.h"

class	CsheetWithTab;
// CPageEff_18_Ghosting dialog

class CPageEff_18_Ghosting : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_18_Ghosting)

public:
	CPageEff_18_Ghosting(LOGFONT logfont);
	virtual ~CPageEff_18_Ghosting();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_18_GHOSTING };

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
	DxEffCharGhosting*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharGhosting* pEff, BOOL bADD );
	void ResetTool();
	void DataShow();
	BOOL DataSave();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonCghColor();
};

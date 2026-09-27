#pragma once

#include "DxEffChar.h"
#include "DxEffCharArrow.h"

class	CsheetWithTab;
// CPageEff_06_Arrow dialog

class CPageEff_06_Arrow : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_06_Arrow)

public:
	CPageEff_06_Arrow(LOGFONT logfont);
	virtual ~CPageEff_06_Arrow();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_06_ARROW };

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
	DxEffCharArrow*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharArrow* pEff, BOOL bADD );
	void ResetTool();
	void DataShow();
	BOOL DataSave();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonCaEffbase();
	afx_msg void OnBnClickedButtonCaEffup();
	afx_msg void OnBnClickedButtonCaColor();
	afx_msg void OnBnClickedButtonCaMesh();
};

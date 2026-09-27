#pragma once

#include "DxEffChar.h"
#include "DxEffCharMark.h"

class	CsheetWithTab;
// CPageEff_12_Mark dialog

class CPageEff_12_Mark : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_12_Mark)

public:
	CPageEff_12_Mark(LOGFONT logfont);
	virtual ~CPageEff_12_Mark();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_12_MARK };

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
	DxEffCharMark*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharMark* pEff, BOOL bADD );
	void ResetTool();
	void DataShow();
	BOOL DataSave();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonCmTexture1();
	afx_msg void OnBnClickedButtonCmColor1();
	afx_msg void OnBnClickedButtonCmTexture2();
	afx_msg void OnBnClickedButtonCmColor2();
	afx_msg void OnBnClickedCheckCmUse1();
	afx_msg void OnBnClickedCheckCmUse2();
};

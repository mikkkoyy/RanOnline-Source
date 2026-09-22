#pragma once

#include "DxEffChar.h"
#include "DxEffCharMultiTex.h"

class	CsheetWithTab;
// CPageEff_17_MultiTex dialog

class CPageEff_17_MultiTex : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_17_MultiTex)

public:
	CPageEff_17_MultiTex(LOGFONT logfont);
	virtual ~CPageEff_17_MultiTex();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_17_MULTITEX };

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
	DxEffCharMultiTex*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharMultiTex* pEff, BOOL bADD );
	void ResetTool();
	void DataShow();
	BOOL DataSave();

public:
	CListCtrl		m_List_Material;
	int				m_nMatEdit;

public:
	void	MaterialShow();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonCmtTexture1();
	afx_msg void OnBnClickedButtonCmtColor1();
	afx_msg void OnBnClickedButtonCmtTexture2();
	afx_msg void OnBnClickedButtonCmtColor2();
	afx_msg void OnBnClickedButtonCmtTexmap();
	afx_msg void OnNMDblclkListMaterial(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonMaterialOk();
	afx_msg void OnBnClickedCheckCmtUse1();
	afx_msg void OnBnClickedCheckCmtUse2();
	afx_msg void OnBnClickedCheckCmtUseAlltex();
	afx_msg void OnBnClickedCheckCmtUseSelecttex();
};

#pragma once

#include "DxEffChar.h"
#include "DxEffCharNeon.h"

class	CsheetWithTab;
// CPageEff_11_Neon dialog

class CPageEff_11_Neon : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_11_Neon)

public:
	CPageEff_11_Neon(LOGFONT logfont);
	virtual ~CPageEff_11_Neon();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_11_NEON };

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
	DxEffCharNeon*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharNeon* pEff, BOOL bADD );
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
	afx_msg void OnBnClickedCheckCnUseGeneral();
	afx_msg void OnBnClickedCheckCnUseBurn();
	afx_msg void OnBnClickedCheckCnUseAlltex();
	afx_msg void OnBnClickedCheckCnUseSelecttex();
	afx_msg void OnBnClickedCheckCnUseGlowSrc();
	afx_msg void OnBnClickedCheckCnUseGlowUsr();
	afx_msg void OnBnClickedButtonCnTexture();
	afx_msg void OnBnClickedButtonCnColor();
	afx_msg void OnNMDblclkListMaterial(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonMaterialOk();
};

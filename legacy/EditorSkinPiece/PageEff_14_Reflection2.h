#pragma once

#include "DxEffChar.h"
#include "DxEffCharReflection2.h"

class	CsheetWithTab;
// CPageEff_14_Reflection2 dialog

class CPageEff_14_Reflection2 : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_14_Reflection2)

public:
	CPageEff_14_Reflection2(LOGFONT logfont);
	virtual ~CPageEff_14_Reflection2();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_14_REFLECTION2 };

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
	DxEffCharReflection2*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharReflection2* pEff, BOOL bADD );
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
	afx_msg void OnBnClickedButtonCrfColor();
	afx_msg void OnBnClickedButtonCrfTexture();
	afx_msg void OnNMDblclkListMaterial(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonMaterialTexture();
	afx_msg void OnBnClickedButtonMaterialOk();
};

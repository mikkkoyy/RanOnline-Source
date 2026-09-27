#pragma once

#include "DxEffChar.h"
#include "DxEffCharSpecular2.h"

class	CsheetWithTab;
// CPageEff_19_Specular2 dialog

class CPageEff_19_Specular2 : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_19_Specular2)

public:
	CPageEff_19_Specular2(LOGFONT logfont);
	virtual ~CPageEff_19_Specular2();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_19_SPECULAR2 };

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
	DxEffCharSpecular2*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharSpecular2* pEff, BOOL bADD );
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
	afx_msg void OnBnClickedButtonCs2Color();
	afx_msg void OnNMDblclkListMaterial(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonMaterialTexture();
	afx_msg void OnBnClickedButtonMaterialOk();
	afx_msg void OnBnClickedButtonCs2Texture();
};

#pragma once

#include "DxEffChar.h"
#include "DxEffCharUserColor.h"

class	CsheetWithTab;
// CPageEff_25_UserColor dialog

class CPageEff_25_UserColor : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_25_UserColor)

public:
	CPageEff_25_UserColor(LOGFONT logfont);
	virtual ~CPageEff_25_UserColor();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_25_USERCOLOR };

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
	DxEffCharUserColor*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharUserColor* pEff, BOOL bADD );
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
	afx_msg void OnBnClickedButtonCucColor();
	afx_msg void OnNMDblclkListMaterial(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonMaterialTexture();
	afx_msg void OnBnClickedButtonMaterialOk();
};

#pragma once

#include "DxEffChar.h"
#include "DxEffCharAroundEff.h"

class	CsheetWithTab;
// CPageEff_28_AroundEff dialog

class CPageEff_28_AroundEff : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_28_AroundEff)

public:
	CPageEff_28_AroundEff(LOGFONT logfont);
	virtual ~CPageEff_28_AroundEff();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_28_AROUNDEFF };

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
	DxEffCharAroundEff*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharAroundEff* pEff, BOOL bADD );
	void ResetTool();
	void DataShow();
	BOOL DataSave();

public:
	CListCtrl		m_List_Pos;

public:
	void	PositionShow();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonCarTexture();
	afx_msg void OnBnClickedButtonCarColor();
	afx_msg void OnBnClickedButtonCarEditBone();
	afx_msg void OnBnClickedButtonCarEditPos();
	afx_msg void OnBnClickedButtonCarEditInsert();
	afx_msg void OnBnClickedButtonCarEditDelete();
	afx_msg void OnNMClickListPos(NMHDR *pNMHDR, LRESULT *pResult);
};

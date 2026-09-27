#pragma once

#include "DxEffChar.h"
#include "DxEffCharLine2BoneEff.h"

class	CsheetWithTab;
// CPageEff_27_Line2Bone dialog

class CPageEff_27_Line2Bone : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_27_Line2Bone)

public:
	CPageEff_27_Line2Bone(LOGFONT logfont);
	virtual ~CPageEff_27_Line2Bone();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_27_LINE2BONE };

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
	DxEffCharLine2BoneEff*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharLine2BoneEff* pEff, BOOL bADD );
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
	afx_msg void OnBnClickedButtonClbTexture1();
	afx_msg void OnBnClickedButtonClbTexture2();
	afx_msg void OnBnClickedButtonClbColor1();
	afx_msg void OnBnClickedButtonClbColor2();
	afx_msg void OnBnClickedButtonClbBone1();
	afx_msg void OnBnClickedButtonClbBone2();
	afx_msg void OnBnClickedButtonClbInsert();
	afx_msg void OnBnClickedButtonClbDelete();
	afx_msg void OnNMDblclkListPos(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickListPos(NMHDR *pNMHDR, LRESULT *pResult);
};

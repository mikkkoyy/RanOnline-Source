#pragma once

#include "DxEffChar.h"
#include "DxEffCharBoneListEff.h"

class	CsheetWithTab;
// CPageEff_24_BoneList dialog

class CPageEff_24_BoneList : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_24_BoneList)

public:
	CPageEff_24_BoneList(LOGFONT logfont);
	virtual ~CPageEff_24_BoneList();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_24_BONELIST };

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
	DxEffCharBoneListEff*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharBoneListEff* pEff, BOOL bADD );
	void ResetTool();
	void DataShow();
	BOOL DataSave();

public:
	CListCtrl		m_List_Pos;
	D3DXMATRIX		m_matEdit;

public:
	void	PosEdit( BOOL bEDIT );
	void	PositionShow();
	void	PositionEdit();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonCblTexture();
	afx_msg void OnBnClickedButtonCblColor();
	afx_msg void OnNMDblclkListPos(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonCblListAdd();
	afx_msg void OnBnClickedButtonCblListEdit();
	afx_msg void OnBnClickedButtonCblListDel();
	afx_msg void OnBnClickedButtonCblListUp();
	afx_msg void OnBnClickedButtonCblListDown();
	afx_msg void OnBnClickedButtonEditposReset();
	afx_msg void OnBnClickedButtonEditposSave();
};

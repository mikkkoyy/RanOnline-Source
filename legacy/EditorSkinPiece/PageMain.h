#pragma once

#include "DxSkinPieceContainer.h"

class	CsheetWithTab;
// CPageMain dialog

class CPageMain : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageMain)

public:
	CPageMain(LOGFONT logfont);
	virtual ~CPageMain();

// Dialog Data
	enum { IDD = IDD_PAGE_MAIN };

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
	DxSkinPiece*	m_pPiece;
	int				m_nMaterialEdit;

public:
	CListCtrl		m_List_Trace;
	CListCtrl		m_List_Mesh;
	CListCtrl		m_List_Effect;
	CListCtrl		m_List_Material;
	
public:
	void	SetData( DxSkinPiece* pDATA );
	void	ResetTool();
	void	ShowData();
	void	ShowMesh();
	void	ShowTrace();
	void	ShowMaterial();
	void	ShowEffectChar();

public:
	virtual BOOL OnInitDialog();


	afx_msg void OnBnClickedButtonSkin();
	afx_msg void OnBnClickedButtonSkeleton();
	afx_msg void OnBnClickedButtonCreateskin();
	afx_msg void OnBnClickedButtonMesh();
	afx_msg void OnBnClickedButtonTraceAdd();
	afx_msg void OnBnClickedButtonTraceDel();
	afx_msg void OnBnClickedButtonTraceDelall();
	afx_msg void OnNMDblclkListTrace(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonEffAdd();
	afx_msg void OnBnClickedButtonEffEdit();
	afx_msg void OnBnClickedButtonEffDel();
	afx_msg void OnBnClickedButtonEffUp();
	afx_msg void OnBnClickedButtonEffDown();
	afx_msg void OnBnClickedButtonEffOk();
	afx_msg void OnBnClickedButtonEffSort();
	afx_msg void OnCbnSelchangeComboPiece();
	afx_msg void OnCbnSelchangeComboSlot();
	afx_msg void OnBnClickedButtonMaterialSave();
	afx_msg void OnNMDblclkListMaterial(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonMaterialTexture();
};

#pragma once

#include "DxEffChar.h"
#include "DxEffCharParticle.h"

class	CsheetWithTab;
// CPageEff_22_Particle dialog

class CPageEff_22_Particle : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_22_Particle)

public:
	CPageEff_22_Particle(LOGFONT logfont);
	virtual ~CPageEff_22_Particle();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_22_PARTICLE };

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
	DxEffCharParticle*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharParticle* pEff, BOOL bADD );
	void ResetTool();
	void DataShow();
	BOOL DataSave();

public:
	CListCtrl		m_List_Pos;
	D3DXMATRIX		m_matEdit;

public:
	void	PositionShow();
	void	PosEdit( BOOL bEDIT );
	void	PositionEdit();


public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonCptTexture();
	afx_msg void OnBnClickedButtonCptColor();
	afx_msg void OnBnClickedButtonCptListAdd();
	afx_msg void OnBnClickedButtonCptListDel();
	afx_msg void OnBnClickedButtonCptListUp();
	afx_msg void OnBnClickedButtonCptListDown();
	afx_msg void OnNMDblclkListPos(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickListPos(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonCptListApply();
	afx_msg void OnBnClickedButtonCptListEdit();
};

#pragma once

#include "DxEffChar.h"
#include "DxEffCharBonePosEff.h"

class	CsheetWithTab;
// CPageEff_23_BonePos dialog

class CPageEff_23_BonePos : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEff_23_BonePos)

public:
	CPageEff_23_BonePos(LOGFONT logfont);
	virtual ~CPageEff_23_BonePos();

// Dialog Data
	enum { IDD = IDD_PAGE_EFF_23_BONEPOS };

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
	DxEffCharBonePosEff*		m_pEff;

private:
	BOOL	m_bADD;

public:
	void DataSet( DxSkinPiece* pPiece, DxEffCharBonePosEff* pEff, BOOL bADD );
	void ResetTool();
	void DataShow();
	BOOL DataSave();

public:
	D3DXMATRIX		m_matEdit;

public:
	void	LocalPosShow();	

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonCbpTexture();
	afx_msg void OnBnClickedButtonCbpColor();
	afx_msg void OnBnClickedButtonCbpSelbone();
	afx_msg void OnBnClickedCheckCbpUseOffset();
	afx_msg void OnBnClickedButtonCbpGetoffset();
	afx_msg void OnBnClickedButtonCbpResetoffset();
	afx_msg void OnBnClickedButtonCbpEditoffset();
};

#pragma once

class	CsheetWithTab;

#include "DxVehicleData.h"
#include "DxVehicle.h"

// CPageVCF dialog

class CPageVCF : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageVCF)

public:
	CPageVCF(LOGFONT logfont);
	virtual ~CPageVCF();

// Dialog Data
	enum { IDD = IDD_PAGE_VCF };

private:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

public:
	void	ResetTool();

	//page specific 
public:
	DxVehicle*			m_pSkin;
	DxVehicleData*		m_pData;

public:
	CListCtrl		m_List_Anim;
	CListCtrl		m_List_Piece;

public:
	void	DataShow();
	void	DataShowAnim();
	void	DataShowPiece();
	void	ResetAnimation();

	void	ManualMatrixReset1();
	void	ManualMatrixShow1();
	void	ManualMatrixSave1();

	void	ManualMatrixReset2();
	void	ManualMatrixShow2();
	void	ManualMatrixSave2();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonVcfNew();
	afx_msg void OnBnClickedButtonVcfLoad();
	afx_msg void OnBnClickedButtonVcfSave();
	afx_msg void OnBnClickedButtonVcfSkeleton();
	afx_msg void OnBnClickedButtonVcfApply();
	afx_msg void OnBnClickedButtonVcfPieceCps();
	afx_msg void OnBnClickedButtonVcfPieceReset();
	afx_msg void OnBnClickedButtonVcfAnimPlay();
	afx_msg void OnBnClickedButtonVcfAnimStop();
	afx_msg void OnBnClickedButtonVcfAnimAddcfg();
	afx_msg void OnBnClickedButtonVcfAnimDelete();
	afx_msg void OnBnClickedButtonVcfAnimClear();
	afx_msg void OnNMDblclkListVcfAnim(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedVcfButtonBone1();
	afx_msg void OnBnClickedVcfButtonEditmat1();
	afx_msg void OnBnClickedVcfButtonResetMat1();
	afx_msg void OnBnClickedVcfButtonBone2();
	afx_msg void OnBnClickedVcfButtonEditmat2();
	afx_msg void OnBnClickedVcfButtonResetMat2();
	afx_msg void OnBnClickedButtonVcfTestSkin();
	afx_msg void OnBnClickedButtonVcfTestSkinP();
	afx_msg void OnBnClickedButtonVcfTestSkinS();
	afx_msg void OnBnClickedButtonAniInsertVcf();
	afx_msg void OnBnClickedVcfButtonManualMat1();
	afx_msg void OnBnClickedVcfButtonManualMat2();
	afx_msg void OnBnClickedVcfButtonCancelMat1();
	afx_msg void OnBnClickedVcfButtonCancelMat2();
	afx_msg void OnBnClickedVcfButtonTestMat1();
	afx_msg void OnBnClickedVcfButtonTestMat2();
	afx_msg void OnBnClickedVcfButtonSaveMat1();
	afx_msg void OnBnClickedVcfButtonSaveMat2();
};

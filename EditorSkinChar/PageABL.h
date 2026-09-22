#pragma once

class	CsheetWithTab;

#include "DxAttBone.h"
#include "DxAttBoneLink.h"

// CPageABL dialog

class CPageABL : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageABL)

public:
	CPageABL(LOGFONT logfont);
	virtual ~CPageABL();

// Dialog Data
	enum { IDD = IDD_PAGE_ABL };

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
	DxAttBone*			m_pSkin;
	DxAttBoneLink*		m_pData;
	

public:
	void DataShow();
	void ManualMatrixReset1();
	void ManualMatrixShow1();
	void ManualMatrixSave1();

	void ManualMatrixReset2();
	void ManualMatrixShow2();
	void ManualMatrixSave2();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonAblNew();
	afx_msg void OnBnClickedButtonAblLoad();
	afx_msg void OnBnClickedButtonAblSave();
	afx_msg void OnBnClickedAblButtonSkeleton();
	afx_msg void OnBnClickedAblButtonBonedata();
	afx_msg void OnBnClickedAblButtonMaskp();
	afx_msg void OnBnClickedAblButtonBonelink();
	afx_msg void OnBnClickedAblButtonEditmat1();
	afx_msg void OnBnClickedAblButtonEditmat2();
	afx_msg void OnBnClickedAblButtonResetMat01();
	afx_msg void OnBnClickedAblButtonResetMat02();
	afx_msg void OnBnClickedAblButtonMaskpReset();
	afx_msg void OnCbnSelchangeAblComboStrike();
	afx_msg void OnCbnSelchangeAblComboPtype();
	afx_msg void OnCbnSelchangeAblComboSlotb();
	afx_msg void OnBnClickedAblButtonAnimPlay();
	afx_msg void OnBnClickedAblButtonAnimStop();
	afx_msg void OnBnClickedAblButtonManualMat1();
	afx_msg void OnBnClickedAblButtonManualMat2();
	afx_msg void OnBnClickedAblButtonTestMat1();
	afx_msg void OnBnClickedAblButtonTestMat2();
	afx_msg void OnBnClickedAblButtonSaveMat1();
	afx_msg void OnBnClickedAblButtonSaveMat2();
	afx_msg void OnBnClickedAblButtonCancelMat1();
	afx_msg void OnBnClickedAblButtonCancelMat2();
};

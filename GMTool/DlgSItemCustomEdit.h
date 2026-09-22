#pragma once

struct SITEMCUSTOM;

#include "Logic/GMToolGlobal.h"
// CDlgSItemCustomEdit dialog

class CDlgSItemCustomEdit : public CDialog
{
	DECLARE_DYNAMIC(CDlgSItemCustomEdit)

public:
	CDlgSItemCustomEdit(CWnd* pParent = NULL);   // standard constructor
	CDlgSItemCustomEdit( SITEMCUSTOM* pITEMCUSTOM, EMITEMEDIT_TYPE emEDIT, WORD wSLOT, int nPOSX, int nPOSY, CWnd* pParent = NULL );
	virtual ~CDlgSItemCustomEdit();

// Dialog Data
	enum { IDD = IDD_DLG_SITEMCUSTOM_EDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CComboBox	m_cPreset;

private:
	EMITEMEDIT_TYPE	m_emEditType;
	WORD			m_wPutOnSlot;
	int				m_nInvenPosX;
	int				m_nInvenPosY;
	SITEMCUSTOM*	m_pItemCustom;
	__time64_t		m_tDisguiseDate;

private:
	void	DataDefault();
	void	DataShow();
	BOOL	DataSave();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedSitemcustomEditButtonItemid();
	afx_msg void OnBnClickedSitemcustomEditButtonDisId();
	afx_msg void OnBnClickedSitemcustomEditButtonDisDate();
	afx_msg void OnBnClickedSitemcustomEditButtonDisRemove();
	afx_msg void OnBnClickedSitemcustomEditButtonDisDefault();
	afx_msg void OnBnClickedSitemcustomEditButtonPreset();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnEnChangeSitemcustomEditEditItemMid();
	afx_msg void OnEnChangeSitemcustomEditEditItemSid();
	afx_msg void OnEnChangeSitemcustomEditEditDisMid();
	afx_msg void OnEnChangeSitemcustomEditEditDisSid();
	afx_msg void OnBnClickedSitemcustomEditButtonColor1();
	afx_msg void OnBnClickedSitemcustomEditButtonColor2();
	afx_msg void OnBnClickedSitemcustomEditButtonColorDefault();
	afx_msg void OnBnClickedSitemcustomEditButtonColorDefaultCostume();
	afx_msg void OnBnClickedSitemcustomEditButtonColorDefaultReset();
};

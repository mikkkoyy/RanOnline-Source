#pragma once

#include "GLItem.h"

// CDlgItemEdit dialog

class CDlgItemEdit : public CDialog
{
	DECLARE_DYNAMIC(CDlgItemEdit)

public:
	CDlgItemEdit(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgItemEdit();

// Dialog Data
	enum { IDD = IDD_DIALOG_ITEMEDIT };

public:
	SITEMCUSTOM		m_sITEMCUSTOM;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};

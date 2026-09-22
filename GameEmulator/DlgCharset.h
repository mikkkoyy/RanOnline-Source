#pragma once

//character Setting
#include "EmulatorData.h"

// CDlgCharset dialog

class CDlgCharset : public CDialog
{
	DECLARE_DYNAMIC(CDlgCharset)

public:
	CDlgCharset(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgCharset();

// Dialog Data
	enum { IDD = IDD_DLG_CHARSET };

public:
	SCHARSET sCharacter;

	void GetSettings();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();

public:
	afx_msg void OnBnClickedBtnCharset();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedChkMaxlevel();
};

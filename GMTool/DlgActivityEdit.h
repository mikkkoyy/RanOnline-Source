#pragma once

#include "GLActivityData.h"

// CDlgActivityEdit dialog

class CDlgActivityEdit : public CDialog
{
	DECLARE_DYNAMIC(CDlgActivityEdit)

public:
	CDlgActivityEdit(CWnd* pParent = NULL, SACTIVITY_CHAR_DATA* pData = NULL );   // standard constructor
	virtual ~CDlgActivityEdit();

// Dialog Data
	enum { IDD = IDD_DLG_ACTIVITY_EDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	SACTIVITY_CHAR_DATA* m_pData;

private:
	BOOL DataDefault();
	BOOL DataShow();
	BOOL DataSave();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};

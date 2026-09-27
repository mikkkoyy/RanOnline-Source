#pragma once

#include "GLCodexData.h"

// CDlgCodexEdit dialog

class CDlgCodexEdit : public CDialog
{
	DECLARE_DYNAMIC(CDlgCodexEdit)

public:
	CDlgCodexEdit(CWnd* pParent = NULL, SCODEX_CHAR_DATA* pData = NULL );   // standard constructor
	virtual ~CDlgCodexEdit();

// Dialog Data
	enum { IDD = IDD_DLG_CODEX_EDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	SCODEX_CHAR_DATA* m_pData;

private:
	BOOL DataDefault();
	BOOL DataShow();
	BOOL DataSave();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};

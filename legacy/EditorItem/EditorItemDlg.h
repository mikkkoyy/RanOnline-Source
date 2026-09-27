
// EditorItemDlg.h : header file
//

#pragma once

#include "Search.h"

class	CsheetWithTab;

// CEditorItemDlg dialog
class CEditorItemDlg : public CDialog
{
// Construction
public:
	CEditorItemDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_EDITORITEM_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	char			m_szAppPath[MAX_PATH];
	CString			m_strAppPath;
	CsheetWithTab	*m_pSheetTab;
	CSearch			m_SearchDlg;

	bool			DataLoad();
	bool			DataSave();

public:
	std::vector<SSEARCH>	m_vec;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

protected:
	virtual void PostNcDestroy();

public:
	afx_msg void OnClose();
	afx_msg void OnBnClickedLoadItem();
	afx_msg void OnBnClickedSaveItem();
	afx_msg void OnBnClickedLoadCsv();
	afx_msg void OnBnClickedSaveCsv();
	afx_msg void OnBnClickedSearch();
	afx_msg void OnBnClickedLoadCsvRopt();
	afx_msg void OnBnClickedSaveCsvRopt();
};

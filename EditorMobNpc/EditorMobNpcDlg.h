
// EditorMobNpcDlg.h : header file
//

#pragma once

#include "DlgSearch.h"

class	CsheetWithTab;

// CEditorMobNpcDlg dialog
class CEditorMobNpcDlg : public CDialog
{
// Construction
public:
	CEditorMobNpcDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_EDITORMOBNPC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	char			m_szAppPath[MAX_PATH];
	CString			m_strAppPath;
	CsheetWithTab	*m_pSheetTab;
	CDlgSearch		m_SearchDlg;

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
	afx_msg void OnBnClickedLoadMnsf();
	afx_msg void OnBnClickedSaveMnsf();
	afx_msg void OnBnClickedLoadCsv();
	afx_msg void OnBnClickedSaveCsv();
	afx_msg void OnBnClickedSearch();
	afx_msg void OnBnClickedDivideHp();
	afx_msg void OnBnClickedDivideDamage();
};

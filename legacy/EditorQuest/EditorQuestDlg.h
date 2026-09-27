
// EditorQuestDlg.h : header file
//

#pragma once

#include "GLQuestMan.h"
#include "GLMapList.h"
#include "DlgSearch.h"

class CsheetWithTab;
// CEditorQuestDlg dialog
class CEditorQuestDlg : public CDialog
{
// Construction
public:
	CEditorQuestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_EDITORQUEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	char m_szAppPath[MAX_PATH];
	CString m_strAppPath;

public:
	CsheetWithTab	*m_pSheetTab;

public:
	std::vector<SSEARCH>	m_vec;
	CDlgSearch			m_SearchDlg;


public:
	static GLMapList m_sMapList;

public:
	BOOL	DoSave();
	int		GetID ( CString& strID );

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

	afx_msg void OnBnClickedQuestNew();
	afx_msg void OnBnClickedQuestLoad();
	afx_msg void OnBnClickedQuestSave();
	afx_msg void OnBnClickedButtonCsvLoad();
	afx_msg void OnBnClickedButtonCsvSave();
	afx_msg void OnBnClickedButtonTextImport();
	afx_msg void OnBnClickedButtonTextExport();
	afx_msg void OnBnClickedButtonClassConvert();
	afx_msg void OnBnClickedButtonSearchQuest();
};

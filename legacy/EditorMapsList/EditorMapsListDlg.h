
// EditorMapsListDlg.h : header file
//

#include "GLMapList.h"

#pragma once


// CEditorMapsListDlg dialog
class CEditorMapsListDlg : public CDialog
{
// Construction
public:
	CEditorMapsListDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_EDITORMAPSLIST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	char			m_szAppPath[MAX_PATH];
	CString			m_strAppPath;
	GLMapList		m_mapList;

public:
	CListCtrl		m_ListMap;
	BOOL			m_bAdd;

public:
	void	UpdateList();
	void	MapEdit();
	void	ShowMapData( SMAPNODE_DATA sMapNode, BOOL bEnable, BOOL bAdd );

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
	afx_msg void OnBnClickedButtonFileLoad();
	afx_msg void OnBnClickedButtonFileSave();
	afx_msg void OnBnClickedButtonFileImport();
	afx_msg void OnBnClickedButtonMapslistAdd();
	afx_msg void OnBnClickedButtonMapslistEdit();
	afx_msg void OnBnClickedButtonMapslistDelete();
	afx_msg void OnBnClickedButtonMapslistClear();
	afx_msg void OnNMDblclkListMap(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonMapLevelfile();
	afx_msg void OnBnClickedButtonMapBgmfile();
	afx_msg void OnBnClickedButtonMapLoadingimage();
	afx_msg void OnBnClickedButtonMapSave();
	afx_msg void OnBnClickedButtonMapslistSetfield0();
};

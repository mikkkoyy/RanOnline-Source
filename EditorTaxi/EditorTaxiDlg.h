
// EditorTaxiDlg.h : header file
//

#pragma once

#include "GLTaxiData.h"
#include "GLMapList.h"
#include "GLDefine.h"

// CEditorTaxiDlg dialog
class CEditorTaxiDlg : public CDialog
{
// Construction
public:
	CEditorTaxiDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_EDITORTAXI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	char			m_szAppPath[MAX_PATH];
	CString			m_strAppPath;
	GLMapList		m_mapList;

public:
	CListCtrl		m_ListTaxi;

public:
	BOOL	DataShow();
	BOOL	DataSave();
	void	UpdateTaxiList();
	void	TaxiEdit();
	void	TaxiDataShow( int nIndex, STAXI_STATION sTaxi, BOOL bENABLE );

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
	afx_msg void OnBnClickedButtonTaxiAdd();
	afx_msg void OnBnClickedButtonTaxiEdit();
	afx_msg void OnBnClickedButtonTaxiDelete();
	afx_msg void OnBnClickedButtonTaxiClear();
	afx_msg void OnNMDblclkListTaxi(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonTaxieditMap();
	afx_msg void OnBnClickedButtonTaxieditNpc();
	afx_msg void OnBnClickedButtonTaxieditSave();
	afx_msg void OnEnChangeEditTaxiMapMid();
	afx_msg void OnEnChangeEditTaxiMapSid();
	afx_msg void OnEnChangeEditTaxiNpcMid();
	afx_msg void OnEnChangeEditTaxiNpcSid();
};

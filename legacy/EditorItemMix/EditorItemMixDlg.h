
// EditorItemMixDlg.h : header file
//

#pragma once

#include "GLItemMix.h"

// CEditorItemMixDlg dialog
class CEditorItemMixDlg : public CDialog
{
// Construction
public:
	CEditorItemMixDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_EDITORITEMMIX_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	char			m_szAppPath[MAX_PATH];
	CString			m_strAppPath;

public:
	CListCtrl		m_ListItemMix;
	BOOL			m_bAdd;

public:
	void	UpdateItemMixList();
	void	ItemMixEdit();
	void	CheckItem( int nMID, int nSID, int nNAME );
	void	ItemMixShow( ITEM_MIX sITEMMIX, BOOL bENABLE, BOOL bADD );

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
	afx_msg void OnBnClickedButtonItemmixAdd();
	afx_msg void OnBnClickedButtonItemmixEdit();
	afx_msg void OnBnClickedButtonItemmixDelete();
	afx_msg void OnBnClickedButtonItemmixClear();

	afx_msg void OnNMDblclkListItemmix(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonItemmixResult();
	afx_msg void OnBnClickedButtonItemmix0();
	afx_msg void OnBnClickedButtonItemmix1();
	afx_msg void OnBnClickedButtonItemmix2();
	afx_msg void OnBnClickedButtonItemmix3();
	afx_msg void OnBnClickedButtonItemmix4();
	afx_msg void OnBnClickedButtonItemmixDelResult();
	afx_msg void OnBnClickedButtonItemmixDel0();
	afx_msg void OnBnClickedButtonItemmixDel1();
	afx_msg void OnBnClickedButtonItemmixDel2();
	afx_msg void OnBnClickedButtonItemmixDel3();
	afx_msg void OnBnClickedButtonItemmixDel4();
	afx_msg void OnEnChangeEditMidResult();
	afx_msg void OnEnChangeEditSidResult();
	afx_msg void OnEnChangeEditMidItemmix0();
	afx_msg void OnEnChangeEditSidItemmix0();
	afx_msg void OnEnChangeEditMidItemmix1();
	afx_msg void OnEnChangeEditSidItemmix1();
	afx_msg void OnEnChangeEditMidItemmix2();
	afx_msg void OnEnChangeEditSidItemmix2();
	afx_msg void OnEnChangeEditMidItemmix3();
	afx_msg void OnEnChangeEditSidItemmix3();
	afx_msg void OnEnChangeEditMidItemmix4();
	afx_msg void OnEnChangeEditSidItemmix4();
	afx_msg void OnBnClickedButtonItemmixSave();
};

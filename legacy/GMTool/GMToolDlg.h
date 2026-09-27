
// GMToolDlg.h : header file
//

#pragma once

class CsheetWithTab;

// CGMToolDlg dialog
class CGMToolDlg : public CDialog
{
// Construction
public:
	CGMToolDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_GMTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	char		m_szAppPath[MAX_PATH];
	CString		m_strAppPath;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	CsheetWithTab*		m_pSheetTab;

protected:
	virtual void PostNcDestroy();

public:
	afx_msg void OnClose();
	afx_msg void OnBnClickedMainpageButtonSetting();
};

#include "afxcmn.h"
#include "../../Lib_Engine/Common/GLTexFile.h"
#pragma once


// CEditGenItemDlg dialog
class CEditGenItemDlg : public CDialog
{
// Construction
public:
	CEditGenItemDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ABF_DIALOG };


	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	HRESULT hr;
	CString strTemp, strLine,strSaleType,AppendAllTexts;
	CStringFile StrFile;
	WORD dwVERSION;
	gltexfile glTextFile;
	DWORD dwNUM;
	DWORD dwSpecNum;
	float m_fGenRate;
	DWORD dwIDNUM;
	DWORD wNum[100];
	float fRate[100];
	DWORD dwSpecID[256];
	float ifRate[256];
	DWORD wMainID[256];
	DWORD wSubID[256];
	float idfRate[256];
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	BOOL		 UpdateData(const char* szFileName);
	BOOL		 SaveData(const char* lpszPathName);
	void		 InitSaleList();
	void		 InitBoneList();
	CListCtrl Sale_List;
	CListCtrl ItemSpec_List;
	CListCtrl ItemID_List;
	afx_msg void OnBnClickedButtonSalelistAdd();
	afx_msg void OnBnClickedButtonSalelistDelete();
	afx_msg void OnNMDblclkListSale(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonCrowsaleLoad();
	afx_msg void OnBnClickedButtonCrowsaleSave();
	afx_msg void OnBnClickedButtonSalelistOk();

	afx_msg void OnBnClickedButtonSpeclistAdd();
	afx_msg void OnBnClickedButtonSpeclistDelete();
	afx_msg void OnNMDblclkListSpec(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonSpeclistOk();

	afx_msg void OnBnClickedButtonItemlistAdd();
	afx_msg void OnBnClickedButtonItemlistDelete();
	afx_msg void OnNMDblclkListItem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonItemlistOk();
	afx_msg void OnBnClickedButtonUpdateGenItem();
};

#pragma once

#include "GLDefine.h"

struct SCHARDATA2;

// DlgCharCopy dialog

class CDlgCharCopy : public CDialog
{
	DECLARE_DYNAMIC(CDlgCharCopy)

public:
	CDlgCharCopy(CWnd* pParent = NULL, 
		DWORD dwCHARID = SNATIVEID::ID_NULL, 
		DWORD dwUSERID = SNATIVEID::ID_NULL, 
		std::string strNAME = "" );   // standard constructor
	virtual ~CDlgCharCopy();

// Dialog Data
	enum { IDD = IDD_DLG_CHARCOPY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	SCHARDATA2*		m_pData;
	DWORD m_dwCHARID;
	DWORD m_dwUSERID;
	std::string m_strNAME;

private:
	BOOL CheckUser();
	BOOL CheckName();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedCopyButtonCheckuser();
	afx_msg void OnBnClickedCopyButtonCheckname();
	afx_msg void OnBnClickedCopyButtonCopy();
};

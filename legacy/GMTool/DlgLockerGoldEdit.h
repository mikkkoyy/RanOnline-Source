#pragma once


// CDlgLockerGoldEdit dialog

class CDlgLockerGoldEdit : public CDialog
{
	DECLARE_DYNAMIC(CDlgLockerGoldEdit)

public:
	CDlgLockerGoldEdit(CWnd* pParent = NULL);   // standard constructor
	CDlgLockerGoldEdit(LONGLONG llVal, CWnd* pParent = NULL); 
	virtual ~CDlgLockerGoldEdit();

// Dialog Data
	enum { IDD = IDD_DLG_LOCKER_EDIT_GOLD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	LONGLONG	m_llValue;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};

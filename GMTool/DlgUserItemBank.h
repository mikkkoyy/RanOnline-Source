#pragma once

#include "Logic/GMToolData.h"

// CDlgUserItemBank dialog

class CDlgUserItemBank : public CDialog
{
	DECLARE_DYNAMIC(CDlgUserItemBank)

public:
	CDlgUserItemBank(CWnd* pParent = NULL);   // standard constructor
	CDlgUserItemBank(int nDialogID, DWORD dwID, std::string strNAME, CWnd* pParent = NULL);
	virtual ~CDlgUserItemBank();

	BOOL Create(CWnd * pWnd){ 
		return CDialog::Create( CDlgUserItemBank::IDD, pWnd );
	}
// Dialog Data
	enum { IDD = IDD_DLG_USER_ITEMBANK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()


public:
	CWnd*		m_pParentWnd;

private:
	int			m_nDialogID;
	DWORD		m_dwID;
	std::string m_strNAME;

public:
	CListCtrl	m_List;

public:
	SGMTOOL_USER_BANK_ITEM_MAP m_mapUserBank;

public:
	void	DataGet();
	void	DataShow();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedUserItembankButtonRefresh();
	afx_msg void OnBnClickedUserItembankButtonInsertitem();
	afx_msg void OnBnClickedUserItembankButtonDelete();
	afx_msg void OnBnClickedUserItembankButtonClear();
	afx_msg void OnBnClickedCancel();
};

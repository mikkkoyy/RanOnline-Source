#pragma once

class	CsheetWithTab;
// CPageEquipmentLock dialog

class CPageEquipmentLock : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageEquipmentLock)

public:
	CPageEquipmentLock(LOGFONT logfont);
	virtual ~CPageEquipmentLock();

// Dialog Data
	enum { IDD = IDD_MAINPAGE_EQUIPMENT_LOCK };
	enum { MAX_PAGE_EQUIPMENT_LOCK_MODAL_DIALOG = 10 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

public:
	CListCtrl	m_List;

public:
	CDialog*	m_pToolDialog[MAX_PAGE_EQUIPMENT_LOCK_MODAL_DIALOG];

public:
	int		DialogNewID();
	void	DialogDeleteID( int nDialogID );
	int		DialogGetTotalActive();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedEquipmentLockPageButtonAdd();
	afx_msg void OnBnClickedEquipmentLockPageButtonEdit();
	afx_msg void OnBnClickedEquipmentLockPageButtonDelete();
	afx_msg void OnBnClickedEquipmentLockPageButtonSearch();
	afx_msg void OnNMDblclkListEquipmentLock(NMHDR *pNMHDR, LRESULT *pResult);
};

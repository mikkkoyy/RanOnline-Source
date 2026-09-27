#pragma once

class	CLockerTab;
struct SCHARDATA2;

// CLockerPageItem4 dialog

class CLockerPageItem4 : public CPropertyPage
{
	DECLARE_DYNAMIC(CLockerPageItem4)

public:
	CLockerPageItem4(LOGFONT logfont, SCHARDATA2* pData);
	virtual ~CLockerPageItem4();

// Dialog Data
	enum { IDD = IDD_LOCKER_PAGE_ITEM_4 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CLockerTab*		m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;
	SCHARDATA2*		m_pData;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CLockerTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

private:
	CListCtrl	m_List;

public:
	int		m_nLockerChannel;

public:
	void	ShowData();
	void	EditItem();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedLockerPageButtonAdd();
	afx_msg void OnBnClickedLockerPageButtonEdit();
	afx_msg void OnBnClickedLockerPageButtonDelete();
	afx_msg void OnBnClickedLockerPageButtonClear();
	afx_msg void OnNMDblclkLockerPageList(NMHDR *pNMHDR, LRESULT *pResult);
};

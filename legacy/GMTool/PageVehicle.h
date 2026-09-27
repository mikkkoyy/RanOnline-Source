#pragma once

class	CsheetWithTab;
// CPageVehicle dialog

class CPageVehicle : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageVehicle)

public:
	CPageVehicle(LOGFONT logfont);
	virtual ~CPageVehicle();

// Dialog Data
	enum { IDD = IDD_MAINPAGE_VEHICLE };
	enum { MAX_PAGEVEHICLE_MODAL_DIALOG = 10 };

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
	CDialog*	m_pToolDialog[MAX_PAGEVEHICLE_MODAL_DIALOG];

public:
	int		DialogNewID();
	void	DialogDeleteID( int nDialogID );
	int		DialogGetTotalActive();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedVehiclePageButtonSearch();
	afx_msg void OnNMDblclkListVehicle(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedVehiclePageButtonEdit();
};

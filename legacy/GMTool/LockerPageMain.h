#pragma once

class	CLockerTab;
struct SCHARDATA2;

// CLockerPageMain dialog

class CLockerPageMain : public CPropertyPage
{
	DECLARE_DYNAMIC(CLockerPageMain)

public:
	CLockerPageMain(LOGFONT logfont, SCHARDATA2* pData);
	virtual ~CLockerPageMain();

// Dialog Data
	enum { IDD = IDD_LOCKER_PAGE_MAIN };

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

public:
	void	ShowData();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedLockerMainRentalButtonSet1();
	afx_msg void OnBnClickedLockerMainRentalButtonSet2();
	afx_msg void OnBnClickedLockerMainRentalButtonSet3();
	afx_msg void OnBnClickedLockerMainRentalButtonReset1();
	afx_msg void OnBnClickedLockerMainRentalButtonReset2();
	afx_msg void OnBnClickedLockerMainRentalButtonReset3();
	afx_msg void OnBnClickedLockerMainGoldButton();
};

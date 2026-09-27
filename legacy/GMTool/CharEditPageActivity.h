#pragma once

class	CCharEditTab;
struct SCHARDATA2;

// CCharEditPageActivity dialog

class CCharEditPageActivity : public CPropertyPage
{
	DECLARE_DYNAMIC(CCharEditPageActivity)

public:
	CCharEditPageActivity(LOGFONT logfont, SCHARDATA2* pData);
	virtual ~CCharEditPageActivity();

// Dialog Data
	enum { IDD = IDD_CHAREDIT_PAGE_ACTIVITY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CCharEditTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;
	SCHARDATA2*		m_pData;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CCharEditTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

private:
	CListCtrl	m_ListProg;
	CListCtrl	m_ListDone;

public:
	void UpdateProgList();
	void UpdateDoneList();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedChareditActivityButtonProgEdit();
	afx_msg void OnBnClickedChareditActivityButtonProgComplete();
	afx_msg void OnBnClickedChareditActivityButtonProgDelete();
	afx_msg void OnBnClickedChareditActivityButtonProgClear();
	afx_msg void OnBnClickedChareditActivityButtonDoneDelete();
	afx_msg void OnBnClickedChareditActivityButtonDoneClear();
};

#pragma once

class	CCharEditTab;
struct SCHARDATA2;

// CCharEditPageCodex dialog

class CCharEditPageCodex : public CPropertyPage
{
	DECLARE_DYNAMIC(CCharEditPageCodex)

public:
	CCharEditPageCodex(LOGFONT logfont, SCHARDATA2* pData);
	virtual ~CCharEditPageCodex();

// Dialog Data
	enum { IDD = IDD_CHAREDIT_PAGE_CODEX };

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

	afx_msg void OnBnClickedChareditCodexButtonProgEdit();
	afx_msg void OnBnClickedChareditCodexButtonProgComplete();
	afx_msg void OnBnClickedChareditCodexButtonProgDelete();
	afx_msg void OnBnClickedChareditCodexButtonProgClear();
	afx_msg void OnBnClickedChareditCodexButtonDoneDelete();
	afx_msg void OnBnClickedChareditCodexButtonDoneClear();
	afx_msg void OnBnClickedChareditActivityButtonProgComplete2();
	afx_msg void OnBnClickedChareditCodexButtonProgCompleteall();
};

#pragma once

class	CCharEditTab;
struct SCHARDATA2;

// CCharEditPageQuest dialog

class CCharEditPageQuest : public CPropertyPage
{
	DECLARE_DYNAMIC(CCharEditPageQuest)

public:
	CCharEditPageQuest(LOGFONT logfont, SCHARDATA2* pData);
	virtual ~CCharEditPageQuest();

// Dialog Data
	enum { IDD = IDD_CHAREDIT_PAGE_QUEST };

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
	CListCtrl	m_ListProc;
	CListCtrl	m_ListEnd;

public:
	void UpdateQuestProc();
	void UpdateQuestEnd();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedChareditQuestButtonProcInsert();
	afx_msg void OnBnClickedChareditQuestButtonProcComplete();
	afx_msg void OnBnClickedChareditQuestButtonProcDelete();
	afx_msg void OnBnClickedChareditQuestButtonProcClear();
	afx_msg void OnBnClickedChareditQuestButtonEndDelete();
	afx_msg void OnBnClickedChareditQuestButtonEndClear();
};

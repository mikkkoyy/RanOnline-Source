#pragma once

class	CCharEditTab;
struct SCHARDATA2;

// CCharEditPageCoolTime dialog

class CCharEditPageCoolTime : public CPropertyPage
{
	DECLARE_DYNAMIC(CCharEditPageCoolTime)

public:
	CCharEditPageCoolTime(LOGFONT logfont, SCHARDATA2* pData);
	virtual ~CCharEditPageCoolTime();

// Dialog Data
	enum { IDD = IDD_CHAREDIT_PAGE_COOLTIME };

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
	CListCtrl	m_ListID;
	CListCtrl	m_ListTYPE;

public:
	void UpdateCoolTimeID();
	void UpdateCoolTimeTYPE();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedChareditCooltimeButtonIdDelete();
	afx_msg void OnBnClickedChareditCooltimeButtonIdClear();
	afx_msg void OnBnClickedChareditCooltimeButtonTypeDelete();
	afx_msg void OnBnClickedChareditCooltimeButtonTypeClear();
	afx_msg void OnBnClickedChareditCooltimeButtonAdd();
	afx_msg void OnBnClickedChareditCooltimeButtonInsert();
	afx_msg void OnEnChangeChareditCooltimeEditMid();
	afx_msg void OnEnChangeChareditCooltimeEditSid();
};

#pragma once

class	CPetEditTab;
struct GLPET;

// CPetEditPageSkill dialog

class CPetEditPageSkill : public CPropertyPage
{
	DECLARE_DYNAMIC(CPetEditPageSkill)

public:
	CPetEditPageSkill(LOGFONT logfont, GLPET* pData);
	virtual ~CPetEditPageSkill();

// Dialog Data
	enum { IDD = IDD_PETEDIT_PAGE_SKILL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CPetEditTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;
	GLPET*			m_pData;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CPetEditTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

private:
	CListCtrl	m_List;

public:
	void DataShow();
	void DataEdit();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedPeteditskillButtonAdd();
	afx_msg void OnBnClickedPeteditskillButtonDelete();
	afx_msg void OnBnClickedPeteditskillButtonClear();
};

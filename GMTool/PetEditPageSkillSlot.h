#pragma once

class	CPetEditTab;
struct GLPET;

// CPetEditPageSkillSlot dialog

class CPetEditPageSkillSlot : public CPropertyPage
{
	DECLARE_DYNAMIC(CPetEditPageSkillSlot)

public:
	CPetEditPageSkillSlot(LOGFONT logfont, GLPET* pData);
	virtual ~CPetEditPageSkillSlot();

// Dialog Data
	enum { IDD = IDD_PETEDIT_PAGE_SKILLSLOT };

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

public:
	void	DataShow();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedPeteditskillslotButtonSet0();
	afx_msg void OnBnClickedPeteditskillslotButtonReset0();
	afx_msg void OnBnClickedPeteditskillslotButtonSet1();
	afx_msg void OnBnClickedPeteditskillslotButtonReset1();
};

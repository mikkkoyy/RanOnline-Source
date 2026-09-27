#pragma once

class	CCharEditTab;
struct SCHARDATA2;

// CCharEditPageSlotSkill dialog

class CCharEditPageSlotSkill : public CPropertyPage
{
	DECLARE_DYNAMIC(CCharEditPageSlotSkill)

public:
	CCharEditPageSlotSkill(LOGFONT logfont, SCHARDATA2* pData);
	virtual ~CCharEditPageSlotSkill();

// Dialog Data
	enum { IDD = IDD_CHAREDIT_PAGE_SLOT_SKILL };

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
	CListCtrl	m_List;

public:
	void DataShow();
	void DataEdit();

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedChareditSkillslotButtonEdit();
	afx_msg void OnBnClickedChareditSkillslotButtonDelete();
	afx_msg void OnBnClickedChareditSkillslotButtonClear();
	afx_msg void OnNMDblclkChareditSkillslotList(NMHDR *pNMHDR, LRESULT *pResult);
};

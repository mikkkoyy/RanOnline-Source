#pragma once


// CDlgPresetSkill dialog

class CDlgPresetSkill : public CDialog
{
	DECLARE_DYNAMIC(CDlgPresetSkill)

public:
	CDlgPresetSkill(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgPresetSkill();

// Dialog Data
	enum { IDD = IDD_DLG_PRESET_SKILL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CListCtrl	m_List;

public:
	void DataShow();
	void DataEdit();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedPresetSkillButtonNew();
	afx_msg void OnBnClickedPresetSkillButtonEdit();
	afx_msg void OnBnClickedPresetSkillButtonDelete();
	afx_msg void OnBnClickedPresetSkillButtonClear();
	afx_msg void OnBnClickedOk();
	afx_msg void OnNMDblclkListPresetSkill(NMHDR *pNMHDR, LRESULT *pResult);
};

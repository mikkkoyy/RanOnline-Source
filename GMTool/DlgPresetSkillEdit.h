#pragma once

struct SGMTOOL_PRESET_SKILL;
// CDlgPresetSkillEdit dialog

class CDlgPresetSkillEdit : public CDialog
{
	DECLARE_DYNAMIC(CDlgPresetSkillEdit)

public:
	CDlgPresetSkillEdit(CWnd* pParent = NULL);   // standard constructor
	CDlgPresetSkillEdit(SGMTOOL_PRESET_SKILL* pData, CWnd* pParent = NULL); 
	virtual ~CDlgPresetSkillEdit();

// Dialog Data
	enum { IDD = IDD_DLG_PRESET_SKILL_EDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CListCtrl	m_List;

private:
	SGMTOOL_PRESET_SKILL*	m_pData;

public:
	void	DataShow();
	BOOL	DataSave();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedPresetSkillEditButtonNew();
	afx_msg void OnBnClickedPresetSkillEditButtonDelete();
	afx_msg void OnBnClickedPresetSkillEditButtonClear();
	afx_msg void OnBnClickedPresetSkillEditButtonSkill();
	afx_msg void OnBnClickedPresetSkillEditButtonSave();
	afx_msg void OnBnClickedPresetSkillEditButtonCancel();
	afx_msg void OnBnClickedPresetSkillEditButtonClose();
	afx_msg void OnEnChangePresetSkillEditEditSkillMid();
	afx_msg void OnEnChangePresetSkillEditEditSkillSid();
};

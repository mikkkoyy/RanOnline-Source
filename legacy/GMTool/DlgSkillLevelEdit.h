#pragma once

struct SCHARSKILL;
// CDlgSkillLevelEdit dialog

class CDlgSkillLevelEdit : public CDialog
{
	DECLARE_DYNAMIC(CDlgSkillLevelEdit)

public:
	CDlgSkillLevelEdit(CWnd* pParent = NULL);   // standard constructor
	CDlgSkillLevelEdit(SCHARSKILL* pData, CWnd* pParent = NULL); 
	virtual ~CDlgSkillLevelEdit();

// Dialog Data
	enum { IDD = IDD_DLG_SKILLLEVEL_EDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	SCHARSKILL*	m_pData;

public:
	void DataShow();
	BOOL DataSave();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};

#pragma once

#include "GLPet.h"

// CDlgPetSkillList dialog

class CDlgPetSkillList : public CDialog
{
	DECLARE_DYNAMIC(CDlgPetSkillList)

public:
	CDlgPetSkillList(CWnd* pParent = NULL);   // standard constructor
	CDlgPetSkillList( GLPET* pData, CWnd* pParent = NULL );
	virtual ~CDlgPetSkillList();

// Dialog Data
	enum { IDD = IDD_DLG_PETSKILL_LIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	GLPET*		m_pData;
	SNATIVEID	m_SELECTED;

private:
	CListCtrl	m_List;

public:
	void	ReadData();
	void	SelectionReset();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnLvnItemchangedSkillList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};

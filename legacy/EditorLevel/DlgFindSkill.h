#pragma once

#include "GLSkill.h"
// CDlgFindSkill dialog

struct SSKILLDATA
{
	SNATIVEID sSKILLID;
	std::string strSKILLNAME;

	SSKILLDATA ()
		: sSKILLID( false )
	{
		strSKILLNAME = "";
	};
};


class CDlgFindSkill : public CDialog
{
	DECLARE_DYNAMIC(CDlgFindSkill)

public:
	CDlgFindSkill(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgFindSkill();

// Dialog Data
	enum { IDD = IDD_DLG_FIND_SKILL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CListCtrl	m_List;
	std::vector<SSKILLDATA>	m_vecSkill;
	std::vector<SSKILLDATA>	m_vecResult;

public:
	SNATIVEID	m_sSELECTED;

public:
	void ReadSkill();
	void ShowResults();
	void DataSearch();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonOk2();
	afx_msg void OnBnClickedButtonHelperSearch();
};

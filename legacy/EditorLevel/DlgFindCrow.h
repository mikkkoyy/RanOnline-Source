#pragma once

#include "GLCrowData.h"

struct SCROW_DATA_SIMPLE
{
	SNATIVEID sCrowID;
	std::string strCrowName;
	std::string strCrowSkin;

	SCROW_DATA_SIMPLE ()
		: sCrowID( false )
	{
		strCrowName = "";
		strCrowSkin = "";
	};
};

// CDlgFindCrow dialog

class CDlgFindCrow : public CDialog
{
	DECLARE_DYNAMIC(CDlgFindCrow)

public:
	CDlgFindCrow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgFindCrow();

// Dialog Data
	enum { IDD = IDD_DLG_FIND_CROW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CListCtrl	m_List;
	std::vector<SCROW_DATA_SIMPLE>	m_vecCrow;
	std::vector<SCROW_DATA_SIMPLE>	m_vecResult;

public:
	SNATIVEID	m_sSELECTED;
	std::string m_strTitle;

public:
	void GetCrows();
	void ShowResults();
	void DataSearch();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnBnClickedButtonHelperSearch();
};

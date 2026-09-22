#pragma once

#include "GLItemMan.h"
#include "GLItem.h"

// CDlgFindItem dialog
struct SITEMDATA
{
	SNATIVEID sITEMID;
	std::string strITEMNAME;

	SITEMDATA ()
		: sITEMID( false )
	{
		strITEMNAME = "";
	};
};

class CDlgFindItem : public CDialog
{
	DECLARE_DYNAMIC(CDlgFindItem)

public:
	CDlgFindItem(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgFindItem();

// Dialog Data
	enum { IDD = IDD_DLG_FIND_ITEM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CListCtrl	m_List;
	std::vector<SITEMDATA>	m_vecITEM;
	std::vector<SITEMDATA>	m_vecResult;

public:
	SNATIVEID	m_sSELECTED;

public:
	void GetItems();
	void ShowResults();
	void DataSearch();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnBnClickedButtonHelperSearch();
};

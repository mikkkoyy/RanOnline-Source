#pragma once

#include <vector>
#include "GLMapList.h"

struct SMAPDATA_LIST
{
	SNATIVEID sMAPID;
	std::string strNAME;

	SMAPDATA_LIST ()
		/*:*/
	{
		strNAME = "";
	};
};
// CDlgFindMap dialog

class CDlgFindMap : public CDialog
{
	DECLARE_DYNAMIC(CDlgFindMap)

public:
	CDlgFindMap(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgFindMap();

// Dialog Data
	enum { IDD = IDD_DLG_FIND_MAP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CListCtrl	m_List;
	std::vector<SMAPDATA_LIST>	m_vecMap;
	std::vector<SMAPDATA_LIST>	m_vecResult;

public:
	SMAPDATA_LIST	m_sSELECTED;
	GLMapList		m_mapList;

public:
	void GetMaps();
	void ShowResults();
	void DataSearch();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnBnClickedButtonHelperSearch();
};

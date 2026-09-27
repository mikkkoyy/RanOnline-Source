#pragma once

#include "GLDefine.h"

enum EMSEARCH
{
	EMSEARCH_NAME	= 0,
	EMSEARCH_MID	= 1,
	EMSEARCH_SID	= 2,
	EMSEARCH_WEAR	= 3,
	EMSEARCH_DDS	= 4,
	EMSEARCH_DDSXY	= 5,
	EMSEARCH_TYPE	= 6,
	EMSEARCH_GENID	= 7,
	EMSEARCH_RANDOM_OPT	= 8,
	EMSEARCH_TOTAL	= 9,
};

struct SSEARCH
{
	SNATIVEID	sID;
	std::string strNAME;
	std::string strFIND;

	SSEARCH () 
	{
		sID = NATIVEID_NULL();
		strNAME = "";
		strFIND = "";
	};
};

// CSearch dialog

class CSearch : public CDialog
{
	DECLARE_DYNAMIC(CSearch)

public:
	CSearch(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSearch();

// Dialog Data
	enum { IDD = IDD_DIALOG_SEARCH };

public:
	CListCtrl	m_List;
	SNATIVEID	m_sID;

public:
	std::vector<SSEARCH> *	m_vec;

	void SetContainer( std::vector<SSEARCH>* vec )
	{	
		m_vec = vec;	
	}

	void ShowResults();
	void SearchInit();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonGo();
	afx_msg void OnBnClickedButtonSearch();
	afx_msg void OnCbnSelchangeComboSearchtype();
};

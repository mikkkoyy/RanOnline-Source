#pragma once

#include "GLDefine.h"
#include "GLQUEST.h"
#include "GLQuestMan.h"

// CDlgSearch dialog
enum EMSEARCH
{
	EMSEARCH_TITLE		= 0,
	EMSEARCH_FILENAME	= 1,
	EMSEARCH_ID			= 2,
	EMSEARCH_TOTAL		= 3,
};

struct SSEARCH
{
	DWORD	dwQUESTID;
	std::string strNAME;
	std::string strFIND;
	std::string strFILENAME;

	SSEARCH () 
	{
		dwQUESTID = SNATIVEID::ID_NULL;
		strNAME = "";
		strFIND = "";
		strFILENAME = "";
	};
};


typedef GLQuestMan::SNODE QUESTNODE2;
typedef QUESTNODE2*		  PQUESTNODE2;

class CDlgSearch : public CDialog
{
	DECLARE_DYNAMIC(CDlgSearch)

public:
	CDlgSearch(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSearch();

// Dialog Data
	enum { IDD = IDD_DIALOG_SEARCH };

public:
	CListCtrl	m_List;
	DWORD		m_dwQUESTID;

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
	afx_msg void OnBnClickedButtonSearch();
	afx_msg void OnBnClickedButtonGo();
	afx_msg void OnCbnSelchangeComboSearchtype();
};

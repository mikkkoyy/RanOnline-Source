#pragma once

#include "GLDefine.h"


enum EMSEARCH
{
	EMSEARCH_NAME	= 0,
	EMSEARCH_MID	= 1,
	EMSEARCH_SID	= 2,
	EMSEARCH_EFFECT	= 3,
	EMSEARCH_DDS	= 4,
	EMSEARCH_DDSXY	= 5,
	EMSEARCH_SKILL_ANIMAIN	= 6,
	EMSEARCH_SKILL_ANISUB	= 7,
	EMSEARCH_SKILL_APPLYTYPE	= 8,
	EMSEARCH_SKILL_STATETROUBLE	= 9,
	EMSEARCH_SKILL_IMPACTADDON	= 10,
	EMSEARCH_SKILL_SPECADDON	= 11,
	EMSEARCH_SKILL_SPECIALTYPE	= 12,
	EMSEARCH_SKILL_ACTIONTYPE	= 13,
	EMSEARCH_TOTAL	= 14,
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
	afx_msg void OnBnClickedButtonSearch();
	afx_msg void OnCbnSelchangeComboSearchtype();
	afx_msg void OnBnClickedButtonGo();
};

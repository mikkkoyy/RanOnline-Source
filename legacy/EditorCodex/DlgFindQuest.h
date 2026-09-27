#pragma once


#include "GLQuestMan.h"

typedef GLQuestMan::SNODE QUESTNODE;
typedef QUESTNODE*		  PQUESTNODE;

struct SQUESTDATA
{
	DWORD	dwQUESTID;
	std::string strQUESTNAME;
	std::string strQUESTFILE;

	SQUESTDATA ()
		: dwQUESTID( NULL )
	{
		strQUESTNAME = "";
		strQUESTFILE = "";
	};
};

// CDlgFindQuest dialog

class CDlgFindQuest : public CDialog
{
	DECLARE_DYNAMIC(CDlgFindQuest)

public:
	CDlgFindQuest(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgFindQuest();

// Dialog Data
	enum { IDD = IDD_DLG_FIND_QUEST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CListCtrl	m_List;
	std::vector<SQUESTDATA>	m_vecData;
	std::vector<SQUESTDATA>	m_vecResult;

public:
	SQUESTDATA	m_sSelected;

public:
	void ReadQuests();
	void ShowResults();
	void DataSearch();

	SQUESTDATA* GetQuestData( DWORD dwQUESTID );

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnBnClickedButtonHelperSearch();
};

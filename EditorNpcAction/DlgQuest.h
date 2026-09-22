#pragma once

#include "GLQUEST.h"
#include "GLQuestMan.h"

typedef GLQuestMan::SNODE QUESTNODE;
typedef QUESTNODE*		  PQUESTNODE;

struct SQUESTDATA_DLG
{
	DWORD	dwQUESTID;
	std::string strQUESTNAME;
	std::string strQUESTFILE;

	SQUESTDATA_DLG ()
		: dwQUESTID( NULL )
	{
		strQUESTNAME = "";
		strQUESTFILE = "";
	};
};
// CDlgQuest dialog

class CDlgQuest : public CDialog
{
	DECLARE_DYNAMIC(CDlgQuest)

public:
	CDlgQuest(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgQuest();

// Dialog Data
	enum { IDD = IDD_DIALOG_QUEST };

public:
	CListBox	m_listSELECT;
	DWORD		m_SelectedQuest;

	std::vector<SQUESTDATA_DLG>	m_vecData;
	std::vector<SQUESTDATA_DLG>	m_vecResult;

public:
	void	ReadData();
	void	ShowData();
	void	SelectionReset();
	void	SearchData();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnLbnSelchangeListSelect();
	afx_msg void OnBnClickedButtonSearch();
};

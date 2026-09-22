#pragma once

#include "GLCharDefine.h"
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
	CDlgQuest(EMCHARCLASS emCLASS, WORD wSCHOOL, CWnd* pParent = NULL);
	virtual ~CDlgQuest();

// Dialog Data
	enum { IDD = IDD_DLG_QUEST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()


public:
	CListBox	m_listSELECT;
	DWORD		m_dwSelectedQuest;

	std::vector<SQUESTDATA_DLG>	m_vecData;
	std::vector<SQUESTDATA_DLG>	m_vecResult;

private:
	EMCHARCLASS m_emCLASS;
	WORD		m_wSCHOOL;

public:
	void	ReadData();
	void	ShowData();
	void	SelectionReset();
	void	SearchData();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonSearch();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnLbnSelchangeListSelect();
};

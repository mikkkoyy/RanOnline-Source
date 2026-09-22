#pragma once

#include "GLQUEST.h"
#include "GLQuestMan.h"

class	CsheetWithTab;
// CPageQuestBasic dialog

class CPageQuestBasic : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageQuestBasic)

public:
	CPageQuestBasic( LOGFONT logfont );
	virtual ~CPageQuestBasic();

// Dialog Data
	enum { IDD = IDD_PAGE_QUEST_BASIC };

protected:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;
	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	GLQuestMan::SNODE	m_DummyQuest;
	GLQuestMan::SNODE*	m_pQuest;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

public:
	BOOL	QuestSet( GLQuestMan::SNODE* pQUEST );
	void	QuestClear();
	void	QuestSave();

	BOOL	DataDefault();
	BOOL	DataShow();
	BOOL	DataSave();

	void	ProgressShow();
	void	ProgressSave();

	BOOL	ValidateProgress();

public:
	GLQUEST_PROGRESS m_tempProgressEvent;
	CComboBox	m_comboSchool;
	CListBox	m_ListBoxItem;
	CListBox	m_ListBoxSkill;

public:
	void	CheckItem();
	void	CheckSkill();
	void	CheckMap();

	void	CheckProgressMapStart();
	void	CheckProgressMapEnd();
	void	CheckProgressMapFail();

	void	UpdateRewardItem();
	void	UpdateRewardSkill();

	void	UpdateClass( BOOL bMALE, BOOL bFEMALE );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnClickedButtonNext();
	afx_msg void OnBnClickedButtonItemAdd();
	afx_msg void OnBnClickedButtonItemDel();
	afx_msg void OnEnChangeEditStartItemMid();
	afx_msg void OnEnChangeEditStartItemSid();
	afx_msg void OnBnClickedButtonSkillAdd();
	afx_msg void OnEnChangeEditStartSkillMid();
	afx_msg void OnEnChangeEditStartSkillSid();
	afx_msg void OnBnClickedButtonSkillDel();
	afx_msg void OnBnClickedButtonMapAdd();
	afx_msg void OnEnChangeEditStartQuestMapMid();
	afx_msg void OnEnChangeEditStartQuestMapSid();
	afx_msg void OnBnClickedButtonMapDel();
	afx_msg void OnBnClickedButtonMapAddStart();
	afx_msg void OnEnChangeEditMapMidStart();
	afx_msg void OnEnChangeEditMapSidStart();
	afx_msg void OnBnClickedButtonMapDelStart();
	afx_msg void OnBnClickedButtonMapAddEnd();
	afx_msg void OnEnChangeEditMapMidEnd();
	afx_msg void OnEnChangeEditMapSidEnd();
	afx_msg void OnBnClickedButtonMapDelEnd();
	afx_msg void OnBnClickedButtonMapAddFail();
	afx_msg void OnEnChangeEditMapMidFail();
	afx_msg void OnEnChangeEditMapSidFail();
	afx_msg void OnBnClickedButtonMapDelFail();
	afx_msg void OnBnClickedButtonGiftItemAdd();
	afx_msg void OnBnClickedButtonGiftItemDel();
	afx_msg void OnBnClickedButtonGiftItemEdit();
	afx_msg void OnBnClickedButtonGiftItemInsert();
	afx_msg void OnBnClickedButtonGiftSkillAdd();
	afx_msg void OnBnClickedButtonGiftSkillDel();
	afx_msg void OnBnClickedButtonGiftSkillInsert();
	afx_msg void OnBnClickedButtonAllMale();
	afx_msg void OnBnClickedButtonAllFemale();
	afx_msg void OnBnClickedButtonAll();
	afx_msg void OnBnClickedButtonNothing();
	afx_msg void OnBnClickedButtonMapDelStart2();
	afx_msg void OnBnClickedButtonMapDelEnd2();
	afx_msg void OnBnClickedButtonMapDelFail2();
};

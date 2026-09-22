#pragma once

#include "GLQuestMan.h"

class	CsheetWithTab;
// CPageQuestStep dialog

class CPageQuestStep : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageQuestStep)

public:
	CPageQuestStep( LOGFONT logfont );
	virtual ~CPageQuestStep();

// Dialog Data
	enum { IDD = IDD_PAGE_QUEST_STEP };

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

public:
	DWORD		m_dwCurStep;
	CListBox	m_ListStep;
	CListBox	m_ListItemGain;
	CListBox	m_ListItemRemove;
	CListBox	m_ListItemGenerate;

public:
	void	StepShowControls( BOOL bSHOW );
	void	StepShow();
	void	StepSave();

public:
	void	CheckCrowNpctalk();
	void	CheckCrowGuardNPC();
	void	CheckCrowKillMob();
	void	CheckMapReachMap();
	void	CheckMapDefendMap();
	void	CheckMapMoveMap();
	
	void	ShowItemGain();
	void	ShowItemRemove();
	void	ShowItemGen();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonBack();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonStepAdd();
	afx_msg void OnBnClickedButtonStepDel();
	afx_msg void OnBnClickedButtonStepMoveup();
	afx_msg void OnBnClickedButtonStepMovedown();
	afx_msg void OnLbnSelchangeListStep();
	afx_msg void OnLbnDblclkListStep();
	afx_msg void OnBnClickedButtonTalknpc();
	afx_msg void OnBnClickedButtonTalknpcDel();
	afx_msg void OnBnClickedButtonGuardnpc();
	afx_msg void OnBnClickedButtonGuardnpcDel();
	afx_msg void OnBnClickedButtonDiemob();
	afx_msg void OnBnClickedButtonDiemobDel();
	afx_msg void OnBnClickedButtonReachMap();
	afx_msg void OnBnClickedButtonReachMapDel();
	afx_msg void OnBnClickedButtonGuardMap();
	afx_msg void OnBnClickedButtonGuardMapDel();
	afx_msg void OnBnClickedStepMapEditButton();
	afx_msg void OnBnClickedStepMapEditDel();
	afx_msg void OnBnClickedButtonItemAddGain();
	afx_msg void OnBnClickedButtonItemDelGain();
	afx_msg void OnBnClickedButtonItemEditGain();
	afx_msg void OnBnClickedButtonItemInsertGain();
	afx_msg void OnBnClickedButtonItemAddRemove();
	afx_msg void OnBnClickedButtonItemDelRemove();
	afx_msg void OnBnClickedButtonItemInsertRemove();
	afx_msg void OnBnClickedButtonItemAddGen();
	afx_msg void OnBnClickedButtonItemDelGen();
	afx_msg void OnBnClickedButtonItemEditGen();
	afx_msg void OnBnClickedButtonItemInsertGen();
	afx_msg void OnBnClickedButtonStepOk();
	afx_msg void OnBnClickedButtonStepEdit();
	afx_msg void OnBnClickedButtonStepCancel();
};

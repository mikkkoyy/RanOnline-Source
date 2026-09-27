#pragma once

#include "NpcTalkCondition.h"
#include "NpcTalk.h"

// CPageCondition dialog
class	CsheetWithTab;
class	CNpcDialogue;

class CPageCondition : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageCondition)

public:
	CPageCondition( LOGFONT logfont );
	virtual ~CPageCondition();

// Dialog Data
	enum { IDD = IDD_PAGE_CONDITION };

protected:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;

	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

protected:
	SNpcTalkCondition*	m_pOrgCondition;
	SNpcTalkCondition	m_DummyCondition;
	SNPC_ITEM			m_TalkNeedItem[5];

	CListCtrl	m_ListItem;
	CListBox	m_listSkill;
	CListBox	m_listDisSkill;
	CListBox	m_listDisQuest;
	CListBox	m_listComQuest;
	CListBox	m_listQuestStep;
	CListBox	m_listBuff; /*npc talk buff condition, Juver, 2017/10/03 */
	CComboBox	m_comboSchool;

public:
	BOOL	SetCondition ( SNpcTalkCondition* pConditon );
	BOOL	DataDefault();
	BOOL	DataUpdate();
	BOOL	DataSave();

	void	UpdateTime();
	void	UpdateItem();
	void	UpdateSkill();
	void	UpdateSkillDis();
	void	UpdateQuestCom();
	void	UpdateQuestDis();
	void	UpdateQuestProg();
	void	UpdateBuff(); /*npc talk buff condition, Juver, 2017/10/03 */
	void	UpdateClass( BOOL bMALE, BOOL bFEMALE );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnBnClickedButtonItemAdd();
	afx_msg void OnBnClickedButtonItemDel();
	afx_msg void OnBnClickedButtonItemDelall();
	afx_msg void OnBnClickedButtonItemInsert();
	afx_msg void OnBnClickedButtonSkillAdd();
	afx_msg void OnBnClickedButtonSkillDel();
	afx_msg void OnBnClickedButtonSkillDelall();
	afx_msg void OnBnClickedButtonSkillInsert();
	afx_msg void OnBnClickedButtonDisSkillAdd();
	afx_msg void OnBnClickedButtonDisSkillDel();
	afx_msg void OnBnClickedButtonDisSkillDellall();
	afx_msg void OnBnClickedButtonDisSkillInsert();
	afx_msg void OnBnClickedButtonComquestAdd();
	afx_msg void OnBnClickedButtonComquestDel();
	afx_msg void OnBnClickedButtonComquestDelall();
	afx_msg void OnBnClickedButtonComquestInsert();
	afx_msg void OnBnClickedButtonDisQuestAdd();
	afx_msg void OnBnClickedButtonDisQuestDel();
	afx_msg void OnBnClickedButtonDisQuestDelall();
	afx_msg void OnBnClickedButtonDisQuestInsert();
	afx_msg void OnBnClickedButtonQuestAdd();
	afx_msg void OnBnClickedButtonQuestDel();
	afx_msg void OnBnClickedButtonQuestInsert();
	afx_msg void OnBnClickedButtonQuestStep();
	afx_msg void OnCbnSelchangeComboLevelSign();
	afx_msg void OnCbnSelchangeComboElementalSign();
	afx_msg void OnCbnSelchangeComboBrightpointSign();
	afx_msg void OnBnClickedCheckTime();
	afx_msg void OnBnClickedButtonAllMale();
	afx_msg void OnBnClickedButtonAllFemale();
	afx_msg void OnBnClickedButtonAll();
	afx_msg void OnBnClickedButtonNothing();
	afx_msg void OnBnClickedButtonBuffInsert();
	afx_msg void OnBnClickedButtonBuffAdd();
	afx_msg void OnBnClickedButtonBuffDel();
	afx_msg void OnBnClickedButtonBuffDelall();
};

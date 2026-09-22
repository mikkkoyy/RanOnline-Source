#pragma once

#include "NpcTalk.h"

class	CsheetWithTab;
class	CNpcTalkControl;
class	CNpcDialogueSet;
// CPageTalk dialog

class CPageTalk : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageTalk)

public:
	CPageTalk( LOGFONT logfont );
	virtual ~CPageTalk();

// Dialog Data
	enum { IDD = IDD_PAGE_TALK };

protected:
	CsheetWithTab*	m_pSheetTab;
	int				m_CallPage;

	BOOL			m_bDlgInit;
	CFont*			m_pFont;

public:
	void	SetCallPage ( int CallPage ) { m_CallPage = CallPage; }
	void	SetSheetTab ( CsheetWithTab* pSheetTab ) { m_pSheetTab = pSheetTab; }

private:
	BOOL		m_bModify;
	SNpcTalk*	m_pOrgTalk;
	SNpcTalk	m_Talk;
	CNpcTalkControl*	m_pTalkControl;
	CNpcDialogueSet*	m_pNpcDialogueSet;
	CComboBox			m_ComboType;
	CComboBox			m_ComboData;
	CListBox			m_listQuestStep;
	CListCtrl			m_listRandom;

public:
	void	SetTalkData ( CNpcDialogueSet* pNpcDialogueSet, CNpcTalkControl* pTalkControl );
	void	SetModifyData ( BOOL bModify, SNpcTalk* pTalk );

	SNPC_ITEM	GetNeedItem( DWORD dwItemNum ) { return m_Talk.m_sNeedItem[dwItemNum]; }
	void	SetNeedItem( DWORD dwSetItemNum, SNPC_ITEM sNeedItem ) { m_Talk.m_sNeedItem[dwSetItemNum] = sNeedItem; }

	int		GetActionType() { return m_Talk.m_nACTION; }
	int		GetActionNO()	{ return m_Talk.m_dwACTION_NO; }

	BOOL	DataDefault();
	BOOL	DataUpdate();
	BOOL	DataSave();

	void	InitComboType();
	void	InitComboData();

	void	SetWndView ( int nACTION, DWORD dwNO );

	void	ShowCondition();
	void	ShowQuestStart();
	void	ShowQuestProgress();
	void	ShowItem();
	void	ShowRandom();

	void	QuestStartInsert( int nINDEX, DWORD dwGUID );
	void	QuestStartAdd( int nINDEX, DWORD dwQUESTID );
	void	QuestStartDel( int nINDEX );
	

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnCbnSelchangeComboActionType();
	afx_msg void OnCbnSelchangeComboActionData();
	afx_msg void OnBnClickedButtonConditionEdit();
	afx_msg void OnBnClickedButtonConditionDelete();
	afx_msg void OnBnClickedButtonQuestInsert0();
	afx_msg void OnBnClickedButtonQuestInsert1();
	afx_msg void OnBnClickedButtonQuestInsert2();
	afx_msg void OnBnClickedButtonQuestInsert3();
	afx_msg void OnBnClickedButtonQuestInsert4();
	afx_msg void OnBnClickedButtonQuestAdd0();
	afx_msg void OnBnClickedButtonQuestAdd1();
	afx_msg void OnBnClickedButtonQuestAdd2();
	afx_msg void OnBnClickedButtonQuestAdd3();
	afx_msg void OnBnClickedButtonQuestAdd4();
	afx_msg void OnBnClickedButtonQuestDel0();
	afx_msg void OnBnClickedButtonQuestDel1();
	afx_msg void OnBnClickedButtonQuestDel2();
	afx_msg void OnBnClickedButtonQuestDel3();
	afx_msg void OnBnClickedButtonQuestDel4();
	afx_msg void OnBnClickedButtonQuestInsertProg();
	afx_msg void OnBnClickedButtonQuestAddProg();
	afx_msg void OnBnClickedButtonQuestDelProg();
	afx_msg void OnBnClickedButtonQuestStep();
	afx_msg void OnBnClickedButtonItemInsert();
	afx_msg void OnBnClickedButtonItemAdd();
	afx_msg void OnBnClickedButtonItemDel();
	afx_msg void OnBnClickedButtonRandomAdd();
	afx_msg void OnBnClickedButtonRandomDel();
	afx_msg void OnBnClickedButtonRandomReset();
	afx_msg void OnBnClickedButtonBuslist();
};

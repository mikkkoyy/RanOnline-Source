/*!
 * \file ModernQuestInfoProg.h
 *
 * \author Juver
 * \date November 2017
 *
 * 
 */
#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CBasicTextBox;
class CBasicScrollBarEx;
class CBasicTextButton;
class CItemSlot;

const DWORD UIMSG_INFO_PROG_BUTTON_GIVEUP = UIMSG_USER4;
const DWORD UIMSG_INFO_PROG_BUTTON_COMPLETE = UIMSG_USER5;

class	CModernQuestInfoProg : public CUIGroup
{
private:

private:
	enum
	{
		MODERN_QUEST_WINDOW_PROG_INFO_DESC_SCROLL = NO_ID + 1,
		MODERN_QUEST_WINDOW_PROG_INFO_DETAIL_SCROLL,
		MODERN_QUEST_WINDOW_PROG_INFO_STEP_SCROLL,
		MODERN_QUEST_WINDOW_PROG_INFO_QUEST_ITEM_SLOT,

		MODERN_QUEST_WINDOW_PROG_INFO_REWARD_SCROLL,
		MODERN_QUEST_WINDOW_PROG_INFO_REWARD_ITEM_SLOT,

		MODERN_QUEST_WINDOW_PROG_BUTTON_GIVEUP,
		MODERN_QUEST_WINDOW_PROG_BUTTON_COMPLETE,
	};


public:
	CModernQuestInfoProg ();
	virtual	~CModernQuestInfoProg ();

public:
	void	CreateSubControl ();

public:
	CBasicTextBox*		m_pTextTitle;

	CBasicTextBox*		m_pTextDescTitle;
	CBasicTextBox*		m_pTextDescInfo;
	CBasicScrollBarEx*	m_pScrollDesc;

	CBasicTextBox*		m_pTextDetailTitle;
	CBasicTextBox*		m_pTextDetailInfo;
	CBasicScrollBarEx*	m_pScrollDetail;

	CBasicTextBox*		m_pTextStepInfo;
	CBasicScrollBarEx*	m_pScrollStep;


	CBasicTextBox*		m_pTextQuestItemTitle;
	CItemSlot*			m_pItemSlotQuestItem;

	CBasicTextBox*		m_pTextRewardTitle;
	CBasicTextBox*		m_pTextRewardInfo;
	CBasicScrollBarEx*	m_pScrollReward;
	CItemSlot*			m_pItemSlotReward;

	CBasicTextButton*	m_pButtonGiveUp;
	CBasicTextButton*	m_pButtonComplete;

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

public:
	DWORD m_dwQuestID;
	DWORD	GetQuestID()	{ return m_dwQuestID; }
	void DataReset();
	void DataSet( DWORD dwQuestID );
	
};
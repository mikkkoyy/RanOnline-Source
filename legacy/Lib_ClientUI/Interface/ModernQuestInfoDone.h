/*!
 * \file ModernQuestInfoDone.h
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
class CItemSlot;

class	CModernQuestInfoDone : public CUIGroup
{
private:

private:
	enum
	{
		MODERN_QUEST_WINDOW_DONE_INFO_DESC_SCROLL = NO_ID + 1,
		MODERN_QUEST_WINDOW_DONE_INFO_DETAIL_SCROLL,
		MODERN_QUEST_WINDOW_DONE_INFO_STEP_SCROLL,
		MODERN_QUEST_WINDOW_DONE_INFO_REWARD_SCROLL,
		MODERN_QUEST_WINDOW_DONE_INFO_REWARD_ITEM_SLOT,
	};


public:
	CModernQuestInfoDone ();
	virtual	~CModernQuestInfoDone ();

public:
	void	CreateSubControl ();

public:
	CBasicTextBox*		m_pTextTitle;

	CBasicTextBox*		m_pTextResultTitle;
	CBasicTextBox*		m_pTextResultInfo;

	CBasicTextBox*		m_pTextDescTitle;
	CBasicTextBox*		m_pTextDescInfo;
	CBasicScrollBarEx*	m_pScrollDesc;

	CBasicTextBox*		m_pTextDetailTitle;
	CBasicTextBox*		m_pTextDetailInfo;
	CBasicScrollBarEx*	m_pScrollDetail;

	CBasicTextBox*		m_pTextStepInfo;
	CBasicScrollBarEx*	m_pScrollStep;

	CBasicTextBox*		m_pTextRewardTitle;
	CBasicTextBox*		m_pTextRewardInfo;
	CBasicScrollBarEx*	m_pScrollReward;
	CItemSlot*			m_pItemSlotReward;

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
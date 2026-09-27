#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "../Lib_Client/G-Logic/GLContrlServerMsg.h"
#include "../Lib_Client/G-Logic/GLItemMan.h"
#include "../Lib_Client/G-Logic/GlogicData.h"
#include "GLItemMan.h"

#include "LargeMapWindowCheckBox.h"

class CBasicScrollBarEx;
class CLargeMapWindowCheckBox;
class CLargeMapWindowMobDrop;
class CBasicTextBoxEx;
class CBasicLineBox;
class CItemImage;

class CLargeMapWindowMob : public CUIWindowEx
{
private:
static	const	int	nSTARTLINE;
static	const	int	nOUTOFRANGE;

protected:
	enum
	{
		nONE_VIEW_SLOT_DEFAULT = 10,
		nMAX_ONE_VIEW_SLOT = 10,
	};
	
	enum
	{
		MONSTER_LIST_SCROLL_BAR = NO_ID + 1,
		MONSTER_LIST_SCROLL_BAR_SPACE,
		CROW_SLOT0,
		CROW_SLOT1,
		CROW_SLOT_END = CROW_SLOT0 + MAXCROW,
		CROW_MAIN,ITEM_LIST_TEXTBOX,
		ITEM_LIST_SCROLLBAR,

		REWARD_SCROLL_BAR,
		REWARD_SCROLL_BAR_SPACE,
		REWARD_ITEM1,
		REWARD_ITEM2,
		REWARD_ITEM3,
		REWARD_ITEM4,
		REWARD_ITEM5,
		REWARD_ITEM6,
		REWARD_ITEM7,
		REWARD_ITEM8,
		REWARD_ITEM9,
		REWARD_ITEM10,
		REWARD_ITEM11,
		REWARD_ITEM12,
		REWARD_ITEM13,
		REWARD_ITEM14,
		REWARD_ITEM15,
		REWARD_ITEM16,
		REWARD_ITEM17,
		REWARD_ITEM18,
		REWARD_ITEM19,
		REWARD_ITEM20,
		REWARD_ITEM21,
		REWARD_ITEM22,
		REWARD_ITEM23,
		REWARD_ITEM24,
	};

	static const int REWARD_H_LINE_COUNT = 4;
	static const int REWARD_V_LINE_COUNT = 6;
	static const int REWARD_COUNT = REWARD_H_LINE_COUNT * REWARD_V_LINE_COUNT;

    enum
    {
        TITLE_HEAD  = 0,
        TITLE_BODY  = 1,
        TITLE_TAIL  = 2,
        TITLE_MAX,
    };

public:
	CLargeMapWindowMob ();
	virtual	~CLargeMapWindowMob ();
	void CreateSubControl();
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID cID, DWORD dwMsg );

protected:
    CUIControl*					m_pMobMainBG[TITLE_MAX];
    CUIControl*					m_pMobRewardBG[TITLE_MAX];

	CLargeMapWindowCheckBox*	m_pMobMain;
	CLargeMapWindowCheckBox*	m_pMob[MAXCROW];
	CUIControl*					m_pMobArrayDummy[nMAX_ONE_VIEW_SLOT];

	int							m_nONE_VIEW_SLOT;

	CUIControl*			m_pMonsterScrollBarSpace;
	CUIControl*			m_pRewardScrollBarSpace;

	CBasicScrollBarEx*			m_pScrollBar;
	CBasicScrollBarEx*			m_pRewardScrollBar;

	int							m_nCurPos;
	int							m_nIndex;

	typedef std::map<DWORD,SNATIVEID>	REWARDITEM_MAP;
	typedef REWARDITEM_MAP::iterator			REWARDITEM_MAP_ITER;
	REWARDITEM_MAP								m_RewardItemMap;
	std::set<DWORD>								m_UniqueRewardItemSet;

	CItemImage*					m_pRewardItems[REWARD_V_LINE_COUNT][REWARD_H_LINE_COUNT];

public:
	void LoadMob ( PCROWDATA pCrow );
	void ClearMobList();
	void SetMonsterInfo();
	void UpdateMonsterInfo();
	void UpdateMonsterList();

	CUIControl*	CreateControl ( const char* szControl );
	CLargeMapWindowCheckBox*	CreateMobSlot ( CString strKeyword, UIGUID ControlID );

	void SetCrowRender ( int nStartIndex, int nTotal );
	void ResetAllCrowRender ( int nTotal );

	SNATIVEID					m_sNativeID[MAXCROW];

	BOOL						IsChecked(int nIndex) { return m_pMob[nIndex]->IsChecked(); }

	int							GetTotalCrow() { return m_nIndex; }

	CItemImage*					CreateItemImage ( const char* szControl, UIGUID ControlID );

private:
	CBasicTextBoxEx*	m_pListText;
	CBasicScrollBarEx*	m_pListScrollBar;
	int		m_nSelectIndex;

	
	CBasicTextBox*			m_pMobDropText;

	CLargeMapWindowMobDrop* m_pLargeMapWindowMobDrop;	//--CLASS

	
	CBasicLineBox*			m_pBasicLineBox;

public:
	void					MobDropLoad ( SNATIVEID sMobID );
};
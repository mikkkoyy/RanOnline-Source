#pragma	once

#include "UIWindowEx.h"
#include "../Lib_Client/G-Logic/GLCrowData.h"
#include "BasicTextBox.h"

struct	GLCHARLOGIC;
class	CBasicTextBox;
class	CLargeMapWindowMob;

enum
{
	SLOT_MOB			 = 0,
	SLOT_MOB_LOW		 = 1,
	SLOT_MOB_QUEST		 = 2,
	SLOT_MOB_BOSS		 = 3,
	SLOT_MOB_TOWER		 = 4,
	SLOT_NPC_NORMAL		 = 5,
	SLOT_NPC_QUEST_NEW	 = 6,
	SLOT_NPC_LOCKER		 = 7,
	SLOT_NPC_TRADE		 = 8,
	SLOT_NPC_SAVE		 = 9,
	SLOT_NPC_BUS		 = 10,
	SLOT_NPC_QUEST_DONE  = 11,
	SLOT_NPC_MAILBOX	 = 12,
	SLOT_SIZE			 = 13,
};

enum
{
	TYPE_GATE			= 0,
	TYPE_NPC			= 1,

	TYPE_BUS			= 3,
	TYPE_LOCKER			= 4,
	TYPE_SAVE			= 5,
	TYPE_CLUB_LOCKER	= 6,
	TYPE_MAIL			= 7,
	TYPE_PARTY			= 8,
	
	TYPE_MOB			= 9,
};

class	CLargeMapWindowSlotRender : public CUIWindowEx
{
protected:
	enum 
	{ 
		MAP_WINDOW_MONSTER_ICON0 = NO_ID + 1, 
		MAP_WINDOW_MONSTER_ICON1, MAP_WINDOW_MONSTER_ICON2, MAP_WINDOW_MONSTER_ICON3,
		MAP_WINDOW_MONSTER_ICON4, MAP_WINDOW_MONSTER_ICON5, MAP_WINDOW_MONSTER_ICON6,
		MAP_WINDOW_MONSTER_ICON7, MAP_WINDOW_MONSTER_ICON8, MAP_WINDOW_MONSTER_ICON9,
		MAP_WINDOW_MONSTER_ICON10, MAP_WINDOW_MONSTER_ICON11, MAP_WINDOW_MONSTER_ICON12,

		ITEM_LIST_TEXTBOX,

		MAP_WINDOW_MONSTER_ICON_GATE,
	};

public:
	CLargeMapWindowSlotRender ();
	virtual	~CLargeMapWindowSlotRender ();

	void CreateSubControl();
	virtual void Update ( int x ,int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual void TranslateUIMessage ( UIGUID cID, DWORD dwMsg );

private:
	CUIControl*				m_pSlot[SLOT_SIZE];
	CUIControl*				m_pGate;
	CBasicTextBox*			m_pText;
	PCROWDATA				m_pCrowData;
	CString					m_strDesc;
	int						m_nType;
	SNATIVEID				m_sNativeID;
	bool					m_bDispName;
	const char*				m_szMapName;

	CLargeMapWindowMob*		m_pLargeMapWindowMob;

public:
	void					SetCrow( PCROWDATA pCrow, GLCHARLOGIC *pCHAR );
	void					UnLoadAll();
	void					SetType(int nType) { m_nType = nType; }
	void					SetID(SNATIVEID sID) { m_sNativeID = sID; }

	int						SetText(CString strText);
	int						GetType() { return m_nType; }

	SNATIVEID				GetID() { return m_sNativeID; }

	bool					IsMob()			{ return m_nType == TYPE_MOB;			}
	bool					IsGate()		{ return m_nType == TYPE_GATE;			}
	bool					IsNPC()			{ return m_nType == TYPE_NPC;			}
	bool					IsBus()			{ return m_nType == TYPE_BUS;			}
	bool					IsLocker()		{ return m_nType == TYPE_LOCKER;		}
	bool					IsSave()		{ return m_nType == TYPE_SAVE;			}
	bool					IsClubLocker()	{ return m_nType == TYPE_CLUB_LOCKER;	}
	bool					IsMail()		{ return m_nType == TYPE_MAIL;			}
	bool					IsParty()		{ return m_nType == TYPE_PARTY;			}
	
	bool					IsDisplayName() { return m_bDispName; }
	void					SetDispName(bool bDisp) { if (IsDisplayName()) m_pText->SetVisibleSingle(bDisp); }

	void					SetGate(const char * szMapName);
};
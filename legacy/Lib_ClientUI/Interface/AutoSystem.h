#pragma once

#include "../Lib_Engine/GUInterface/UIGroup.h"
class CBasicTextButton;
class CBasicLineBox;

class CAutoSystem : public CUIGroup
{
public:
	CAutoSystem(void);
	~CAutoSystem(void);

public:
	enum
	{
		AUTO_COMBAT_BUTTON = NO_ID + 1,
		AUTO_POT_LOOT_BUTTON,
	};

public:
	void	CreateSubControl();


public:
	virtual	void	TranslateUIMessage(UIGUID ControlID, DWORD dwMsg);
	virtual	void	Update(int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl);

private:
	CBasicLineBox*	m_pBackground;
	CBasicTextButton*	m_pAutoCombatButton;
	CBasicTextButton*	m_pAutoPotLootButton;
};

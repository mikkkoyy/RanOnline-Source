#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CBasicTextBox;
class CBasicLineBox;
class CD3DFontPar;
class CBasicTextButton;

class CPVPTyrannyPageBattle : public CUIGroup
{
private:
	enum
	{
		TYRANNY_PAGE_INFO_BATTLE_BUTTON_REGISTER = NO_ID + 1,
	};

public:
	CPVPTyrannyPageBattle ();
	virtual	~CPVPTyrannyPageBattle ();

public:
	void	CreateSubControl ();
	CBasicTextBox*	CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign );

public:
	CBasicTextBox*		m_pTextBattleTitleStatic;
	CBasicTextBox*		m_pTextBattleTimeStartStatic;
	CBasicTextBox*		m_pTextBattleTimeEndStatic;
	CBasicTextBox*		m_pTextBattleProgressStatic;
	CBasicTextBox*		m_pTextBattleStatusStatic;

	CBasicTextBox*		m_pTextBattleTimeStartTimer;
	CBasicTextBox*		m_pTextBattleTimeEndTimer;
	CBasicTextBox*		m_pTextBattleProgress;
	CBasicTextBox*		m_pTextBattleStatus;

	CBasicTextButton*	m_pButtonRegister;

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

	float	m_fUpdateTime;

	void	ShowInfo();
};
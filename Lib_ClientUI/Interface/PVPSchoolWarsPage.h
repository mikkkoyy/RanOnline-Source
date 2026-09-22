#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CBasicTextBox;
class CBasicLineBox;
class CD3DFontPar;

class CPVPSchoolWarsPageBattle;
class CPVPSchoolWarsPageHistory;

const DWORD UIMSG_SCHOOLWARS_PAGE_BUTTON_CLOSE = UIMSG_USER1;

class CPVPSchoolWarsPage : public CUIGroup
{
private:
	enum
	{
		SCHOOLWARS_PAGE_INFO_BATTLE = NO_ID + 1,
		SCHOOLWARS_PAGE_INFO_HISTORY,
	};

	
public:
	CPVPSchoolWarsPage ();
	virtual	~CPVPSchoolWarsPage ();

public:
	void	CreateSubControl ();
	CBasicTextBox*	CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign );
	CBasicLineBox*	CreateUILineBoxQuestList( char* szBaseControl );
	CBasicLineBox*	CreateUILineBoxWhiteNoBody( char* szBaseControl );
	CUIControl*		CreateUIControl( const char* szControl );

public:
	CBasicTextBox*		m_pTextMapName;

	CBasicTextBox*		m_pTextInfoTitle;
	CBasicTextBox*		m_pTextInfoGoal1;
	CBasicTextBox*		m_pTextInfoGoal2;
	CBasicTextBox*		m_pTextInfoMatch;
	CBasicTextBox*		m_pTextInfoMatch1;
	CBasicTextBox*		m_pTextInfoMatch2;
	CBasicTextBox*		m_pTextInfoPlayer;
	CBasicTextBox*		m_pTextInfoLevel;
	CBasicTextBox*		m_pTextInfoContri;
	CBasicTextBox*		m_pTextInfoMaxScore;

	CBasicTextBox*		m_pTextReward;

	CPVPSchoolWarsPageBattle*	m_pInfoBattle;
	CPVPSchoolWarsPageHistory*	m_pInfoHistory;

public:
	float	m_fUpdateTime;

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

	void	UpdateInfo();
};
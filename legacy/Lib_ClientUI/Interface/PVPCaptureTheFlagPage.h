#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CBasicTextBox;
class CBasicLineBox;
class CD3DFontPar;

class CPVPCaptureTheFlagPageBattle;
class CPVPCaptureTheFlagPageHistory;

const DWORD UIMSG_CAPTURE_THE_FLAG_PAGE_BUTTON_CLOSE = UIMSG_USER1;

class CPVPCaptureTheFlagPage : public CUIGroup
{
private:
	enum
	{
		CAPTURE_THE_FLAG_PAGE_INFO_BATTLE = NO_ID + 1,
		CAPTURE_THE_FLAG_PAGE_INFO_HISTORY,
	};

	
public:
	CPVPCaptureTheFlagPage ();
	virtual	~CPVPCaptureTheFlagPage ();

public:
	void	CreateSubControl ();
	CBasicTextBox*	CreateUITextBox( const char* szControl, CD3DFontPar* pFont, int nTextAlign );
	CBasicLineBox*	CreateUILineBoxQuestList( char* szBaseControl );
	CBasicLineBox*	CreateUILineBoxWhiteNoBody( char* szBaseControl );
	CUIControl*		CreateUIControl( const char* szControl );

public:
	CBasicTextBox*		m_pTextMapName;
	CBasicTextBox*		m_pTextInfoTarget;
	CBasicTextBox*		m_pTextInfoDetail;
	CBasicTextBox*		m_pTextInfoReward;

	CPVPCaptureTheFlagPageBattle*	m_pInfoBattle;
	CPVPCaptureTheFlagPageHistory*	m_pInfoHistory;

public:
	float	m_fUpdateTime;

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

	void	UpdateInfo();
};
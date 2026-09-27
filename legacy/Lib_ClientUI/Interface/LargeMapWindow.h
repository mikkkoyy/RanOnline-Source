#pragma	once

#include "UIWindowEx.h"
#include "GLCrowData.h"
#include "LargeMapWindowSetting.h"

struct GLCHARLOGIC;

class GLMapAxisInfo;
class CBasicTextBox;
class CLargeMapWindowMob;
class CLargeMapWindowImage;
class CLargeMapWindowRadio;
class CLargeMapWindowSetting;
class CLargeMapWindowInfo;
/*dmk14 map preview*/
class CBasicButtonText;
class CBasicComboBoxRollOver;
class RnButton;
enum
{
	MAX_RADIO_SETTINGS = 2,
};

class CLargeMapWindow : public CUIWindowEx
{
protected:
	enum
	{
		MAP_WINDOW_MAIN_SETTING_RADIO_BUTTON = ET_CONTROL_NEXT,
		MAP_WINDOW_MAIN_INFO_RADIO_BUTTON,
		MAP_WINDOW_MAIN_PARTY_RADIO_BUTTON,
		MAP_WINDOW_MAIN_SEARCH_RADIO_BUTTON,
		MAP_WINDOW_TAP_SETTINGS,
		MAP_WINDOW_BACK_BUTTON,
		MAP_WINDOW_MAP_BUTTON,
		MAP_WINDOW_NEXT_BUTTON,
		MAP_WINDOW_TAP_INFO,
		/*dmk14 map preview*/
		MAP_WINDOW_MAP_MYPOS_BTN,
		MAP_WINDOW_MAP_PRE_BTN,
		MAP_WINDOW_MAP_NEXT_BTN,
		MAP_WINDOW_MAP_BTN,
		MAP_WINDOW_MAP_ROLLOVER,
	};

public:
	CLargeMapWindow ();
	virtual	~CLargeMapWindow ();
	void CreateSubControl();
	
	virtual	void	Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void	TranslateUIMessage ( UIGUID cID, DWORD dwMsg );

	CString & GetMapName() { return m_strMapName; }
	void SetMapName(const CString & strMapName) { m_strMapName = strMapName; }

private:
	CString					m_strMapName;	// ∏  ¿Ã∏ß
	CLargeMapWindowInfo*	m_pLargeMapWindowInfo;

private:
	void						LoadMap( SNATIVEID sMapID );

protected:
	CUIControl*					m_pBackGround;
	CLargeMapWindowImage*		m_pImage;
	CLargeMapWindowRadio*		m_pRadio[MAX_RADIO_SETTINGS];
	CLargeMapWindowSetting*		m_pSetting;
	CLargeMapWindowMob*			m_pInfo;

	GLCHARLOGIC*				m_pCHAR;
	SNATIVEID					m_sActiveMapID;
	SNATIVEID					m_sSelectedMapID;

public:
	void						SetLargeMap( SNATIVEID sMapID, GLCHARLOGIC *pCHAR );
	void						SetPage(int nIndex);
	void						SetMapAxisInfo ( GLMapAxisInfo &sInfo, CString strMapName );
	void						VISIBLE_TARGET_POS( BOOL bVisible = FALSE );
	void						ClearMobList( SNATIVEID sMapID );
	void						LoadMob(PCROWDATA pCrow, D3DXVECTOR3 vPos, GLCHARLOGIC *pCHAR );
	void						LoadMob(PCROWDATA pCrow);

	BOOL						m_bSUBSETTING[MAX_SUB_SETTING];
	BOOL						m_bSUBMOTION;

	BOOL						m_bMOB[MAXCROW];
	int							m_nIndex;
	SNATIVEID					m_sNativeID[MAXCROW];

	
    RnButton*					m_pButtonMyPos;
    RnButton*					m_pButtonPrev;
    RnButton*					m_pButtonNext;
    RnButton*					m_pButtonMapList;

	/*dmk14 map preview*/
	CBasicComboBoxRollOver*		m_pMapListRollOver;
	bool						m_bLoadMap;
};
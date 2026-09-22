#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "GLCharDefine.h"
#include "PKRankData.h"

class CBasicTextBoxEx;
class CBasicLineBox;


class CPKRankNotificationInfo : public CUIGroup
{
	enum
	{
		RANK_INFO_ICON_SCHOOL = 3,
		RANK_INFO_ICON_CLASS = GLCI_NUM_7CLASS,
	};

public:
	CPKRankNotificationInfo();
	virtual ~CPKRankNotificationInfo();

public:
	void CreateSubControl();

public:
	virtual	void TranslateUIMessage( UIGUID cID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

public:
	CUIControl*		m_pClassIconKiller[RANK_INFO_ICON_CLASS];
	CUIControl*		m_pSchoolIconKiller[RANK_INFO_ICON_SCHOOL];

	CUIControl*		m_pClassIconKilled[RANK_INFO_ICON_CLASS];
	CUIControl*		m_pSchoolIconKilled[RANK_INFO_ICON_SCHOOL];

	CBasicTextBoxEx*	m_pNameKiller;
	CBasicTextBoxEx*	m_pNameKilled;

	CBasicLineBox*		m_pLineBox;
	CUIControl*			m_pKillIcon;


public:
	void SetData( SPK_HISTORY sHistory );

};
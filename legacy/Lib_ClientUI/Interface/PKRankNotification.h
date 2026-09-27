#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "PKRankData.h"

class CPKRankNotificationInfo;

class CPKRankNotification : public CUIGroup
{
	enum
	{
		//INBOX_LIST_TEXTBOX = NO_ID + 1,
	};

public:
	CPKRankNotification();
	virtual ~CPKRankNotification();

public:
	void CreateSubControl();

public:
	virtual	void TranslateUIMessage( UIGUID cID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

protected:
	CUIControl * CreateControl( char* szControl, const UIGUID& cID = NO_ID );

private:

public:
	CPKRankNotificationInfo*		m_pInfo[PKNOTIF_NUM];

};
#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "../Lib_Engine/GUInterface/UIMan.h"
#include "GLCrowData.h"
class CPartyFinderIcon : public CUIGroup
{
public:
	enum
	{
		MouseOpen = NO_ID + 1,
	};

public:
	CPartyFinderIcon();
	virtual ~CPartyFinderIcon();
public:
	void	CreateSubControl ();
public:
	virtual	void	TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void	Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	void	SetPartyFinderIcon();

public:
	CUIControl* m_pImage;
};
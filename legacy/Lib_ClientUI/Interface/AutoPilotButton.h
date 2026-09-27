#pragma once

#include "../Lib_Engine/GUInterface/UIGroup.h"
class CBasicTextButton;

class CAutoPilotButton : public CUIGroup
{
public:
	CAutoPilotButton(void);
	~CAutoPilotButton(void);

public:
	enum
	{
		AUTOPILOT_BUTTON = NO_ID + 1,
	};

public:
	void	CreateSubControl();


public:
	virtual	void	TranslateUIMessage(UIGUID ControlID, DWORD dwMsg);
	virtual	void	Update(int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl);

private:
	CBasicTextButton* m_pAutoPilotButton;
	LPDIRECT3DDEVICEQ	m_pd3dDevice;

};


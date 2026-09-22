#pragma once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CQBoxButtonIcon;
class CBasicMiniBarGauge;

class CQBoxButton : public CUIGroup
{
public:
	CQBoxButton(void);
	~CQBoxButton(void);

protected:
	enum
	{
		QBOX_ON_OFF_BUTTON = NO_ID + 1,
		QBOX_GAUGE,
	};

public:
	void	CreateSubControl ();
	bool	GetQBoxEnable() { return m_bQBoxEnable; }
	void	SetQBoxEnable( bool bQBoxEnable );
	void	SetGauge( CBasicMiniBarGauge* pGauge )	{	m_pQBoxGauge = pGauge;	}

public:
	virtual	void	TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void	Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

private:
	CQBoxButtonIcon*		m_pQBoxButton;
	CUIControl*				m_pAlarmBlink;
	CUIControl*				m_pQBoxLock;
	CBasicMiniBarGauge*		m_pQBoxGauge;

public:
	bool	m_bQBoxEnable;
	bool	m_bQBoxAlarm;
	float	m_fBLINK_TIME;
	float	m_fOrigTime;
	float	m_fRemainTime;

public:
	void	ResetType();
};

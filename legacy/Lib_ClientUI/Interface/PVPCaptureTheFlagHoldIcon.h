#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"
#include "GLPVPCaptureTheFlagDefine.h"

class CBasicMiniBarGauge;

class	CPVPCaptureTheFlagHoldIcon : public CUIGroup
{
	enum
	{
		TIMER_GAUGE = NO_ID + 1,
	};

private:
	CUIControl*		m_pTeamImage[CAPTURE_THE_FLAG_TEAM_SIZE];
	CBasicMiniBarGauge*		m_pProgressTimer;

public:
	CPVPCaptureTheFlagHoldIcon ();
	virtual	~CPVPCaptureTheFlagHoldIcon ();

public:
	void	CreateSubControl ();
	void	UpdateInfo();

	float	m_fUpdateTimer;


public:
	virtual void	Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

};
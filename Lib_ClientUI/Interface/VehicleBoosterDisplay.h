#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class	CBasicProgressBar;
struct  STARGETID;

class CVehicleBoosterDisplay : public CUIGroup
{
public:
	enum
	{
		VEHICLE_BOOSTER_PROGRESS = NO_ID + 1,
		VEHICLE_BOOSTER_BUTTON_OFF,
		VEHICLE_BOOSTER_BUTTON_ON
	};

	CVehicleBoosterDisplay ();
	virtual	~CVehicleBoosterDisplay ();

public:
	void	CreateSubControl ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

private:
	CBasicProgressBar*		m_pProgress;
	CUIControl*				m_pButtonOff;
	CUIControl*				m_pButtonOn;
	float					m_fCheckTimer;
};
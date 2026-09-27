#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CPKComboType;
class CPKComboDisplay : public CUIGroup
{
public:
	enum
	{
		DOUBLE_KILL = NO_ID + 1,
		TRIPLE_KILL,
		QUARD_KILL,
		MASTER_KILL,
		KILLING_SPREE,
		DOMINATING,
		MEGA_KILL,
		UNSTOPPABLE,
		WICKED_SICK,
		MONSTER_KILL,
		GODLIKE,
		HOLY_SHIT,
		OWNAGE,

		MAX_KILL = 13
	};

public:
	CPKComboDisplay ();
	virtual ~CPKComboDisplay();

public:
	void	CreateSubControl ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

public:
	bool	START ( UIGUID cID );
	bool	RESET ( UIGUID cID );
	bool	STOP ();

	void	ALLSTOP ();

public:
	bool	KEEP_START ( UIGUID cID );
	void	KEEP_STOP ();

private:
	CPKComboType*	m_pKILL_COUNT[MAX_KILL];
	CUIControl*		m_pPositionControl;	
};
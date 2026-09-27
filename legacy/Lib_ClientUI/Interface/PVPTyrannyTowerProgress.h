#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "GLPVPTyrannyData.h"

class	CBasicProgressBar;

class CPVPTyrannyTowerProgress : public CUIGroup
{
private:
	enum
	{
	};

public:
	CPVPTyrannyTowerProgress ();
	virtual	~CPVPTyrannyTowerProgress ();

public:
	void	CreateSubControl ();

	CBasicProgressBar*	m_pTowerStatus[TYRANNY_SCHOOL_SIZE];
	CUIControl*			m_pTowerOwner[TYRANNY_SCHOOL_SIZE];

public:
	void	UpdateInfo( WORD wOwner, float* fDamage );

}; 
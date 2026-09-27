#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CPVPTyrannyBattleNotice : public CUIGroup
{
private:
	enum
	{
		
	};


public:
	CPVPTyrannyBattleNotice ();
	virtual	~CPVPTyrannyBattleNotice ();

public:
	void	CreateSubControlStartImage ();
	void	CreateSubControlEndImage ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

	CUIControl*		m_pImageBG;
	CUIControl*		m_pImage;
	

	float			m_fTimer;
};
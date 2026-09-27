#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class	CBasicProgressBar;

class	CSimpleHP : public CUIGroup
{
public:
	CSimpleHP ();
	virtual	~CSimpleHP ();

public:
	void	SetHP ( WORD wNOW, WORD wMAX );
	void	CreateSubControl ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

private:
	CBasicProgressBar*	m_pHP;
};
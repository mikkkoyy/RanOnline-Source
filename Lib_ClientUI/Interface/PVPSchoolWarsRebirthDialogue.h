#pragma	once

#include "./UIWindow.h"

class	CBasicTextBox;
class	CBasicProgressBar;

class CPVPSchoolWarsRebirthDialogue : public CUIWindow
{
private:
	enum
	{
	};	

public:
	CPVPSchoolWarsRebirthDialogue ();
	virtual	~CPVPSchoolWarsRebirthDialogue ();

public:
	void	CreateSubControl ();
	
public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

public:
	CBasicTextBox*	m_pTextDesc;
	CBasicProgressBar*	m_pTimer;

public:
	__time64_t	timeStart;
};
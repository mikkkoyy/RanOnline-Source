/*!
 * \file CompetitionNotifyButton.h
 *
 * \author Juver
 * \date June 2017
 *
 * 
 */

#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CCompetitionNotifyButtonAlarm;

class	CCompetitionNotifyButton : public CUIGroup
{
private:
	enum
	{
		NOTIFYCOMPETITION_BUTTON_IMAGE = NO_ID + 1,
		NOTIFYCOMPETITION_BUTTON_ALARM,
	};

public:
	CCompetitionNotifyButton ();
	virtual	~CCompetitionNotifyButton ();

public:
	void	CreateSubControl ();

public:
	CUIControl*							m_pButtonImage;
	CUIControl*							m_pButtonBlink;
	CUIControl*							m_pButtonLock;
	CCompetitionNotifyButtonAlarm*		m_pButtonAlarm;
	

public:
	virtual	void	Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void	TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

private:
	float	m_fBLINK_TIME;
	BOOL	m_bAlarm;
	BOOL	m_bLocked;

public:
	void	SetButtonAlarm( BOOL bSet );
	void	SetButtonLock( BOOL bSet );
};
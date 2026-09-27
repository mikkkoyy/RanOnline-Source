#pragma once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CBasicTextBox;

class CGameStats : public CUIGroup
{
protected:
	
public:
	CGameStats();
	virtual ~CGameStats(void);

public:
	void	CreateSubControl ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

private:
	float			m_fPing;
	float			m_fFPS;
	float			m_fPingBack;
	float			m_fFPSBack;
    CBasicTextBox*	m_pPingText;
	CBasicTextBox*	m_pFpsText;

	void UpdateInfo();

public:
	void UpdatePing( float fPing ) { m_fPing = fPing; }
	void UpdateFPS( float fFPS ) { m_fFPS = fFPS; }
};
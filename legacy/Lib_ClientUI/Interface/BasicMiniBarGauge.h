#pragma	once

#include "UIGroup.h"

class	CBasicMiniBarGauge : public CUIGroup
{
public:
private:
	static	const	float	fZERO_PERCENT;
	static	const	float	fHUNDRED_PERCENT;
	static	const	float	fSIXTY_PERCENT;
	static	const	float	fTHIRTY_PERCENT;

private:
	int		m_nType;
	float	m_fPercent;

	CUIControl*	m_pBackGround;
	CUIControl*	m_pOverImageRed;
	CUIControl*	m_pOverImageYellow;
	CUIControl*	m_pOverImageGreen;
	CUIControl*	m_pOverImageRedDummy;
	CUIControl*	m_pOverImageYellowDummy;
	CUIControl*	m_pOverImageGreenDummy;

public:
	CBasicMiniBarGauge ();
	virtual	~CBasicMiniBarGauge ();

public:	
	void	CreateSubControl();

public:
	void	SetAlignFlag ( WORD wFlag );

public:
	virtual void	SetGlobalPos ( const UIRECT& rcPos );
	virtual void	SetGlobalPos ( const D3DXVECTOR2& vPos );
	virtual	void	AlignSubControl ( const UIRECT& rcParentOldPos, const UIRECT& rcParentNewPos );

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );		

protected:
	void	UpdateProgress ();

public:
	void	SetPercent ( float fPercent );
	float	GetPercent ( void );
};
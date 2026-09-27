#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class CSlotCountMsg : public CUIGroup
{
public:
	CSlotCountMsg ();
	virtual	~CSlotCountMsg ();

public:
	void CreateSubControl();
	CUIControl*	CreateControl( CONST TCHAR* szControl );
	void SetCount( WORD wMin, WORD wMax, WORD wWin, bool bRoll );

public:
	virtual void Update( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

private:
	CUIControl * m_pHUN[10];
	CUIControl * m_pTEN[10];
	CUIControl * m_pONE[10];

	WORD m_wMin;
	WORD m_wMax;
	WORD m_wWin;
	bool m_bRoll;
	bool m_bStandby;

	float m_fElapsedTime;
};
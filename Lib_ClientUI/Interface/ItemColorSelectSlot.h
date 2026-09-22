/*!
 * \file ItemColorSelectSlot.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */

#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class	CItemColorSelectSlot : public CUIGroup
{
private:
	enum
	{
	};


public:
	CItemColorSelectSlot ();
	virtual	~CItemColorSelectSlot ();

public:
	void	CreateSubControl( BOOL bPrimaryColor );

public:
	BOOL		m_bPrimary;
	CUIControl*	m_pColor;
	CUIControl*	m_pSelect;

	void SetSelect( BOOL bSelect );

	virtual	void	Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

};
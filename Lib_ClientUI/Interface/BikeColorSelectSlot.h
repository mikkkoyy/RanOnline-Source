/*!
 * \file BikeColorSelectSlot.h
 *
 * \author Juver
 * \date November 2017
 *
 * 
 */

#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "GLItemVehicle.h"

class	CBasicTextButton;

class	CBikeColorSelectSlot : public CUIGroup
{
private:
	enum
	{
		//MARK_IMAGE_UP = NO_ID + 1,
		//MARK_IMAGE_DOWN,
	};


public:
	CBikeColorSelectSlot ();
	virtual	~CBikeColorSelectSlot ();

public:
	void	CreateSubControl( BIKE_COLOR_SLOT_PART emSlot );

public:
	BIKE_COLOR_SLOT_PART	m_emSlot; 
	CUIControl*	m_pColor;
	CUIControl*	m_pSelect;

	void SetSelect( BOOL bSelect );

	virtual	void	Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

};
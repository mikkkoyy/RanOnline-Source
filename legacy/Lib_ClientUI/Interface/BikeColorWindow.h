/*!
 * \file BikeColorWindow.h
 *
 * \author Juver
 * \date November 2017
 *
 * 
 */

#pragma once

#include "UIWindowEx.h"

class CBikeColorSelect;

class CBikeColorWindow : public CUIWindowEx
{
	enum
	{
		BIKE_COLOR_SELECT = ET_CONTROL_NEXT,
		
	};

public:
	CBikeColorWindow(void);
	virtual ~CBikeColorWindow(void);

public:
	void CreateSubControl();

public:
	CBikeColorSelect*	m_pSelect;



public:
	virtual	void TranslateUIMessage( UIGUID cID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

};
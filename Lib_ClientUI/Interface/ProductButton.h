/*!
 * \file ProductButton.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */

#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

class	CProductButton : public CUIGroup
{
private:
	enum
	{
		PRODUCT_BUTTON_IMAGE = NO_ID + 1,
	};

public:
	CProductButton ();
	virtual	~CProductButton ();

public:
	void	CreateSubControl ();

public:
	CUIControl*			m_pButtonImage;

public:
	virtual	void	Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void	TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
};
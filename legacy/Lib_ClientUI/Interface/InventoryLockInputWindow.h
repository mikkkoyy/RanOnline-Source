/*!
 * \file InventoryLockInputWindow.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */

#pragma once

#include "UIWindowEx.h"

class CBasicTextBox;
class CUIEditBoxMan;
class CBasicTextButton;


class CInventoryLockInputWindow : public CUIWindowEx
{
	enum
	{
		INVENTORY_LOCK_INPUT_WINDOW_EDIT_MAN = ET_CONTROL_NEXT,
		INVENTORY_LOCK_INPUT_WINDOW_EDIT_PIN,
		INVENTORY_LOCK_INPUT_WINDOW_BUTTON_OK,
		INVENTORY_LOCK_INPUT_WINDOW_BUTTON_CANCEL,
		
	};

public:
	static int nLIMIT_PIN;

public:
	CInventoryLockInputWindow(void);
	virtual ~CInventoryLockInputWindow(void);

public:
	void CreateSubControl();

public:
	CBasicTextBox*		m_pInfoDesc;
	CBasicTextBox*		m_pInfoPin;
	CUIEditBoxMan*		m_pEditBoxMan;
	CBasicTextButton*	m_pButtonClose;
	CBasicTextButton*	m_pButtonSave;

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage( UIGUID cID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

	void	CheckInfo();

};
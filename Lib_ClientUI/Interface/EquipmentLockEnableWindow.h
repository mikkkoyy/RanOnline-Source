/*!
 * \file EquipmentLockEnableWindow.h
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


class CEquipmentLockEnableWindow : public CUIWindowEx
{
	enum
	{
		EQUIPMENT_LOCK_ENABLE_WINDOW_EDIT_MAN = ET_CONTROL_NEXT,
		EQUIPMENT_LOCK_ENABLE_WINDOW_EDIT_PIN,
		EQUIPMENT_LOCK_ENABLE_WINDOW_EDIT_PIN2,
		EQUIPMENT_LOCK_ENABLE_WINDOW_EDIT_AGREE,

		EQUIPMENT_LOCK_ENABLE_WINDOW_BUTTON_OK,
		EQUIPMENT_LOCK_ENABLE_WINDOW_BUTTON_CANCEL,
		
	};

public:
	static int nLIMIT_PIN;
	static int nLIMIT_AGREE;

public:
	CEquipmentLockEnableWindow(void);
	virtual ~CEquipmentLockEnableWindow(void);

public:
	void CreateSubControl();

public:
	CBasicTextBox*		m_pInfoDesc;
	CBasicTextBox*		m_pInfoPin;
	CBasicTextBox*		m_pInfoPin2;
	CBasicTextBox*		m_pInfoAgree;

	CUIEditBoxMan*		m_pEditBoxMan;

	CBasicTextButton*	m_pButtonClose;
	CBasicTextButton*	m_pButtonSave;

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage( UIGUID cID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

	void	CheckInfo();

};
/*!
 * \file StorageLockEnableWindow.h
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


class CStorageLockEnableWindow : public CUIWindowEx
{
	enum
	{
		STORAGE_LOCK_ENABLE_WINDOW_EDIT_MAN = ET_CONTROL_NEXT,
		STORAGE_LOCK_ENABLE_WINDOW_EDIT_PIN,
		STORAGE_LOCK_ENABLE_WINDOW_EDIT_PIN2,
		STORAGE_LOCK_ENABLE_WINDOW_EDIT_AGREE,

		STORAGE_LOCK_ENABLE_WINDOW_BUTTON_OK,
		STORAGE_LOCK_ENABLE_WINDOW_BUTTON_CANCEL,
		
	};

public:
	static int nLIMIT_PIN;
	static int nLIMIT_AGREE;

public:
	CStorageLockEnableWindow(void);
	virtual ~CStorageLockEnableWindow(void);

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
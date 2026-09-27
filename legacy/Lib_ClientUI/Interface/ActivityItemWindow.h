/*!
 * \file ActivityItemWindow.h
 *
 * \author Juver
 * \date November 2017
 *
 * 
 */

#pragma once

#include "UIWindowEx.h"

class CBasicTextBox;
class CBasicTextButton;
class CActivityItemList;

class CActivityItemWindow : public CUIWindowEx
{
	enum
	{
		ACTIVITY_ITEM_WINDOW_BUTTON_CLOSE = ET_CONTROL_NEXT,
		ACTIVITY_ITEM_WINDOW_ITEM_LIST,
	};

public:
	CActivityItemWindow(void);
	virtual ~CActivityItemWindow(void);

public:
	void CreateSubControl();

public:
	CBasicTextBox*		m_pTextDesc;
	CBasicTextButton*	m_pButtonClose;
	CActivityItemList*	m_pItemList;

public:
	virtual	void TranslateUIMessage( UIGUID cID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

};
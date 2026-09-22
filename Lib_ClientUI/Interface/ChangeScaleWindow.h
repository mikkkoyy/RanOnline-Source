/*!
 * \file ChangeScaleWindow.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */

#pragma once

#include "UIWindowEx.h"

class CBasicProgressBar;
class CBasicButton;
class CBasicTextButton;

class CChangeScaleWindow : public CUIWindowEx
{
	enum
	{
		CHANGE_SCALE_WINDOW_BUTTON_SAVE = ET_CONTROL_NEXT,
		CHANGE_SCALE_WINDOW_BUTTON_CANCEL,
		CHANGE_SCALE_WINDOW_BUTTON_LEFT,
		CHANGE_SCALE_WINDOW_BUTTON_RIGHT,
	};

public:
	CChangeScaleWindow(void);
	virtual ~CChangeScaleWindow(void);

public:
	void CreateSubControl();

private:
	CBasicProgressBar*	m_pCharacterScaleBar;
	CBasicButton*		m_pCharacterScaleButtonLeft;
	CBasicButton*		m_pCharacterScaleButtonRight;
	CBasicTextButton*	m_pCharacterScaleButtonClose;
	CBasicTextButton*	m_pCharacterScaleButtonSave;

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage( UIGUID cID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

	void	CharacterScaleIncrease();
	void	CharacterScaleDecrease();

};
/*!
 * \file ItemColorWindow.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */

#pragma once

#include "UIWindowEx.h"

class CItemColorSelectSlot;
class CBasicTextBox;
class CBasicProgressBar;
class CUIEditBox;
class CBasicTextButton;

class CItemColorWindow : public CUIWindowEx
{
	static const int nLIMIT_NUMBER_CHAR;
	static const int nMAX_COLOR;
	static const float fRGB_RATIO;

	enum
	{
		ITEM_COLOR_SLOT_1 = ET_CONTROL_NEXT,
		ITEM_COLOR_SLOT_2,
		ITEM_COLOR_SELECT_R_PROGRESS,
		ITEM_COLOR_SELECT_G_PROGRESS,
		ITEM_COLOR_SELECT_B_PROGRESS,
		ITEM_COLOR_SELECT_R_EDITBOX,
		ITEM_COLOR_SELECT_G_EDITBOX,
		ITEM_COLOR_SELECT_B_EDITBOX,
		ITEM_COLOR_SELECT_CLOSE,
		ITEM_COLOR_SELECT_SAVE,
		
	};

public:
	CItemColorWindow(void);
	virtual ~CItemColorWindow(void);

public:
	void CreateSubControl();

public:
	CItemColorSelectSlot*	m_pColor1;
	CItemColorSelectSlot*	m_pColor2;
	CBasicTextBox*			m_pText1;
	CBasicTextBox*			m_pText2;

	CBasicProgressBar*	m_pRProgressBar;
	CBasicProgressBar*	m_pGProgressBar;
	CBasicProgressBar*	m_pBProgressBar;

	CUIEditBox*		m_pREditBox;
	CUIEditBox*		m_pGEditBox;
	CUIEditBox*		m_pBEditBox;

	CBasicTextButton*	m_pButtonSave;
	CBasicTextButton*	m_pButtonClose;

	BOOL	m_bPrimary;

	INT m_nR;
	INT m_nG;
	INT m_nB;

	INT m_nRBack;
	INT m_nGBack;
	INT m_nBBack;

	INT m_nRegR;
	INT m_nRegG;
	INT m_nRegB;

public:
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage( UIGUID cID, DWORD dwMsg );
	virtual	void SetVisibleSingle ( BOOL bVisible );

public:
	void SelectSlot( BOOL bPrimary );
	void ClearEditBox();
	void RegulateRGB();

	BOOL UpdateProgressBar( CBasicProgressBar * pBar, int x );
	void UpdateProgressBarToEditBox( CBasicProgressBar * pBar, CUIEditBox * pEditBox, INT & nColorVal );
	BOOL UpdateEditBoxToProgressBar( CUIEditBox * pEditBox, CBasicProgressBar * pBar, INT & nColorVal );

	void TranslateProgressBarMsg( CBasicProgressBar * pBar, DWORD dwMsg );
	void TranslateEditBoxMsg( CUIEditBox * pEditBox, DWORD dwMsg );
};
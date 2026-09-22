/*!
 * \file BikeColorSelect.h
 *
 * \author Juver
 * \date November 2017
 *
 * 
 */

#pragma	once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "GLItemVehicle.h"

class CBasicTextButton;
class CUIEditBox;
class CBasicProgressBar;
class CBikeColorSelectSlot;

const DWORD UIMSG_BIKE_COLOR_SELECT_CLOSE = UIMSG_USER1;
const DWORD UIMSG_BIKE_COLOR_SELECT_SAVE = UIMSG_USER2;

class	CBikeColorSelect : public CUIGroup
{
	static const int nLIMIT_NUMBER_CHAR;
	static const int nMAX_COLOR;
	static const float fRGB_RATIO;

private:
	enum
	{
		BIKE_COLOR_SELECT_SLOT_A1 = NO_ID + 1,
		BIKE_COLOR_SELECT_SLOT_A2,
		BIKE_COLOR_SELECT_SLOT_B1,
		BIKE_COLOR_SELECT_SLOT_B2,
		BIKE_COLOR_SELECT_SLOT_C1,
		BIKE_COLOR_SELECT_SLOT_C2,
		BIKE_COLOR_SELECT_SLOT_D1,
		BIKE_COLOR_SELECT_SLOT_D2,
		BIKE_COLOR_SELECT_SLOT_E1,
		BIKE_COLOR_SELECT_SLOT_E2,
		BIKE_COLOR_SELECT_SLOT_F1,
		BIKE_COLOR_SELECT_SLOT_F2,

		BIKE_COLOR_SELECT_R_PROGRESS,
		BIKE_COLOR_SELECT_G_PROGRESS,
		BIKE_COLOR_SELECT_B_PROGRESS,

		BIKE_COLOR_SELECT_R_EDITBOX,
		BIKE_COLOR_SELECT_G_EDITBOX,
		BIKE_COLOR_SELECT_B_EDITBOX,

		BIKE_COLOR_SELECT_CLOSE,
		BIKE_COLOR_SELECT_SAVE,
	};


public:
	CBikeColorSelect ();
	virtual	~CBikeColorSelect ();

public:
	void	CreateSubControl();

public:
	CBikeColorSelectSlot*		m_pColorPart[BIKE_COLOR_SLOT_PART_SIZE];

	CBasicProgressBar*	m_pRProgressBar;
	CBasicProgressBar*	m_pGProgressBar;
	CBasicProgressBar*	m_pBProgressBar;

	CUIEditBox*		m_pREditBox;
	CUIEditBox*		m_pGEditBox;
	CUIEditBox*		m_pBEditBox;

	CBasicTextButton*	m_pButtonSave;
	CBasicTextButton*	m_pButtonClose;

public:
	BIKE_COLOR_SLOT_PART m_emSelected;

	INT m_nR;
	INT m_nG;
	INT m_nB;

	INT m_nRBack;
	INT m_nGBack;
	INT m_nBBack;

	INT m_nRegR;
	INT m_nRegG;
	INT m_nRegB;

	int m_nSelected;

public:
	virtual	void	Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void	TranslateUIMessage ( UIGUID cID, DWORD dwMsg );

public:
	BOOL UpdateProgressBar( CBasicProgressBar * pBar, int x );
	void UpdateProgressBarToEditBox( CBasicProgressBar * pBar, CUIEditBox * pEditBox, INT & nColorVal );
	BOOL UpdateEditBoxToProgressBar( CUIEditBox * pEditBox, CBasicProgressBar * pBar, INT & nColorVal );

	void TranslateProgressBarMsg( CBasicProgressBar * pBar, DWORD dwMsg );
	void TranslateEditBoxMsg( CUIEditBox * pEditBox, DWORD dwMsg );

	void ClearEditBox();
	void RegulateRGB();

	void SelectSlot( int nSlot );

};
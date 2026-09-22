/*!
 * \file InfoBoxContents.h
 *
 * \author Juver
 * \date August 2017
 *
 * 

 */
#pragma once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "GLItem.h"
#include "GLItemBox.h"

class CBasicButton;
class CBasicTextBox;
class CBasicTextButton;
class CItemImage;

const DWORD UIMSG_INFO_BOX_CONTENTS_MOUSE_LBDUP	= UIMSG_USER1;

class	CInfoBoxContents : public CUIGroup
{
	enum
	{
		INFOBOX_CONTENTS_BUTTON_CLOSE = NO_ID + 1,
		INFOBOX_CONTENTS_ITEM_MAIN,
		INFOBOX_CONTENTS_ITEM_BOX_0,
		INFOBOX_CONTENTS_ITEM_BOX_1,
		INFOBOX_CONTENTS_ITEM_BOX_2,
		INFOBOX_CONTENTS_ITEM_BOX_3,
		INFOBOX_CONTENTS_ITEM_BOX_4,
		INFOBOX_CONTENTS_ITEM_BOX_5,
		INFOBOX_CONTENTS_ITEM_BOX_6,
		INFOBOX_CONTENTS_ITEM_BOX_7,
		INFOBOX_CONTENTS_ITEM_BOX_8,
		INFOBOX_CONTENTS_ITEM_BOX_9,
		INFOBOX_CONTENTS_ITEM_BOX_10,
		INFOBOX_CONTENTS_ITEM_BOX_11,
		INFOBOX_CONTENTS_ITEM_BOX_12,
		INFOBOX_CONTENTS_ITEM_BOX_13,
		INFOBOX_CONTENTS_ITEM_BOX_14,
		INFOBOX_CONTENTS_ITEM_BOX_15,
		INFOBOX_CONTENTS_ITEM_BOX_16,
		INFOBOX_CONTENTS_ITEM_BOX_17,
		INFOBOX_CONTENTS_ITEM_BOX_18,
		INFOBOX_CONTENTS_ITEM_BOX_19,
		INFOBOX_CONTENTS_BUTTON_SELECT,
	};

public:
	CInfoBoxContents ();
	virtual	~CInfoBoxContents ();

public:
	void	CreateSubControl ();


public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

private:
	CBasicButton*	m_pCloseButton;
	CBasicTextBox*	m_pTextName;	
	CBasicTextBox*	m_pTextDesc;
	CItemImage*		m_pItemImageMain;
	CItemImage*		m_pItemImageBox[ITEM::SBOX::ITEM_SIZE];
	CUIControl*		m_pMouseOver;
	CBasicTextButton*	m_pButtonSelect;

	//
	CUIControl*		m_pSelectedImageBox[ITEM::SBOX::ITEM_SIZE];
	BOOL			m_bSelectIndex[ITEM::SBOX::ITEM_SIZE];
	int				m_nMaxSelect;
	int				m_nSelectCount;

	WORD			m_wInvenPosX;
	WORD			m_wInvenPosY;

private:
	SNATIVEID		m_sItemID;
	SNATIVEID		m_sItemIDBox[ITEM::SBOX::ITEM_SIZE];
	BOOL			m_bSpecialBox;

public:
	void	DataReset();
	void	DataShow( SITEM* pItem, BOOL bBoxSelect, WORD wPosX, WORD wPosY );

	//
	void	ResetSelected();
	BOOL	GetSelectedIndex( int i )	{ return m_bSelectIndex[i]; }

public:
	void	CheckMouseState();

private:
	BOOL		m_bCHECK_MOUSE_STATE;
	BOOL		m_bFirstGap;
	D3DXVECTOR2	m_vGap;
	int			m_nPosX;
	int			m_nPosY;
	float		m_fUpdateTime;
};
/*!
 * \file ActivityItemListSlot.h
 *
 * \author Juver
 * \date November 2017
 *
 * 
 */
#pragma once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "GLDefine.h"

class CBasicTextBox;
class CItemImage;

class	CActivityItemListSlot : public CUIGroup
{
private:
	enum
	{
		ACTIVITY_ITEM_LIST_SLOT_ITEM_IMAGE = NO_ID + 1,
	};

public:
	CActivityItemListSlot ();
	virtual	~CActivityItemListSlot ();

public:
	void	CreateSubControl ();

public:
	CItemImage*			m_pItemImage;
	CBasicTextBox*		m_pTextName;
	CBasicTextBox*		m_pTextPrice;
	CBasicTextBox*		m_pTextPoint;

	SNATIVEID			m_sidItem;

public:
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

public:
	void	DataReset();
	void	DataSet( SNATIVEID sidItem );
};
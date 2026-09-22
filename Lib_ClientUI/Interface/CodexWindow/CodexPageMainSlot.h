/*!
 * \file CodexPageMainSlot.h
 *
 * \author Juver
 * \date October 2017
 *
 * 
 */
#pragma once

#include "../Lib_Engine/GUInterface/UIGroup.h"

#include "GLDefine.h"
#define MAX_CODEX_ITEM	5
class	CBasicTextBox;
class	CItemImage;
class	CBasicLineBox;

class	CCodexPageMainSlot : public CUIGroup
{
private:
	enum
	{
		ITEM_SLOT_IMAGE0 = NO_ID + 1,
		ITEM_SLOT_IMAGE1,
		ITEM_SLOT_IMAGE2,
		ITEM_SLOT_IMAGE3,
		ITEM_SLOT_IMAGE4,
	};

public:
	CCodexPageMainSlot ();
	virtual	~CCodexPageMainSlot ();

public:
	void	CreateSubControl ();

public:
	CUIControl*		m_pStatusComplete;
	CUIControl*		m_pProgressComplete;

	CUIControl*		m_pImageBadge;
	CUIControl*		m_pImagePoint;

	CBasicTextBox*	m_pTextComplete;
	CBasicTextBox*	m_pTextTitle;
	CBasicTextBox*	m_pTextDesc;
	CBasicTextBox*	m_pTextProgress;

	CBasicTextBox*	m_pTextBadge;
	CBasicTextBox*	m_pTextPoint;
	
	CBasicTextBox*	m_pTextRegistered[MAX_CODEX_ITEM];

	float			m_fUpdateTime;
	DWORD			m_dwCodexID;
	CItemImage*			m_pItemImage0;
	CItemImage*			m_pItemImage1;
	CItemImage*			m_pItemImage2;
	CItemImage*			m_pItemImage3;
	CItemImage*			m_pItemImage4;
	
	SNATIVEID			m_sidItem0;
	SNATIVEID			m_sidItem1;
	SNATIVEID			m_sidItem2;
	SNATIVEID			m_sidItem3;
	SNATIVEID			m_sidItem4;

	WORD				m_wQuantity0;
	WORD				m_wQuantity1;
	WORD				m_wQuantity2;
	WORD				m_wQuantity3;
	WORD				m_wQuantity4;
	
	BOOL				bItem0;
	BOOL				bItem1;
	BOOL				bItem2;
	BOOL				bItem3;
	BOOL				bItem4;
	BOOL				bItem5;
	
	WORD				m_wGradeItem0;
	WORD				m_wGradeItem1;
	WORD				m_wGradeItem2;
	WORD				m_wGradeItem3;
	WORD				m_wGradeItem4;
	
	CBasicLineBox*		m_pItemImageRequire[MAX_CODEX_ITEM];
	CBasicTextBox*		m_pImageAvailable[MAX_CODEX_ITEM];

	
	//select highlight
	CUIControl*		m_pSelectImage;

protected:
	CItemImage*		CreateItemImage0 ();

public:
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );
	virtual	void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );

	void	ResetData();
	void	SetData( DWORD dwID );
	void	UpdateInfo();

	void	SetOverVisible();
	void	SetOverVisibleOFF();
};
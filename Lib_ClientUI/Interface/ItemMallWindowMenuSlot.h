#pragma	once

#include "UIGroup.h"
#include "GLCharData.h"

//	NOTE
//		??? ?? ???
const DWORD UIMSG_MOUSEIN_BUTTON = UIMSG_USER1;

class	CItemImage;
class	CBasicButton;
class	CUIControl;
class	CBasicTextBox;
class	CBasicLineBox;
class	CBasicLineBoxEx;
class	CBasicTextButton;

class	CItemMallWindowMenuSlot : public CUIGroup
{
protected:
	enum
	{
		ITEM_BUTTON_BUY = NO_ID + 1,
		ITEM_SLOT_IMAGE,
	};

public:
	CItemMallWindowMenuSlot ();
	virtual	~CItemMallWindowMenuSlot ();

public:
	void	CreateSubControl ();

public:
	virtual void Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg );

protected:
	CUIControl*		CreateControl ( char* szControl );
	CItemImage*		CreateItemImage ();
	CBasicButton*	CreateFlipButton ( char* szButton, char* szButtonFlip, char* szMouseOver, UIGUID ControlID );
	CBasicTextBox*	CreateStaticControl ( char* szControlKeyword, CD3DFontPar* pFont, D3DCOLOR dwColor, int nAlign );

private:
	CItemImage*			m_pItemImage;
	CBasicTextBox*		m_pTextBox;
	CBasicTextBox*		m_pTextBoxQty;
	CBasicTextBox*		m_pTextBoxDesc;
	CBasicTextButton*	m_pBuyButton;
	CUIControl*			m_pImage[2];
	CString				m_strName;
	CString				m_strDesc;
	CString				m_strStocks;
	CString				m_strPrice;

private:
	ITEMMALLDATA		m_sItemMallData;

public:
	void				SetNativeItemID ( SNATIVEID sNativeID )			{ m_sItemMallData.sNativeID = sNativeID; }
	SNATIVEID			GetNativeItemID ()								{ return m_sItemMallData.sNativeID; }

	ITEMMALLDATA		GetItemMallData()								{ return m_sItemMallData; }

	void				SetItem( ITEMMALLDATA sItemData );
};
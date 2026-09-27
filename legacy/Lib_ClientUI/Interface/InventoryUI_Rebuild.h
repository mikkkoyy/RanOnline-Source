#pragma	once

#include "UIWindowEx.h"

class CInventoryUI_RebuildInven;
class	CBasicTextButton;

//--------------------------------------------------------------------
// InventoryUI_Rebuild : √÷¡ÿ«ı
//--------------------------------------------------------------------
class CInventoryUI_Rebuild : public CUIWindowEx	// ITEMREBUILD_MARK
{
protected:
	enum
	{
		REBUILDINVENTORY_MONEY_BUTTON = ET_CONTROL_NEXT,
		REBUILDINVENTORY_POINT_BUTTON,
		REBUILDINVENTORY_INVEN_BUTTON,
		REBUILDINVENTORY_PLUS_MONEY_BUTTON,
		REBUILDINVENTORY_MINUS_MONEY_BUTTON,
		REBUILDINVENTORY_PAGE
	};

private:
	CInventoryUI_RebuildInven*	m_pPage;
	CUIControl*					m_pBackGround;
	CBasicTextButton*			m_pInvenButton;
	CBasicTextBox*			m_pMoneyTextBox;
	CBasicTextBox*			m_pPointTextBox;
	INT						m_nONE_VIEW_SLOT;

public:
	CInventoryUI_Rebuild();
	virtual	~CInventoryUI_Rebuild();

public:
	VOID CreateSubControl();
	CBasicTextButton*	CreateTextButton23 ( const char* szButton, UIGUID ControlID, const char* szText );

public:
	virtual	VOID Update( INT x, INT y, BYTE LB, BYTE MB, BYTE RB, INT nScroll, FLOAT fElapsedTime, BOOL bFirstControl );
	virtual	VOID TranslateUIMessage( UIGUID ControlID, DWORD dwMsg );
	virtual HRESULT RestoreDeviceObjects( LPDIRECT3DDEVICEQ pd3dDevice );

public:
	VOID SetOneViewSlot( const INT& nONE_VIEW_SLOT );
};
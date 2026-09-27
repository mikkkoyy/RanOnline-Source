#pragma	once

#include "UIWindowEx.h"

class CInventoryUI_ItemMixInven;
class	CBasicTextButton;

//--------------------------------------------------------------------
// CInventoryUI_ItemMix : 
//--------------------------------------------------------------------
class CInventoryUI_ItemMix : public CUIWindowEx	// »ﬁ¡ˆ≈Î
{
protected:
	enum
	{
		ITEMMIXINVEN_MONEY_BUTTON = ET_CONTROL_NEXT,
		ITEMMIXINVEN_POINT_BUTTON,
		ITEMMIXINVEN_INVEN_BUTTON,
		ITEMMIXINVEN_PAGE
	};

private:
	CInventoryUI_ItemMixInven*	m_pPage;
	CBasicTextBox*				m_pMoneyTextBox;
	CBasicTextBox*				m_pPointTextBox;

	CUIControl*					m_pBackGround;
	CBasicTextButton*			m_pInvenButton;

	INT							m_nONE_VIEW_SLOT;

public:
	CInventoryUI_ItemMix();
	virtual	~CInventoryUI_ItemMix();

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
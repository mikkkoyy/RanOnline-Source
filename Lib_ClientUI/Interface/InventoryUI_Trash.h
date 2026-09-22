#pragma	once

#include "UIWindowEx.h"

class	CBasicTextButton;
class CInventoryUI_TrashInven;

//--------------------------------------------------------------------
// CInventoryUI_Trash : 
//--------------------------------------------------------------------
class CInventoryUI_Trash : public CUIWindowEx	// »ﬁ¡ˆ≈Î
{
protected:
	enum
	{
		GARBAGEINVENTORY_MONEY_BUTTON = ET_CONTROL_NEXT,
		GARBAGEINVENTORY_POINT_BUTTON,
		GARBAGEINVENTORY_INVEN_BUTTON,
		GARBAGEINVENTORY_PAGE
	};

private:
	CUIControl*					m_pBackGround;
	CInventoryUI_TrashInven*	m_pPage;
	CBasicTextButton*			m_pInvenButton;
	CBasicTextBox*				m_pMoneyTextBox;
	CBasicTextBox*				m_pPointTextBox;
	INT							m_nONE_VIEW_SLOT;

public:
	CInventoryUI_Trash();
	virtual	~CInventoryUI_Trash();

public:
	CBasicTextButton*	CreateTextButton23 ( const char* szButton, UIGUID ControlID, const char* szText );
	VOID CreateSubControl();

public:
	virtual	VOID Update( INT x, INT y, BYTE LB, BYTE MB, BYTE RB, INT nScroll, FLOAT fElapsedTime, BOOL bFirstControl );
	virtual	VOID TranslateUIMessage( UIGUID ControlID, DWORD dwMsg );
	virtual HRESULT RestoreDeviceObjects( LPDIRECT3DDEVICEQ pd3dDevice );

public:
	VOID SetOneViewSlot( const INT& nONE_VIEW_SLOT );
};
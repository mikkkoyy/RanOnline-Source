/*!
 * \file ItemTransferInventoryWindow.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */

#pragma	once

#include "UIWindowEx.h"

class CItemTransferInvenPage;

class CItemTransferInvenWindow : public CUIWindowEx	
{
protected:
	enum
	{
		ITEM_TRANSFER_INVEN_MONEY_BUTTON = ET_CONTROL_NEXT,
		ITEM_TRANSFER_INVEN_PAGE
	};

private:
	CItemTransferInvenPage*	m_pPage;
	CBasicTextBox*			m_pMoneyTextBox;
	INT						m_nONE_VIEW_SLOT;

public:
	CItemTransferInvenWindow();
	virtual	~CItemTransferInvenWindow();

public:
	void CreateSubControl();

public:
	virtual	void Update( INT x, INT y, BYTE LB, BYTE MB, BYTE RB, INT nScroll, FLOAT fElapsedTime, BOOL bFirstControl );
	virtual	void TranslateUIMessage( UIGUID ControlID, DWORD dwMsg );
	virtual HRESULT RestoreDeviceObjects( LPDIRECT3DDEVICEQ pd3dDevice );

public:
	void SetOneViewSlot( const INT& nONE_VIEW_SLOT );
};
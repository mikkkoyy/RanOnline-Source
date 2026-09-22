#include "StdAfx.h"
#include "InventoryUI_TrashInven.h"

#include "ItemSlotEx_Renew.h"
#include "BasicScrollThumbFrame.h"
#include "GLGaeaClient.h"

#include "BasicButton.h"
#include "BasicTextBox.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "../Interface/UITextControl.h"
#include "GameTextControl.h"
#include "InnerInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const INT CInventoryUI_TrashInven::nSTARTLINE = 0;	
const INT CInventoryUI_TrashInven::nOUTOFRANGE = -1;

CInventoryUI_TrashInven::CInventoryUI_TrashInven()
	: m_nONE_VIEW_SLOT( nONE_VIEW_SLOT_DEFAULT )
	, m_nCurPos ( -1 )
	, m_pHelp(NULL)
	//, m_pPages(NULL)
	//, m_pPagesText(NULL)
{
	for( int i=0; i<EM_INVENSIZE_Y; ++i )
		m_pItemSlotArray[i] = NULL;

	for( int i=0; i<nMAX_ONE_VIEW_SLOT; ++i )
		m_pItemSlotArrayDummy[i] = NULL;

	for ( int i = 0 ; i < nMAX_PAGES; ++i )	
	{
		m_pPages[i] = NULL;
		m_pPagesText[i] = NULL;
	}
}

CInventoryUI_TrashInven::~CInventoryUI_TrashInven()
{
}

VOID CInventoryUI_TrashInven::CreateSubControl()
{
	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_FLAG );

	std::string strPages[nMAX_PAGES] = 
	{
		"INVENTORY_WINDOW_RADIO_BUTTON_1",
		"INVENTORY_WINDOW_RADIO_BUTTON_2",
		"INVENTORY_WINDOW_RADIO_BUTTON_3",
		"INVENTORY_WINDOW_RADIO_BUTTON_4",
		"INVENTORY_WINDOW_RADIO_BUTTON_5"
	};

	std::string strPagesText[nMAX_PAGES] = 
	{
		"INVENTORY_WINDOW_RADIO_BUTTON_TEXT_1",
		"INVENTORY_WINDOW_RADIO_BUTTON_TEXT_2",
		"INVENTORY_WINDOW_RADIO_BUTTON_TEXT_3",
		"INVENTORY_WINDOW_RADIO_BUTTON_TEXT_4",
		"INVENTORY_WINDOW_RADIO_BUTTON_TEXT_5",
	};

	for ( int i = 0 ; i < nMAX_PAGES; ++i ) 
	{
		m_pPages[i] = new CBasicButton;
		m_pPages[i]->CreateSub ( this, strPages[i].c_str(), UI_FLAG_DEFAULT, PAGE_BUTTON0 + i );
		m_pPages[i]->CreateFlip ( "INVENTORY_WINDOW_RADIO_BUTTON_F", CBasicButton::RADIO_FLIP );
		m_pPages[i]->SetFlip( false );
		m_pPages[i]->SetVisibleSingle ( TRUE );
		RegisterControl ( m_pPages[i] );

		CString strNumber;
		strNumber.Format( "%d", i+1 );

		m_pPagesText[i] = new CBasicTextBox;
		m_pPagesText[i]->CreateSub ( this, strPagesText[i].c_str(), UI_FLAG_DEFAULT );
		m_pPagesText[i]->SetFont ( pFont8 );
		m_pPagesText[i]->SetTextAlign ( TEXT_ALIGN_LEFT );	
		m_pPagesText[i]->AddText ( strNumber.GetString(), NS_UITEXTCOLOR::BLACK );
		RegisterControl ( m_pPagesText[i] );
	}

	CString strSlotKeyword[ nMAX_ONE_VIEW_SLOT ] =
	{
		"INVENTORY_WINDOW_ITEM_SLOT0",
		"INVENTORY_WINDOW_ITEM_SLOT1",
		"INVENTORY_WINDOW_ITEM_SLOT2",
		"INVENTORY_WINDOW_ITEM_SLOT3",
		"INVENTORY_WINDOW_ITEM_SLOT4",
		"INVENTORY_WINDOW_ITEM_SLOT5",
		"INVENTORY_WINDOW_ITEM_SLOT6",
		"INVENTORY_WINDOW_ITEM_SLOT7",
		"INVENTORY_WINDOW_ITEM_SLOT8",
		"INVENTORY_WINDOW_ITEM_SLOT9"
	};

	for( INT i=0; i<nMAX_ONE_VIEW_SLOT; ++i )
		m_pItemSlotArrayDummy[i] = CreateControl( strSlotKeyword[i].GetString() );

	for( INT i=0; i<EM_INVENSIZE_Y; ++i )
		m_pItemSlotArray[i] = CreateItemSlot( strSlotKeyword[0], ITEM_SLOT0 + i );

	m_pHelp = new CBasicButton;
	m_pHelp->CreateSub ( this, "INVENTORY_WINDOW_HELP_BUTTON" , UI_FLAG_DEFAULT, HELP_BUTTON );
	m_pHelp->CreateFlip ( "INVENTORY_WINDOW_HELP_BUTTON_OVER", CBasicButton::MOUSEIN_FLIP );
	m_pHelp->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pHelp );

	SetItemSlotRender( nSTARTLINE, m_nONE_VIEW_SLOT );
	OpenPage ( 0 );
}

CItemSlotEx_Renew* CInventoryUI_TrashInven::CreateItemSlot( CString strKeyword, UIGUID ControlID )
{
	CItemSlotEx_Renew* pItemSlot = new CItemSlotEx_Renew;
	pItemSlot->CreateSub ( this, strKeyword.GetString(), UI_FLAG_DEFAULT, ControlID );
	pItemSlot->CreateSubTradeInventory( EM_INVENSIZE_X );
	RegisterControl( pItemSlot );

	return pItemSlot;
}

CUIControl*	CInventoryUI_TrashInven::CreateControl( const TCHAR* szControl )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub( this, szControl );
	RegisterControl( pControl );
	return pControl;
}

VOID CInventoryUI_TrashInven::Update( INT x, INT y, BYTE LB, BYTE MB, BYTE RB, INT nScroll, FLOAT fElapsedTime, BOOL bFirstControl )
{
	SetItemIndex( nOUTOFRANGE, nOUTOFRANGE );
	CUIGroup::Update( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
	UpdateBLOCK();

}

VOID CInventoryUI_TrashInven::TranslateUIMessage( UIGUID ControlID, DWORD dwMsg )
{
	if( ITEM_SLOT0 <= ControlID && ControlID < ITEM_SLOT_END )
	{
		if( CHECK_MOUSE_IN( dwMsg ) )
		{
			const INT nPosY = ControlID - ITEM_SLOT0;
			CItemSlot_Renew* pSlot = m_pItemSlotArray[ nPosY ];
			const INT nPosX = pSlot->GetItemIndex();

			if( nPosY < 0 || nPosX < 0 )
				return;

			SetItemIndex( nPosX, nPosY );
		}
	}

	switch ( ControlID )
	{
	case PAGE_BUTTON0:
		{	
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )	OpenPage( 0 );
		}break;
	
	case PAGE_BUTTON1:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )	OpenPage( 1 );
		}break;
	case PAGE_BUTTON2:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )	OpenPage( 2 );
		}break;
	case PAGE_BUTTON3:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )	OpenPage( 3 );
		}break;
	case PAGE_BUTTON4:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )	OpenPage( 4 );
		}break;

	case HELP_BUTTON:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )	
			{
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEINTEXT( "INVENTORY_TRASH_HELP_INVEN" ) , NS_UITEXTCOLOR::WHITE );
			}
		}break;
	};
}

void CInventoryUI_TrashInven::SetVisibleSingle ( BOOL bVisible )
{
	CUIGroup::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		OpenPage( 0 );
	}
}

VOID CInventoryUI_TrashInven::ResetAllItemSlotRender( INT nStartIndex, INT nTotal )
{
	if( nTotal < 0 )
		return;

	for( INT i=nStartIndex; i<nTotal; ++i )
	{
		CItemSlot_Renew* pItemSlot = m_pItemSlotArray[i];
		if( pItemSlot )
			pItemSlot->SetVisibleSingle( FALSE );
	}
}

VOID CInventoryUI_TrashInven::SetItemSlotRender( INT nStartIndex, INT nTotal )
{
	if( nTotal < 0 )
		return;

	const UIRECT& rcParentPos = GetGlobalPos();

	for( INT i=nStartIndex; i<nTotal; ++i )
	{
		CItemSlotEx_Renew* pItemSlot = m_pItemSlotArray[i];
		if( pItemSlot )
		{
			INT nAbsoluteIndex = i - nStartIndex;

			CUIControl* pDummyControl = m_pItemSlotArrayDummy[ nAbsoluteIndex ];
			const UIRECT& rcSlotPos = pDummyControl->GetGlobalPos();
			const UIRECT& rcSlotLocalPos = pDummyControl->GetLocalPos();

			pItemSlot->SetLocalPos( rcSlotLocalPos );
			pItemSlot->SetGlobalPos( rcSlotPos );

			pItemSlot->SetVisibleSingle( TRUE );
		}
	}
}

VOID CInventoryUI_TrashInven::SetItemIndex( INT nPosX, INT nPosY )
{
	m_nPosX = nPosX;
	m_nPosY = nPosY;
}

VOID CInventoryUI_TrashInven::GetItemIndex( INT* pnPosX, INT* pnPosY )
{
	*pnPosX = m_nPosX;
	*pnPosY = m_nPosY;
}

VOID CInventoryUI_TrashInven::LoadItemPage( GLInventory &ItemData )
{
	GLInventory::CELL_MAP* pItemList = ItemData.GetItemList();

	if( pItemList->size() > ( EM_INVENSIZE_X * EM_INVENSIZE_Y ) )
	{
		GASSERT( 0 && "인터페이스 사이즈보다, 데이타크기가 큽니다." );
		return;
	}

	{
		GLInventory::CELL_MAP_ITER iter = pItemList->begin();
		GLInventory::CELL_MAP_ITER iter_end = pItemList->end();

		for( WORD y=0; y<EM_INVENSIZE_Y; ++y )
		{
			for( WORD x=0; x<EM_INVENSIZE_X; ++x )
			{
				SINVENITEM& ref_InvenItem = GetItem( x, y );
				if( ref_InvenItem.sItemCustom.sNativeID != NATIVEID_NULL() )
				{
					GLInventory::CELL_KEY foundkey( x, y );
					GLInventory::CELL_MAP_ITER found = pItemList->find( foundkey );
					if( found == iter_end )
						UnLoadItem( x, y );
				}
			}
		}
	}

	{
		GLInventory::CELL_MAP_ITER iter = pItemList->begin();
		GLInventory::CELL_MAP_ITER iter_end = pItemList->end();

		for( ; iter != iter_end; ++iter )
		{
			SINVENITEM* pInvenItem = (*iter).second;
			SINVENITEM& ref_InvenItemOld = GetItem( pInvenItem->wPosX, pInvenItem->wPosY );
			if( *pInvenItem != ref_InvenItemOld )
				LoadItem( *pInvenItem );
		}
	}
}

VOID CInventoryUI_TrashInven::LoadItem( SINVENITEM& ref_InvenItem )
{	
	INT nPosX = ref_InvenItem.wPosX;
	INT nPosY = ref_InvenItem.wPosY;
	m_pItemSlotArray[ nPosY ]->SetItemImage( nPosX, ref_InvenItem );
}

VOID CInventoryUI_TrashInven::UnLoadItemPage()
{
	for( INT y=0; y<EM_INVENSIZE_Y; ++y )
		for( INT x=0; x<EM_INVENSIZE_X; ++x )
			UnLoadItem( x, y );
}

VOID CInventoryUI_TrashInven::UnLoadItem( INT nPosX, INT nPosY )
{
	m_pItemSlotArray[ nPosY ]->ResetItemImage( nPosX );
}

VOID CInventoryUI_TrashInven::ResetAllFlipItem()
{
	for( WORD y=0; y<EM_INVENSIZE_Y; ++y )
		for( WORD x=0; x<EM_INVENSIZE_X; ++x )
			m_pItemSlotArray[y]->SetFlipItem( x, FALSE );
}

VOID CInventoryUI_TrashInven::UpdateFlipItem( GLInventory& ref_Inventory )
{
	ResetAllFlipItem();

	GLInventory::CELL_MAP_ITER iter = ref_Inventory.GetItemList()->begin();
	GLInventory::CELL_MAP_ITER iter_end = ref_Inventory.GetItemList()->end();

	for( ; iter != iter_end; ++iter )
	{
		SINVENITEM* pInvenItem = (*iter).second;
		if( pInvenItem )
		{
			INT nPosX = pInvenItem->wBackX;
			INT nPosY = pInvenItem->wBackY;
			m_pItemSlotArray[ nPosY ]->SetFlipItem( nPosX, TRUE );
		}
	}
}

VOID CInventoryUI_TrashInven::UpdateFlipItem( SINVEN_POS& ref_GarbageItem )
{
	//ResetAllFlipItem();

	if( ref_GarbageItem.wPosY >= 0 && ref_GarbageItem.wPosY < EM_INVENSIZE_Y )
	{
		if( ref_GarbageItem.wPosX >= 0 && ref_GarbageItem.wPosX < EM_INVENSIZE_X )
			m_pItemSlotArray[ ref_GarbageItem.wPosY ]->SetFlipItem( ref_GarbageItem.wPosX, TRUE );
	}
}

VOID CInventoryUI_TrashInven::UpdateBLOCK()
{
	const WORD& wAddINVENLINE = GLGaeaClient::GetInstance().GetCharacter()->GetOnINVENLINE();
	INT nUSABLE_INVENLINE = EM_INVEN_DEF_SIZE_Y + wAddINVENLINE;

	if( EM_INVENSIZE_Y < nUSABLE_INVENLINE )
	{		
		GASSERT( 0 && "데이타 오류입니다." );

		nUSABLE_INVENLINE = EM_INVENSIZE_Y;
	}

	for( INT i=0; i<nUSABLE_INVENLINE; ++i )
		m_pItemSlotArray[i]->ResetBLOCK();

	for( INT i=nUSABLE_INVENLINE; i<EM_INVENSIZE_Y; ++i )
		m_pItemSlotArray[i]->SetBLOCK();
}

SINVENITEM& CInventoryUI_TrashInven::GetItem( INT nPosX, INT nPosY )
{
	return m_pItemSlotArray[ nPosY ]->GetItemImage( nPosX );
}

VOID CInventoryUI_TrashInven::SetOneViewSlot( const INT& nMAX_ONE_VIEW )
{
	m_nONE_VIEW_SLOT = nMAX_ONE_VIEW;
	m_nCurPos = -1;
}

void CInventoryUI_TrashInven::OpenPage ( int nPage )
{
	for ( int i = 0 ; i < nMAX_PAGES; ++i )	
	{
		m_pPages[i]->SetFlip ( false );
	}

	m_pPages[nPage]->SetFlip ( true );

	switch ( nPage )
	{
	case 0:
		{
			ResetAllItemSlotRender ( 10, 50 );
			SetItemSlotRender ( 0, 10 );
		}break;
	case 1:
		{
			ResetAllItemSlotRender ( 0, 10 );
			ResetAllItemSlotRender ( 20, 50 );
			SetItemSlotRender ( 10, 20 );
		}break;
	case 2:
		{
			ResetAllItemSlotRender ( 0, 20 );
			ResetAllItemSlotRender ( 30, 50 );
			SetItemSlotRender ( 20, 30 );
		}break;
	case 3:
		{
			ResetAllItemSlotRender ( 0, 30 );
			ResetAllItemSlotRender ( 40, 50 );
			SetItemSlotRender ( 30, 40 );
		}break;
	case 4:
		{
			ResetAllItemSlotRender ( 0, 40 );
			SetItemSlotRender ( 40, 50 );
		}break;
	};
}
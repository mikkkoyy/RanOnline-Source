#include "StdAfx.h"

#include "InventoryUIBank.h"
#include "ItemSlot.h"
#include "BasicScrollBarEx.h"
#include "BasicScrollThumbFrame.h"
#include "GLGaeaClient.h"
#include "GLItemMan.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "UITextControl.h"
#include "BasicTextBox.h"
#include "BasicButton.h"
#include "InnerInterface.h"
#include "GameTextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int CInventoryUIBank::nSTARTLINE = 0;
const int CInventoryUIBank::nOUTOFRANGE = -1;

CInventoryUIBank::CInventoryUIBank ()
	: m_nCurPos ( -1 ),
	m_pHelp(NULL)
{
	memset( m_pItemSlotArray, 0, sizeof( m_pItemSlotArray ) );
	memset( m_pItemSlotArrayDummy, 0, sizeof( m_pItemSlotArrayDummy ) );

	for ( int i = 0 ; i < nMAX_PAGES; ++i )	
	{
		m_pPages[i] = NULL;
		m_pPagesText[i] = NULL;
	}
}

CInventoryUIBank::~CInventoryUIBank ()
{
}

CUIControl*	CInventoryUIBank::CreateControl ( const char* szControl )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl );
	RegisterControl ( pControl );
	return pControl;
}

void CInventoryUIBank::CreateSubControl ()
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

	static CString strSlotKeyword[nMAX_ONE_VIEW_SLOT] =
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

	for ( int i = 0; i < nMAX_ONE_VIEW_SLOT; i++ )
	{
		m_pItemSlotArrayDummy[i] = CreateControl ( strSlotKeyword[i].GetString () );
	}

	for ( int i = 0; i < ITEMBANK_INVEN_Y; i++ )
	{
		m_pItemSlotArray[i] = CreateItemSlot ( strSlotKeyword[0], ITEM_SLOT0 + i );
	}

	m_pHelp = new CBasicButton;
	m_pHelp->CreateSub ( this, "INVENTORY_WINDOW_HELP_BUTTON" , UI_FLAG_DEFAULT, HELP_BUTTON );
	m_pHelp->CreateFlip ( "INVENTORY_WINDOW_HELP_BUTTON_OVER", CBasicButton::MOUSEIN_FLIP );
	m_pHelp->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pHelp );
	
	SetItemSlotRender ( nSTARTLINE, nMAX_ONE_VIEW_SLOT );
	OpenPage ( 0 );
}

void CInventoryUIBank::SetVisibleSingle ( BOOL bVisible )
{
	CUIGroup::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		OpenPage( 0 );
	}
}

void CInventoryUIBank::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	SetItemIndex ( nOUTOFRANGE, nOUTOFRANGE );
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

void CInventoryUIBank::ResetAllItemSlotRender ( int index, int nTotal )
{
	if ( nTotal < 0 ) return ;

	for ( int i = index; i < nTotal; i++ )
	{
		CItemSlot* pItemSlot = m_pItemSlotArray[i];
		if ( pItemSlot )
		{
			pItemSlot->SetVisibleSingle ( FALSE );
		}
	}
}

void CInventoryUIBank::SetItemSlotRender ( int nStartIndex, int nTotal )
{
	if ( nTotal < 0 ) return ;

	const UIRECT& rcParentPos = GetGlobalPos ();

	for ( int i = nStartIndex; i < nTotal; i++ )
	{
		CItemSlot* pItemSlot = m_pItemSlotArray[i];
		if ( pItemSlot )
		{
			int nAbsoluteIndex = i - nStartIndex;

			CUIControl* pDummyControl = m_pItemSlotArrayDummy[nAbsoluteIndex];			
			const UIRECT& rcSlotPos = pDummyControl->GetGlobalPos ();
			const UIRECT& rcSlotLocalPos = pDummyControl->GetLocalPos ();

			pItemSlot->SetLocalPos ( rcSlotLocalPos );
			pItemSlot->SetGlobalPos ( rcSlotPos );

			pItemSlot->SetVisibleSingle ( TRUE );
		}
	}
}

CItemSlot* CInventoryUIBank::CreateItemSlot ( CString strKeyword, UIGUID ControlID )
{
	CItemSlot* pItemSlot = new CItemSlot;
	pItemSlot->CreateSub ( this, strKeyword.GetString (), UI_FLAG_DEFAULT, ControlID );
	pItemSlot->CreateSubControl ( ITEMBANK_INVEN_X, TRUE );
	RegisterControl ( pItemSlot );

	return pItemSlot;
}

void CInventoryUIBank::LoadItem ( SINVENITEM& ref_InvenItem )
{	
	int nPosX = ref_InvenItem.wPosX;
	int nPosY = ref_InvenItem.wPosY;
	m_pItemSlotArray[nPosY]->SetItemImage ( nPosX, ref_InvenItem );
}

void CInventoryUIBank::LoadItemPage ( GLInventory &ItemData )
{
	GLInventory::CELL_MAP *pItemList = ItemData.GetItemList();
	
	if ( pItemList->size () > (ITEMBANK_INVEN_Y * ITEMBANK_INVEN_X) )
	{
		GASSERT ( 0 && "인터페이스 사이즈보다, 데이타크기가 큽니다." );
		return ;
	}

	GLInventory::CELL_MAP_ITER iter = pItemList->begin();
	GLInventory::CELL_MAP_ITER iter_end = pItemList->end();

	for ( ; iter!=iter_end; ++iter )
	{
		SINVENITEM* pInvenItem = (*iter).second;
		SINVENITEM& ref_InvenItemOld = GetItem ( pInvenItem->wPosX, pInvenItem->wPosY );
		if ( *pInvenItem != ref_InvenItemOld )
		{
			LoadItem ( *pInvenItem );
		}	
	}
}

void CInventoryUIBank::UnLoadItemPage ()
{
	for ( int y = 0; y < ITEMBANK_INVEN_Y; y++ )
	{
		for ( int x = 0; x < ITEMBANK_INVEN_X; x++ )
		{
			UnLoadItem ( x, y );
		}
	}
}

SINVENITEM& CInventoryUIBank::GetItem ( int nPosX, int nPosY )
{
	return m_pItemSlotArray[nPosY]->GetItemImage ( nPosX );
}

void CInventoryUIBank::UnLoadItem ( int nPosX, int nPosY )
{
	m_pItemSlotArray[nPosY]->ResetItemImage ( nPosX );
}

void CInventoryUIBank::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	if ( ITEM_SLOT0 <= ControlID && ControlID < ITEM_SLOT_END )
	{
		if ( CHECK_MOUSE_IN ( dwMsg ) )
		{
			const int nPosY = ControlID - ITEM_SLOT0;
			CItemSlot* pSlot = m_pItemSlotArray[nPosY];
			const int nPosX = pSlot->GetItemIndex ();

			if ( nPosY < 0 || nPosX < 0 ) return ;

			SetItemIndex ( nPosX, nPosY );
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
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEINTEXT( "INVENTORY_HELP_BANK"), NS_UITEXTCOLOR::WHITE );
			}
		}break;
	};
}

void CInventoryUIBank::SetItemIndex ( int nPosX, int nPosY )
{
	m_nPosX = nPosX;
	m_nPosY = nPosY;
}

void CInventoryUIBank::GetItemIndex ( int* pnPosX, int* pnPosY )
{
	*pnPosX = m_nPosX;
	*pnPosY = m_nPosY;
}

void CInventoryUIBank::OpenPage ( int nPage )
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

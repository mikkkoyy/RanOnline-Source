#include "StdAfx.h"
#include "InventoryPage.h"
#include "ItemSlot.h"
#include "BasicScrollBarEx.h"
#include "./BasicScrollThumbFrame.h"
#include "BasicButton.h"
#include "BasicTextBox.h"
#include "GLGaeaClient.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "UITextControl.h"
#include "BasicTextBox.h"
#include "InnerInterface.h"
#include "GameTextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int CInventoryPage::nSTARTLINE = 0;
const int CInventoryPage::nOUTOFRANGE = -1;

CInventoryPage::CInventoryPage () :
	m_nCurPos ( -1 ),
	m_nONE_VIEW_SLOT ( nONE_VIEW_SLOT_DEFAULT )
{
	memset ( m_pItemSlotArray, 0, sizeof( m_pItemSlotArray ) );
	memset ( m_pItemSlotArrayDummy, 0, sizeof( m_pItemSlotArrayDummy ) );
	memset ( m_Button, 0, sizeof ( CUIControl* ) * 2 );	
	memset ( m_ButtonSet, 0, sizeof ( CUIControl* ) * 2 );	
	memset ( m_ButtonText, 0, sizeof ( CBasicTextBox* ) * 2 );	
}

CInventoryPage::~CInventoryPage ()
{
}

CUIControl*	CInventoryPage::CreateControl ( const char* szControl )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl );
	RegisterControl ( pControl );
	return pControl;
}

void CInventoryPage::CreateSubControl ()
{
	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );
	CString strSlotKeyword[nMAX_ONE_VIEW_SLOT2] =
	{
		"INVENTORY_ITEM_SLOT0",
		"INVENTORY_ITEM_SLOT1",
		"INVENTORY_ITEM_SLOT2",
		"INVENTORY_ITEM_SLOT3",
		"INVENTORY_ITEM_SLOT4",
		"INVENTORY_ITEM_SLOT5",
		"INVENTORY_ITEM_SLOT6",
		"INVENTORY_ITEM_SLOT7",
		"INVENTORY_ITEM_SLOT8",
		"INVENTORY_ITEM_SLOT9",
		"INVENTORY_ITEM_SLOT10",
		"INVENTORY_ITEM_SLOT11",
		"INVENTORY_ITEM_SLOT12",
		"INVENTORY_ITEM_SLOT13",
		"INVENTORY_ITEM_SLOT14",
		"INVENTORY_ITEM_SLOT15",
		"INVENTORY_ITEM_SLOT16",
		"INVENTORY_ITEM_SLOT17",
		"INVENTORY_ITEM_SLOT18",
		"INVENTORY_ITEM_SLOT19"
	};

	CString strButton[2] =
	{
		"INVENTORY_BUTTON0",
		"INVENTORY_BUTTON1"
	};

	CString strButtonSet[2] =
	{
		"INVENTORY_BUTTON0_F",
		"INVENTORY_BUTTON1_F"
	};

	CString strButtonText[2] =
	{
		"INVENTORY_BUTTON_TEXT0",
		"INVENTORY_BUTTON_TEXT1"
	};

	for ( int i = 0; i < 2 ; i++ )
	{
		m_Button[i] = new CUIControl;
		m_Button[i]->CreateSub ( this, strButton[i].GetString (), UI_FLAG_DEFAULT, INVENTORY_BUTTON0 + i );
		m_Button[i]->SetVisibleSingle ( TRUE );
		m_Button[i]->SetTransparentOption( TRUE );
		RegisterControl ( m_Button[i] );

		m_ButtonSet[i] = new CUIControl;
		m_ButtonSet[i]->CreateSub ( this, strButtonSet[i].GetString (), UI_FLAG_DEFAULT );
		if ( m_ButtonSet[0] )
		{
			m_ButtonSet[0]->SetVisibleSingle ( TRUE );
		}
		m_ButtonSet[i]->SetVisibleSingle ( FALSE );
		m_ButtonSet[i]->SetTransparentOption( TRUE );
		RegisterControl ( m_ButtonSet[i] );

		m_ButtonText[i] = new CBasicTextBox;
		m_ButtonText[i]->CreateSub ( this, strButtonText[i].GetString (), UI_FLAG_DEFAULT );
		m_ButtonText[i]->SetFont ( pFont8 );
		m_ButtonText[i]->SetTextAlign ( TEXT_ALIGN_LEFT );	
		m_ButtonText[i]->AddText ( ID2GAMEWORD ( "INVENTORY_TAB_STATIC", i ) );
		RegisterControl ( m_ButtonText[i] );
	}
	//	더미 슬롯
	for ( int i = 0; i < nMAX_ONE_VIEW_SLOT2; i++ )
	{
		m_pItemSlotArrayDummy[i] = CreateControl ( strSlotKeyword[i].GetString () );
	}
	
	//	실제 데이타
	for ( int i = 0; i < EM_INVENSIZE_Y_FORCE; i++ )
	{
		m_pItemSlotArray[i] = CreateItemSlot ( strSlotKeyword[0], ITEM_SLOT0 + i );
	}
	
	ResetAllItemSlotRender ( 10,20 );
	SetItemSlotRender ( 0, 10 );
}

void CInventoryPage::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	SetItemIndex ( nOUTOFRANGE, nOUTOFRANGE );
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
	UpdateBLOCK ();
}

void CInventoryPage::ResetAllItemSlotRender ( int index ,int nTotal )
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

void CInventoryPage::SetItemSlotRender ( int nStartIndex, int nTotal )
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

CItemSlot* CInventoryPage::CreateItemSlot ( CString strKeyword, UIGUID ControlID )
{
	CItemSlot* pItemSlot = new CItemSlot;
	pItemSlot->CreateSub ( this, strKeyword.GetString (), UI_FLAG_DEFAULT, ControlID );
	pItemSlot->CreateSubControl ( EM_INVENSIZE_X, TRUE );	
	RegisterControl ( pItemSlot );
	return pItemSlot;
}

void CInventoryPage::SetItemIndex ( int nPosX, int nPosY )
{
	m_nPosX = nPosX;
	m_nPosY = nPosY;
}

void CInventoryPage::GetItemIndex ( int* pnPosX, int* pnPosY )
{
	*pnPosX = m_nPosX;
	*pnPosY = m_nPosY;
}

void CInventoryPage::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
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
	case INVENTORY_BUTTON0:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				INVENTORY1();
			}
		}
		break;
	case INVENTORY_BUTTON1:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				INVENTORY2();
			}
		}
		break;
	}
}
void CInventoryPage::INVENTORY1()
{
	for ( int i = 0; i < 2; i++ )
	{
		m_Button[i]->SetVisibleSingle ( TRUE );
		m_ButtonSet[i]->SetVisibleSingle ( FALSE );
	}
	m_ButtonSet[0]->SetVisibleSingle ( TRUE );
	m_Button[0]->SetVisibleSingle ( FALSE );
	ResetAllItemSlotRender ( 10,20 );
	SetItemSlotRender ( 0, 10 );
}
void CInventoryPage::INVENTORY2()
{
	for ( int i = 0; i < 2; i++ )
	{
		m_Button[i]->SetVisibleSingle ( TRUE );
		m_ButtonSet[i]->SetVisibleSingle ( FALSE );
	}
	m_ButtonSet[1]->SetVisibleSingle ( TRUE );
	m_Button[1]->SetVisibleSingle ( FALSE );
	ResetAllItemSlotRender ( 0,10 );
	SetItemSlotRender ( 10, 20 );
}
void CInventoryPage::LoadItem ( SINVENITEM& ref_InvenItem )
{	
	int nPosX = ref_InvenItem.wPosX;
	int nPosY = ref_InvenItem.wPosY;
	m_pItemSlotArray[nPosY]->SetItemImage ( nPosX, ref_InvenItem );
}

void CInventoryPage::UnLoadItemPage ()
{
	for ( int y = 0; y < EM_INVENSIZE_Y_FORCE; y++ )
	{
		for ( int x = 0; x < EM_INVENSIZE_X; x++ )
		{
			UnLoadItem ( x, y );
		}
	}
}

SINVENITEM& CInventoryPage::GetItem ( int nPosX, int nPosY )
{
	return m_pItemSlotArray[nPosY]->GetItemImage ( nPosX );
}

void CInventoryPage::UnLoadItem ( int nPosX, int nPosY )
{
	m_pItemSlotArray[nPosY]->ResetItemImage ( nPosX );
}

void CInventoryPage::LoadItemPage ( GLInventory &ItemData )
{
	GLInventory::CELL_MAP *pItemList = ItemData.GetItemList();
	
	if ( pItemList->size () > (EM_INVENSIZE_Y_FORCE * EM_INVENSIZE_X) )
	{
		GASSERT ( 0 && "인터페이스 사이즈보다, 데이타크기가 큽니다." );
		return ;
	}


	{	//	삭제

		GLInventory::CELL_MAP_ITER iter = pItemList->begin();
		GLInventory::CELL_MAP_ITER iter_end = pItemList->end();

		for ( WORD y = 0; y < EM_INVENSIZE_Y_FORCE; y++ )
		{
			for ( WORD x = 0; x < EM_INVENSIZE_X; x++ )
			{
				SINVENITEM& ref_InvenItem = GetItem ( x, y );
				if ( ref_InvenItem.sItemCustom.sNativeID != NATIVEID_NULL() )
				{
					GLInventory::CELL_KEY foundkey ( x, y );
					GLInventory::CELL_MAP_ITER found = pItemList->find ( foundkey );
					if ( found == iter_end )
					{
						UnLoadItem ( x, y );
					}
				}
			}
		}
	}

	{	//	등록
		GLInventory::CELL_MAP_ITER iter = pItemList->begin();
		GLInventory::CELL_MAP_ITER iter_end = pItemList->end();

		for ( ; iter!=iter_end; ++iter )
		{
			SINVENITEM* pInvenItem = (*iter).second;
			SINVENITEM& ref_InvenItemOld = GetItem ( pInvenItem->wPosX, pInvenItem->wPosY );
			if ( pInvenItem->sItemCustom != ref_InvenItemOld.sItemCustom )
			{
				LoadItem ( *pInvenItem );
			}
		}
	}
}

void CInventoryPage::UpdateBLOCK ()
{
	const WORD& wAddINVENLINE = GLGaeaClient::GetInstance().GetCharacter()->GetOnINVENLINE();
	int nUSABLE_INVENLINE = EM_INVEN_DEF_SIZE_Y + wAddINVENLINE;

	if ( EM_INVENSIZE_Y_FORCE < nUSABLE_INVENLINE )
	{		
		GASSERT ( 0 && "데이타 오류입니다." );

		nUSABLE_INVENLINE = EM_INVENSIZE_Y_FORCE;
	}
	
	//	사용 가능
	for ( int i = 0; i < nUSABLE_INVENLINE; i++ )
		m_pItemSlotArray[i]->ResetBLOCK ();

	//	사용 불가능
	for ( int i = nUSABLE_INVENLINE; i < EM_INVENSIZE_Y; i++ )
		m_pItemSlotArray[i]->SetBLOCK ();
}

void CInventoryPage::SetOneViewSlot ( const int& nMAX_ONE_VIEW )
{
	m_nONE_VIEW_SLOT = nMAX_ONE_VIEW;
	m_nCurPos = -1;
}
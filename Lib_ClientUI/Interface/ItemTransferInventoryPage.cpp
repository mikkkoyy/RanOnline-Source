#include "StdAfx.h"
#include "ItemTransferInventoryPage.h"
#include "ItemSlotEx.h"
#include "BasicScrollBarEx.h"
#include "./BasicScrollThumbFrame.h"
#include "GLGaeaClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


const INT CItemTransferInvenPage::nSTARTLINE = 0;	
const INT CItemTransferInvenPage::nOUTOFRANGE = -1;

CItemTransferInvenPage::CItemTransferInvenPage()
	: m_pScrollBar( NULL )
	, m_nONE_VIEW_SLOT( nONE_VIEW_SLOT_DEFAULT )
	, m_nCurPos ( -1 )
{
	for( INT i=0; i<EM_INVENSIZE_Y; ++i )
		m_pItemSlotArray[i] = NULL;

	for( INT i=0; i<nMAX_ONE_VIEW_SLOT; ++i )
		m_pItemSlotArrayDummy[i] = NULL;
}

CItemTransferInvenPage::~CItemTransferInvenPage()
{
}

void CItemTransferInvenPage::CreateSubControl()
{
	CString strSlotKeyword[ nMAX_ONE_VIEW_SLOT ] =
	{
		"ITEM_TRANSFER_INVENSLOT_ITEM_SLOT0",
		"ITEM_TRANSFER_INVENSLOT_ITEM_SLOT1",
		"ITEM_TRANSFER_INVENSLOT_ITEM_SLOT2",
		"ITEM_TRANSFER_INVENSLOT_ITEM_SLOT3",
		"ITEM_TRANSFER_INVENSLOT_ITEM_SLOT4",
		"ITEM_TRANSFER_INVENSLOT_ITEM_SLOT5",
		"ITEM_TRANSFER_INVENSLOT_ITEM_SLOT6",
		"ITEM_TRANSFER_INVENSLOT_ITEM_SLOT7",
		"ITEM_TRANSFER_INVENSLOT_ITEM_SLOT8",
		"ITEM_TRANSFER_INVENSLOT_ITEM_SLOT9",
	};

	for( INT i=0; i<nMAX_ONE_VIEW_SLOT; ++i )
		m_pItemSlotArrayDummy[i] = CreateControl( strSlotKeyword[i].GetString() );

	for( INT i=0; i<EM_INVENSIZE_Y; ++i )
		m_pItemSlotArray[i] = CreateItemSlot( strSlotKeyword[0], ITEM_SLOT0 + i );

	{
		m_pScrollBar = new CBasicScrollBarEx;
		m_pScrollBar->CreateSub( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE );
		m_pScrollBar->CreateBaseScrollBar( "ITEM_TRANSFER_INVEN_PAGE_SCROLL" );
		m_pScrollBar->GetThumbFrame()->SetState( EM_INVENSIZE_Y, m_nONE_VIEW_SLOT );
		RegisterControl( m_pScrollBar );
	}

	SetItemSlotRender( nSTARTLINE, m_nONE_VIEW_SLOT );
}

CItemSlotEx* CItemTransferInvenPage::CreateItemSlot( CString strKeyword, UIGUID ControlID )
{
	CItemSlotEx* pItemSlot = new CItemSlotEx;
	pItemSlot->CreateSub ( this, strKeyword.GetString(), UI_FLAG_DEFAULT, ControlID );
	pItemSlot->CreateSubTradeInventory( EM_INVENSIZE_X );
	RegisterControl( pItemSlot );

	return pItemSlot;
}

CUIControl*	CItemTransferInvenPage::CreateControl( const TCHAR* szControl )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub( this, szControl );
	RegisterControl( pControl );
	return pControl;
}

void CItemTransferInvenPage::Update( INT x, INT y, BYTE LB, BYTE MB, BYTE RB, INT nScroll, FLOAT fElapsedTime, BOOL bFirstControl )
{
	SetItemIndex( nOUTOFRANGE, nOUTOFRANGE );

	CUIGroup::Update( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	UpdateBLOCK();

	if( m_pScrollBar )
	{
		CBasicScrollThumbFrame* pThumbFrame = m_pScrollBar->GetThumbFrame();
		INT nTotal = pThumbFrame->GetTotal();

		if( nTotal < m_nONE_VIEW_SLOT )
			return;

		const INT nViewPerPage = pThumbFrame->GetViewPerPage();
		if( nViewPerPage <= nTotal )
		{
			INT nCurPos = nSTARTLINE;
			const INT nMovableLine = nTotal - nViewPerPage;
			FLOAT fPercent = pThumbFrame->GetPercent();

			nCurPos = (INT)floor( fPercent * nMovableLine );
			if( nCurPos < nSTARTLINE ) nCurPos = nSTARTLINE;

			if ( m_nCurPos == nCurPos ) return;
			
			m_nCurPos = nCurPos;

			ResetAllItemSlotRender( nTotal );    
			SetItemSlotRender( nCurPos, nCurPos + nViewPerPage );
		}
	}
}

void CItemTransferInvenPage::TranslateUIMessage( UIGUID ControlID, DWORD dwMsg )
{
	if( ITEM_SLOT0 <= ControlID && ControlID < ITEM_SLOT_END )
	{
		if( CHECK_MOUSE_IN( dwMsg ) )
		{
			const INT nPosY = ControlID - ITEM_SLOT0;
			CItemSlot* pSlot = m_pItemSlotArray[ nPosY ];
			const INT nPosX = pSlot->GetItemIndex();

			if( nPosY < 0 || nPosX < 0 )
				return;

			SetItemIndex( nPosX, nPosY );
		}
	}
}

void CItemTransferInvenPage::ResetAllItemSlotRender( INT nTotal )
{
	if( nTotal < 0 )
		return;

	for( INT i=0; i<nTotal; ++i )
	{
		CItemSlot* pItemSlot = m_pItemSlotArray[i];
		if( pItemSlot )
			pItemSlot->SetVisibleSingle( FALSE );
	}
}

void CItemTransferInvenPage::SetItemSlotRender( INT nStartIndex, INT nTotal )
{
	if( nTotal < 0 )
		return;

	const UIRECT& rcParentPos = GetGlobalPos();

	for( INT i=nStartIndex; i<nTotal; ++i )
	{
		CItemSlotEx* pItemSlot = m_pItemSlotArray[i];
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

void CItemTransferInvenPage::SetItemIndex( INT nPosX, INT nPosY )
{
	m_nPosX = nPosX;
	m_nPosY = nPosY;
}

void CItemTransferInvenPage::GetItemIndex( INT* pnPosX, INT* pnPosY )
{
	*pnPosX = m_nPosX;
	*pnPosY = m_nPosY;
}

void CItemTransferInvenPage::LoadItemPage( GLInventory &ItemData )
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

void CItemTransferInvenPage::LoadItem( SINVENITEM& ref_InvenItem )
{	
	INT nPosX = ref_InvenItem.wPosX;
	INT nPosY = ref_InvenItem.wPosY;
	m_pItemSlotArray[ nPosY ]->SetItemImage( nPosX, ref_InvenItem );
}

void CItemTransferInvenPage::UnLoadItemPage()
{
	for( INT y=0; y<EM_INVENSIZE_Y; ++y )
		for( INT x=0; x<EM_INVENSIZE_X; ++x )
			UnLoadItem( x, y );
}

void CItemTransferInvenPage::UnLoadItem( INT nPosX, INT nPosY )
{
	m_pItemSlotArray[ nPosY ]->ResetItemImage( nPosX );
}

void CItemTransferInvenPage::ResetAllFlipItem()
{
	for( WORD y=0; y<EM_INVENSIZE_Y; ++y )
		for( WORD x=0; x<EM_INVENSIZE_X; ++x )
			m_pItemSlotArray[y]->SetFlipItem( x, FALSE );
}

void CItemTransferInvenPage::UpdateFlipItem( GLInventory& ref_Inventory )
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

void CItemTransferInvenPage::UpdateFlipItem( SINVEN_POS& ref_GarbageItem )
{
	if( ref_GarbageItem.wPosY >= 0 && ref_GarbageItem.wPosY < EM_INVENSIZE_Y )
	{
		if( ref_GarbageItem.wPosX >= 0 && ref_GarbageItem.wPosX < EM_INVENSIZE_X )
			m_pItemSlotArray[ ref_GarbageItem.wPosY ]->SetFlipItem( ref_GarbageItem.wPosX, TRUE );
	}
}

void CItemTransferInvenPage::UpdateBLOCK()
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

SINVENITEM& CItemTransferInvenPage::GetItem( INT nPosX, INT nPosY )
{
	return m_pItemSlotArray[ nPosY ]->GetItemImage( nPosX );
}

void CItemTransferInvenPage::SetOneViewSlot( const INT& nMAX_ONE_VIEW )
{
	m_nONE_VIEW_SLOT = nMAX_ONE_VIEW;
	m_nCurPos = -1;
	m_pScrollBar->GetThumbFrame()->SetState( EM_INVENSIZE_Y, nMAX_ONE_VIEW );
}
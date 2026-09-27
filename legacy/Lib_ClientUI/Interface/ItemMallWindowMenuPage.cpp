#include "StdAfx.h"
#include "GLGaeaClient.h"
#include "BasicLineBox.h"
#include "ItemMallWindowMenuPage.h"
#include "BasicScrollBarEx.h"
#include "BasicScrollThumbFrame.h"
#include "ItemMallWindowMenuSlot.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int CItemMallWindowMenuPage::nSTARTLINE = 0;
const int CItemMallWindowMenuPage::nOUTOFRANGE = -1;

CItemMallWindowMenuPage::CItemMallWindowMenuPage () :
	m_pScrollBar ( NULL ),
	m_nCurPos ( -1 ),
	m_nMaxItemCnt( 0 ),
	m_nType( 0 )
{
	memset ( m_pItemSlotArray, 0, sizeof ( CItemMallWindowMenuSlot* ) * MAX_ITEM_MALL );	
	memset ( m_pItemSlotArrayDummy, 0, sizeof ( CUIControl* ) * nMAX_ONE_VIEW_SLOT );		
}

CItemMallWindowMenuPage::~CItemMallWindowMenuPage ()
{
}

void CItemMallWindowMenuPage::CreateSubControl ( )
{	
	CString strSlotKeyword[nMAX_ONE_VIEW_SLOT] =
	{
		"ITEMMALL_SLOT0",
		"ITEMMALL_SLOT1",
		"ITEMMALL_SLOT2",
		"ITEMMALL_SLOT3",
		"ITEMMALL_SLOT4",
		"ITEMMALL_SLOT5",
		"ITEMMALL_SLOT6",
		"ITEMMALL_SLOT7",
		"ITEMMALL_SLOT8",
		"ITEMMALL_SLOT9"
	};

	for ( int i = 0; i < nMAX_ONE_VIEW_SLOT; i++ )
	{
		m_pItemSlotArrayDummy[i] = CreateControl ( strSlotKeyword[i].GetString () );
	}

	for ( int i = 0; i < MAX_ITEM_MALL; i++ )
	{
		m_pItemSlotArray[i] = CreateItemSlot ( strSlotKeyword[0], ITEMMALL_SLOT_0 + i );
	}

	CBasicScrollBarEx* pScrollBar = new CBasicScrollBarEx;
	pScrollBar->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE );
	pScrollBar->CreateBaseScrollBar ( "ITEMMALL_SCROLLBAR" );
	pScrollBar->GetThumbFrame()->SetState ( 0, nMAX_ONE_VIEW_SLOT );
	RegisterControl ( pScrollBar );
	m_pScrollBar = pScrollBar;

	m_nMaxItemCnt = 0;
	m_vecViewItem.clear();
}

CUIControl*	CItemMallWindowMenuPage::CreateControl ( const char* szControl )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl );
	RegisterControl ( pControl );
	return pControl;
}

CItemMallWindowMenuSlot* CItemMallWindowMenuPage::CreateItemSlot ( CString strKeyword, UIGUID ControlID )
{
	ITEMMALLDATA sData;
	sData.sNativeID = NATIVEID_NULL();

	CItemMallWindowMenuSlot* pItemSlot = new CItemMallWindowMenuSlot;
	pItemSlot->CreateSub ( this, strKeyword.GetString (), UI_FLAG_DEFAULT, ControlID );
	pItemSlot->CreateSubControl ();
	pItemSlot->SetVisibleSingle( FALSE );
	pItemSlot->SetItem( sData );

	RegisterControl ( pItemSlot );
	return pItemSlot;
}

void CItemMallWindowMenuPage::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;

	m_nSlotIndex = nOUTOFRANGE;
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( m_pScrollBar )
	{
		{
			CBasicScrollThumbFrame* pThumbFrame = m_pScrollBar->GetThumbFrame ();
			int nViewPerPage = pThumbFrame->GetViewPerPage ();
			int nBeforeItemCnt = m_vecViewItem.size();

			m_vecViewItem.clear();
			for ( int i = 0; i < m_nMaxItemCnt; i++ )
			{
				SNATIVEID sItemID = m_pItemSlotArray[i]->GetNativeItemID();
				SITEM* pITEM = GLItemMan::GetInstance().GetItem ( sItemID );
				if( pITEM )
				{
					m_vecViewItem.push_back( i );
				}
			}
			if( nBeforeItemCnt != m_vecViewItem.size() )
			{
				m_pScrollBar->GetThumbFrame()->SetState ( m_vecViewItem.size(), nMAX_ONE_VIEW_SLOT );
				ResetAllItemSlotRender( m_nMaxItemCnt );
				SetItemSlotRender ( 0, 0 + nViewPerPage );			
			}
		}

		{
			CBasicScrollThumbFrame* pThumbFrame = m_pScrollBar->GetThumbFrame ();
			int nTotal = pThumbFrame->GetTotal ();
			if ( nTotal <= nMAX_ONE_VIEW_SLOT ) return ;

			const int nViewPerPage = pThumbFrame->GetViewPerPage ();

			if ( nViewPerPage < nTotal )
			{
				int nCurPos = nSTARTLINE;
				const int nMovableLine = nTotal - nViewPerPage;
				float fPercent = pThumbFrame->GetPercent ();

				nCurPos = (int)floor(fPercent * nMovableLine);
				if ( nCurPos < nSTARTLINE ) nCurPos = nSTARTLINE;

				if ( m_nCurPos == nCurPos ) return;

				m_nCurPos = nCurPos;

				ResetAllItemSlotRender ( nTotal );
				SetItemSlotRender ( nCurPos, nCurPos + nViewPerPage );
			}
		}
	}
}

void CItemMallWindowMenuPage::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	if ( ITEMMALL_SLOT_0 <= ControlID && ControlID <= ITEMMALL_SLOT_END )
	{
		if ( CHECK_MOUSE_IN ( dwMsg ) )
		{
			m_nSlotIndex = ControlID - ITEMMALL_SLOT_0;
			m_sItemData = m_pItemSlotArray[m_nSlotIndex]->GetItemMallData();
		}
	}
}


void CItemMallWindowMenuPage::ResetAllItemSlotRender ( int nTotal )
{
	if ( nTotal < 0 ) return ;

	for ( int i = 0; i < MAX_ITEM_MALL; i++ )
	{
		CItemMallWindowMenuSlot* pItemSlot = m_pItemSlotArray[i];
		if ( pItemSlot )
		{
			pItemSlot->SetVisibleSingle ( FALSE );
		}
	}
}

void CItemMallWindowMenuPage::SetItemSlotRender ( int nStartIndex, int nTotal )
{
	if ( nTotal < 0 ) return ;

	const UIRECT& rcParentPos = GetGlobalPos ();

	int iViewIndex = 0;
	int iSlotNum   = 0;
	int i = 0;
	for ( i = nStartIndex; i < m_vecViewItem.size(); i++ )
	{
		if( i >= m_nMaxItemCnt ) break;
		if( i >= nTotal ) break;

		iSlotNum = m_vecViewItem[i];

		CItemMallWindowMenuSlot* pItemSlot = m_pItemSlotArray[iSlotNum];
		if ( pItemSlot )
		{
						
			int nAbsoluteIndex = iViewIndex;
			CUIControl* pDummyControl = m_pItemSlotArrayDummy[nAbsoluteIndex];

			const UIRECT& rcSlotPos = pDummyControl->GetGlobalPos ();
			const UIRECT& rcSlotLocalPos = pDummyControl->GetLocalPos ();

			pItemSlot->SetLocalPos ( rcSlotLocalPos );			
			pItemSlot->SetGlobalPos ( rcSlotPos );

			pItemSlot->SetVisibleSingle ( TRUE );

			iViewIndex++;
		}
	}

	if( m_vecViewItem.size() < nTotal )
	{
		int iMaxLackTotal = nTotal - m_vecViewItem.size();		
		for( i = 0; i < iMaxLackTotal; i++ )
		{
			iSlotNum++;
			CItemMallWindowMenuSlot* pItemSlot = m_pItemSlotArray[iSlotNum];
			if ( pItemSlot )
			{

				int nAbsoluteIndex = iViewIndex;
				CUIControl* pDummyControl = m_pItemSlotArrayDummy[nAbsoluteIndex];

				const UIRECT& rcSlotPos = pDummyControl->GetGlobalPos ();
				const UIRECT& rcSlotLocalPos = pDummyControl->GetLocalPos ();

				pItemSlot->SetLocalPos ( rcSlotLocalPos );			
				pItemSlot->SetGlobalPos ( rcSlotPos );

				pItemSlot->SetVisibleSingle ( FALSE );

				iViewIndex++;
			}
		}
	}
}

void CItemMallWindowMenuPage::SetVisibleType( WORD wType, int nType  )
{
	m_nType = wType;
	m_vecViewItem.clear();
	ResetSlotCount();
	for( int i=0;i<MAX_ITEM_MALL;i++)	
	{
		m_pItemSlotArray[i]->SetVisibleSingle( FALSE );
	}

	VECITEMMALL& vecItem = GLGaeaClient::GetInstance().m_vecItemMall;
	
	int size = static_cast<int>( vecItem.size() );

	if ( size > MAX_ITEM_MALL ) size = MAX_ITEM_MALL;
	int nCounter = 0;
	
	for( int j=0;j<size;j++)
	{
		const ITEMMALLDATA& sItemData = vecItem[j];
		if( sItemData.dwItemStocks <= 0 )	continue;
		if( sItemData.wItemCtg != nType && nType != 2 )	continue;

		SITEM* pItem = GLItemMan::GetInstance().GetItem ( sItemData.sNativeID );
		if ( !pItem ) continue;
		if ( wType != 0 )
		{
			if ( wType >= 1 && wType <= 9 ) 
			{
				if ( pItem->sBasicOp.emItemType != ITEM_SUIT )
					continue;

				if ( wType == 1 )
				{
					if ( pItem->sSuitOp.emSuit != SUIT_HEADGEAR )
						continue;
				}
				else if ( wType == 2 )
				{
					if ( pItem->sSuitOp.emSuit != SUIT_UPPER )
						continue;
				}
				else if ( wType == 3 )
				{
					if ( pItem->sSuitOp.emSuit != SUIT_LOWER )
						continue;
				}
				else if ( wType == 4 )
				{
					if ( pItem->sSuitOp.emSuit != SUIT_HAND )
						continue;
				}
				else if ( wType == 5 )
				{
					if ( pItem->sSuitOp.emSuit != SUIT_FOOT )
						continue;
				}
				else if ( wType == 6 )
				{
					if ( pItem->sSuitOp.emSuit != SUIT_HANDHELD )
						continue;
				}
				else if ( wType == 7 )
				{
					if ( pItem->sSuitOp.emSuit != SUIT_NECK )
						continue;
				}
				else if ( wType == 8 )
				{
					if ( pItem->sSuitOp.emSuit != SUIT_WRIST )
						continue;
				}
				else if ( wType == 9 )
				{
					if ( pItem->sSuitOp.emSuit != SUIT_FINGER )
						continue;
				}
			}
			else if ( wType == 10 )
			{
				if ( pItem->sBasicOp.emItemType == ITEM_SUIT )
					continue;

				if ( pItem->sBasicOp.emItemType == ITEM_BOX )
					continue;

				if ( pItem->sBasicOp.emItemType == ITEM_RANDOMITEM )
					continue;

				if ( pItem->sBasicOp.IsDISGUISE() )
					continue;
			}
			else if ( wType == 11 )
			{
				if ( pItem->sBasicOp.emItemType != ITEM_BOX && pItem->sBasicOp.emItemType != ITEM_RANDOMITEM )
					continue;
			}
			else if ( wType == 12 )
			{
				if ( !pItem->sBasicOp.IsDISGUISE() )
					continue;
			}
		}
		
		m_pItemSlotArray[nCounter]->SetItem( sItemData );
		nCounter++;
	}	
	m_pScrollBar->GetThumbFrame()->SetState ( nCounter, nMAX_ONE_VIEW_SLOT );
	SetItemCount(nCounter);
}
#include "StdAfx.h"
#include "ActivityItemList.h"
#include "ActivityItemListSlot.h"

#include "BasicLineBox.h"
#include "BasicScrollBarEx.h"
#include "BasicScrollThumbFrame.h"

#include "GLCrowData.h"
#include "GLInventory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CActivityItemList::CActivityItemList ()	
	: m_pScrollBar(NULL)
	, m_nStart(0)
	, m_nTotal(0)
{
	for( int i=0; i<ACTIVITY_ITEM_LIST_SLOT_MAX; ++i )
	{
		m_pSlot[i] = NULL;
	}

	m_vecData.clear();
}

CActivityItemList::~CActivityItemList ()
{
}

void CActivityItemList::CreateSubControl ()
{
	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_ACTIVITY_SALESITEM_LIST_LINEBOX" );
	RegisterControl ( pLineBox );

	m_pScrollBar = new CBasicScrollBarEx;
	m_pScrollBar->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, ACTIVITY_ITEM_LIST_SCROLLBAR );
	m_pScrollBar->CreateBaseScrollBar ( "RAN_ACTIVITY_SALESITEM_LIST_SCROLLBAR" );
	m_pScrollBar->GetThumbFrame()->SetState ( 1, ACTIVITY_ITEM_LIST_SLOT_MAX );
	RegisterControl ( m_pScrollBar );

	std::string strSlot[ACTIVITY_ITEM_LIST_SLOT_MAX] = 
	{
		"RAN_ACTIVITY_SALESITEM_LIST_ITEM_0",
		"RAN_ACTIVITY_SALESITEM_LIST_ITEM_1",
		"RAN_ACTIVITY_SALESITEM_LIST_ITEM_2",
		"RAN_ACTIVITY_SALESITEM_LIST_ITEM_3",
		"RAN_ACTIVITY_SALESITEM_LIST_ITEM_4",
		"RAN_ACTIVITY_SALESITEM_LIST_ITEM_5",
		"RAN_ACTIVITY_SALESITEM_LIST_ITEM_6",
		"RAN_ACTIVITY_SALESITEM_LIST_ITEM_7",
	};

	for( int i=0; i<ACTIVITY_ITEM_LIST_SLOT_MAX; ++i )
	{
		m_pSlot[i] = new CActivityItemListSlot;
		m_pSlot[i]->CreateSub( this, strSlot[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE, ACTIVITY_ITEM_LIST_SLOT_0 + i );
		m_pSlot[i]->CreateSubControl ();
		m_pSlot[i]->SetVisibleSingle( FALSE );
		RegisterControl ( m_pSlot[i] );
	}
}

void CActivityItemList::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

}

void CActivityItemList::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	m_nTotal = (int)m_vecData.size();

	if ( m_pScrollBar )
	{
		{
			CBasicScrollThumbFrame* pThumbFrame = m_pScrollBar->GetThumbFrame ();
			int nTotal = pThumbFrame->GetTotal();

			if( nTotal != m_nTotal )
				m_pScrollBar->GetThumbFrame()->SetState ( m_nTotal, ACTIVITY_ITEM_LIST_SLOT_MAX );	
		}
		{
			CBasicScrollThumbFrame* pThumbFrame = m_pScrollBar->GetThumbFrame ();
			int nTotal = pThumbFrame->GetTotal ();
			if ( nTotal <= ACTIVITY_ITEM_LIST_SLOT_MAX ) return ;
			const int nViewPerPage = pThumbFrame->GetViewPerPage ();

			if ( nViewPerPage < nTotal )
			{
				int nCurPos = 0;
				const int nMovableLine = nTotal - nViewPerPage;
				float fPercent = pThumbFrame->GetPercent ();
				nCurPos = (int)floor(fPercent * nMovableLine);
				if ( nCurPos < 0 ) nCurPos = 0;
				if ( m_nStart == nCurPos ) return;
				m_nStart = nCurPos;
				ShowData();
			}
		}
	}
}

void CActivityItemList::ResetData()
{
	m_vecData.clear();

	for( int i=0; i<ACTIVITY_ITEM_LIST_SLOT_MAX; ++i )
	{
		if ( m_pSlot[i] )
		{
			m_pSlot[i]->DataReset();
			m_pSlot[i]->SetVisibleSingle( FALSE );
		}
	}

	m_nTotal = 0;
	m_nStart = 0;
}

void CActivityItemList::LoadData()
{
	ResetData();

	SNATIVEID sidNPC[3];
	sidNPC[0] = SNATIVEID( 51, 13 );
	sidNPC[1] = SNATIVEID( 51, 14 );
	sidNPC[1] = SNATIVEID( 51, 6 );

	for( int i=0; i<3; ++i )
	{
		PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( sidNPC[i] );
		if ( pCrowData )
		{
			for ( int i = 0; i < SCROWACTION::SALENUM; i++ )
			{
				GLInventory& ref_Inventory = *pCrowData->GetSaleInven(i);
				GLInventory::CELL_MAP *ItemList = ref_Inventory.GetItemList();
				if ( ItemList )
				{
					for( GLInventory::CELL_MAP_ITER iter = ItemList->begin(); 
						iter != ItemList->end(); ++iter )
					{
						SINVENITEM* pInvenItem = (*iter).second;
						if ( pInvenItem )
							m_vecData.push_back( pInvenItem->sItemCustom.sNativeID );
					}
				}
			}
		}	
	}
	
	ShowData();
}

void CActivityItemList::ShowData()
{
	for ( int i=0; i < ACTIVITY_ITEM_LIST_SLOT_MAX; ++ i )
	{
		if ( m_pSlot[i] )
		{
			m_pSlot[i]->DataReset();
			m_pSlot[i]->SetVisibleSingle( FALSE );
		}
	}

	int nEnd = m_nStart + ACTIVITY_ITEM_LIST_SLOT_MAX;
	int nSlot = 0;

	for ( int i = m_nStart; i < nEnd; ++ i )
	{
		if ( i >= (int)m_vecData.size() )	continue;

		if ( m_pSlot[nSlot] )
		{
			m_pSlot[nSlot]->SetVisibleSingle( TRUE );
			m_pSlot[nSlot]->DataSet( m_vecData[i] );
			nSlot ++;
		}
	}
}
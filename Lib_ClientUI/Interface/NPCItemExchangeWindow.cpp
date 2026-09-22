#include "StdAfx.h"
#include "NPCItemExchangeWindow.h"
#include "NPCItemExchangeSlot.h"
#include "NPCShopItemSlot.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicButton.h"

#include "UITextControl.h"
#include "GameTextControl.h"

#include "GLCrowData.h"

#include "../Lib_Engine/DxCommon/d3dfont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CNPCItemExchangeWindow::CNPCItemExchangeWindow ()
	: m_pTextHead1(NULL)
	, m_pTextHead2(NULL)
	, m_pButtonPrev(NULL)
	, m_pButtonNext(NULL)
	, m_pTextPage(NULL)
	, m_sidNPC(NATIVEID_NULL())
	, m_nCurrentPage(1)
	, m_nMaxPage(1)
	, m_pCurrentSlot(NULL)
{
	for ( int i=0; i<NPC_EXCHANGE_MAX_SLOT; ++i )
		m_pSlot[i] = NULL;
}

CNPCItemExchangeWindow::~CNPCItemExchangeWindow ()
{
}

void CNPCItemExchangeWindow::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxWhite ( "NPC_ITEM_EXCHANGE_WINDOW_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "NPC_ITEM_EXCHANGE_HEAD_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "NPC_ITEM_EXCHANGE_SLOT_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "NPC_ITEM_EXCHANGE_PAGE_LINE" );
	RegisterControl ( pBasicLineBox );

	m_pTextHead1 = new CBasicTextBox;
	m_pTextHead1->CreateSub ( this, "NPC_ITEM_EXCHANGE_HEAD_TEXT_1" );
	m_pTextHead1->SetFont ( pFont9 );
	m_pTextHead1->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextHead1->SetText( ID2GAMEWORD( "NPC_ITEM_EXCHANGE_TEXT", 1 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextHead1 );

	m_pTextHead2 = new CBasicTextBox;
	m_pTextHead2->CreateSub ( this, "NPC_ITEM_EXCHANGE_HEAD_TEXT_2" );
	m_pTextHead2->SetFont ( pFont9 );
	m_pTextHead2->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextHead2->SetText( ID2GAMEWORD( "NPC_ITEM_EXCHANGE_TEXT", 2 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextHead2 );

	std::string strslot[NPC_EXCHANGE_MAX_SLOT] = 
	{
		"NPC_ITEM_EXCHANGE_SLOT_0",
		"NPC_ITEM_EXCHANGE_SLOT_1",
		"NPC_ITEM_EXCHANGE_SLOT_2",
		"NPC_ITEM_EXCHANGE_SLOT_3",
		"NPC_ITEM_EXCHANGE_SLOT_4",
	};

	for ( int i=0; i<NPC_EXCHANGE_MAX_SLOT; ++i )
	{
		m_pSlot[i] = new CNPCItemExchangeSlot;
		m_pSlot[i]->CreateSub ( this, strslot[i].c_str(), UI_FLAG_DEFAULT, NPC_ITEM_EXCHANGE_ITEM00 + i );
		m_pSlot[i]->CreateSubControl ();
		m_pSlot[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pSlot[i] );
	}

	m_pTextPage = new CBasicTextBox;
	m_pTextPage->CreateSub ( this, "NPC_ITEM_EXCHANGE_CUR_PAGE_TEXT" );
	m_pTextPage->SetFont ( pFont9 );
	m_pTextPage->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextPage->SetText( "0" );
	RegisterControl ( m_pTextPage );
	
	m_pButtonPrev = new CBasicButton;
	m_pButtonPrev->CreateSub ( this, "NPC_ITEM_EXCHANGE_PREV_PAGE_BUTTON", UI_FLAG_DEFAULT, NPC_ITEM_EXCHANGE_PREV_BUTTON );
	m_pButtonPrev->CreateFlip ( "NPC_ITEM_EXCHANGE_PREV_PAGE_BUTTON_F", CBasicButton::CLICK_FLIP );
	m_pButtonPrev->CreateMouseOver ( "NPC_ITEM_EXCHANGE_PREV_PAGE_BUTTON_OVER" );
	RegisterControl ( m_pButtonPrev );

	m_pButtonNext = new CBasicButton;
	m_pButtonNext->CreateSub ( this, "NPC_ITEM_EXCHANGE_NEXT_PAGE_BUTTON", UI_FLAG_DEFAULT, NPC_ITEM_EXCHANGE_NEXT_BUTTON );
	m_pButtonNext->CreateFlip ( "NPC_ITEM_EXCHANGE_NEXT_PAGE_BUTTON_F", CBasicButton::CLICK_FLIP );
	m_pButtonNext->CreateMouseOver ( "NPC_ITEM_EXCHANGE_NEXT_PAGE_BUTTON_OVER" );
	RegisterControl ( m_pButtonNext );
}

void CNPCItemExchangeWindow::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

void CNPCItemExchangeWindow::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIWindowEx::TranslateUIMessage ( ControlID, dwMsg );

	switch( ControlID )
	{

	case NPC_ITEM_EXCHANGE_ITEM00:
	case NPC_ITEM_EXCHANGE_ITEM01:
	case NPC_ITEM_EXCHANGE_ITEM02:
	case NPC_ITEM_EXCHANGE_ITEM03:
	case NPC_ITEM_EXCHANGE_ITEM04:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( CHECK_LB_UP_LIKE ( dwMsg ) )	
				{
					int nIndex = ControlID - NPC_ITEM_EXCHANGE_ITEM00;
					m_pCurrentSlot = m_pSlot[nIndex];
				}
			}
		}
		break;

	case NPC_ITEM_EXCHANGE_PREV_BUTTON:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( CHECK_LB_UP_LIKE ( dwMsg ) )	
				{
					m_nCurrentPage --;
					if ( m_nCurrentPage < 1 )
						m_nCurrentPage = 1;
					ShowPage( m_nCurrentPage );
				}
			}
		}break;

	case NPC_ITEM_EXCHANGE_NEXT_BUTTON:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( CHECK_LB_UP_LIKE ( dwMsg ) )	
				{
					if ( m_nCurrentPage >= m_nMaxPage )	return;

					m_nCurrentPage ++;
					ShowPage( m_nCurrentPage );
				}
			}
		}break;
	};
}

void CNPCItemExchangeWindow::SetVisibleSingle ( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle( bVisible );

	if( bVisible )
	{
	}else{
	}
}

void CNPCItemExchangeWindow::LoadList( SNATIVEID sid_npc )
{
	ResetList();

	SCROWDATA* pcrow = GLCrowDataMan::GetInstance().GetCrowData( sid_npc );
	if ( !pcrow )	return;

	SNPC_ITEM_EXCHANGE& npc_exchange = pcrow->m_sNPCItemExchange;

	if ( npc_exchange.strTitle.size() )
		SetTitleName( npc_exchange.strTitle.c_str() );

	m_sidNPC = pcrow->sNativeID;

	for ( SNPC_ITEM_EXCHANGE_DATA_MAP_ITER iter = npc_exchange.mapItem.begin();
		iter != npc_exchange.mapItem.end(); ++iter )
	{
		const SNPC_ITEM_EXCHANGE_DATA& npc_exchange_data = (*iter).second;
		m_vecList.push_back( npc_exchange_data );
	}

	std::sort( m_vecList.begin(), m_vecList.end() );

	m_nMaxPage = 1 + ( (int)m_vecList.size() / NPC_EXCHANGE_MAX_SLOT );

	m_nCurrentPage = 1;
	ShowPage( m_nCurrentPage );
}

void CNPCItemExchangeWindow::ResetList()
{
	m_nCurrentPage = 1;
	m_vecList.clear();
	m_sidNPC = NATIVEID_NULL();
	m_nMaxPage = 1;

	for ( int i=0; i<NPC_EXCHANGE_MAX_SLOT; ++i )
	{
		m_pSlot[i]->DataReset();
		m_pSlot[i]->SetVisibleSingle( FALSE );
	}	
}

void CNPCItemExchangeWindow::ShowPage( int nPage )
{
	for ( int i=0; i< NPC_EXCHANGE_MAX_SLOT; ++ i )
	{
		m_pSlot[i]->DataReset();
		m_pSlot[i]->SetVisibleSingle( FALSE );
	}

	if ( nPage < 1 )	return;

	int nEnd = nPage * NPC_EXCHANGE_MAX_SLOT;
	int nStart = nEnd - NPC_EXCHANGE_MAX_SLOT;
	int nSlot = 0;

	for ( int i=nStart; i<nEnd; ++i )
	{
		if ( i >= m_vecList.size() )	continue;

		if ( m_pSlot[nSlot] )
		{
			m_pSlot[nSlot]->SetVisibleSingle( TRUE );
			m_pSlot[nSlot]->DataSet( m_sidNPC, m_vecList[i] );
			nSlot ++;
		}
	}

	if ( m_pTextPage )
	{
		CString strTemp;
		strTemp.Format( "%d", nPage );
		m_pTextPage->ClearText();
		m_pTextPage->SetText( strTemp.GetString() );
	}
}

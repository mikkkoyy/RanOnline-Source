#include "StdAfx.h"
#include "ActivityPageMain.h"

#include "BasicLineBox.h"
#include "BasicScrollBarEx.h"
#include "BasicScrollThumbFrame.h"
#include "BasicTextBox.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "UITextControl.h"
#include "GameTextControl.h"

#include "ActivityPageMainSlot.h"

#include "GLActivity.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CActivityPageMain::CActivityPageMain ()	
	: m_pScrollBar(NULL)
	, m_pTextInfoArival(NULL)
	, m_pTextInfoStatus(NULL)
	, m_pTextInfoReward(NULL)
	, m_nStart(0)
	, m_nTotal(0)
{
	for( int i=0; i<ACTIVITY_PAGE_MAIN_SLOT_MAX; ++i )
		m_pSlot[i] = NULL;

	m_vecData.clear();
}

CActivityPageMain::~CActivityPageMain ()
{
}

void CActivityPageMain::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_ACT_MAIN_PAGE_LINEBOX" );
	RegisterControl ( pLineBox );

	m_pScrollBar = new CBasicScrollBarEx;
	m_pScrollBar->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, ACTIVITY_PAGE_MAIN_SCROLLBAR );
	m_pScrollBar->CreateBaseScrollBar ( "RAN_SPEC_ACT_SLOT_LIST_SCROLL" );
	m_pScrollBar->GetThumbFrame()->SetState ( 1, ACTIVITY_PAGE_MAIN_SLOT_MAX );
	RegisterControl ( m_pScrollBar );

	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, "RAN_SPEC_ACT_PAGE_LEFT_BACK", UI_FLAG_DEFAULT );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "RAN_SPEC_ACT_PAGE_CENTER_BACK", UI_FLAG_DEFAULT );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "RAN_SPEC_ACT_PAGE_RIGHT_BACK", UI_FLAG_DEFAULT );
	RegisterControl ( pControl );

	m_pTextInfoArival = new CBasicTextBox;
	m_pTextInfoArival->CreateSub ( this, "RAN_SPEC_ACT_PAGE_ARRIVAL_TEXT" );
	m_pTextInfoArival->SetFont ( pFont9 );
	m_pTextInfoArival->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextInfoArival->SetText( ID2GAMEWORD( "RAN_SPEC_ACT_PAGE_TEXT", 0 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextInfoArival );

	m_pTextInfoStatus = new CBasicTextBox;
	m_pTextInfoStatus->CreateSub ( this, "RAN_SPEC_ACT_PAGE_PRESENT_TEXT" );
	m_pTextInfoStatus->SetFont ( pFont9 );
	m_pTextInfoStatus->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextInfoStatus->SetText( ID2GAMEWORD( "RAN_SPEC_ACT_PAGE_TEXT", 1 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextInfoStatus );

	m_pTextInfoReward = new CBasicTextBox;
	m_pTextInfoReward->CreateSub ( this, "RAN_SPEC_ACT_PAGE_REWARD_TEXT" );
	m_pTextInfoReward->SetFont ( pFont9 );
	m_pTextInfoReward->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextInfoReward->SetText( ID2GAMEWORD( "RAN_SPEC_ACT_PAGE_TEXT", 2 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextInfoReward );

	std::string strslot[ACTIVITY_PAGE_MAIN_SLOT_MAX] = 
	{
		"RAN_SPEC_ACT_LIST_SLOT0",
		"RAN_SPEC_ACT_LIST_SLOT1",
		"RAN_SPEC_ACT_LIST_SLOT2",
		"RAN_SPEC_ACT_LIST_SLOT3",
		"RAN_SPEC_ACT_LIST_SLOT4",
		"RAN_SPEC_ACT_LIST_SLOT5",
	};

	for( int i=0; i<ACTIVITY_PAGE_MAIN_SLOT_MAX; ++i )
	{
		m_pSlot[i] = new CActivityPageMainSlot;
		m_pSlot[i]->CreateSub( this, strslot[i].c_str(), UI_FLAG_XSIZE | UI_FLAG_YSIZE, ACTIVITY_PAGE_MAIN_SLOT_0 + i );
		m_pSlot[i]->CreateSubControl ();
		m_pSlot[i]->SetVisibleSingle( FALSE );
		RegisterControl ( m_pSlot[i] );
	}
}

void CActivityPageMain::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );
}

void CActivityPageMain::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	m_nTotal = (int)m_vecData.size();

	if ( m_pScrollBar )
	{
		{
			CBasicScrollThumbFrame* pThumbFrame = m_pScrollBar->GetThumbFrame ();
			int nTotal = pThumbFrame->GetTotal();

			if( nTotal != m_nTotal )
				m_pScrollBar->GetThumbFrame()->SetState ( m_nTotal, ACTIVITY_PAGE_MAIN_SLOT_MAX );	
		}
		{
			CBasicScrollThumbFrame* pThumbFrame = m_pScrollBar->GetThumbFrame ();
			int nTotal = pThumbFrame->GetTotal ();
			if ( nTotal <= ACTIVITY_PAGE_MAIN_SLOT_MAX ) return ;
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

void CActivityPageMain::ResetData()
{
	m_vecData.clear();

	for( int i=0; i<ACTIVITY_PAGE_MAIN_SLOT_MAX; ++i )
	{
		if ( m_pSlot[i] )
		{
			m_pSlot[i]->ResetData();
			m_pSlot[i]->SetVisibleSingle( FALSE );
		}
	}

	m_nTotal = 0;
	m_nStart = 0;
}

void CActivityPageMain::LoadData( DWORD dwSelect )
{
	ResetData();

	SACTIVITY_FILE_DATA_MAP map_activity = GLActivity::GetInstance().m_mapActivity;
	for( SACTIVITY_FILE_DATA_MAP_ITER iter = map_activity.begin(); 
		iter != map_activity.end(); ++ iter )
	{
		SACTIVITY_FILE_DATA &sactivity_data = (*iter).second;

		if ( dwSelect == 0 )
		{	
			m_vecData.push_back( sactivity_data );
			continue;
		}
		else
		{
			EMACTIVITY_TYPE emType = (EMACTIVITY_TYPE) ( dwSelect - 1 );
			if( sactivity_data.emType == emType )
			{
				m_vecData.push_back( sactivity_data );
				continue;
			}
		}
	}

	std::sort( m_vecData.begin(), m_vecData.end() );

	ShowData();
}

void CActivityPageMain::ShowData()
{
	for ( int i=0; i < ACTIVITY_PAGE_MAIN_SLOT_MAX; ++ i )
	{
		if ( m_pSlot[i] )
		{
			m_pSlot[i]->ResetData();
			m_pSlot[i]->SetVisibleSingle( FALSE );
		}
	}

	int nEnd = m_nStart + ACTIVITY_PAGE_MAIN_SLOT_MAX;
	int nSlot = 0;

	for ( int i = m_nStart; i < nEnd; ++ i )
	{
		if ( i >= (int)m_vecData.size() )	continue;

		if ( m_pSlot[nSlot] )
		{
			m_pSlot[nSlot]->SetVisibleSingle( TRUE );
			m_pSlot[nSlot]->SetData( m_vecData[i].dwActivityID );
			nSlot ++;
		}
	}
}
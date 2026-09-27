#include "StdAfx.h"
#include "ActivityPage.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "UITextControl.h"
#include "GameTextControl.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicTextBoxEx.h"
#include "BasicScrollBarEx.h"
#include "BasicScrollThumbFrame.h"
#include "BasicButton.h"

#include "GLGaeaClient.h"
#include "GLActivity.h"

#include "ActivityPageMain.h"

#include "InnerInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CActivityPage::CActivityPage ()	
	: m_pListTextBox(NULL)
	, m_pListScrollBar(NULL)
	, m_nSelectIndex(-1)
	, m_pTextTitle(NULL)
	, m_pTextCompleted(NULL)
	, m_pTextPoint(NULL)
	, m_fActPointUpdate(0.0f)
	, m_pMainPage(NULL)
	, m_pButtonItem(NULL)
{
}

CActivityPage::~CActivityPage ()
{
}

void CActivityPage::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_STUDENTRECORD_OBJECT_LINE" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhite ( "RAN_STUDENTRECORD_OBJECT_LINE" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_ACT_LINEBOX_RESULT_TITLE" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_ACT_LINEBOX_RESULT_TITLE" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_ACT_LINEBOX_RESULT_CONTENT" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody ( "RAN_SPEC_ACT_LINEBOX_RESULT_TITLE" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody ( "RAN_SPEC_ACT_LINEBOX_RESULT_CONTENT" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_ACT_LINEBOX_REPORT_LIST" );
	RegisterControl ( pLineBox );

	/*pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody ( "RAN_SPEC_ACT_LINEBOX_REPORT_LIST" );
	RegisterControl ( pLineBox );*/

	m_pListTextBox = new CBasicTextBoxEx;
	m_pListTextBox->CreateSub ( this, "RAN_SPEC_ACT_REPORT_LIST", UI_FLAG_DEFAULT, ACTIVITY_PAGE_LIST_TEXTBOX );
	m_pListTextBox->SetFont ( pFont9 );		
	m_pListTextBox->SetLineInterval ( 3.0f );
	m_pListTextBox->SetSensitive ( true );
	m_pListTextBox->SetLimitLine ( 10000 );
	RegisterControl ( m_pListTextBox );

	int nTotalLine = m_pListTextBox->GetVisibleLine ();

	m_pListScrollBar = new CBasicScrollBarEx;
	m_pListScrollBar->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, ACTIVITY_PAGE_LIST_SCROLLBAR );
	m_pListScrollBar->CreateBaseScrollBar ( "RAN_SPEC_ACT_REPORT_LIST_SCROLL" );
	m_pListScrollBar->GetThumbFrame()->SetState ( 1, nTotalLine );
	RegisterControl ( m_pListScrollBar );

	m_pTextTitle = new CBasicTextBox;
	m_pTextTitle->CreateSub ( this, "RAN_SPEC_ACT_WINDOW_RESULT_TITLE" );
	m_pTextTitle->SetFont ( pFont9 );
	m_pTextTitle->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextTitle->SetText( ID2GAMEWORD( "RAN_SPEC_ACT_WINDOW_RESULT", 0 ), NS_UITEXTCOLOR::ORANGE );
	RegisterControl ( m_pTextTitle );

	m_pTextCompleted = new CBasicTextBox;
	m_pTextCompleted->CreateSub ( this, "RAN_SPEC_ACT_WINDOW_RESULT_CONTENT_COMPLETED" );
	m_pTextCompleted->SetFont ( pFont9 );
	m_pTextCompleted->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextCompleted->SetText( "--" );
	RegisterControl ( m_pTextCompleted );

	m_pTextPoint = new CBasicTextBox;
	m_pTextPoint->CreateSub ( this, "RAN_SPEC_ACT_WINDOW_RESULT_CONTENT_POINT" );
	m_pTextPoint->SetFont ( pFont9 );
	m_pTextPoint->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextPoint->SetText( "--" );
	RegisterControl ( m_pTextPoint );

	m_pMainPage = new CActivityPageMain;
	m_pMainPage->CreateSub( this, "RAN_SPEC_ACT_MAIN_PAGE", UI_FLAG_XSIZE | UI_FLAG_YSIZE, ACTIVITY_PAGE_MAIN );
	m_pMainPage->CreateSubControl ();
	RegisterControl ( m_pMainPage );

	m_pButtonItem = new CBasicButton;
	m_pButtonItem->CreateSub( this, "RAN_SPEC_ACT_SALESITEM_BUTTON_LOW", UI_FLAG_DEFAULT, ACTIVITY_PAGE_ITEM_BUTTON );
	m_pButtonItem->CreateFlip( "RAN_SPEC_ACT_SALESITEM_BUTTON_OVER", CBasicButton::MOUSEIN_FLIP );
	RegisterControl( m_pButtonItem );
}

void CActivityPage::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch( ControlID )
	{
	case ACTIVITY_PAGE_LIST_TEXTBOX:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				int nIndex = m_pListTextBox->GetSelectPos ();
				if ( m_pListTextBox->GetCount () <= nIndex || nIndex < 0  ) return ;

				if ( UIMSG_LB_UP & dwMsg )
				{
					DWORD dwType = m_pListTextBox->GetTextData ( nIndex );
					m_nSelectIndex = nIndex;
					SelectType( dwType );
				}

				m_pListTextBox->SetUseOverColor ( TRUE );
				m_pListTextBox->SetOverColor ( nIndex, NS_UITEXTCOLOR::ORANGE );
			}
		}break;

	case ACTIVITY_PAGE_ITEM_BUTTON:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( UIMSG_LB_UP & dwMsg )
				{
					CInnerInterface::GetInstance().OpenActivityItemWindow();
				}
			}
		}break;
	};
}

void CActivityPage::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	m_pListTextBox->SetUseOverColor ( FALSE );
	if ( m_pListTextBox->GetCount () && 0 <= m_nSelectIndex )
		m_pListTextBox->SetUseTextColor ( m_nSelectIndex, FALSE );

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( m_pListTextBox->GetCount () && 0 <= m_nSelectIndex )
	{
		m_pListTextBox->SetUseTextColor ( m_nSelectIndex, TRUE );
		m_pListTextBox->SetTextColor ( m_nSelectIndex, NS_UITEXTCOLOR::ORANGE );
	}

	CBasicScrollThumbFrame* const pThumbFrame = m_pListScrollBar->GetThumbFrame ();
	const int nTotalLine = m_pListTextBox->GetTotalLine ();
	const int nLinePerOneView = m_pListTextBox->GetVisibleLine ();
	pThumbFrame->SetState ( nTotalLine, nLinePerOneView );
	if ( nLinePerOneView < nTotalLine )
	{
		const int nMovableLine = nTotalLine - nLinePerOneView;
		float fPercent = pThumbFrame->GetPercent ();
		int nPos = (int)floor(fPercent * nMovableLine);
		m_pListTextBox->SetCurLine ( nPos );			
	}

	m_fActPointUpdate += fElapsedTime;
	if ( m_fActPointUpdate >= 0.2f )
	{
		m_fActPointUpdate = 0.0f;
		UpdateActPoint();
	}
}

void CActivityPage::SetVisibleSingle ( BOOL bVisible )
{
	CUIGroup::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		LoadData();
	}
}

void CActivityPage::UpdateActPoint()
{
	if ( m_pTextCompleted ) m_pTextCompleted->ClearText();
	if ( m_pTextPoint )		m_pTextPoint->ClearText();

	GLCharacter* pcharacter = GLGaeaClient::GetInstance().GetCharacter();
	if( !pcharacter ) return;

	CString strText;
	strText.Format( "%s %u/%u", ID2GAMEWORD( "RAN_SPEC_ACT_WINDOW_RESULT", 1 ), 
		pcharacter->GetActivityDoneNum(), (DWORD)GLActivity::GetInstance().m_mapActivity.size() );
	if ( m_pTextCompleted )	m_pTextCompleted->AddText( strText.GetString(), NS_UITEXTCOLOR::WHITE );

	strText.Format( "%s %u", ID2GAMEWORD( "RAN_SPEC_ACT_WINDOW_RESULT", 2 ), pcharacter->m_dwActivityPoint );
	if ( m_pTextPoint )	m_pTextPoint->AddText( strText.GetString(), NS_UITEXTCOLOR::WHITE );
}

void CActivityPage::LoadData()
{
	if ( m_pListTextBox )	m_pListTextBox->ClearText();
	if ( m_pMainPage )		m_pMainPage->ResetData();

	if ( m_pListTextBox )
	{
		int nIndex = m_pListTextBox->AddText ( ID2GAMEWORD( "RAN_SPEC_ACT_WINDOW_TEXT", 0 ), NS_UITEXTCOLOR::WHITE );
		m_pListTextBox->SetTextData ( nIndex, 0 );

		for( int i=0; i<EMACTIVITY_TYPE_SIZE; ++i )
		{
			nIndex = m_pListTextBox->AddText ( COMMENT::ACTIVITY_MID[i].c_str(), NS_UITEXTCOLOR::WHITE );
			m_pListTextBox->SetTextData ( nIndex, i + 1 );
		}

		//select first line 
		if ( m_pListTextBox->GetCount () > 0 )
		{
			m_pListTextBox->SetSelectPos( 0 );
			int nIndex = m_pListTextBox->GetSelectPos ();
			if ( m_pListTextBox->GetCount () <= nIndex || nIndex < 0  ) return;
			DWORD dwType = m_pListTextBox->GetTextData ( nIndex );
			m_nSelectIndex = nIndex;
			SelectType( dwType );
			m_pListTextBox->SetUseOverColor ( TRUE );
			m_pListTextBox->SetOverColor ( nIndex, NS_UITEXTCOLOR::ORANGE );
		}
	}
}

void CActivityPage::SelectType( DWORD dwSelect )
{
	if ( !m_pMainPage )	return;

	m_pMainPage->LoadData( dwSelect );
}
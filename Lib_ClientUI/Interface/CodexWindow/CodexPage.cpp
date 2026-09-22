#include "StdAfx.h"
#include "CodexPage.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "UITextControl.h"
#include "GameTextControl.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicTextBoxEx.h"
#include "BasicScrollBarEx.h"
#include "BasicScrollThumbFrame.h"
#include "BasicButton.h"

#include "BasicComboBox.h"
#include "BasicComboBoxRollOver.h"

#include "GLGaeaClient.h"
#include "GLCodex.h"

#include "CodexPageMain.h"

#include "InnerInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CCodexPage::CCodexPage ()	
	: m_pListTextBox(NULL)
	, m_pListScrollBar(NULL)
	, m_nSelectIndex(-1)
	, m_pTextTitle(NULL)
	, m_pTextReportTitle(NULL)
	, m_pTextCompleted(NULL)
	, m_pTextPoint(NULL)
	, m_fActPointUpdate(0.0f)
	, m_pMainPage(NULL)
	, m_pButtonItem(NULL)
	
	, m_dwHPIncr(0)
	, m_dwMPIncr(0)
	, m_dwSPIncr(0)
	, m_dwAttIncr(0)
	, m_dwDefIncr(0)
	, m_dwShootIncr(0)
	, m_dwMeleeIncr(0)
	, m_dwEnerIncr(0)
	, m_dwResistIncr(0)
	, m_dwHRIncr(0)
	, m_dwARIncr(0)

{
	for( int i=0; i<11; ++i )
		m_pTextList[i] = NULL;
}

CCodexPage::~CCodexPage ()
{
}

void CCodexPage::CreateSubControl ()
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
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_CODEX_LINEBOX_RESULT_TITLE" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_CODEX_LINEBOX_RESULT_TITLE" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_CODEX_LINEBOX_RESULT_CONTENT" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody ( "RAN_SPEC_CODEX_LINEBOX_RESULT_TITLE" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody ( "RAN_SPEC_CODEX_LINEBOX_RESULT_CONTENT" );
	RegisterControl ( pLineBox );
	
	
	/*Report*/
	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_CODEX_LINEBOX_REPORT_TITLE" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_CODEX_LINEBOX_REPORT_TITLE" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody ( "RAN_SPEC_CODEX_LINEBOX_REPORT_TITLE" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_CODEX_LINEBOX_REPORT_LIST" );
	RegisterControl ( pLineBox );
	
	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody ( "RAN_SPEC_CODEX_LINEBOX_REPORT_LIST" );
	RegisterControl ( pLineBox );

	/*pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody ( "RAN_SPEC_CODEX_LINEBOX_REPORT_LIST" );
	RegisterControl ( pLineBox );*/

	m_pListTextBox = new CBasicTextBoxEx;
	m_pListTextBox->CreateSub ( this, "RAN_SPEC_CODEX_REPORT_LIST", UI_FLAG_DEFAULT, CODEX_PAGE_LIST_TEXTBOX );
	m_pListTextBox->SetFont ( pFont9 );		
	m_pListTextBox->SetLineInterval ( 3.0f );
	m_pListTextBox->SetSensitive ( true );
	m_pListTextBox->SetLimitLine ( 10000 );
	RegisterControl ( m_pListTextBox );

	int nTotalLine = m_pListTextBox->GetVisibleLine ();

	m_pListScrollBar = new CBasicScrollBarEx;
	m_pListScrollBar->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, CODEX_PAGE_LIST_SCROLLBAR );
	m_pListScrollBar->CreateBaseScrollBar ( "RAN_SPEC_CODEX_REPORT_LIST_SCROLL" );
	m_pListScrollBar->GetThumbFrame()->SetState ( 1, nTotalLine );
	RegisterControl ( m_pListScrollBar );

	m_pTextTitle = new CBasicTextBox;
	m_pTextTitle->CreateSub ( this, "RAN_SPEC_CODEX_WINDOW_RESULT_TITLE" );
	m_pTextTitle->SetFont ( pFont9 );
	m_pTextTitle->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextTitle->SetText( ID2GAMEWORD( "RAN_SPEC_CODEX_WINDOW_RESULT", 4 ), NS_UITEXTCOLOR::ORANGE );
	RegisterControl ( m_pTextTitle );
	
	m_pTextReportTitle = new CBasicTextBox;
	m_pTextReportTitle->CreateSub ( this, "RAN_SPEC_CODEX_WINDOW_REPORT_TITLE" );
	m_pTextReportTitle->SetFont ( pFont9 );
	m_pTextReportTitle->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextReportTitle->SetText( ID2GAMEWORD( "RAN_SPEC_CODEX_WINDOW_RESULT", 3 ), NS_UITEXTCOLOR::ORANGE );
	RegisterControl ( m_pTextReportTitle );

	m_pTextCompleted = new CBasicTextBox;
	m_pTextCompleted->CreateSub ( this, "RAN_SPEC_CODEX_WINDOW_RESULT_CONTENT_COMPLETED" );
	m_pTextCompleted->SetFont ( pFont9 );
	m_pTextCompleted->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextCompleted->SetText( "--" );
	RegisterControl ( m_pTextCompleted );

	m_pTextPoint = new CBasicTextBox;
	m_pTextPoint->CreateSub ( this, "RAN_SPEC_CODEX_WINDOW_RESULT_CONTENT_POINT" );
	m_pTextPoint->SetFont ( pFont9 );
	m_pTextPoint->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextPoint->SetText( "--" );
	RegisterControl ( m_pTextPoint );

	m_pMainPage = new CCodexPageMain;
	m_pMainPage->CreateSub( this, "RAN_SPEC_CODEX_MAIN_PAGE", UI_FLAG_XSIZE | UI_FLAG_YSIZE, CODEX_PAGE_MAIN );
	m_pMainPage->CreateSubControl ();
	RegisterControl ( m_pMainPage );

	m_pButtonItem = new CBasicButton;
	m_pButtonItem->CreateSub( this, "RAN_SPEC_CODEX_SALESITEM_BUTTON_LOW", UI_FLAG_DEFAULT, CODEX_PAGE_ITEM_BUTTON );
	m_pButtonItem->CreateFlip( "RAN_SPEC_CODEX_SALESITEM_BUTTON_OVER", CBasicButton::MOUSEIN_FLIP );
	RegisterControl( m_pButtonItem );
	
	CBasicComboBox* pComboBox = NULL;
	CBasicComboBoxRollOver*	pComboBoxRollOver = NULL;

	pComboBox = new CBasicComboBox;
	pComboBox->CreateSub ( this, "BASIC_COMBOBOX", UI_FLAG_XSIZE, ITEMMALL_COMBO_OPEN );
	pComboBox->CreateBaseComboBox ( "CODEX_COMBO_OPEN" );			
	RegisterControl ( pComboBox );
	m_pComboBoxTypeOpen = pComboBox;

	pComboBoxRollOver = new CBasicComboBoxRollOver;
	pComboBoxRollOver->CreateSub ( this, "BASIC_COMBOBOX_ROLLOVER", UI_FLAG_XSIZE | UI_FLAG_YSIZE, ITEMMALL_COMBO_ROLLOVER );
	pComboBoxRollOver->CreateBaseComboBoxRollOver ( "CODEX_COMBO_ROLLOVER" );			
	pComboBoxRollOver->SetVisibleSingle ( FALSE );
	RegisterControl ( pComboBoxRollOver );
	m_pComboBoxTypeRollOver = pComboBoxRollOver;

	m_pComboBoxTypeRollOver->ClearText ();

	m_pComboBoxTypeRollOver->AddText("All Types", NS_UITEXTCOLOR::WHITE);
	for ( int i = 0; i < EMCODEX_TYPE_SIZE; i++ )
	{
		m_pComboBoxTypeRollOver->AddText( COMMENT::CODEX_MID[i].c_str(), NS_UITEXTCOLOR::WHITE );
	}

	//SetItemType(2);
	m_pComboBoxTypeOpen->SetText ( "All Types" );
	
	
	std::string strtext[11] = 
	{
		"RAN_SPEC_CODEX_REPORT_LIST00",
		"RAN_SPEC_CODEX_REPORT_LIST01",
		"RAN_SPEC_CODEX_REPORT_LIST02",
		"RAN_SPEC_CODEX_REPORT_LIST03",
		"RAN_SPEC_CODEX_REPORT_LIST04",
		"RAN_SPEC_CODEX_REPORT_LIST05",
		"RAN_SPEC_CODEX_REPORT_LIST06",
		"RAN_SPEC_CODEX_REPORT_LIST07",
		"RAN_SPEC_CODEX_REPORT_LIST08",
		"RAN_SPEC_CODEX_REPORT_LIST09",
		"RAN_SPEC_CODEX_REPORT_LIST10",
	};
	for( int i=0; i<11; ++i )
	{
		m_pTextList[i] = new CBasicTextBox;
		m_pTextList[i]->CreateSub ( this, strtext[i].c_str() );
		m_pTextList[i]->SetFont ( pFont9 );
		m_pTextList[i]->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
		m_pTextList[i]->SetText( "~", NS_UITEXTCOLOR::WHITE );
		RegisterControl ( m_pTextList[i] );
	}
}

void CCodexPage::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch( ControlID )
	{
	case ITEMMALL_COMBO_OPEN:
	{
		if ( CHECK_MOUSE_IN ( dwMsg ) )
		{
			if ( dwMsg & UIMSG_LB_DOWN )
			{
				if( !m_pComboBoxTypeRollOver->IsVisible() )	m_pComboBoxTypeRollOver->SetVisibleSingle ( TRUE );
				else										m_pComboBoxTypeRollOver->SetVisibleSingle ( FALSE );
				m_pComboBoxTypeRollOver->SetScrollPercent ( 100.0f );				
			}
		}
	}
	break;

	case ITEMMALL_COMBO_ROLLOVER:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{				
				int nIndex = m_pComboBoxTypeRollOver->GetSelectIndex ();
				if ( nIndex < 0 ) return ;

				m_pComboBoxTypeRollOver->SetVisibleSingle ( FALSE );
				m_pComboBoxTypeOpen->SetText ( COMMENT::CODEX_MID[nIndex-1].c_str() );
				//set type here
				
				if ( nIndex == 0 )
					m_pComboBoxTypeOpen->SetText ( "All Types" );

				SelectType( nIndex );
			}
		}
		break;
	case CODEX_PAGE_LIST_TEXTBOX:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				/*int nIndex = m_pListTextBox->GetSelectPos ();
				if ( m_pListTextBox->GetCount () <= nIndex || nIndex < 0  ) return ;

				if ( UIMSG_LB_UP & dwMsg )
				{
					DWORD dwType = m_pListTextBox->GetTextData ( nIndex );
					m_nSelectIndex = nIndex;
					SelectType( dwType );
				}

				m_pListTextBox->SetUseOverColor ( TRUE );
				m_pListTextBox->SetOverColor ( nIndex, NS_UITEXTCOLOR::ORANGE );*/
			}
		}break;

	case CODEX_PAGE_ITEM_BUTTON:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( UIMSG_LB_UP & dwMsg )
				{
					//CInnerInterface::GetInstance().OpenCodexItemWindow();
				}
			}
		}break;
	};
}

void CCodexPage::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	m_pListTextBox->SetUseOverColor ( FALSE );
	if ( m_pListTextBox->GetCount () && 0 <= m_nSelectIndex )
		m_pListTextBox->SetUseTextColor ( m_nSelectIndex, FALSE );

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( m_pListTextBox->GetCount () && 0 <= m_nSelectIndex )
	{
		//m_pListTextBox->SetUseTextColor ( m_nSelectIndex, TRUE );
		//m_pListTextBox->SetTextColor ( m_nSelectIndex, NS_UITEXTCOLOR::ORANGE );
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
	const GLCHARLOGIC& sCharData = GLGaeaClient::GetInstance().GetCharacterLogic ();
	m_fActPointUpdate += fElapsedTime;
	if ( m_fActPointUpdate >= 0.2f )
	{
		m_fActPointUpdate = 0.0f;
		UpdateActPoint();
		
		m_pListTextBox->SetVisibleSingle(false);

		/*Visual codex stats, jhoniex 2/5/2024*/
		m_dwHPIncr = sCharData.GETHPINC();
		m_dwMPIncr = sCharData.GETMPINC();
		m_dwSPIncr = sCharData.GETSPINC();
		m_dwAttIncr = sCharData.GETATTINC();
		m_dwDefIncr = sCharData.GETDEFINC();
		m_dwShootIncr = sCharData.GETSHOOTINC();
		m_dwMeleeIncr = sCharData.GETMELEEINC();
		m_dwEnerIncr = sCharData.GETENERGYINC();
		m_dwResistIncr = sCharData.GETRESISTINC();
		m_dwHRIncr = sCharData.GETHITRATEINC();
		m_dwARIncr = sCharData.GETAVOIDINC();
		
		CString strText;
		strText.Format( "%s +%d", COMMENT::CODEX_MID[0].c_str(), m_dwHPIncr);
		m_pTextList[0]->SetText( strText, NS_UITEXTCOLOR::WHITE );
		strText.Format( "%s +%d", COMMENT::CODEX_MID[1].c_str(), m_dwMPIncr);
		m_pTextList[1]->SetText( strText, NS_UITEXTCOLOR::WHITE );
		strText.Format( "%s +%d", COMMENT::CODEX_MID[2].c_str(), m_dwSPIncr);
		m_pTextList[2]->SetText( strText, NS_UITEXTCOLOR::WHITE );
		strText.Format( "%s +%d", COMMENT::CODEX_MID[3].c_str(), m_dwAttIncr);
		m_pTextList[3]->SetText( strText, NS_UITEXTCOLOR::WHITE );
		strText.Format( "%s +%d", COMMENT::CODEX_MID[4].c_str(), m_dwDefIncr);
		m_pTextList[4]->SetText( strText, NS_UITEXTCOLOR::WHITE );
		strText.Format( "%s +%d", COMMENT::CODEX_MID[5].c_str(), m_dwShootIncr);
		m_pTextList[5]->SetText( strText, NS_UITEXTCOLOR::WHITE );
		strText.Format( "%s +%d", COMMENT::CODEX_MID[6].c_str(), m_dwMeleeIncr);
		m_pTextList[6]->SetText( strText, NS_UITEXTCOLOR::WHITE );
		strText.Format( "%s +%d", COMMENT::CODEX_MID[7].c_str(), m_dwEnerIncr);
		m_pTextList[7]->SetText( strText, NS_UITEXTCOLOR::WHITE );
		strText.Format( "%s +%d", COMMENT::CODEX_MID[8].c_str(), m_dwResistIncr);
		m_pTextList[8]->SetText( strText, NS_UITEXTCOLOR::WHITE );
		strText.Format( "%s +%d", COMMENT::CODEX_MID[9].c_str(), m_dwHRIncr);
		m_pTextList[9]->SetText( strText, NS_UITEXTCOLOR::WHITE );
		strText.Format( "%s +%d", COMMENT::CODEX_MID[10].c_str(), m_dwARIncr);
		m_pTextList[10]->SetText( strText, NS_UITEXTCOLOR::WHITE );
		
	}
}

void CCodexPage::SetVisibleSingle ( BOOL bVisible )
{
	CUIGroup::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		LoadData();
	}
}

void CCodexPage::UpdateActPoint()
{
	if ( m_pTextCompleted ) m_pTextCompleted->ClearText();
	if ( m_pTextPoint )		m_pTextPoint->ClearText();

	GLCharacter* pcharacter = GLGaeaClient::GetInstance().GetCharacter();
	if( !pcharacter ) return;

	CString strText;
	strText.Format( "%s %u/%u", ID2GAMEWORD( "RAN_SPEC_CODEX_WINDOW_RESULT", 1 ), 
		pcharacter->GetCodexDoneNum(), (DWORD)GLCodex::GetInstance().m_mapCodex.size() );
	if ( m_pTextCompleted )	m_pTextCompleted->AddText( strText.GetString(), NS_UITEXTCOLOR::WHITE );

	strText.Format( "%s %u/%u", ID2GAMEWORD( "RAN_SPEC_CODEX_WINDOW_RESULT", 2 ), //pcharacter->m_dwCodexPoint );
	pcharacter->GetCodexProgNum(), (DWORD)GLCodex::GetInstance().m_mapCodex.size() );
	if ( m_pTextPoint )	m_pTextPoint->AddText( strText.GetString(), NS_UITEXTCOLOR::WHITE );
}

void CCodexPage::LoadData()
{
	if ( m_pListTextBox )	m_pListTextBox->ClearText();
	if ( m_pMainPage )		m_pMainPage->ResetData();
	
	SelectType(0);
	const GLCHARLOGIC& sCharData = GLGaeaClient::GetInstance().GetCharacterLogic ();
	
	//CString strCombine;
	//strCombine.Format ( "%d", sCharData.GETHPINC() );
	//if ( m_pTextReportTitle ) m_pTextReportTitle->SetOneLineText ( strCombine, NS_UITEXTCOLOR::WHITE );

	if ( m_pListTextBox )
	{
		int nIndex;//= m_pListTextBox->AddText ( ID2GAMEWORD( "RAN_SPEC_CODEX_WINDOW_TEXT", 0 ), NS_UITEXTCOLOR::WHITE );
		//m_pListTextBox->SetTextData ( nIndex, 0 );
		CString strText;
		

		strText.Format( "%s + %d", COMMENT::CODEX_MID[0].c_str(), m_dwHPIncr);
		nIndex = m_pListTextBox->AddText ( strText.GetString(), NS_UITEXTCOLOR::WHITE );
		m_pListTextBox->SetTextData ( nIndex, 0 + 1 );
		
		strText.Format( "%s + %d", COMMENT::CODEX_MID[1].c_str(), m_dwMPIncr);
		nIndex = m_pListTextBox->AddText ( strText.GetString(), NS_UITEXTCOLOR::WHITE );
		m_pListTextBox->SetTextData ( nIndex, 1 + 1 );
		
		strText.Format( "%s + %d", COMMENT::CODEX_MID[2].c_str(), m_dwSPIncr);
		nIndex = m_pListTextBox->AddText ( strText.GetString(), NS_UITEXTCOLOR::WHITE );
		m_pListTextBox->SetTextData ( nIndex, 2 + 1 );
		
		strText.Format( "%s + %d", COMMENT::CODEX_MID[3].c_str(), m_dwAttIncr);
		nIndex = m_pListTextBox->AddText ( strText.GetString(), NS_UITEXTCOLOR::WHITE );
		m_pListTextBox->SetTextData ( nIndex, 3 + 1 );
		
		strText.Format( "%s + %d", COMMENT::CODEX_MID[4].c_str(), m_dwDefIncr);
		nIndex = m_pListTextBox->AddText ( strText.GetString(), NS_UITEXTCOLOR::WHITE );
		m_pListTextBox->SetTextData ( nIndex, 4 + 1 );
		
		strText.Format( "%s + %d", COMMENT::CODEX_MID[5].c_str(), m_dwShootIncr);
		nIndex = m_pListTextBox->AddText ( strText.GetString(), NS_UITEXTCOLOR::WHITE );
		m_pListTextBox->SetTextData ( nIndex, 5 + 1 );
		
		strText.Format( "%s + %d", COMMENT::CODEX_MID[6].c_str(), m_dwMeleeIncr);
		nIndex = m_pListTextBox->AddText ( strText.GetString(), NS_UITEXTCOLOR::WHITE );
		m_pListTextBox->SetTextData ( nIndex, 6 + 1 );
		
		strText.Format( "%s + %d", COMMENT::CODEX_MID[7].c_str(), m_dwEnerIncr);
		nIndex = m_pListTextBox->AddText ( strText.GetString(), NS_UITEXTCOLOR::WHITE );
		m_pListTextBox->SetTextData ( nIndex, 7 + 1 );
		
		strText.Format( "%s + %d", COMMENT::CODEX_MID[8].c_str(), m_dwResistIncr);
		nIndex = m_pListTextBox->AddText ( strText.GetString(), NS_UITEXTCOLOR::WHITE );
		m_pListTextBox->SetTextData ( nIndex, 8 + 1 );
		
		strText.Format( "%s + %d", COMMENT::CODEX_MID[9].c_str(), m_dwHRIncr);
		nIndex = m_pListTextBox->AddText ( strText.GetString(), NS_UITEXTCOLOR::WHITE );
		m_pListTextBox->SetTextData ( nIndex, 9 + 1 );
		
		strText.Format( "%s + %d", COMMENT::CODEX_MID[10].c_str(), m_dwARIncr);
		nIndex = m_pListTextBox->AddText ( strText.GetString(), NS_UITEXTCOLOR::WHITE );
		m_pListTextBox->SetTextData ( nIndex, 10 + 1 );

		//select first line 
		/*if ( m_pListTextBox->GetCount () > 0 )
		{
			m_pListTextBox->SetSelectPos( 0 );
			int nIndex = m_pListTextBox->GetSelectPos ();
			if ( m_pListTextBox->GetCount () <= nIndex || nIndex < 0  ) return;
			DWORD dwType = m_pListTextBox->GetTextData ( nIndex );
			m_nSelectIndex = nIndex;
			SelectType( dwType );
			//m_pListTextBox->SetUseOverColor ( TRUE );
			m_pListTextBox->SetOverColor ( nIndex, NS_UITEXTCOLOR::ORANGE );
		}*/
	}
}

void CCodexPage::SelectType( DWORD dwSelect )
{
	if ( !m_pMainPage )	return;

	m_pMainPage->LoadData( dwSelect );
}
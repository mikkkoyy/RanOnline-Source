#include "StdAfx.h"
#include "ModernQuestListDone.h"

#include "BasicLineBox.h"
#include "BasicTextBoxEx.h"
#include "BasicScrollBarEx.h"
#include "./BasicScrollThumbFrame.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#include "GLQuestPlay.h"
#include "GLQuest.h"
#include "GLQuestMan.h"
#include "GLGaeaClient.h"
#include "RANPARAM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const	int	CModernQuestListDone::nOUTOFRANGE = -1;
const	int CModernQuestListDone::nLimitQuest = 1000;

CModernQuestListDone::CModernQuestListDone ()
	: m_pListText(NULL)
	, m_pListScrollBar(NULL)
	, m_nSelectIndex ( nOUTOFRANGE )
	, m_dwQuestID( NATIVEID_NULL().dwID )
{
}

CModernQuestListDone::~CModernQuestListDone ()
{
}

void CModernQuestListDone::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_QUEST_WINDOW_LIST_PAGE_BACK_LUXES" );
	RegisterControl ( pLineBox );

	m_pListText = new CBasicTextBoxEx;
	m_pListText->CreateSub ( this, "RAN_QUEST_WINDOW_LIST_PAGE_TEXTBOX", UI_FLAG_DEFAULT, MODERN_QUEST_WINDOW_LIST_TEXTBOX );
	m_pListText->SetFont ( pFont9 );		
	m_pListText->SetLineInterval ( 3.0f );
	m_pListText->SetSensitive ( true );
	m_pListText->SetLimitLine ( nLimitQuest );
	RegisterControl ( m_pListText );

	m_pListScrollBar = new CBasicScrollBarEx;
	m_pListScrollBar->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, MODERN_QUEST_WINDOW_LIST_SCROLLBAR );
	m_pListScrollBar->CreateBaseScrollBar ( "RAN_QUEST_WINDOW_LIST_SCROLL_LUXES" );
	m_pListScrollBar->GetThumbFrame()->SetState ( 1, m_pListText->GetVisibleLine() );
	m_pListScrollBar->GetThumbFrame()->SetUseParent( m_pListText );
	RegisterControl ( m_pListScrollBar );
}

void CModernQuestListDone::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	m_pListText->SetUseOverColor ( FALSE );
	if ( m_pListText->GetCount () && 0 <= m_nSelectIndex )
		m_pListText->SetUseTextColor ( m_nSelectIndex, FALSE );

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( m_pListText->GetCount () && 0 <= m_nSelectIndex )
	{
		m_pListText->SetUseTextColor ( m_nSelectIndex, TRUE );
		m_pListText->SetTextColor ( m_nSelectIndex, NS_UITEXTCOLOR::ORANGE );
	}

	CBasicScrollThumbFrame* const pThumbFrame = m_pListScrollBar->GetThumbFrame ();

	const int nTotalLine = m_pListText->GetTotalLine ();
	const int nLinePerOneView = m_pListText->GetVisibleLine ();	
	pThumbFrame->SetState ( nTotalLine, nLinePerOneView );
	if ( nLinePerOneView < nTotalLine )
	{
		const int nMovableLine = nTotalLine - nLinePerOneView;
		float fPercent = pThumbFrame->GetPercent ();
		int nPos = (int)floor(fPercent * nMovableLine);
		m_pListText->SetCurLine ( nPos );			
	}
}

void CModernQuestListDone::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case MODERN_QUEST_WINDOW_LIST_TEXTBOX:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				int nIndex = m_pListText->GetSelectPos ();
				if ( m_pListText->GetCount () <= nIndex ) return ;

				if ( UIMSG_LB_UP & dwMsg )
				{
					m_dwQuestID = m_pListText->GetTextData ( nIndex );
					m_nSelectIndex = nIndex;
					AddMessageEx ( UIMSG_LIST_DONE_LB_UP );
				}

				m_pListText->SetUseOverColor ( TRUE );
				m_pListText->SetOverColor ( nIndex, NS_UITEXTCOLOR::ORANGE );
			}
		}
		break;
	}
}

void CModernQuestListDone::SetVisibleSingle( BOOL bVisible )
{
	CUIGroup::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		LoadQuestList();
	}
}

void CModernQuestListDone::LoadQuestList ()
{
	GLQuestPlay& cQuestPlay = GLGaeaClient::GetInstance().GetCharacter ()->m_cQuestPlay;

	m_pListText->ClearText ();

	GLQuestPlay::MAPQUEST QuestEndMap = cQuestPlay.GetQuestEnd ();
	GLQuestPlay::MAPQUEST_ITER iter = QuestEndMap.begin ();
	GLQuestPlay::MAPQUEST_ITER iter_end = QuestEndMap.end ();
	for ( ; iter != iter_end; ++iter )
	{
		GLQUESTPROG* pQuestProg = (*iter).second;
		DWORD dwNID = pQuestProg->m_sNID.dwID;
		GLQUEST* pQuest = GLQuestMan::GetInstance().Find ( dwNID );
		if ( !pQuest ) continue;

		D3DCOLOR dwColor = NS_UITEXTCOLOR::GREENYELLOW;
		if ( !pQuestProg->m_bCOMPLETE ) dwColor = NS_UITEXTCOLOR::RED;

		int nIndex;
		if ( RANPARAM::strLangPlacer != "en" )
		{
			//xml translation
			CString strTemp;				
			strTemp.Format ( "QN_%03d_%03d", pQuestProg->m_sNID.wMainID, pQuestProg->m_sNID.wSubID );
			nIndex = m_pListText->AddText ( ID2GAMEWORDQUEST( strTemp ), dwColor );
		}
		else
		{
			nIndex = m_pListText->AddText ( pQuest->m_strTITLE.c_str(), dwColor );
		}
		m_pListText->SetTextData ( nIndex, dwNID );

		if ( !pQuestProg->m_bCOMPLETE )	m_pListText->SetTextImage ( nIndex, "RAN_MODERN_QUEST_WINDOW_LIST_FAIL_ICON_LUXES" );	
		else	m_pListText->SetTextImage ( nIndex, "RAN_MODERN_QUEST_WINDOW_LIST_DONE_ICON_LUXES" );
	}

	DWORD dwLastQuestID = m_dwQuestID;
	m_nSelectIndex = nOUTOFRANGE;
	m_dwQuestID = NATIVEID_NULL().dwID;

	if ( m_pListText->GetCount () > 0 )
	{
		//select last selected
		for( int i=0; i<m_pListText->GetCount(); ++i )
		{
			DWORD dwData = m_pListText->GetTextData(i);
			if ( dwData == dwLastQuestID )
			{
				m_dwQuestID = dwData;
				m_nSelectIndex = i;

				m_pListText->SetUseOverColor ( TRUE );
				m_pListText->SetOverColor ( i, NS_UITEXTCOLOR::ORANGE );

				break;
			}
		}

		//select first line 
		if ( m_dwQuestID == NATIVEID_NULL().dwID )
		{
			m_pListText->SetSelectPos( 0 );
			int nIndex = m_pListText->GetSelectPos ();
			if ( m_pListText->GetCount () <= nIndex || nIndex < 0  ) return;

			m_dwQuestID = m_pListText->GetTextData ( nIndex );
			m_nSelectIndex = nIndex;

			m_pListText->SetUseOverColor ( TRUE );
			m_pListText->SetOverColor ( nIndex, NS_UITEXTCOLOR::ORANGE );
		}
	}
}
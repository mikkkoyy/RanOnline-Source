#include "StdAfx.h"
#include "ModernQuestWindow.h"

#include "BasicTextButton.h"
#include "BasicTextBox.h"

#include "ModernQuestListProg.h"
#include "ModernQuestListDone.h"
#include "ModernQuestInfoProg.h"
#include "ModernQuestInfoDone.h"

#include "InnerInterface.h"
#include "GameTextControl.h"
#include "UITextControl.h"
#include "QuestHelper.h"

#include "ModalWindow.h"
#include "ModalCallerID.h"

#include "GLQuestPlay.h"
#include "GLQuest.h"
#include "GLQuestMan.h"
#include "GLGaeaClient.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CModernQuestWindow::CModernQuestWindow(void)
	: m_nPage(-1)
	, m_dwQuestID(NATIVEID_NULL().dwID)
	, m_pListProg(NULL)
	, m_pListDone(NULL)
	, m_pButtonProg(NULL)
	, m_pButtonDone(NULL)
	, m_pInfoProg(NULL)
	, m_pInfoDone(NULL)
	, m_pTextHelper(NULL)
{
}

CModernQuestWindow::~CModernQuestWindow(void)
{
}

void CModernQuestWindow::CreateSubControl()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

	m_pListProg = new CModernQuestListProg;
	m_pListProg->CreateSub ( this, "RAN_QUEST_WINDOW_LIST_PAGE_LUXES", UI_FLAG_DEFAULT, MODERN_QUEST_WINDOW_LIST_PROG );
	m_pListProg->CreateSubControl ();
	RegisterControl ( m_pListProg );

	m_pListDone = new CModernQuestListDone;
	m_pListDone->CreateSub ( this, "RAN_QUEST_WINDOW_LIST_PAGE_LUXES", UI_FLAG_DEFAULT, MODERN_QUEST_WINDOW_LIST_DONE );
	m_pListDone->CreateSubControl ();
	RegisterControl ( m_pListDone );

	m_pButtonProg = new CBasicTextButton;
	m_pButtonProg->CreateSub ( this, "BASIC_TEXT_BUTTON19", UI_FLAG_XSIZE, MODERN_QUEST_WINDOW_BUTTON_PROG );
	m_pButtonProg->CreateBaseButton ( "RAN_QUEST_WINDOW_ING_BUTTON_LUXES", CBasicTextButton::SIZE19, CBasicButton::RADIO_FLIP, ID2GAMEWORD("QUEST_LIST_BUTTON",0) );
	RegisterControl ( m_pButtonProg );

	m_pButtonDone = new CBasicTextButton;
	m_pButtonDone->CreateSub ( this, "BASIC_TEXT_BUTTON19", UI_FLAG_XSIZE, MODERN_QUEST_WINDOW_BUTTON_DONE );
	m_pButtonDone->CreateBaseButton ( "RAN_QUEST_WINDOW_DONE_BUTTON_LUXES", CBasicTextButton::SIZE19, CBasicButton::RADIO_FLIP, ID2GAMEWORD("QUEST_LIST_BUTTON",1) );
	RegisterControl ( m_pButtonDone );

	m_pInfoProg = new CModernQuestInfoProg;
	m_pInfoProg->CreateSub ( this, "RAN_QUEST_WINDOW_MORE_PAGE_LUXES", UI_FLAG_DEFAULT, MODERN_QUEST_WINDOW_INFO_PROG );
	m_pInfoProg->CreateSubControl ();
	RegisterControl ( m_pInfoProg );

	m_pInfoDone = new CModernQuestInfoDone;
	m_pInfoDone->CreateSub ( this, "RAN_QUEST_WINDOW_MORE_PAGE_LUXES", UI_FLAG_DEFAULT, MODERN_QUEST_WINDOW_INFO_DONE );
	m_pInfoDone->CreateSubControl ();
	RegisterControl ( m_pInfoDone );

	m_pTextHelper = new CBasicTextBox;
	m_pTextHelper->CreateSub ( this, "RAN_QUEST_WINDOW_COUNTER_LUXES" );
	m_pTextHelper->SetFont ( pFont9 );
	m_pTextHelper->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextHelper->AddText( "-/-", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextHelper );
}

void CModernQuestWindow::TranslateUIMessage( UIGUID cID, DWORD dwMsg )
{
	CUIWindowEx::TranslateUIMessage( cID, dwMsg );

	switch ( cID )
	{
	case ET_CONTROL_TITLE:
	case ET_CONTROL_TITLE_F:
		{
			if ( (dwMsg & UIMSG_LB_DUP) && CHECK_MOUSE_IN ( dwMsg ) )
			{
				CInnerInterface::GetInstance().SetDefaultPosInterface( GetWndID() );
			}
		}
		break;
	
	case MODERN_QUEST_WINDOW_BUTTON_PROG:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				OpenPage ( MODERN_QUEST_PAGE_PROG );
			}
		}break;

	case MODERN_QUEST_WINDOW_BUTTON_DONE:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				OpenPage ( MODERN_QUEST_PAGE_DONE );
			}
		}break;

	case MODERN_QUEST_WINDOW_LIST_PROG:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( m_pListProg && m_pInfoProg )
				{
					if ( UIMSG_LIST_PROG_LB_UP & dwMsg )
					{
						DWORD dwQuestID = m_pListProg->GetQuestID();
						m_dwQuestID = dwQuestID;
						m_pInfoProg->DataSet( m_dwQuestID );
					}

					if ( UIMSG_LIST_PROG_LB_DUP & dwMsg )
					{
						DWORD dwQuestID = m_pListProg->GetQuestID();
						m_dwQuestID = dwQuestID;

						if ( m_dwQuestID != NATIVEID_NULL().dwID )
						{
							CInnerInterface::GetInstance().SET_QUEST_HELPER( m_dwQuestID );	
							AUTOSELECT_PAGE ( m_dwQuestID );
						}
					}
				}
			}
		}break;

	case MODERN_QUEST_WINDOW_LIST_DONE:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( m_pListDone && m_pInfoDone )
				{
					if ( UIMSG_LIST_DONE_LB_UP & dwMsg )
					{
						DWORD dwQuestID = m_pListDone->GetQuestID();
						m_dwQuestID = dwQuestID;
						m_pInfoDone->DataSet( m_dwQuestID );
					}
				}
			}
		}break;


	case MODERN_QUEST_WINDOW_INFO_PROG:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( m_pInfoProg && m_pInfoProg->GetQuestID() != NATIVEID_NULL().dwID )
				{
					m_dwQuestID = m_pInfoProg->GetQuestID();

					if ( UIMSG_INFO_PROG_BUTTON_GIVEUP & dwMsg )
					{
						DoModal ( ID2GAMEINTEXT("QUEST_GIVEUP_CONFIRM"), MODAL_QUESTION, YESNO, MODAL_QUEST_GIVEUP );
					}

					if ( UIMSG_INFO_PROG_BUTTON_COMPLETE & dwMsg )
					{
						GLGaeaClient::GetInstance().GetCharacter()->ReqQuestComplete ( m_dwQuestID );
					}
				}
			}
		}break;
	}
}

void CModernQuestWindow::SetVisibleSingle( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		OpenPage ( MODERN_QUEST_PAGE_PROG );
	}
	else
	{
	}
}

void CModernQuestWindow::OpenPage ( int nPage )
{
	if ( m_pButtonProg ) m_pButtonProg->SetFlip( FALSE );
	if ( m_pButtonDone ) m_pButtonDone->SetFlip( FALSE );
	if ( m_pListProg )	m_pListProg->SetVisibleSingle( FALSE );
	if ( m_pListDone )	m_pListDone->SetVisibleSingle( FALSE );
	if ( m_pInfoProg )	m_pInfoProg->SetVisibleSingle( FALSE );
	if ( m_pInfoDone )	m_pInfoDone->SetVisibleSingle( FALSE );

	switch( nPage )
	{
	case MODERN_QUEST_PAGE_PROG:
		{
			if ( m_pButtonProg ) m_pButtonProg->SetFlip ( TRUE );
			if ( m_pListProg )	m_pListProg->SetVisibleSingle( TRUE );

			if ( m_pInfoProg )
			{
				m_pInfoProg->SetVisibleSingle( TRUE );

				if ( m_pListProg )
				{
					m_dwQuestID = m_pListProg->GetQuestID();
					m_pInfoProg->DataSet( m_dwQuestID );
				}
			}
			
		}break;

	case MODERN_QUEST_PAGE_DONE:
		{
			if ( m_pButtonDone ) m_pButtonDone->SetFlip ( TRUE );
			if ( m_pListDone )	m_pListDone->SetVisibleSingle( TRUE );

			if ( m_pInfoDone )
			{
				m_pInfoDone->SetVisibleSingle( TRUE );

				if ( m_pListDone )
				{
					m_dwQuestID = m_pListDone->GetQuestID();
					m_pInfoDone->DataSet( m_dwQuestID );
				}
			}
		}break;
	};

	QuestHelperCount();
}

void CModernQuestWindow::AUTOSELECT_PAGE ( DWORD dwQuestID )
{
	if ( !IsVisible () ) return;

	OpenPage ( MODERN_QUEST_PAGE_PROG );

	if ( m_pListProg && m_pListProg->IsVisible() )
	{
		if ( m_pListProg->SelectQuestID( dwQuestID ) )
		{
			if ( m_pInfoProg )
			{
				m_dwQuestID = m_pListProg->GetQuestID();
				m_pInfoProg->DataSet( m_dwQuestID );
			}

			QuestHelperCount();
		}
	}
}

void	CModernQuestWindow::REFRESH_QUEST_WINDOW ()
{	
	if ( !IsVisible () ) return;

	OpenPage ( MODERN_QUEST_PAGE_PROG );

	QuestHelperCount();
}

void	CModernQuestWindow::QuestHelperCount()
{
	GLQuestPlay& cQuestPlay = GLGaeaClient::GetInstance().GetCharacter ()->m_cQuestPlay;

	int nTotal = 0;
	for ( int i=0; i<MAX_QUEST_DISPLAY; ++i )
	{
		DWORD dwQuestID = CInnerInterface::GetInstance().GetQuestHelper()->m_dwQuestID[i];

		GLQUESTPROG* pQuestProg = cQuestPlay.FindProc ( dwQuestID );
		if ( !pQuestProg ) continue;

		GLQUEST* pQuest = GLQuestMan::GetInstance().Find ( dwQuestID );
		if ( !pQuest ) continue;

		nTotal ++;
	}

	CString strCombine = CInnerInterface::GetInstance().MakeString ( ID2GAMEINTEXT("RAN_QUEST_HELPER_COUNT"), nTotal, MAX_QUEST_DISPLAY );

	if ( m_pTextHelper->IsVisible() )
	{
		m_pTextHelper->ClearText();
		m_pTextHelper->AddText( strCombine, NS_UITEXTCOLOR::WHITE );
	}
}
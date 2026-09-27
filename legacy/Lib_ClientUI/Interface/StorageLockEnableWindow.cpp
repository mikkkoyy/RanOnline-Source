#include "StdAfx.h"
#include "StorageLockEnableWindow.h"

#include "d3dfont.h"
#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicTextButton.h"

#include "InnerInterface.h"
#include "UITextControl.h"
#include "GameTextControl.h"
#include "GLGaeaClient.h"

#include "UIEditBoxMan.h"
#include "UIKeyCheck.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int	CStorageLockEnableWindow::nLIMIT_PIN		= 4;
int	CStorageLockEnableWindow::nLIMIT_AGREE	= 6;

CStorageLockEnableWindow::CStorageLockEnableWindow(void)
	: m_pInfoDesc(NULL)
	, m_pInfoPin(NULL)
	, m_pInfoPin2(NULL)
	, m_pInfoAgree(NULL)
	, m_pEditBoxMan(NULL)
	, m_pButtonClose(NULL)
	, m_pButtonSave(NULL)
{
}

CStorageLockEnableWindow::~CStorageLockEnableWindow(void)
{
}

void CStorageLockEnableWindow::CreateSubControl()
{
	CD3DFontPar* pFont08 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_SHADOW_FLAG );
	CD3DFontPar* pFont09 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_STORAGE_LOCK_ENABLE_WINDOW_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_STORAGE_LOCK_ENABLE_WINDOW_INFO_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_STORAGE_LOCK_ENABLE_WINDOW_INPUT_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_STORAGE_LOCK_ENABLE_WINDOW_LINE_PIN" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_STORAGE_LOCK_ENABLE_WINDOW_LINE_PIN2" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_STORAGE_LOCK_ENABLE_WINDOW_LINE_AGREE" );
	RegisterControl ( pBasicLineBox );

	m_pInfoDesc = new CBasicTextBox;
	m_pInfoDesc->CreateSub ( this, "RAN_STORAGE_LOCK_ENABLE_WINDOW_INFO_TEXT", UI_FLAG_DEFAULT );
	m_pInfoDesc->SetFont ( pFont08 );
	m_pInfoDesc->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pInfoDesc->SetText( ID2GAMEINTEXT( "RAN_STORAGE_LOCK_ENABLE_INFO" ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pInfoDesc );

	m_pInfoPin = new CBasicTextBox;
	m_pInfoPin->CreateSub ( this, "RAN_STORAGE_LOCK_ENABLE_WINDOW_INFO_PIN", UI_FLAG_DEFAULT );
	m_pInfoPin->SetFont ( pFont09 );
	m_pInfoPin->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pInfoPin->SetText( ID2GAMEWORD( "RAN_STORAGE_LOCK_ENABLE_WINDOW", 1 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pInfoPin );

	m_pInfoPin2 = new CBasicTextBox;
	m_pInfoPin2->CreateSub ( this, "RAN_STORAGE_LOCK_ENABLE_WINDOW_INFO_PIN2", UI_FLAG_DEFAULT );
	m_pInfoPin2->SetFont ( pFont09 );
	m_pInfoPin2->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pInfoPin2->SetText( ID2GAMEWORD( "RAN_STORAGE_LOCK_ENABLE_WINDOW", 2 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pInfoPin2 );

	m_pInfoAgree = new CBasicTextBox;
	m_pInfoAgree->CreateSub ( this, "RAN_STORAGE_LOCK_ENABLE_WINDOW_INFO_AGREE", UI_FLAG_DEFAULT );
	m_pInfoAgree->SetFont ( pFont09 );
	m_pInfoAgree->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pInfoAgree->SetText( ID2GAMEWORD( "RAN_STORAGE_LOCK_ENABLE_WINDOW", 3 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pInfoAgree );

	m_pEditBoxMan = new CUIEditBoxMan;
	m_pEditBoxMan->CreateSub ( this, "RAN_STORAGE_LOCK_ENABLE_WINDOW_EDITMAN", UI_FLAG_DEFAULT, STORAGE_LOCK_ENABLE_WINDOW_EDIT_MAN );
	m_pEditBoxMan->CreateEditBox ( STORAGE_LOCK_ENABLE_WINDOW_EDIT_PIN, "RAN_STORAGE_LOCK_ENABLE_WINDOW_TEXT_PIN", "RAN_STORAGE_LOCK_ENABLE_WINDOW_CARRAT", TRUE, UINT_MAX, pFont09, nLIMIT_PIN );
	m_pEditBoxMan->CreateEditBox ( STORAGE_LOCK_ENABLE_WINDOW_EDIT_PIN2, "RAN_STORAGE_LOCK_ENABLE_WINDOW_TEXT_PIN2", "RAN_STORAGE_LOCK_ENABLE_WINDOW_CARRAT", TRUE, UINT_MAX, pFont09, nLIMIT_PIN );
	m_pEditBoxMan->CreateEditBox ( STORAGE_LOCK_ENABLE_WINDOW_EDIT_AGREE, "RAN_STORAGE_LOCK_ENABLE_WINDOW_TEXT_AGREE", "RAN_STORAGE_LOCK_ENABLE_WINDOW_CARRAT", TRUE, UINT_MAX, pFont09, nLIMIT_AGREE );
	m_pEditBoxMan->SetHide ( STORAGE_LOCK_ENABLE_WINDOW_EDIT_PIN, TRUE );
	m_pEditBoxMan->SetHide ( STORAGE_LOCK_ENABLE_WINDOW_EDIT_PIN2, TRUE );
	RegisterControl ( m_pEditBoxMan );

	m_pButtonSave = new CBasicTextButton;
	m_pButtonSave->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, STORAGE_LOCK_ENABLE_WINDOW_BUTTON_OK );
	m_pButtonSave->CreateBaseButton ( "RAN_STORAGE_LOCK_ENABLE_WINDOW_BUTTON_OK", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_STORAGE_LOCK_ENABLE_WINDOW", 4 ) );
	RegisterControl ( m_pButtonSave );

	m_pButtonClose = new CBasicTextButton;
	m_pButtonClose->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, STORAGE_LOCK_ENABLE_WINDOW_BUTTON_CANCEL );
	m_pButtonClose->CreateBaseButton ( "RAN_STORAGE_LOCK_ENABLE_WINDOW_BUTTON_CANCEL", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_STORAGE_LOCK_ENABLE_WINDOW", 5 ) );
	RegisterControl ( m_pButtonClose );
}

void CStorageLockEnableWindow::TranslateUIMessage( UIGUID cID, DWORD dwMsg )
{
	CUIWindowEx::TranslateUIMessage( cID, dwMsg );

	switch ( cID )
	{
	case ET_CONTROL_TITLE:
	case ET_CONTROL_TITLE_F:
		{
			if ( (dwMsg & UIMSG_LB_DUP) && CHECK_MOUSE_IN ( dwMsg ) ){
				CInnerInterface::GetInstance().SetDefaultPosInterface( GetWndID() );
			}
		}break;

	case ET_CONTROL_BUTTON:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			{
				CInnerInterface::GetInstance().HideGroup( GetWndID() );
			}
		}break;

	case STORAGE_LOCK_ENABLE_WINDOW_BUTTON_CANCEL:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			{
				CInnerInterface::GetInstance().HideGroup( GetWndID() );
			}
		}break;

	case STORAGE_LOCK_ENABLE_WINDOW_BUTTON_OK:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			{
				CheckInfo();
			}
		}break;
	};

}

void CStorageLockEnableWindow::Update ( int x ,int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( m_pEditBoxMan )
	{
		CUIControl* pParent = m_pEditBoxMan->GetTopParent ();
		if ( !pParent )	pParent = this;
		BOOL bFocus = ( pParent->IsFocusControl() );

		if ( bFocus )
		{
			if ( UIKeyCheck::GetInstance()->Check ( DIK_TAB, DXKEY_DOWN ) )
			{
				m_pEditBoxMan->GoNextTab ();
			}
		}
	}
}

void CStorageLockEnableWindow::SetVisibleSingle( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		if ( m_pEditBoxMan )
		{
			m_pEditBoxMan->EndEdit ();
			m_pEditBoxMan->ClearEdit ( STORAGE_LOCK_ENABLE_WINDOW_EDIT_PIN );
			m_pEditBoxMan->ClearEdit ( STORAGE_LOCK_ENABLE_WINDOW_EDIT_PIN2 );
			m_pEditBoxMan->ClearEdit ( STORAGE_LOCK_ENABLE_WINDOW_EDIT_AGREE );

			m_pEditBoxMan->Init();
			m_pEditBoxMan->BeginEdit();

			if( m_pEditBoxMan->IsMODE_NATIVE() )
			{
				m_pEditBoxMan->DoMODE_TOGGLE();
			}
		}
		
	}
	else
	{
		if ( m_pEditBoxMan )	m_pEditBoxMan->EndEdit ();
	}
}

void CStorageLockEnableWindow::CheckInfo()
{
	if ( !m_pEditBoxMan )	return;

	std::string strpin = m_pEditBoxMan->GetEditString( STORAGE_LOCK_ENABLE_WINDOW_EDIT_PIN );
	std::string strpin2 = m_pEditBoxMan->GetEditString( STORAGE_LOCK_ENABLE_WINDOW_EDIT_PIN2 );
	std::string stragree = m_pEditBoxMan->GetEditString( STORAGE_LOCK_ENABLE_WINDOW_EDIT_AGREE );
	
	if ( strpin.empty() || strpin2.empty() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_KEY_EMPTY") );
		return;
	}

	if ( strpin.size() < nLIMIT_PIN || strpin.size() > nLIMIT_PIN || strpin2.size() < nLIMIT_PIN || strpin2.size() > nLIMIT_PIN )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_KEY_SIZE") );
		return;
	}

	if ( strcmp( strpin.c_str(), strpin2.c_str() ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_KEY_NOT_SAME") );
		return;
	}

	if ( strpin.find_first_not_of( "0123456789" ) != string::npos || strpin2.find_first_not_of( "0123456789" ) != string::npos )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_KEY_NUMBER_ONLY") );
		return;
	}

	if ( stragree.empty() || stragree.size() < nLIMIT_AGREE || stragree.size() > nLIMIT_AGREE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_AGREE_MISMATCH") );
		return;
	}

	if ( strcmp( stragree.c_str(), "ACCEPT" ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_AGREE_MISMATCH") );
		return;
	}

	GLGaeaClient::GetInstance().GetCharacter()->ReqEnableStorageLockMsg( strpin.c_str(), strpin2.c_str(), stragree.c_str() );
	CInnerInterface::GetInstance().HideGroup( GetWndID() );
}
#include "StdAfx.h"

#include "RegisterPage.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicTextButton.h"

#include "../Lib_Engine/DxCommon/d3dfont.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#include "UIEditBoxMan.h"
#include "UIKeyCheck.h"

#include "DxGlobalStage.h"
#include "OuterInterface.h"
#include "ModalWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int	CRegisterPage::nLIMIT_USER		= 11;
int	CRegisterPage::nLIMIT_PASS		= 11;
int	CRegisterPage::nLIMIT_MAIL		= 50;
int	CRegisterPage::nLIMIT_SA		= 11;
int	CRegisterPage::nLIMIT_CAPTCHA	= 7;
int	CRegisterPage::nMIN_TEXT		= 4;

CRegisterPage::CRegisterPage ()
	: m_pEditBoxMan(NULL)
	, m_pInfoUser(NULL)
	, m_pInfoPass(NULL)
	, m_pInfoPass2(NULL)
	, m_pInfoPin(NULL)
	, m_pInfoPin2(NULL)
	, m_pInfoMail(NULL)
	, m_pInfoSA(NULL)
	, m_pInfoSA2(NULL)
	, m_pInfoCaptcha(NULL)
	, m_pButtonPass(NULL)
	, m_pButtonPass2(NULL)
	, m_pButtonPin(NULL)
	, m_pButtonPin2(NULL)
	, m_pButtonSA(NULL)
	, m_pButtonSA2(NULL)
	, m_pTextCaptcha(NULL)
	, m_pTextStatus(NULL)
	, m_bShowPass(FALSE)
	, m_bShowPass2(FALSE)
	, m_bShowPin(FALSE)
	, m_bShowPin2(FALSE)
	, m_bShowSA(FALSE)
	, m_bShowSA2(FALSE)
{

}

CRegisterPage::~CRegisterPage ()
{
}

void CRegisterPage::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_REGISTER_PAGE_LINE_USER" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_REGISTER_PAGE_LINE_PASSWORD" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_REGISTER_PAGE_LINE_PASSWORD2" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_REGISTER_PAGE_LINE_PIN" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_REGISTER_PAGE_LINE_PIN2" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_REGISTER_PAGE_LINE_MAIL" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_REGISTER_PAGE_LINE_SA" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_REGISTER_PAGE_LINE_SA2" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_REGISTER_PAGE_LINE_CAPTCHA" );
	RegisterControl ( pBasicLineBox );


	CBasicTextBox* pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_REGISTER_PAGE_STATIC_USER" );
	pStaticText->SetFont ( pFont9 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT );	
	pStaticText->AddText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 1 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_REGISTER_PAGE_STATIC_PASSWORD" );
	pStaticText->SetFont ( pFont9 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT );	
	pStaticText->AddText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 2 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_REGISTER_PAGE_STATIC_PASSWORD2" );
	pStaticText->SetFont ( pFont9 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT );	
	pStaticText->AddText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 3 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_REGISTER_PAGE_STATIC_PIN" );
	pStaticText->SetFont ( pFont9 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT );	
	pStaticText->AddText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 4 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_REGISTER_PAGE_STATIC_PIN2" );
	pStaticText->SetFont ( pFont9 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT );	
	pStaticText->AddText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 5 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_REGISTER_PAGE_STATIC_MAIL" );
	pStaticText->SetFont ( pFont9 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT );	
	pStaticText->AddText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 6 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_REGISTER_PAGE_STATIC_SA" );
	pStaticText->SetFont ( pFont9 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT );	
	pStaticText->AddText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 7 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_REGISTER_PAGE_STATIC_SA2" );
	pStaticText->SetFont ( pFont9 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT );	
	pStaticText->AddText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 8 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_REGISTER_PAGE_STATIC_CAPTCHA" );
	pStaticText->SetFont ( pFont9 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT );	
	pStaticText->AddText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 9 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );

	pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, "RAN_REGISTER_PAGE_STATIC_INFO" );
	pStaticText->SetFont ( pFont9 );
	pStaticText->SetTextAlign ( TEXT_ALIGN_LEFT );	
	pStaticText->AddText( ID2GAMEINTEXT ( "RAN_REGISTER_INFO_NOT_CASE_SENSITIVE" ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( pStaticText );


	m_pEditBoxMan = new CUIEditBoxMan;
	m_pEditBoxMan->CreateSub ( this, "RAN_REGISTER_PAGE_EDITMAN", UI_FLAG_DEFAULT, LOBBY_REGISTER_EDIT_MAN );
	m_pEditBoxMan->CreateEditBox ( LOBBY_REGISTER_EDIT_USER, "RAN_REGISTER_PAGE_TEXT_USER", "RAN_REGISTER_PAGE_CARRAT", TRUE, UINT_MAX, pFont9, nLIMIT_USER );
	m_pEditBoxMan->CreateEditBox ( LOBBY_REGISTER_EDIT_PASS, "RAN_REGISTER_PAGE_TEXT_PASSWORD", "RAN_REGISTER_PAGE_CARRAT", TRUE, UINT_MAX, pFont9, nLIMIT_PASS );
	m_pEditBoxMan->CreateEditBox ( LOBBY_REGISTER_EDIT_PASS2, "RAN_REGISTER_PAGE_TEXT_PASSWORD2", "RAN_REGISTER_PAGE_CARRAT", TRUE, UINT_MAX, pFont9, nLIMIT_PASS );
	m_pEditBoxMan->CreateEditBox ( LOBBY_REGISTER_EDIT_PIN, "RAN_REGISTER_PAGE_TEXT_PIN", "RAN_REGISTER_PAGE_CARRAT", TRUE, UINT_MAX, pFont9, nLIMIT_PASS );
	m_pEditBoxMan->CreateEditBox ( LOBBY_REGISTER_EDIT_PIN2, "RAN_REGISTER_PAGE_TEXT_PIN2", "RAN_REGISTER_PAGE_CARRAT", TRUE, UINT_MAX, pFont9, nLIMIT_PASS );
	m_pEditBoxMan->CreateEditBox ( LOBBY_REGISTER_EDIT_MAIL, "RAN_REGISTER_PAGE_TEXT_MAIL", "RAN_REGISTER_PAGE_CARRAT", TRUE, UINT_MAX, pFont9, nLIMIT_MAIL );
	m_pEditBoxMan->CreateEditBox ( LOBBY_REGISTER_EDIT_SA, "RAN_REGISTER_PAGE_TEXT_SA", "RAN_REGISTER_PAGE_CARRAT", TRUE, UINT_MAX, pFont9, nLIMIT_SA );
	m_pEditBoxMan->CreateEditBox ( LOBBY_REGISTER_EDIT_SA2, "RAN_REGISTER_PAGE_TEXT_SA2", "RAN_REGISTER_PAGE_CARRAT", TRUE, UINT_MAX, pFont9, nLIMIT_SA );
	m_pEditBoxMan->CreateEditBox ( LOBBY_REGISTER_EDIT_CAPTCHA, "RAN_REGISTER_PAGE_TEXT_CAPTCHA", "RAN_REGISTER_PAGE_CARRAT", TRUE, UINT_MAX, pFont9, nLIMIT_CAPTCHA );
	m_pEditBoxMan->SetHide ( LOBBY_REGISTER_EDIT_PASS, TRUE );
	m_pEditBoxMan->SetHide ( LOBBY_REGISTER_EDIT_PASS2, TRUE );
	m_pEditBoxMan->SetHide ( LOBBY_REGISTER_EDIT_PIN, TRUE );
	m_pEditBoxMan->SetHide ( LOBBY_REGISTER_EDIT_PIN2, TRUE );
	m_pEditBoxMan->SetHide ( LOBBY_REGISTER_EDIT_SA, TRUE );
	m_pEditBoxMan->SetHide ( LOBBY_REGISTER_EDIT_SA2, TRUE );
	RegisterControl( m_pEditBoxMan );

	m_pButtonPass = new CBasicTextButton;
	m_pButtonPass->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, LOBBY_REGISTER_BUTTON_PASS );
	m_pButtonPass->CreateBaseButton ( "RAN_REGISTER_PAGE_SHOW_PASSWORD", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 10 ) );
	RegisterControl ( m_pButtonPass );

	m_pButtonPass2 = new CBasicTextButton;
	m_pButtonPass2->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, LOBBY_REGISTER_BUTTON_PASS2 );
	m_pButtonPass2->CreateBaseButton ( "RAN_REGISTER_PAGE_SHOW_PASSWORD2", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 10 ) );
	RegisterControl ( m_pButtonPass2 );

	m_pButtonPin = new CBasicTextButton;
	m_pButtonPin->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, LOBBY_REGISTER_BUTTON_PIN );
	m_pButtonPin->CreateBaseButton ( "RAN_REGISTER_PAGE_SHOW_PIN", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 10 ) );
	RegisterControl ( m_pButtonPin );

	m_pButtonPin2 = new CBasicTextButton;
	m_pButtonPin2->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, LOBBY_REGISTER_BUTTON_PIN2 );
	m_pButtonPin2->CreateBaseButton ( "RAN_REGISTER_PAGE_SHOW_PIN2", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 10 ) );
	RegisterControl ( m_pButtonPin2 );

	m_pButtonSA = new CBasicTextButton;
	m_pButtonSA->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, LOBBY_REGISTER_BUTTON_SA );
	m_pButtonSA->CreateBaseButton ( "RAN_REGISTER_PAGE_SHOW_SA", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 10 ) );
	RegisterControl ( m_pButtonSA );

	m_pButtonSA2 = new CBasicTextButton;
	m_pButtonSA2->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, LOBBY_REGISTER_BUTTON_SA2 );
	m_pButtonSA2->CreateBaseButton ( "RAN_REGISTER_PAGE_SHOW_SA2", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 10 ) );
	RegisterControl ( m_pButtonSA2 );

	CBasicTextButton* pTextButton = new CBasicTextButton;
	pTextButton->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, LOBBY_REGISTER_BUTTON_REGISTER );
	pTextButton->CreateBaseButton ( "RAN_REGISTER_PAGE_BUTTON_REGISTER", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 12 ) );
	RegisterControl ( pTextButton );

	pTextButton = new CBasicTextButton;
	pTextButton->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, LOBBY_REGISTER_BUTTON_RESET );
	pTextButton->CreateBaseButton ( "RAN_REGISTER_PAGE_BUTTON_RESET", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 13 ) );
	RegisterControl ( pTextButton );

	pTextButton = new CBasicTextButton;
	pTextButton->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, LOBBY_REGISTER_BUTTON_CANCEL );
	pTextButton->CreateBaseButton ( "RAN_REGISTER_PAGE_BUTTON_CANCEL", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 14 ) );
	RegisterControl ( pTextButton );

	
	m_pTextCaptcha = new CBasicTextBox;
	m_pTextCaptcha->CreateSub ( this, "RAN_REGISTER_PAGE_RANDOM_CAPTCHA" );
	m_pTextCaptcha->SetFont ( pFont9 );
	m_pTextCaptcha->SetTextAlign ( TEXT_ALIGN_LEFT );	
	m_pTextCaptcha->AddText( "", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextCaptcha );

	m_pTextStatus = new CBasicTextBox;
	m_pTextStatus->CreateSub ( this, "RAN_REGISTER_PAGE_INFO_STATUS" );
	m_pTextStatus->SetFont ( pFont9 );
	m_pTextStatus->SetTextAlign ( TEXT_ALIGN_LEFT );	
	m_pTextStatus->AddText( "", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextStatus );


	
}	

void CRegisterPage::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{	
	CUIWindow::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	CNetClient* pNetClient = DxGlobalStage::GetInstance().GetNetClient ();
	if ( !pNetClient->IsOnline() )
	{
		if ( !COuterInterface::GetInstance().IsVisibleGroup ( MODAL_WINDOW_OUTER ) )
		{
			if( !COuterInterface::GetInstance().IsLoginCancel() )
			{
				DoModalOuter ( ID2GAMEEXTEXT ("LOGINSTAGE_1"), MODAL_INFOMATION, OK, OUTER_MODAL_RECONNECT );
			}
		}
	}

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

void CRegisterPage::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIWindow::TranslateUIMessage ( ControlID, dwMsg );

	switch (ControlID)
	{
	case LOBBY_REGISTER_BUTTON_REGISTER:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				DoRegister();
			}
		}break;

	case LOBBY_REGISTER_BUTTON_CANCEL:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				EndRegister();
			}
		}break;

	case LOBBY_REGISTER_BUTTON_RESET:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				ResetAll ();
			}
		}break;

	case LOBBY_REGISTER_BUTTON_PASS:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				m_bShowPass = !m_bShowPass;
				CheckHideStatusPass();
			}
		}break;
	case LOBBY_REGISTER_BUTTON_PASS2:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				m_bShowPass2 = !m_bShowPass2;
				CheckHideStatusPass2();
			}
		}break;
	case LOBBY_REGISTER_BUTTON_PIN:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				m_bShowPin = !m_bShowPin;
				CheckHideStatusPin();
			}
		}break;
	case LOBBY_REGISTER_BUTTON_PIN2:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				m_bShowPin2 = !m_bShowPin2;
				CheckHideStatusPin2();
			}
		}break;

	case LOBBY_REGISTER_BUTTON_SA:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				m_bShowSA = !m_bShowSA;
				CheckHideStatusSA();
			}
		}break;

	case LOBBY_REGISTER_BUTTON_SA2:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				m_bShowSA2 = !m_bShowSA2;
				CheckHideStatusSA2();
			}
		}break;
	};
}

void CRegisterPage::SetVisibleSingle ( BOOL bVisible )
{
	CUIGroup::SetVisibleSingle( bVisible );

	if ( bVisible )
	{
		ResetAll();

		CNetClient* pNetClient = DxGlobalStage::GetInstance().GetNetClient ();	
		if ( pNetClient )
		{
			int nServerGroup, nServerChannel;
			COuterInterface::GetInstance().GetConnectServerInfo( nServerGroup, nServerChannel );
			pNetClient->RegisterInit( TRUE, nServerChannel );
		}

		if ( m_pTextCaptcha )
		{
			WORD wRandomNumber = (WORD)rand() % 32000 + 10001;
			CString strRandom;
			strRandom.Format( "%d", wRandomNumber );
			m_strCaptcha = strRandom.GetString();

			m_pTextCaptcha->ClearText();
			m_pTextCaptcha->SetText( m_strCaptcha.c_str(), NS_UITEXTCOLOR::ORNAGERED );
		}
	}
	else
	{
		if ( m_pEditBoxMan )	m_pEditBoxMan->EndEdit ();

		CNetClient* pNetClient = DxGlobalStage::GetInstance().GetNetClient ();	
		if ( pNetClient )
		{
			int nServerGroup, nServerChannel;
			COuterInterface::GetInstance().GetConnectServerInfo( nServerGroup, nServerChannel );
			pNetClient->RegisterInit( FALSE, nServerChannel );
		}
	}
}

void CRegisterPage::ResetAll ()
{
	m_bShowPass = FALSE;
	m_bShowPass2 = FALSE;
	m_bShowPin = FALSE;
	m_bShowPin2 = FALSE;
	m_bShowSA = FALSE;
	m_bShowSA2 = FALSE;

	CheckHideStatusPass();
	CheckHideStatusPass2();
	CheckHideStatusPin();
	CheckHideStatusPin2();
	CheckHideStatusSA();
	CheckHideStatusSA2();

	m_pEditBoxMan->EndEdit ();
	m_pEditBoxMan->ClearEdit ( LOBBY_REGISTER_EDIT_USER );
	m_pEditBoxMan->ClearEdit ( LOBBY_REGISTER_EDIT_PASS );
	m_pEditBoxMan->ClearEdit ( LOBBY_REGISTER_EDIT_PASS2 );
	m_pEditBoxMan->ClearEdit ( LOBBY_REGISTER_EDIT_PIN );
	m_pEditBoxMan->ClearEdit ( LOBBY_REGISTER_EDIT_PIN2 );
	m_pEditBoxMan->ClearEdit ( LOBBY_REGISTER_EDIT_MAIL );
	m_pEditBoxMan->ClearEdit ( LOBBY_REGISTER_EDIT_SA );
	m_pEditBoxMan->ClearEdit ( LOBBY_REGISTER_EDIT_SA2 );
	m_pEditBoxMan->ClearEdit ( LOBBY_REGISTER_EDIT_CAPTCHA );

	m_pEditBoxMan->Init();
	m_pEditBoxMan->BeginEdit();

	if( m_pEditBoxMan->IsMODE_NATIVE() )
	{
		m_pEditBoxMan->DoMODE_TOGGLE();
	}

	m_pTextStatus->ClearText();
}

void CRegisterPage::DoRegister()
{
	if ( !m_pEditBoxMan )	return;
	if ( !m_pTextStatus )	return;

	m_pTextStatus->ClearText();

	std::string struser = m_pEditBoxMan->GetEditString( LOBBY_REGISTER_EDIT_USER );
	std::string strpass = m_pEditBoxMan->GetEditString( LOBBY_REGISTER_EDIT_PASS );
	std::string strpass2 = m_pEditBoxMan->GetEditString( LOBBY_REGISTER_EDIT_PASS2 );
	std::string strpin = m_pEditBoxMan->GetEditString( LOBBY_REGISTER_EDIT_PIN );
	std::string strpin2 = m_pEditBoxMan->GetEditString( LOBBY_REGISTER_EDIT_PIN2 );
	std::string strmail = m_pEditBoxMan->GetEditString( LOBBY_REGISTER_EDIT_MAIL );
	std::string strsa = m_pEditBoxMan->GetEditString( LOBBY_REGISTER_EDIT_SA );
	std::string strsa2 = m_pEditBoxMan->GetEditString( LOBBY_REGISTER_EDIT_SA2 );
	std::string strcaptcha = m_pEditBoxMan->GetEditString( LOBBY_REGISTER_EDIT_CAPTCHA );

	if ( struser.empty() )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_USER_EMPTY" ), NS_UITEXTCOLOR::RED );
		return;
	}

	if ( struser.size() < nMIN_TEXT || struser.size() > nLIMIT_USER )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_USER_SIZE" ), NS_UITEXTCOLOR::RED );
		return;
	}

	if ( strpass.empty() )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_PASS_EMPTY" ), NS_UITEXTCOLOR::RED );
		return;
	}

	if ( strpass.size() < nMIN_TEXT || strpass.size() > nLIMIT_PASS )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_PASS_SIZE" ), NS_UITEXTCOLOR::RED );
		return;
	}

	if (  strcmp( strpass.c_str(), strpass2.c_str() ) != 0 )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_PASS_MISMATCH" ), NS_UITEXTCOLOR::RED );
		return;
	}

	if ( strpin.empty() )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_PASS2_EMPTY" ), NS_UITEXTCOLOR::RED );
		return;
	}

	if ( strpin.size() < nMIN_TEXT || strpin.size() > nLIMIT_PASS )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_PASS2_SIZE" ), NS_UITEXTCOLOR::RED );
		return;
	}

	if ( strcmp( strpin.c_str(), strpin2.c_str() ) != 0 )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_PASS2_MISMATCH" ), NS_UITEXTCOLOR::RED );
		return;
	}

	if ( strmail.empty() )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_MAIL_EMPTY" ), NS_UITEXTCOLOR::RED );
		return;
	}

	if ( strmail.size() < nMIN_TEXT || strmail.size() > nLIMIT_MAIL )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_MAIL_SIZE" ), NS_UITEXTCOLOR::RED );
		return;
	}


	if ( strsa.empty() )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_SA_EMPTY" ), NS_UITEXTCOLOR::RED );
		return;
	}

	if ( strsa.size() < nMIN_TEXT || strsa.size() > nLIMIT_SA )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_SA_SIZE" ), NS_UITEXTCOLOR::RED );
		return;
	}

	if ( strcmp( strsa.c_str(), strsa2.c_str() ) != 0 )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_SA_MISMATCH" ), NS_UITEXTCOLOR::RED );
		return;
	}

	if ( strcaptcha.empty() )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_CAPTCHA_EMPTY" ), NS_UITEXTCOLOR::RED );
		return;
	}

	if ( strcaptcha.size() < nMIN_TEXT || strcaptcha.size() > nLIMIT_CAPTCHA )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_CAPTCHA_SIZE" ), NS_UITEXTCOLOR::RED );
		return;
	}

	if ( strcmp( strcaptcha.c_str(), m_strCaptcha.c_str() ) != 0 )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_CAPTCHA_MISMATCH" ), NS_UITEXTCOLOR::RED );
		return;
	}


	if ( !CheckString ( struser.c_str() ) )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_USER_STRING" ), NS_UITEXTCOLOR::RED );
		return ;
	}

	if ( !CheckString ( strpass.c_str() ) )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_PASS_STRING" ), NS_UITEXTCOLOR::RED );
		return ;
	}

	if ( !CheckString ( strpin.c_str() ) )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_PASS2_STRING" ), NS_UITEXTCOLOR::RED );
		return ;
	}

	if ( !CheckString ( strsa.c_str() ) )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_SA_STRING" ), NS_UITEXTCOLOR::RED );
		return ;
	}

	if ( !CheckString ( strcaptcha.c_str() ) )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_CAPTCHA_STRING" ), NS_UITEXTCOLOR::RED );
		return ;
	}

	if ( !CheckString_Special2 ( strmail.c_str() ) )
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_MAIL_STRING" ), NS_UITEXTCOLOR::RED );
		return ;
	}


	CNetClient* pNetClient = DxGlobalStage::GetInstance().GetNetClient ();	
	if ( pNetClient && pNetClient->IsOnline() )
	{
		std::string strreg_user = m_pEditBoxMan->GetEditString( LOBBY_REGISTER_EDIT_USER );
		std::string strreg_pass = m_pEditBoxMan->GetEditString( LOBBY_REGISTER_EDIT_PASS );
		std::string strreg_pass2 = m_pEditBoxMan->GetEditString( LOBBY_REGISTER_EDIT_PIN );
		std::string strreg_mail = m_pEditBoxMan->GetEditString( LOBBY_REGISTER_EDIT_MAIL );
		std::string strreg_sa = m_pEditBoxMan->GetEditString( LOBBY_REGISTER_EDIT_SA );

		std::transform ( strreg_user.begin(), strreg_user.end(), strreg_user.begin(), tolower );
		std::transform ( strreg_pass.begin(), strreg_pass.end(), strreg_pass.begin(), tolower );
		std::transform ( strreg_pass2.begin(), strreg_pass2.end(), strreg_pass2.begin(), tolower );
		std::transform ( strreg_mail.begin(), strreg_mail.end(), strreg_mail.begin(), tolower );

		int nServerGroup, nServerChannel;
		COuterInterface::GetInstance().GetConnectServerInfo( nServerGroup, nServerChannel );

		pNetClient->RegisterAction( strreg_user.c_str(), strreg_pass.c_str(), strreg_pass2.c_str(), strreg_mail.c_str(), strreg_sa.c_str(), nServerChannel );
	}
	else
	{
		m_pTextStatus->SetText( ID2GAMEINTEXT( "RAN_REGISTER_FB_ERROR_SERVER" ), NS_UITEXTCOLOR::RED );
		EndRegister();
	}
}

void CRegisterPage::EndRegister()
{
	CNetClient* pNetClient = DxGlobalStage::GetInstance().GetNetClient();
	if( pNetClient->IsOnline() == true )
	{
		pNetClient->CloseConnect();
		COuterInterface::GetInstance().SetCancelToLogin();
	}

	COuterInterface::GetInstance().ToSelectServerPage ( GetWndID () );
}

BOOL CRegisterPage::CheckString( CString strTemp )
{	
	strTemp = strTemp.Trim();
	if( STRUTIL::CheckString( strTemp ) )	return FALSE;
	return TRUE;
}

BOOL CRegisterPage::CheckString_Special2( CString strTemp )
{	
	strTemp = strTemp.Trim();
	if( STRUTIL::CheckString_Special2( strTemp ) )	return FALSE;
	return TRUE;
}

void CRegisterPage::CheckHideStatusPass()
{
	if ( m_pEditBoxMan )
	{
		m_pEditBoxMan->StartEDIT( LOBBY_REGISTER_EDIT_PASS );
		m_pEditBoxMan->SetHide ( LOBBY_REGISTER_EDIT_PASS, !m_bShowPass );
	}

	if ( m_pButtonPass )
	{
		if ( m_bShowPass )
			m_pButtonPass->SetOneLineText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 11 ) );
		else
			m_pButtonPass->SetOneLineText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 10 ) );
	}	
}

void CRegisterPage::CheckHideStatusPass2()
{
	if ( m_pEditBoxMan )
	{
		m_pEditBoxMan->StartEDIT( LOBBY_REGISTER_EDIT_PASS2 );
		m_pEditBoxMan->SetHide ( LOBBY_REGISTER_EDIT_PASS2, !m_bShowPass2 );
	}

	if ( m_pButtonPass2 )
	{
		if ( m_bShowPass2 )
			m_pButtonPass2->SetOneLineText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 11 ) );
		else
			m_pButtonPass2->SetOneLineText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 10 ) );
	}	
}

void CRegisterPage::CheckHideStatusPin()
{
	if ( m_pEditBoxMan )
	{
		m_pEditBoxMan->StartEDIT( LOBBY_REGISTER_EDIT_PIN );
		m_pEditBoxMan->SetHide ( LOBBY_REGISTER_EDIT_PIN, !m_bShowPin );
	}

	if ( m_pButtonPin )
	{
		if ( m_bShowPin )
			m_pButtonPin->SetOneLineText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 11 ) );
		else
			m_pButtonPin->SetOneLineText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 10 ) );
	}	
}

void CRegisterPage::CheckHideStatusPin2()
{
	if ( m_pEditBoxMan )
	{
		m_pEditBoxMan->StartEDIT( LOBBY_REGISTER_EDIT_PIN2 );
		m_pEditBoxMan->SetHide ( LOBBY_REGISTER_EDIT_PIN2, !m_bShowPin2 );
	}

	if ( m_pButtonPin2 )
	{
		if ( m_bShowPin2 )
			m_pButtonPin2->SetOneLineText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 11 ) );
		else
			m_pButtonPin2->SetOneLineText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 10 ) );
	}	
}

void CRegisterPage::CheckHideStatusSA()
{
	if ( m_pEditBoxMan )
	{
		m_pEditBoxMan->StartEDIT( LOBBY_REGISTER_EDIT_SA );
		m_pEditBoxMan->SetHide ( LOBBY_REGISTER_EDIT_SA, !m_bShowSA );
	}

	if ( m_pButtonSA )
	{
		if ( m_bShowSA )
			m_pButtonSA->SetOneLineText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 11 ) );
		else
			m_pButtonSA->SetOneLineText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 10 ) );
	}	
}

void CRegisterPage::CheckHideStatusSA2()
{
	if ( m_pEditBoxMan )
	{
		m_pEditBoxMan->StartEDIT( LOBBY_REGISTER_EDIT_SA2 );
		m_pEditBoxMan->SetHide ( LOBBY_REGISTER_EDIT_SA2, !m_bShowSA2 );
	}

	if ( m_pButtonSA2 )
	{
		if ( m_bShowSA2 )
			m_pButtonSA2->SetOneLineText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 11 ) );
		else
			m_pButtonSA2->SetOneLineText( ID2GAMEWORD ( "RAN_REGISTER_WINDOW", 10 ) );
	}	
}

void CRegisterPage::AddErrorString( std::string strError, DWORD dwColor )
{
	if ( strError.size() == 0 )	return;

	if ( m_pTextStatus )
	{
		m_pTextStatus->ClearText();
		m_pTextStatus->SetText( strError.c_str(), dwColor );
	}
}

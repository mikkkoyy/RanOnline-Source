#include "StdAfx.h"

#include "GameTimeConvertUI.h"


#include "BasicTextButton.h"
#include "GameTextControl.h"
#include "BasicTextBoxEx.h"
#include "UITextControl.h"
#include "BasicLineBoxEx.h"
#include "OuterInterface.h"
#include "ModalWindow.h"
#include "s_NetClient.h"
#include "RANPARAM.h"
#include "DxGlobalStage.h"
#include "DxInputString.h"
#include "DebugSet.h"

#include "../Lib_Engine/Common/StringUtils.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "UIEditBoxMan.h"
#include "UIKeyCheck.h"

/*dmk14 string format test*/
#include "StringFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CGameTimeConvertUI::CGameTimeConvertUI ()
	: nLIMIT_CAPTCHA(6)
	, strCaptcha( "" )
	, pTextBoxCaptcha( NULL )
	, pTextBoxGameTime( NULL )
{
}

CGameTimeConvertUI::~CGameTimeConvertUI ()
{
}

void CGameTimeConvertUI::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );
	const int nBUTTONSIZE = CBasicTextButton::SIZE18;

	CBasicTextBox* pTextBox = NULL;

	CString strFormat; 
	//strFormat = sc::string::format( ID2GAMEWORD("INGAME_WEBFUNCT_GAMETIME", 0 ), 0 ).c_str();		
	pTextBox = CreateStaticControl ( "GAMETIME_TEXTLINE_1", pFont9, NS_UITEXTCOLOR::WHITE, TEXT_ALIGN_LEFT );
	//pTextBox->SetOneLineText ( "Convert your GameTime to GamePoints" );
	pTextBox->SetOneLineText ( ID2GAMEWORD("INGAME_WEBFUNCT_GAMETIME", 0 ) );

	pTextBoxGameTime = CreateStaticControl ( "GAMETIME_TEXTLINE_2", pFont9, NS_UITEXTCOLOR::WHITE, TEXT_ALIGN_LEFT );
	//pTextBoxGameTime->SetOneLineText ( "Current GameTime: " );
	pTextBoxGameTime->SetOneLineText ( ID2GAMEWORD("INGAME_WEBFUNCT_GAMETIME", 1 ) );

	pTextBox = CreateStaticControl ( "GAMETIME_TEXTLINE_3", pFont9, NS_UITEXTCOLOR::WHITE, TEXT_ALIGN_LEFT );
	//pTextBox->SetOneLineText ( "Conversion Rate: 60 GameTime: 1GP" );
	pTextBox->SetOneLineText ( ID2GAMEWORD("INGAME_WEBFUNCT_GAMETIME", 2 ) );

	pTextBox = CreateStaticControl ( "GAMETIME_CAPTCHA", pFont9, NS_UITEXTCOLOR::WHITE, TEXT_ALIGN_RIGHT );
	pTextBox->SetOneLineText ( (char*)ID2GAMEWORD ( "REG_PAGE_IDPW", 5 ) );

	pTextBoxCaptcha = CreateStaticControl ( "GAMETIME_CAPTCHA_DISP", pFont9, NS_UITEXTCOLOR::WHITE, TEXT_ALIGN_CENTER_X );
	{
		CBasicLineBox* pBasicLineBox = new CBasicLineBox;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxQuestList ( "GAMETIME_CAPTCHA_BACK" );
		RegisterControl ( pBasicLineBox );
	}

	{
		CUIEditBoxMan* pEditBoxMan = new CUIEditBoxMan;
		pEditBoxMan->CreateSub ( this, "REG_EDITMAN", UI_FLAG_DEFAULT, SELCHAR_EDIT_MAN );
		pEditBoxMan->CreateEditBox ( SELCHAR_EDIT_CAPTCHA, "GAMETIME_CAPTCHA_EDIT", "REG_CARRAT", TRUE, UINT_MAX, pFont9, nLIMIT_CAPTCHA );
		RegisterControl( pEditBoxMan );
		m_pEditBoxMan = pEditBoxMan;
	}


	CBasicTextButton* pOKButton = new CBasicTextButton;
	pOKButton->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, SELCHAR_BUTTON_OK );
	pOKButton->CreateBaseButton ( "GAMETIME_PAGE_OK", nBUTTONSIZE, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "LOGIN_PAGE_OKCANCEL", 0 ) );
	pOKButton->SetShortcutKey ( DIK_RETURN, DIK_NUMPADENTER );
	RegisterControl ( pOKButton );

	CBasicTextButton* pQuitButton = new CBasicTextButton;
	pQuitButton->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, SELCHAR_BUTTON_CANCEL );
	pQuitButton->CreateBaseButton ( "GAMETIME_PAGE_CANCEL", nBUTTONSIZE, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "LOGIN_PAGE_OKCANCEL", 1 ) );
	pQuitButton->SetShortcutKey ( DIK_ESCAPE );
	RegisterControl ( pQuitButton );
}	

void CGameTimeConvertUI::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{	
	CUIWindow::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	CNetClient* pNetClient = DxGlobalStage::GetInstance().GetNetClient ();
	if ( !pNetClient->IsOnline() )		//???? ??? ??? ??
	{
		if ( !COuterInterface::GetInstance().IsVisibleGroup ( MODAL_WINDOW_OUTER ) )
		{
			DoModalOuter ( ID2GAMEEXTEXT ("CHARACTERSTAGE_2"), MODAL_INFOMATION, OK, OUTER_MODAL_CONNECTCLOSED );
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

void CGameTimeConvertUI::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIWindow::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case SELCHAR_BUTTON_OK:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				SetMessageEx( dwMsg &= ~UIMSG_KEY_FOCUSED ); // Focus ??? ??? ??
				
				std::string strCode = m_pEditBoxMan->GetEditString ( SELCHAR_EDIT_CAPTCHA );

				
				if( strcmp( strCode.c_str(), strCaptcha.c_str() ) != 0 )
				{
					DoModalOuter ( ID2GAMEEXTEXT ( "REG_PAGE_CAPTCHA_ERROR" ) );
					return;
				}
				
				CNetClient* pNetClient = DxGlobalStage::GetInstance().GetNetClient ();

				pNetClient->ReqGameTimeConvert();		

				DoModalOuter ( ID2GAMEEXTEXT ("LOGINSTAGE_6"), MODAL_INFOMATION, CANCEL, OUTER_MODAL_WAITCONFIRM );
				COuterInterface::GetInstance().SetModalCallWindowID( GetWndID() );
			}
		}
		break;

	case SELCHAR_BUTTON_CANCEL:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				ResetAll();
				COuterInterface::GetInstance().ToSelectCharacterPage( GetWndID() );
				COuterInterface::GetInstance().GetGameTimeConvertUI()->ResetAll();
			}
		}
		break;
	}
}

void CGameTimeConvertUI::ResetAll ( bool bEdit )
{
	m_pEditBoxMan->EndEdit ();

	m_pEditBoxMan->ClearEdit ( SELCHAR_EDIT_CAPTCHA );

	pTextBoxCaptcha->ClearText();

	strCaptcha.clear();
	
	WORD wRandomNumber = (WORD)rand() % 32000 + 10001;
	CString strRandom;
	strRandom.Format( "%d", wRandomNumber );
	strCaptcha = strRandom.GetString();

	pTextBoxCaptcha->ClearText();
	pTextBoxCaptcha->SetText( strCaptcha.c_str(), NS_UITEXTCOLOR::ORNAGERED );

	m_pEditBoxMan->Init();
	if( bEdit )	
	{
		const DWORD dwGameTime = COuterInterface::GetInstance().GetGameTime();

		CString strFormat;
		strFormat.Format( ID2GAMEWORD("INGAME_WEBFUNCT_GAMETIME", 1 ), dwGameTime );
		pTextBoxGameTime->SetOneLineText ( strFormat );

		m_pEditBoxMan->BeginEdit();
		COuterInterface::GetInstance().SetModalCallWindowID( GetWndID() );
	}

	if( m_pEditBoxMan->IsMODE_NATIVE() )
	{
		m_pEditBoxMan->DoMODE_TOGGLE();
	}
}

CBasicButton* CGameTimeConvertUI::CreateFlipButton ( char* szButton, char* szButtonFlip, UIGUID ControlID )
{
	CBasicButton* pButton = new CBasicButton;
	pButton->CreateSub ( this, szButton, UI_FLAG_DEFAULT, ControlID );
	pButton->CreateFlip ( szButtonFlip, CBasicButton::RADIO_FLIP );
	pButton->SetControlNameEx ( szButton );
	RegisterControl ( pButton );

	return pButton;
}

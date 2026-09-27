#include "StdAfx.h"

#include "ForgotPassPage.h"


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

#include "LoginPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int	CForgotPassPage::nLIMIT_ID = 12;
int	CForgotPassPage::nLIMIT_PW = 19;

CForgotPassPage::CForgotPassPage ()
	: nLIMIT_CAPTCHA(6)
	, strCaptcha( "" )
	, pTextBoxCaptcha( NULL )
{
}

CForgotPassPage::~CForgotPassPage ()
{
}

void CForgotPassPage::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

	const int nBUTTONSIZE = CBasicTextButton::SIZE18;

	CBasicTextBox* pTextBox = NULL;
	pTextBox = CreateStaticControl ( "REG_PAGE_ID", pFont9, NS_UITEXTCOLOR::WHITE, TEXT_ALIGN_RIGHT );
	pTextBox->SetOneLineText ( (char*)ID2GAMEWORD ( "REG_PAGE_IDPW", 0 ) );
	pTextBox = CreateStaticControl ( "REG_PAGE_PW", pFont9, NS_UITEXTCOLOR::WHITE, TEXT_ALIGN_RIGHT );
	pTextBox->SetOneLineText ( (char*)ID2GAMEWORD ( "REG_PAGE_IDPW", 3 ) );
	
	pTextBox = CreateStaticControl ( "FORGOTPASS_PAGE_CAPTCHA", pFont9, NS_UITEXTCOLOR::WHITE, TEXT_ALIGN_RIGHT );
	pTextBox->SetOneLineText ( (char*)ID2GAMEWORD ( "REG_PAGE_IDPW", 5 ) );

	pTextBoxCaptcha = CreateStaticControl ( "FORGOTPASS_PAGE_CAPTCHA_DISP", pFont9, NS_UITEXTCOLOR::WHITE, TEXT_ALIGN_CENTER_X );
	{
		CBasicLineBoxEx* pBasicLineBox = new CBasicLineBoxEx;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxOuter ( "REG_PAGE_ID_BACK" );
		RegisterControl ( pBasicLineBox );

		pBasicLineBox = new CBasicLineBoxEx;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxOuter ( "REG_PAGE_PW_BACK" );
		RegisterControl ( pBasicLineBox );

		pBasicLineBox = new CBasicLineBoxEx;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxOuter ( "FORGOTPASS_PAGE_CAPTCHA_BACK" );
		RegisterControl ( pBasicLineBox );
	}

	{
		CUIEditBoxMan* pEditBoxMan = new CUIEditBoxMan;
		pEditBoxMan->CreateSub ( this, "REG_EDITMAN", UI_FLAG_DEFAULT, FORGOT_EDIT_MAN );
		pEditBoxMan->CreateEditBox ( FORGOT_EDIT_ID, "REG_EDIT_ID", "REG_CARRAT", TRUE, UINT_MAX, pFont9, nLIMIT_ID );
		pEditBoxMan->CreateEditBox ( FORGOT_EDIT_PW, "REG_EDIT_PW", "REG_CARRAT", TRUE, UINT_MAX, pFont9, nLIMIT_PW );
		pEditBoxMan->CreateEditBox ( FORGOT_EDIT_CAPTCHA, "FORGOTPASS_EDIT_CAPTCHA", "REG_CARRAT", TRUE, UINT_MAX, pFont9, nLIMIT_CAPTCHA );
		pEditBoxMan->SetHide ( FORGOT_EDIT_PW, TRUE );
		
		RegisterControl( pEditBoxMan );
		m_pEditBoxMan = pEditBoxMan;
	}


	CBasicTextButton* pOKButton = new CBasicTextButton;
	pOKButton->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, FORGOT_OK );
	pOKButton->CreateBaseButton ( "REG_PAGE_OK", nBUTTONSIZE, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "LOGIN_PAGE_OKCANCEL", 0 ) );
	pOKButton->SetShortcutKey ( DIK_RETURN, DIK_NUMPADENTER );
	RegisterControl ( pOKButton );

	CBasicTextButton* pQuitButton = new CBasicTextButton;
	pQuitButton->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, FORGOT_CANCEL );
	pQuitButton->CreateBaseButton ( "REG_PAGE_CANCEL", nBUTTONSIZE, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "LOGIN_PAGE_OKCANCEL", 1 ) );
	pQuitButton->SetShortcutKey ( DIK_ESCAPE );
	RegisterControl ( pQuitButton );
}	

void CForgotPassPage::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{	
	CUIWindow::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	CNetClient* pNetClient = DxGlobalStage::GetInstance().GetNetClient ();
	if ( !pNetClient->IsOnline() )		//네트워크 연결이 끊어진 경우
	{
		if ( !COuterInterface::GetInstance().IsVisibleGroup ( MODAL_WINDOW_OUTER ) )
		{
			DoModalOuter ( ID2GAMEEXTEXT ("LOGINSTAGE_1"), MODAL_INFOMATION, OK, OUTER_MODAL_RECONNECT );
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

void CForgotPassPage::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIWindow::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case FORGOT_OK:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				SetMessageEx( dwMsg &= ~UIMSG_KEY_FOCUSED ); // Focus 문제로 메세지 삭제
				
				//CString strID = m_pEditBoxMan->GetEditString ( FORGOT_EDIT_ID );
				//CString strPW = m_pEditBoxMan->GetEditString ( FORGOT_EDIT_PW );
				//CString strCode = m_pEditBoxMan->GetEditString ( FORGOT_EDIT_CAPTCHA );
				
				std::string strID = m_pEditBoxMan->GetEditString ( FORGOT_EDIT_ID );
				std::string strPW = m_pEditBoxMan->GetEditString ( FORGOT_EDIT_PW );

				std::string strCode = m_pEditBoxMan->GetEditString ( FORGOT_EDIT_CAPTCHA );

				if ( strID.empty () || strPW.empty () )
				{
					DoModalOuter ( ID2GAMEEXTEXT ( "REG_STAGE_0" ) );
					return ;
				}

				if ( !CheckString ( strID.c_str() ) )
				{
					DoModalOuter ( ID2GAMEEXTEXT ( "REG_PAGE_ID_ERROR" ) );
					return ;
				}

				if ( !CheckString ( strPW.c_str() ) )
				{
					DoModalOuter ( ID2GAMEEXTEXT ( "REG_PAGE_PW_ERROR" ) );
					return ;
				}

				if( strcmp( strCode.c_str(), strCaptcha.c_str() ) != 0 )
				{
					DoModalOuter ( ID2GAMEEXTEXT ( "REG_PAGE_CAPTCHA_ERROR" ) );
					return;
				}

				CNetClient* pNetClient = DxGlobalStage::GetInstance().GetNetClient ();

				pNetClient->SndForgotPass( strID.c_str(), strPW.c_str() );		

				DoModalOuter ( ID2GAMEEXTEXT ("LOGINSTAGE_6"), MODAL_INFOMATION, CANCEL, OUTER_MODAL_WAITCONFIRM );
				COuterInterface::GetInstance().SetModalCallWindowID( GetWndID() );
			}
		}
		break;

	case FORGOT_CANCEL:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				ResetAll();
				COuterInterface::GetInstance().ToSelectServerPage ( GetWndID () );
				COuterInterface::GetInstance().GetLoginPage()->ResetAll();
			}
		}
		break;
	}
}

BOOL CForgotPassPage::CheckString( CString strTemp )
{	
	strTemp = strTemp.Trim();

	// 문자열 체크 - 들어가면 안되는 특수문자 : ~!@#$%^&*+|":?><\=`',.;[]{}()
	if( STRUTIL::CheckString( strTemp ) )
	{
		DoModalOuter ( ID2GAMEEXTEXT ( "LOGINSTAGE_4" ), MODAL_INFOMATION, OK );		
		return FALSE;
	}

    return TRUE;
}

void CForgotPassPage::ResetAll ( bool bEdit )
{
	m_pEditBoxMan->EndEdit ();

	m_pEditBoxMan->ClearEdit ( FORGOT_EDIT_ID );
	m_pEditBoxMan->ClearEdit ( FORGOT_EDIT_PW );
	m_pEditBoxMan->ClearEdit ( FORGOT_EDIT_CAPTCHA );

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
		m_pEditBoxMan->BeginEdit();
		COuterInterface::GetInstance().SetModalCallWindowID( GetWndID() );
	}

	if( m_pEditBoxMan->IsMODE_NATIVE() )
	{
		m_pEditBoxMan->DoMODE_TOGGLE();
	}
}

CBasicButton* CForgotPassPage::CreateFlipButton ( char* szButton, char* szButtonFlip, UIGUID ControlID )
{
	CBasicButton* pButton = new CBasicButton;
	pButton->CreateSub ( this, szButton, UI_FLAG_DEFAULT, ControlID );
	pButton->CreateFlip ( szButtonFlip, CBasicButton::RADIO_FLIP );
	pButton->SetControlNameEx ( szButton );
	RegisterControl ( pButton );

	return pButton;
}

CString CForgotPassPage::GetUser()
{
	return m_pEditBoxMan->GetEditString ( FORGOT_EDIT_ID );
}

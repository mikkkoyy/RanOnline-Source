#include "StdAfx.h"

#include "SelectCharacterChangePin.h"


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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int	CSelectCharacterChangePin::nLIMIT_ID = 11;
int	CSelectCharacterChangePin::nLIMIT_PW = 11;

CSelectCharacterChangePin::CSelectCharacterChangePin ()
	: nLIMIT_CAPTCHA(6)
	, strCaptcha( "" )
	, pTextBoxCaptcha( NULL )
	, pTextBoxLvlPin ( NULL )
{
}

CSelectCharacterChangePin::~CSelectCharacterChangePin ()
{
}

void CSelectCharacterChangePin::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );
	const int nBUTTONSIZE = CBasicTextButton::SIZE18;

	CBasicTextBox* pTextBox = NULL;
	pTextBox = CreateStaticControl ( "CHARPAGE_CHANGE_PASS_PW", pFont9, NS_UITEXTCOLOR::WHITE, TEXT_ALIGN_RIGHT );
	pTextBox->SetOneLineText ( "Password" );
	pTextBox = CreateStaticControl ( "CHARPAGE_CHANGE_PASS_PIN", pFont9, NS_UITEXTCOLOR::WHITE, TEXT_ALIGN_RIGHT );
	pTextBox->SetOneLineText ( "Pincode" );
	pTextBox = CreateStaticControl ( "CHARPAGE_CHANGE_PASS_NEWPW", pFont9, NS_UITEXTCOLOR::WHITE, TEXT_ALIGN_RIGHT );
	pTextBox->SetOneLineText ( "New-Pincode" );
	pTextBox = CreateStaticControl ( "CHARPAGE_CHANGE_PASS_NEWPW2", pFont9, NS_UITEXTCOLOR::WHITE, TEXT_ALIGN_RIGHT );
	pTextBox->SetOneLineText ( "Repeat" );
	
	pTextBox = CreateStaticControl ( "CHARPAGE_CHANGE_CAPTCHA", pFont9, NS_UITEXTCOLOR::WHITE, TEXT_ALIGN_RIGHT );
	pTextBox->SetOneLineText ( (char*)ID2GAMEWORD ( "REG_PAGE_IDPW", 5 ) );
	pTextBoxCaptcha = CreateStaticControl ( "CHARPAGE_CHANGE_CAPTCHA_DISP", pFont9, NS_UITEXTCOLOR::WHITE, TEXT_ALIGN_CENTER_X );
	pTextBoxLvlPin = CreateStaticControl ( "REG_PAGE_PW_LVL", pFont9, NS_UITEXTCOLOR::WHITE, TEXT_ALIGN_CENTER_X );
	{
		CBasicLineBox* pBasicLineBox = new CBasicLineBox;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxQuestList ( "CHARPAGE_CHANGE_PASS_PW_BACK" );
		RegisterControl ( pBasicLineBox );

		pBasicLineBox = new CBasicLineBox;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxQuestList ( "CHARPAGE_CHANGE_PASS_PIN_BACK" );
		RegisterControl ( pBasicLineBox );

		pBasicLineBox = new CBasicLineBox;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxQuestList ( "CHARPAGE_CHANGE_PASS_NEWPW_BACK" );
		RegisterControl ( pBasicLineBox );

		pBasicLineBox = new CBasicLineBox;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxQuestList ( "CHARPAGE_CHANGE_PASS_NEWPW2_BACK" );
		RegisterControl ( pBasicLineBox );

		pBasicLineBox = new CBasicLineBox;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxQuestList ( "CHARPAGE_CHANGE_CAPTCHA_BACK" );
		RegisterControl ( pBasicLineBox );
	}

	{
		CUIEditBoxMan* pEditBoxMan = new CUIEditBoxMan;
		pEditBoxMan->CreateSub ( this, "REG_EDITMAN", UI_FLAG_DEFAULT, SELCHAR_EDIT_MAN );
		pEditBoxMan->CreateEditBox ( SELCHAR_EDIT_PW, "CHARPAGE_CHANGE_PASS_PW_EDIT", "REG_CARRAT", TRUE, UINT_MAX, pFont9, nLIMIT_PW );		//fix change pass
		pEditBoxMan->CreateEditBox ( SELCHAR_EDIT_PIN, "CHARPAGE_CHANGE_PASS_PIN_EDIT", "REG_CARRAT", TRUE, UINT_MAX, pFont9, nLIMIT_PW );
		pEditBoxMan->CreateEditBox ( SELCHAR_EDIT_NEWPIN, "CHARPAGE_CHANGE_PASS_NEWPW_EDIT", "REG_CARRAT", TRUE, UINT_MAX, pFont9, nLIMIT_PW );
		pEditBoxMan->CreateEditBox ( SELCHAR_EDIT_NEWPIN2, "CHARPAGE_CHANGE_PASS_NEWPW2_EDIT", "REG_CARRAT", TRUE, UINT_MAX, pFont9, nLIMIT_PW );
		pEditBoxMan->CreateEditBox ( SELCHAR_EDIT_CAPTCHA, "CHARPAGE_CHANGE_CAPTCHA_EDIT", "REG_CARRAT", TRUE, UINT_MAX, pFont9, nLIMIT_CAPTCHA );
		pEditBoxMan->SetHide ( SELCHAR_EDIT_PW, TRUE );
		pEditBoxMan->SetHide ( SELCHAR_EDIT_PIN, TRUE );
		pEditBoxMan->SetHide ( SELCHAR_EDIT_NEWPIN, TRUE );
		pEditBoxMan->SetHide ( SELCHAR_EDIT_NEWPIN2, TRUE );
		RegisterControl( pEditBoxMan );
		m_pEditBoxMan = pEditBoxMan;
	}


	CBasicTextButton* pOKButton = new CBasicTextButton;
	pOKButton->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, SELCHAR_BUTTON_OK );
	pOKButton->CreateBaseButton ( "REG_PAGE_OK", nBUTTONSIZE, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "LOGIN_PAGE_OKCANCEL", 0 ) );
	pOKButton->SetShortcutKey ( DIK_RETURN, DIK_NUMPADENTER );
	RegisterControl ( pOKButton );

	CBasicTextButton* pQuitButton = new CBasicTextButton;
	pQuitButton->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, SELCHAR_BUTTON_CANCEL );
	pQuitButton->CreateBaseButton ( "REG_PAGE_CANCEL", nBUTTONSIZE, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "LOGIN_PAGE_OKCANCEL", 1 ) );
	pQuitButton->SetShortcutKey ( DIK_ESCAPE );
	RegisterControl ( pQuitButton );
}	

void CSelectCharacterChangePin::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
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

void CSelectCharacterChangePin::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIWindow::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case SELCHAR_BUTTON_OK:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				SetMessageEx( dwMsg &= ~UIMSG_KEY_FOCUSED ); // Focus ??? ??? ??
				
				std::string strPW = m_pEditBoxMan->GetEditString ( SELCHAR_EDIT_PW );
				std::string strPIN = m_pEditBoxMan->GetEditString ( SELCHAR_EDIT_PIN );
				std::string strNEWPW = m_pEditBoxMan->GetEditString ( SELCHAR_EDIT_NEWPIN );
				std::string strNEWPW2 = m_pEditBoxMan->GetEditString ( SELCHAR_EDIT_NEWPIN2 );

				std::string strCode = m_pEditBoxMan->GetEditString ( SELCHAR_EDIT_CAPTCHA );

				if ( strPW.empty() || strPIN.empty() || strNEWPW.empty() || strNEWPW2.empty() )
				{
					DoModalOuter ( ID2GAMEEXTEXT ( "REG_STAGE_0" ) );
					return ;
				}

				if ( !CheckString ( strPW.c_str() ) || !CheckString ( strPIN.c_str() ) ||
					 !CheckString ( strNEWPW.c_str() ) || !CheckString ( strNEWPW2.c_str() ) )
				{
					DoModalOuter ( ID2GAMEEXTEXT ( "REG_PAGE_PW_ERROR" ) );
					return ;
				}

				if( strcmp( strNEWPW.c_str(), strNEWPW2.c_str() ) != 0 )
				{
					DoModalOuter ( ID2GAMEEXTEXT ( "REG_PAGE_PASS_ERROR" ) );
					return;
				}
								
				if( strcmp( strCode.c_str(), strCaptcha.c_str() ) != 0 )
				{
					DoModalOuter ( ID2GAMEEXTEXT ( "REG_PAGE_CAPTCHA_ERROR" ) );
					return;
				}

				if( strNEWPW.size() < 6 )
				{
					DoModalOuter ( "Password length must be 6 characters long" );
					return;
				}

				CNetClient* pNetClient = DxGlobalStage::GetInstance().GetNetClient ();

				pNetClient->SndAccountChangePin( strPW.c_str(), strPIN.c_str(), strNEWPW.c_str() );		

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
				COuterInterface::GetInstance().GetSelectCharacterChangePin()->ResetAll();
			}
		}
		break;
	}
}

void CSelectCharacterChangePin::ResetAll ( bool bEdit )
{
	m_pEditBoxMan->EndEdit ();

	m_pEditBoxMan->ClearEdit ( SELCHAR_EDIT_PW );
	m_pEditBoxMan->ClearEdit ( SELCHAR_EDIT_PIN );
	m_pEditBoxMan->ClearEdit ( SELCHAR_EDIT_NEWPIN );
	m_pEditBoxMan->ClearEdit ( SELCHAR_EDIT_NEWPIN2 );
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
		m_pEditBoxMan->BeginEdit();
		COuterInterface::GetInstance().SetModalCallWindowID( GetWndID() );
	}

	if( m_pEditBoxMan->IsMODE_NATIVE() )
	{
		m_pEditBoxMan->DoMODE_TOGGLE();
	}
}

CBasicButton* CSelectCharacterChangePin::CreateFlipButton ( char* szButton, char* szButtonFlip, UIGUID ControlID )
{
	CBasicButton* pButton = new CBasicButton;
	pButton->CreateSub ( this, szButton, UI_FLAG_DEFAULT, ControlID );
	pButton->CreateFlip ( szButtonFlip, CBasicButton::RADIO_FLIP );
	pButton->SetControlNameEx ( szButton );
	RegisterControl ( pButton );

	return pButton;
}

BOOL CSelectCharacterChangePin::CheckString( CString strTemp )
{	
	strTemp = strTemp.Trim();
	if( STRUTIL::CheckString( strTemp ) )	return FALSE;
	return TRUE;
}
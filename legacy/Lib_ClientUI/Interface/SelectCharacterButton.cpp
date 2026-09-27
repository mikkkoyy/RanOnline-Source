#include "StdAfx.h"
#include "SelectCharacterButton.h"

#include "d3dfont.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#include "OuterInterface.h"
#include "DxGlobalStage.h"
#include "ModalWindow.h"
#include "RANPARAM.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicTextButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSelectCharacterButton::CSelectCharacterButton ()
	: m_pDelCharInfo(NULL)
	, m_nCharRemainLast(-1)
	, m_nServerGroup(-1)
	, m_nServerChannel(-1)
	, m_pNewButton(NULL)
	, m_pDeleteButton(NULL)
	, m_pChangeServerButton(NULL)
	, m_pGameStartButton(NULL)
	, m_pServerName(NULL)
	/*dmk14 ingame web*/
	, m_pChangePass(NULL)
	, m_pChangePin(NULL)
	, m_pGameTimeClaim(NULL)
	, m_pResetPin(NULL)
	, m_pTopUp(NULL)
	, m_pChangeMail(NULL)
	, m_pPassStatic(NULL)
	, m_pPinStatic(NULL)
	, m_pGameTimeStatic(NULL)
	, m_pPremiumPointStatic(NULL)
	, m_pEmailStatic(NULL)
	, m_pNameStatic(NULL)
{
	
}

CSelectCharacterButton::~CSelectCharacterButton ()
{
}

void CSelectCharacterButton::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );
	CD3DFontPar* pFont10 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 10, _DEFAULT_FONT_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "SELECT_CHAR_WINDOW_CHAR_BUTTON_BACK" );
	RegisterControl ( pBasicLineBox );	

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "SELECT_CHAR_WINDOW_CHAR_BUTTON_SERVER_INFO_BACK" );
	RegisterControl ( pBasicLineBox );	

	m_pNewButton = new CBasicTextButton;
	m_pNewButton->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, SELECT_CHARACTER_NEW );
	m_pNewButton->CreateBaseButton ( "SELECT_CHAR_WINDOW_CHAR_BUTTON_BUTTON_NEW", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "SELECT_CHARACTER_PAGE_BUTTON", 0 ) );		
	RegisterControl ( m_pNewButton );

	m_pDeleteButton = new CBasicTextButton;
	m_pDeleteButton->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, SELECT_CHARACTER_DELETE );
	m_pDeleteButton->CreateBaseButton ( "SELECT_CHAR_WINDOW_CHAR_BUTTON_BUTTON_DELETE", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "SELECT_CHARACTER_PAGE_BUTTON", 2 ) );
	m_pDeleteButton->SetShortcutKey ( DIK_DELETE );
	RegisterControl ( m_pDeleteButton );

	m_pChangeServerButton = new CBasicTextButton;
	m_pChangeServerButton->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, SELECT_CHARACTER_CHANGESERVER );
	m_pChangeServerButton->CreateBaseButton ( "SELECT_CHAR_WINDOW_CHAR_BUTTON_BUTTON_SERVER", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "SELECT_CHARACTER_PAGE_BUTTON", 3 ) );		
	RegisterControl ( m_pChangeServerButton );

	m_pGameStartButton = new CBasicTextButton;
	m_pGameStartButton->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, SELECT_CHARACTER_GAMESTART );
	m_pGameStartButton->CreateBaseButton ( "SELECT_CHAR_WINDOW_CHAR_BUTTON_BUTTON_GAME_START", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "SELECT_CHARACTER_PAGE_BUTTON", 1 ) );		
	RegisterControl ( m_pGameStartButton );

	m_pServerName = new CBasicTextBox;
	m_pServerName->CreateSub ( this, "SELECT_CHAR_WINDOW_CHAR_BUTTON_SERVER_TEXT" );
	m_pServerName->SetFont ( pFont9 );
	m_pServerName->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );	
	RegisterControl ( m_pServerName );

	/*dmk14 ingame web*/
	{
		m_pNameStatic = new CBasicTextBox;
		m_pNameStatic->CreateSub ( this, "SELECT_CHARACTER_PAGE_SERVER_CHANEL", UI_FLAG_DEFAULT );
		m_pNameStatic->SetFont ( pFont9 );
		m_pNameStatic->AddText( ID2GAMEWORD ( "CHARACTER_SELECT_WEB_MENU", 0 ), NS_UITEXTCOLOR::WHITE );
		m_pNameStatic->SetTextAlign ( TEXT_ALIGN_LEFT );		
		RegisterControl ( m_pNameStatic );

		m_pPassStatic = new CBasicTextBox;
		m_pPassStatic->CreateSub ( this, "SELECT_CHARACTER_PAGE_PASS_LABEL", UI_FLAG_DEFAULT );
		m_pPassStatic->SetFont ( pFont9 );
		m_pPassStatic->AddText( ID2GAMEWORD ( "CHARACTER_SELECT_WEB_MENU", 1 ), NS_UITEXTCOLOR::WHITE );
		m_pPassStatic->SetTextAlign ( TEXT_ALIGN_LEFT );		
		RegisterControl ( m_pPassStatic );

		m_pPinStatic = new CBasicTextBox;
		m_pPinStatic->CreateSub ( this, "SELECT_CHARACTER_PAGE_PIN_LABEL", UI_FLAG_DEFAULT );
		m_pPinStatic->SetFont ( pFont9 );
		m_pPinStatic->AddText( ID2GAMEWORD ( "CHARACTER_SELECT_WEB_MENU", 2 ), NS_UITEXTCOLOR::WHITE );
		m_pPinStatic->SetTextAlign ( TEXT_ALIGN_LEFT );		
		RegisterControl ( m_pPinStatic );

		m_pGameTimeStatic = new CBasicTextBox;
		m_pGameTimeStatic->CreateSub ( this, "SELECT_CHARACTER_PAGE_GAMETIME_LABEL", UI_FLAG_DEFAULT );
		m_pGameTimeStatic->SetFont ( pFont9 );
		m_pGameTimeStatic->SetTextAlign ( TEXT_ALIGN_LEFT );		
		RegisterControl ( m_pGameTimeStatic );

		m_pPremiumPointStatic = new CBasicTextBox;
		m_pPremiumPointStatic->CreateSub ( this, "SELECT_CHARACTER_PAGE_PREIUMPOINT_LABEL", UI_FLAG_DEFAULT );
		m_pPremiumPointStatic->SetFont ( pFont9 );
		m_pPremiumPointStatic->SetTextAlign ( TEXT_ALIGN_LEFT );		
		RegisterControl ( m_pPremiumPointStatic );

		m_pEmailStatic = new CBasicTextBox;
		m_pEmailStatic->CreateSub ( this, "SELECT_CHARACTER_PAGE_EMAIL", UI_FLAG_DEFAULT );
		m_pEmailStatic->SetFont ( pFont9 );
		m_pEmailStatic->SetTextAlign ( TEXT_ALIGN_LEFT );		
		RegisterControl ( m_pEmailStatic );

		m_pChangePass = new CBasicTextButton;
		m_pChangePass->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, SELECT_CHARACTER_CHANGEPASS );
		m_pChangePass->CreateBaseButton ( "SELECT_CHARACTER_CHANGEPASS", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "CHARACTER_SELECT_WEB_MENU", 5 ));
		RegisterControl ( m_pChangePass );

		m_pChangePin = new CBasicTextButton;
		m_pChangePin->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, SELECT_CHARACTER_CHANGEPIN );
		m_pChangePin->CreateBaseButton ( "SELECT_CHARACTER_CHANGEPIN", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "CHARACTER_SELECT_WEB_MENU", 5 ) );
		RegisterControl ( m_pChangePin );

		m_pResetPin = new CBasicTextButton;
		m_pResetPin->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, SELECT_CHARACTER_RESETPIN );
		m_pResetPin->CreateBaseButton ( "SELECT_CHARACTER_RESETPIN", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "CHARACTER_SELECT_WEB_MENU", 6 ) );
		RegisterControl ( m_pResetPin );

		m_pGameTimeClaim = new CBasicTextButton;
		m_pGameTimeClaim->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, SELECT_CHARACTER_CLAIM_GAMETIME );
		m_pGameTimeClaim->CreateBaseButton ( "SELECT_CHARACTER_CLAIM_GAMETIME", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "CHARACTER_SELECT_WEB_MENU", 7 ) );
		RegisterControl ( m_pGameTimeClaim );

		m_pTopUp = new CBasicTextButton;
		m_pTopUp->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, SELECT_CHARACTER_TOPUP );
		m_pTopUp->CreateBaseButton ( "SELECT_CHARACTER_TOPUP", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "CHARACTER_SELECT_WEB_MENU", 8 ) );
		RegisterControl ( m_pTopUp );

		m_pChangeMail = new CBasicTextButton;
		m_pChangeMail->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, SELECT_CHARACTER_CHANGEMAIL );
		m_pChangeMail->CreateBaseButton ( "SELECT_CHARACTER_EMAILCHANGE", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "CHARACTER_SELECT_WEB_MENU", 5 ) );
		RegisterControl ( m_pChangeMail );
	}
}

void CSelectCharacterButton::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case SELECT_CHARACTER_NEW:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				const USHORT uCharRemain = COuterInterface::GetInstance().GetCharRemain ();
				const int nChaSNum = DxGlobalStage::GetInstance().GetLobyStage()->GetChaSNum ();

				if ( MAX_ONESERVERCHAR_NUM <= nChaSNum )
				{
					DoModalOuter ( ID2GAMEEXTEXT ("CHARACTER_THIS_SERVER_SLOT_FULL" ), MODAL_INFOMATION );
					break;
				}

				if ( !uCharRemain )
				{
					DoModalOuter ( ID2GAMEEXTEXT ("CHARACTER_NOT_ENOUGH_CHARREMAIN" ), MODAL_INFOMATION );
					break;
				}

				COuterInterface::GetInstance().ToCreateCharacterPage ( GetWndID () );
			}
		}break;

	case SELECT_CHARACTER_DELETE:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				m_pDelCharInfo = DxGlobalStage::GetInstance().GetLobyStage()->GetSelectCharInfo();
				if ( m_pDelCharInfo )
				{
					/*character delete, Juver, 2017/11/18 */
					if ( RANPARAM::bFeatureCharacterDelete )
					{
						CString strTemp;	
						strTemp = COuterInterface::GetInstance().MakeString ( ID2GAMEEXTEXT ("CHARACTERSTAGE_CAUTION_DELETE"), m_pDelCharInfo->m_szName );
						DoModalOuter ( strTemp.GetString(), MODAL_INPUT, EDITBOX, OUTER_MODAL_SECONDPASSWORD, TRUE );
					}
					else
					{
						CString strTemp;					

						switch ( RANPARAM::emSERVICE_TYPE )
						{
						case EMSERVICE_DEFAULT:
						case EMSERVICE_INDONESIA:
						case EMSERVICE_FEYA:
						case EMSERVICE_THAILAND:
						case EMSERVICE_CHINA:
							strTemp = COuterInterface::GetInstance().MakeString ( ID2GAMEEXTEXT ("CHARACTERSTAGE_CAUTION_DELETE"), m_pDelCharInfo->m_szName );
							DoModalOuter ( strTemp.GetString(), MODAL_INPUT, EDITBOX, OUTER_MODAL_SECONDPASSWORD, TRUE );
							break;

						default:
							strTemp = COuterInterface::GetInstance().MakeString ( ID2GAMEEXTEXT ("CHARACTERSTAGE_CAUTION_DELETE_DAUM"), m_pDelCharInfo->m_szName );
							DoModalOuter ( strTemp.GetString(), MODAL_QUESTION, YESNO, OUTER_MODAL_SECONDPASSWORD );
							break;
						};
					}
				}
				else
				{
					DoModalOuter ( ID2GAMEEXTEXT ("CHARACTERSTAGE_NOT_SELECTED"), MODAL_INFOMATION, OK );
				}
			}
		}break;

	case SELECT_CHARACTER_CHANGESERVER:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				DoModalOuter ( ID2GAMEEXTEXT ("CHARACTERSTAGE_6"), MODAL_INFOMATION, OKCANCEL, OUTER_MODAL_LOGOUT );
			}
		}break;

	case SELECT_CHARACTER_GAMESTART:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				SCHARINFO_LOBBY* pCharInfo = DxGlobalStage::GetInstance().GetLobyStage()->GetSelectCharInfo();
				if ( pCharInfo )
				{
					DxGlobalStage::GetInstance().GetNetClient()->SndGameJoin ( pCharInfo->m_dwCharID );
					COuterInterface::GetInstance().START_WAIT_TIME ( 60.0f );
					DoModalOuter ( ID2GAMEEXTEXT ("CHARACTERSTAGE_GAME_JOIN"), MODAL_INFOMATION, CANCEL, OUTER_MODAL_CHARACTERSTAGE_GAME_JOIN_WAIT );
				}else{
					DoModalOuter ( ID2GAMEEXTEXT ("CHARACTERSTAGE_8"), MODAL_INFOMATION, OK );
				}
			}
		}break;
		/*dm1k4 ingame web*/
	case SELECT_CHARACTER_CHANGEPASS:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				COuterInterface::GetInstance().ToSelectCharacterChangePass( GetWndID() );
			}
		}
		break;
	case SELECT_CHARACTER_CHANGEPIN:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				COuterInterface::GetInstance().ToSelectCharacterChangePin( GetWndID() );
			}
		}
		break;
	case SELECT_CHARACTER_CHANGEMAIL:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				COuterInterface::GetInstance().ToSelectCharacterChangeMail( GetWndID() );
			}
		}
		break;
	case SELECT_CHARACTER_RESETPIN:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				COuterInterface::GetInstance().ToSelectCharacterResetPin( GetWndID() );
			}
		}
		break;
	case SELECT_CHARACTER_TOPUP:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				COuterInterface::GetInstance().ToSelectCharacterTopUp( GetWndID() );
			}
		}
		break;
	case SELECT_CHARACTER_CLAIM_GAMETIME:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				COuterInterface::GetInstance().ToSelectCharacterGameTimeConvertUI( GetWndID() );
			}
		}
		break;
	}
}

void CSelectCharacterButton::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	const USHORT uCharRemain = COuterInterface::GetInstance().GetCharRemain();
	const DWORD dwGameTime = COuterInterface::GetInstance().GetGameTime();
	const DWORD dwEpoints = COuterInterface::GetInstance().GetPremiumPoint();

	if ( m_nCharRemainLast != uCharRemain )
	{
		m_nCharRemainLast = uCharRemain;
		if ( m_pNewButton )
		{
			CString strTemp;
			strTemp.Format( "%s(%d)", ID2GAMEWORD ( "SELECT_CHARACTER_PAGE_BUTTON", 0 ), m_nCharRemainLast );
			m_pNewButton->SetOneLineText( strTemp.GetString() );
		}
	}

	/*dmk14 ingame web*/
	CString strTemp;
	strTemp.Format ( "%s%d", ID2GAMEWORD ( "CHARACTER_SELECT_WEB_MENU", 3 ), dwGameTime );
	m_pGameTimeStatic->ClearText();
	m_pGameTimeStatic->SetText( strTemp, NS_UITEXTCOLOR::WHITE );

	strTemp.Format ( "%s%d",ID2GAMEWORD ( "CHARACTER_SELECT_WEB_MENU", 4 ),dwEpoints );
	m_pPremiumPointStatic->ClearText();
	m_pPremiumPointStatic->SetText( strTemp, NS_UITEXTCOLOR::WHITE );

	m_pEmailStatic->ClearText();
	m_pEmailStatic->SetText( COuterInterface::GetInstance().GetEmail(), NS_UITEXTCOLOR::WHITE );

	int nServerGroup, nServerChannel;
	COuterInterface::GetInstance().GetConnectServerInfo( nServerGroup, nServerChannel );

	if ( m_nServerGroup != nServerGroup || m_nServerChannel != nServerChannel )
	{
		CString strChannelName;
		strChannelName.Format("%s %s %d", ID2GAMEEXTEXT("SERVER_NAME", nServerGroup ), ID2GAMEEXTEXT("SERVER_CHANNEL"), nServerChannel );
		m_pServerName->SetOneLineText( strChannelName.GetString() );

		m_nServerGroup = nServerGroup;
		m_nServerChannel = nServerChannel;
	}
}

void CSelectCharacterButton::SetVisibleSingle ( BOOL bVisible )
{
	CUIGroup::SetVisibleSingle( bVisible );
	if ( bVisible )
	{
	}
}

void CSelectCharacterButton::ResetData()
{
}

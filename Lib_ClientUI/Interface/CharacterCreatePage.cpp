#include "StdAfx.h"
#include "CharacterCreatePage.h"
#include "CharacterCreatePageSet.h"
#include "CharacterCreatePageStyle.h"

#include "d3dfont.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#include "BasicLineBox.h"
#include "BasicTextButton.h"
#include "BasicTextBox.h"
#include "ModalWindow.h"
#include "OuterInterface.h"
#include "DxGlobalStage.h"
#include "DxLobyStage.h"
#include "RanFilter.h"
#include "../Lib_Engine/Common/StringUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCharacterCreatePage::CCharacterCreatePage ()
	: m_bUseCancel( TRUE )
	, m_bTypePage( FALSE )
	, m_nServerGroup(-1)
	, m_nServerChannel(-1)
	, m_pPageSet( NULL )
	, m_pPageStyle( NULL )
	, m_pNewButton(NULL)
	, m_pSelectCharacterButton(NULL)
	, m_pChangeServerButton(NULL)
	, m_pGameStartButton(NULL)
	, m_pServerName(NULL)
{
}

CCharacterCreatePage::~CCharacterCreatePage ()
{
}

void CCharacterCreatePage::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER_FAT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuterFat ( "CREATE_CHAR_WINDOW_BACK" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxOuter ( "CHARACTER_CREATE_PAGE_SERVER_TEXT_BACK" );
	RegisterControl ( pBasicLineBox );	

	m_pPageSet = new CCharacterCreatePageSet;
	m_pPageSet->CreateSub( this, "CREATE_CHAR_WINDOW_PAGE", UI_FLAG_DEFAULT, CHARACTER_CREATE_PAGE_SET );
	m_pPageSet->CreateSubControl ();
	m_pPageSet->SetVisibleSingle( FALSE );
	RegisterControl ( m_pPageSet );

	m_pPageStyle = new CCharacterCreatePageStyle;
	m_pPageStyle->CreateSub( this, "CREATE_CHAR_WINDOW_PAGE", UI_FLAG_DEFAULT, CHARACTER_CREATE_PAGE_STYLE );
	m_pPageStyle->CreateSubControl ();
	m_pPageStyle->SetVisibleSingle( FALSE );
	RegisterControl ( m_pPageStyle );

	m_pNewButton = new CBasicTextButton;
	m_pNewButton->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, CHARACTER_CREATE_PAGE_BUTTON_CREATE_NEW );
	m_pNewButton->CreateBaseButton ( "CHARACTER_CREATE_PAGE_BUTTON_CHAR_CREATE", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "CHARACTER_CREATE_PAGE_BUTTON", 0 ) );		
	RegisterControl ( m_pNewButton );

	m_pSelectCharacterButton = new CBasicTextButton;
	m_pSelectCharacterButton->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, CHARACTER_CREATE_PAGE_BUTTON_SELECT_CHAR );
	m_pSelectCharacterButton->CreateBaseButton ( "CHARACTER_CREATE_PAGE_BUTTON_CHAR_SELECT", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "CHARACTER_CREATE_PAGE_BUTTON", 2 ) );
	RegisterControl ( m_pSelectCharacterButton );

	m_pChangeServerButton = new CBasicTextButton;
	m_pChangeServerButton->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE,CHARACTER_CREATE_PAGE_BUTTON_CHANGE_SERVER );
	m_pChangeServerButton->CreateBaseButton ( "CHARACTER_CREATE_PAGE_BUTTON_SERVER_SELECT", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "CHARACTER_CREATE_PAGE_BUTTON", 3 ) );		
	RegisterControl ( m_pChangeServerButton );

	m_pGameStartButton = new CBasicTextButton;
	m_pGameStartButton->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, CHARACTER_CREATE_PAGE_BUTTON_GAME_START );
	m_pGameStartButton->CreateBaseButton ( "CHARACTER_CREATE_PAGE_BUTTON_GAME_START", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, (char*)ID2GAMEWORD ( "CHARACTER_CREATE_PAGE_BUTTON", 1 ) );		
	RegisterControl ( m_pGameStartButton );

	m_pServerName = new CBasicTextBox;
	m_pServerName->CreateSub ( this, "CHARACTER_CREATE_PAGE_SERVER_TEXT" );
	m_pServerName->SetFont ( pFont9 );
	m_pServerName->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );	
	RegisterControl ( m_pServerName );
}

void CCharacterCreatePage::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch( ControlID )
	{
	case CHARACTER_CREATE_PAGE_BUTTON_CREATE_NEW:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				if ( m_bUseCancel )
				{
					if ( !m_bTypePage )
						ToTypePage();
				}
			}
		}break;

	case CHARACTER_CREATE_PAGE_BUTTON_SELECT_CHAR:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				if ( m_bUseCancel ){
					COuterInterface::GetInstance().ToSelectCharacterPage ( GetWndID () );
				}
			}
		}break;

	case CHARACTER_CREATE_PAGE_BUTTON_CHANGE_SERVER:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				if ( m_bUseCancel ){
					DoModalOuter ( ID2GAMEEXTEXT ("CHARACTERSTAGE_6"), MODAL_INFOMATION, OKCANCEL, OUTER_MODAL_LOGOUT );
				}
			}
		}break;

	case CHARACTER_CREATE_PAGE_BUTTON_GAME_START:
		{
			if ( CHECK_KEYFOCUSED ( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				if ( m_bUseCancel )
				{
					if ( m_bTypePage )
						ToStylePage();
					else
						CreateCharacter();
				}
			}
		}break;

	};
}

void CCharacterCreatePage::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	int nServerGroup, nServerChannel;
	COuterInterface::GetInstance().GetConnectServerInfo( nServerGroup, nServerChannel );

	if ( m_nServerGroup != nServerGroup || m_nServerChannel != nServerChannel )
	{
		CString strServerName;
		strServerName.Format("%s %s %d", ID2GAMEEXTEXT("SERVER_NAME", nServerGroup ), ID2GAMEEXTEXT("SERVER_CHANNEL"), nServerChannel );
		m_pServerName->SetOneLineText( strServerName.GetString() );	

		m_nServerGroup = nServerGroup;
		m_nServerChannel = nServerChannel;
	}
}

void CCharacterCreatePage::SetVisibleSingle ( BOOL bVisible )
{
	CUIGroup::SetVisibleSingle( bVisible );
	if ( bVisible )
	{
	}else{
		if ( m_pPageStyle ){
			CUIEditBox* pEditBox = m_pPageStyle->GetEditBox();
			if ( pEditBox )
				pEditBox->EndEdit();
		}
	}
}

void CCharacterCreatePage::ToTypePage()
{
	if ( !m_bUseCancel )	return;
	if ( m_bTypePage )		return;

	m_pPageSet->SetVisibleSingle( TRUE );
	m_pPageStyle->SetVisibleSingle( FALSE );

	m_pGameStartButton->SetOneLineText( ID2GAMEWORD ( "CHARACTER_CREATE_PAGE_BUTTON_TEXT", 1 ) );

	const USHORT uCharRemain = COuterInterface::GetInstance().GetCharRemain();

	CString strTemp;
	strTemp.Format( "%s(%d)", ID2GAMEWORD ( "SELECT_CHARACTER_PAGE_BUTTON", 0 ), uCharRemain );
	m_pNewButton->SetOneLineText( strTemp.GetString() );

	m_bTypePage = TRUE;
}

void CCharacterCreatePage::ToStylePage()
{
	if ( !m_bUseCancel )	return;
	if ( !m_bTypePage )		return;

	if ( !CheckValidCharacterSet() ){
		return;
	}

	m_pPageSet->SetVisibleSingle( FALSE );
	m_pPageStyle->SetVisibleSingle( TRUE );

	m_pGameStartButton->SetOneLineText( ID2GAMEWORD ( "CHARACTER_CREATE_PAGE_BUTTON", 1 ) );
	m_pNewButton->SetOneLineText( ID2GAMEWORD ( "CHARACTER_CREATE_PAGE_BUTTON_TEXT", 0 ) );

	m_bTypePage = FALSE;
}

void CCharacterCreatePage::ResetAll()
{
	m_pPageSet->ResetAll();
	m_pPageStyle->ResetAll();

	ToTypePage();
}

void CCharacterCreatePage::ResetNameEdit()
{
	m_pPageStyle->ResetNameEdit();
}

void CCharacterCreatePage::SetCreateExtremeM( BOOL bCreate )
{
	m_pPageSet->SetCreateExtremeM( bCreate );
}

void CCharacterCreatePage::SetCreateExtremeW( BOOL bCreate )
{
	m_pPageSet->SetCreateExtremeW( bCreate );
}

void CCharacterCreatePage::CreateCharacter()
{
	if ( !CheckValidCharacterSet() ){
		return;
	}

	if ( m_bTypePage )	return;

	CUIEditBox* pEditBox = m_pPageStyle->GetEditBox();

	if ( !pEditBox ){
		return;
	}

	if ( !pEditBox->GetEditLength() ){
		DoModalOuter ( ID2GAMEEXTEXT("CREATE_CHARACTER_NAME_ERROR_EMPTY") );
		return;
	}

	CString strTemp = pEditBox->GetEditString ();				
	pEditBox->EndEdit();

	if ( !CheckStringName ( strTemp ) ){
		pEditBox->ClearEdit();
		return;
	}

	if ( CRanFilter::GetInstance().NameFilter ( strTemp.GetString () ) ){
		DoModalOuter ( ID2GAMEEXTEXT("CHARACTER_BADNAME") );
		return ;
	}

	DxLobyStage *pLobyStage = DxGlobalStage::GetInstance().GetLobyStage();
	const GLCHAR_NEWINFO& sNEWINFO = pLobyStage->GetNewCharInfo ();

	if ( sNEWINFO.CHECKVALID () )
	{
		CNetClient* pNetClient = DxGlobalStage::GetInstance().GetNetClient ();
		pNetClient->SndCreateChaInfo
			(
			sNEWINFO.m_emIndex,		/*부서*/
			sNEWINFO.m_wSchool,		/*학교*/
			sNEWINFO.m_wFace,		/*얼굴*/
			sNEWINFO.m_wHair,		/*헤어*/
			sNEWINFO.m_wHairColor,	/*헤어컬러*/
			sNEWINFO.m_wSex,		/*성별*/
			sNEWINFO.m_fScaleRange,
			strTemp.GetString ()
			);	
	}

	COuterInterface::GetInstance().CreateCharacterSetUseCancel( FALSE );
}

BOOL CCharacterCreatePage::CheckStringName( CString strTemp )
{
	strTemp = strTemp.Trim();

#ifdef TH_PARAM	
	// 태국어 문자 조합 체크 

	if ( !m_pCheckString ) return FALSE;

	if ( !m_pCheckString(strTemp) )
	{
		DoModalOuter ( ID2GAMEEXTEXT ( "CREATESTAGE_9" ), MODAL_INFOMATION, OK, OUTER_MODAL_THAI_CHECK_STRING_ERROR );
		return FALSE;
	}

#endif

#ifdef VN_PARAM
	// 베트남 문자 조합 체크 
	if( STRUTIL::CheckVietnamString( strTemp ) )
	{
		DoModalOuter ( ID2GAMEEXTEXT ( "CREATESTAGE_10" ), MODAL_INFOMATION, OK, OUTER_MODAL_THAI_CHECK_STRING_ERROR );
		return FALSE;
	}

#endif 

	if( STRUTIL::CheckString( strTemp ) )
	{
		DoModalOuter ( ID2GAMEEXTEXT ( "CREATE_CHARACTER_NAME_ERROR" ), MODAL_INFOMATION, OK, OUTER_MODAL_NAME_ERROR );
		return FALSE;
	}

	return TRUE;
}

BOOL CCharacterCreatePage::CheckValidCharacterSet()
{
	if ( !m_pPageSet )	return FALSE;

	int nSchool = m_pPageSet->m_nSelectedSchool;
	int nSex = m_pPageSet->m_nSelectedSex;
	int nClass = m_pPageSet->m_nSelectedClass;

	if ( nSchool < 0 || nSchool >= GLSCHOOL_NUM ){
		DoModalOuter ( ID2GAMEEXTEXT ( "NEW_CHAR_SELECT_SCHOOL_ERROR" ), MODAL_INFOMATION, OK );
		return FALSE;
	}

	if ( nSex < 0 || nSex >= 2 ){
		DoModalOuter ( ID2GAMEEXTEXT ( "NEW_CHAR_SELECT_SEX_ERROR" ), MODAL_INFOMATION, OK );
		return FALSE;
	}

	if ( nClass < 0 || nClass >= GLCI_NUM_8CLASS ){
		DoModalOuter ( ID2GAMEEXTEXT ( "NEW_CHAR_SELECT_CLASS_ERROR" ), MODAL_INFOMATION, OK );
		return FALSE;
	}

	return TRUE;
}

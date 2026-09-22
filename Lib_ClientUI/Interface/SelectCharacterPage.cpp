#include "StdAfx.h"
#include "SelectCharacterPage.h"
#include "SelectCharacterInfoPage.h"
#include "SelectCharacterList.h"
#include "SelectCharacterButton.h"

#include "GameTextControl.h"
#include "OuterInterface.h"
#include "DxGlobalStage.h"
#include "ModalWindow.h"
#include "DxLobyStage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSelectCharacterPage::CSelectCharacterPage ()
	: m_pSelectCharacterPage(NULL)
	, m_pSelectCharacterList(NULL)
	, m_pSelectCharacterButton(NULL)
{
}

CSelectCharacterPage::~CSelectCharacterPage ()
{
}

void CSelectCharacterPage::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

	m_pSelectCharacterPage = new CSelectCharacterInfoPage;
	m_pSelectCharacterPage->CreateSub ( this, "BASIC_WINDOW", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pSelectCharacterPage->CreateBaseWidnow ( "SELECT_CHARACTER_UPWINDOW", (char*)ID2GAMEWORD("SELECT_CHARACTER_PAGE") );
	m_pSelectCharacterPage->CreateSubControl ();
	RegisterControl ( m_pSelectCharacterPage );

	m_pSelectCharacterList = new CSelectCharacterList;
	m_pSelectCharacterList->CreateSub( this, "SELECT_CHAR_WINDOW_CHAR_LIST", UI_FLAG_DEFAULT, SELECT_CHARACTER_LIST );
	m_pSelectCharacterList->CreateSubControl ();
	RegisterControl ( m_pSelectCharacterList );

	m_pSelectCharacterButton = new CSelectCharacterButton;
	m_pSelectCharacterButton->CreateSub( this, "SELECT_CHAR_WINDOW_CHAR_BUTTON", UI_FLAG_DEFAULT, SELECT_CHARACTER_BUTTON );
	m_pSelectCharacterButton->CreateSubControl ();
	RegisterControl ( m_pSelectCharacterButton );
}

BOOL CSelectCharacterPage::SndGameJoin ()
{
	SCHARINFO_LOBBY* pCharInfo = DxGlobalStage::GetInstance().GetLobyStage()->GetSelectCharInfo();
	if ( pCharInfo )
	{
		DxGlobalStage::GetInstance().GetNetClient()->SndGameJoin ( pCharInfo->m_dwCharID );

		COuterInterface::GetInstance().START_WAIT_TIME ( 60.0f );
		DoModalOuter ( ID2GAMEEXTEXT ("CHARACTERSTAGE_GAME_JOIN"), MODAL_INFOMATION, CANCEL, OUTER_MODAL_CHARACTERSTAGE_GAME_JOIN_WAIT );
		return TRUE;
	}

	return FALSE;
}

void CSelectCharacterPage::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	if ( CHECK_MOUSE_IN ( dwMsg ) )
	{
		AddMessageEx ( UIMSG_MOUSEIN_SELECT_CHARACTERPAGE );
	}
}

void CSelectCharacterPage::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

void CSelectCharacterPage::ResetAll ()
{
	if ( m_pSelectCharacterPage ) 
		m_pSelectCharacterPage->ResetAll ();

	if ( m_pSelectCharacterList )
		m_pSelectCharacterList->ResetData();

	if ( m_pSelectCharacterButton )
		m_pSelectCharacterButton->ResetData();
}

SCHARINFO_LOBBY* CSelectCharacterPage::GetDelCharInfo ()
{
	if ( m_pSelectCharacterButton )
	{
		return m_pSelectCharacterButton->GetDelCharInfo();
	}

	return NULL;
}
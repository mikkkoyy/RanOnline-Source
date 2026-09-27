#include "StdAfx.h"
#include "ItemMallWindow.h"
#include "ItemMallWindowMenu.h"
#include "ItemMallWindowMenuPage.h"
#include "BasicTextButton.h"
#include "GameTextControl.h"
#include "GLGaeaClient.h"
#include "InnerInterface.h"
#include "GLItemMan.h"
#include "ModalWindow.h"
#include "ModalCallerID.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "UITextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CItemMallWindow::CItemMallWindow ()
	: m_pMenu( NULL )
	, m_pPage( NULL )
	, m_pTextBox( NULL )
	, m_wType( 0 )
	, m_nType( 0 )
{
}

CItemMallWindow::~CItemMallWindow ()
{
}

void CItemMallWindow::CreateSubControl ()
{
	D3DCOLOR dwColor = NS_UITEXTCOLOR::DEFAULT;
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );

	CBasicLineBox* m_pLineBox = new CBasicLineBox;
	m_pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pLineBox->CreateBaseBoxQuestList( "ITEMMALL_MENU_LINE" );
	m_pLineBox->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pLineBox );

	m_pLineBox = new CBasicLineBox;
	m_pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pLineBox->CreateBaseBoxQuestList( "ITEMMALL_MENU_LINE_TAIL" );
	m_pLineBox->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pLineBox );

	CItemMallWindowMenuPage* pItemPage = new CItemMallWindowMenuPage;
	pItemPage->CreateSub ( this, "ITEMMALL_PAGE", UI_FLAG_DEFAULT, ITEMMALL_WINDOW_PAGE );
	pItemPage->CreateSubControl();
	pItemPage->SetVisibleSingle(TRUE);
	RegisterControl ( pItemPage );
	m_pPage = pItemPage;

	CItemMallWindowMenu* pMenu = new CItemMallWindowMenu;
	pMenu->CreateSub ( this, "ITEMMALL_MENU" );
	pMenu->CreateSubControl();
	pMenu->SetVisibleSingle(TRUE);
	RegisterControl ( pMenu );
	m_pMenu = pMenu;

	m_pTextBox = new CBasicTextBox;
	m_pTextBox->CreateSub ( this, "ITEMMALL_WINDOW_USER_TEXT" );
	m_pTextBox->SetFont ( pFont );
	m_pTextBox->SetTextAlign ( TEXT_ALIGN_LEFT );
	RegisterControl ( m_pTextBox );
}

CBasicTextButton*  CItemMallWindow::CreateTextButton ( const char* szButton, UIGUID ControlID, const char* szText )
{
	const int nBUTTONSIZE = CBasicTextButton::SIZE19;
	CBasicTextButton* pTextButton = new CBasicTextButton;
	pTextButton->CreateSub ( this, "BASIC_TEXT_BUTTON19", UI_FLAG_XSIZE, ControlID );
	pTextButton->CreateBaseButton ( szButton, nBUTTONSIZE, CBasicButton::RADIO_FLIP, szText );
	RegisterControl ( pTextButton );
	return pTextButton;
}

void CItemMallWindow::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	if ( !IsVisible () ) return ;
	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if( m_wType != m_pMenu->GetType() || m_nType != m_pMenu->GetItemType() )
	{
		m_wType = m_pMenu->GetType();
		m_nType = m_pMenu->GetItemType();
		InitItemMall();
	}
}

void CItemMallWindow::InitItemMall ()
{
	const GLCHARLOGIC& sCharData = GLGaeaClient::GetInstance().GetCharacterLogic ();
	SetMoney ( sCharData.m_dwPremiumPoints, sCharData.m_dwCombatPoints );

	m_pPage->SetVisibleType( m_pMenu->GetType(), m_pMenu->GetItemType() );
}

void CItemMallWindow::SetMoney( DWORD dwPrem, DWORD dwCombat )
{
	m_pTextBox->ClearText ();

	CString strMoney;
	strMoney.Format("Account Balance: EP(%d) | GP(%d)", dwPrem,dwCombat );
	m_pTextBox->AddTextNoSplit( strMoney );
}
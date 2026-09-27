#include "StdAfx.h"
#include "InventoryLockInputWindow.h"

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

int	CInventoryLockInputWindow::nLIMIT_PIN	= 4;


CInventoryLockInputWindow::CInventoryLockInputWindow(void)
	: m_pInfoDesc(NULL)
	, m_pInfoPin(NULL)
	, m_pEditBoxMan(NULL)
	, m_pButtonClose(NULL)
	, m_pButtonSave(NULL)
{
}

CInventoryLockInputWindow::~CInventoryLockInputWindow(void)
{
}

void CInventoryLockInputWindow::CreateSubControl()
{
	CD3DFontPar* pFont08 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_SHADOW_FLAG );
	CD3DFontPar* pFont09 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_INVENTORY_LOCK_INPUT_WINDOW_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_INVENTORY_LOCK_INPUT_WINDOW_INFO_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_INVENTORY_LOCK_INPUT_WINDOW_INPUT_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_INVENTORY_LOCK_INPUT_WINDOW_LINE_PIN" );
	RegisterControl ( pBasicLineBox );

	
	m_pInfoDesc = new CBasicTextBox;
	m_pInfoDesc->CreateSub ( this, "RAN_INVENTORY_LOCK_INPUT_WINDOW_INFO_TEXT", UI_FLAG_DEFAULT );
	m_pInfoDesc->SetFont ( pFont08 );
	m_pInfoDesc->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pInfoDesc->SetText( ID2GAMEINTEXT( "RAN_INVENTORY_LOCK_INPUT_INFO" ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pInfoDesc );

	m_pInfoPin = new CBasicTextBox;
	m_pInfoPin->CreateSub ( this, "RAN_INVENTORY_LOCK_INPUT_WINDOW_INFO_PIN", UI_FLAG_DEFAULT );
	m_pInfoPin->SetFont ( pFont09 );
	m_pInfoPin->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pInfoPin->SetText( ID2GAMEWORD( "RAN_INVENTORY_LOCK_INPUT_WINDOW", 1 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pInfoPin );

	
	m_pEditBoxMan = new CUIEditBoxMan;
	m_pEditBoxMan->CreateSub ( this, "RAN_INVENTORY_LOCK_INPUT_WINDOW_EDITMAN", UI_FLAG_DEFAULT, INVENTORY_LOCK_INPUT_WINDOW_EDIT_MAN );
	m_pEditBoxMan->CreateEditBox ( INVENTORY_LOCK_INPUT_WINDOW_EDIT_PIN, "RAN_INVENTORY_LOCK_INPUT_WINDOW_TEXT_PIN", "RAN_INVENTORY_LOCK_INPUT_WINDOW_CARRAT", TRUE, UINT_MAX, pFont09, nLIMIT_PIN );
	m_pEditBoxMan->SetHide ( INVENTORY_LOCK_INPUT_WINDOW_EDIT_PIN, TRUE );
	RegisterControl ( m_pEditBoxMan );

	m_pButtonSave = new CBasicTextButton;
	m_pButtonSave->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, INVENTORY_LOCK_INPUT_WINDOW_BUTTON_OK );
	m_pButtonSave->CreateBaseButton ( "RAN_INVENTORY_LOCK_INPUT_WINDOW_BUTTON_OK", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_INVENTORY_LOCK_INPUT_WINDOW", 2 ) );
	RegisterControl ( m_pButtonSave );

	m_pButtonClose = new CBasicTextButton;
	m_pButtonClose->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, INVENTORY_LOCK_INPUT_WINDOW_BUTTON_CANCEL );
	m_pButtonClose->CreateBaseButton ( "RAN_INVENTORY_LOCK_INPUT_WINDOW_BUTTON_CANCEL", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_INVENTORY_LOCK_INPUT_WINDOW", 3 ) );
	RegisterControl ( m_pButtonClose );
}

void CInventoryLockInputWindow::TranslateUIMessage( UIGUID cID, DWORD dwMsg )
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

	case INVENTORY_LOCK_INPUT_WINDOW_BUTTON_CANCEL:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			{
				CInnerInterface::GetInstance().HideGroup( GetWndID() );
			}
		}break;

	case INVENTORY_LOCK_INPUT_WINDOW_BUTTON_OK:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			{
				CheckInfo();
			}
		}break;
	};

}

void CInventoryLockInputWindow::Update ( int x ,int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
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

void CInventoryLockInputWindow::SetVisibleSingle( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		if ( m_pEditBoxMan )
		{
			m_pEditBoxMan->EndEdit ();
			m_pEditBoxMan->ClearEdit ( INVENTORY_LOCK_INPUT_WINDOW_EDIT_PIN );

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

void CInventoryLockInputWindow::CheckInfo()
{
	if ( !m_pEditBoxMan )	return;

	std::string strpin = m_pEditBoxMan->GetEditString( INVENTORY_LOCK_INPUT_WINDOW_EDIT_PIN );

	if ( strpin.empty() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_INPUT_KEY_EMPTY") );
		return;
	}

	if ( strpin.size() < nLIMIT_PIN || strpin.size() > nLIMIT_PIN  )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_INPUT_KEY_SIZE") );
		return;
	}

	if ( strpin.find_first_not_of( "0123456789" ) != string::npos )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_INPUT_KEY_NUMBER_ONLY") );
		return;
	}

	GLGaeaClient::GetInstance().GetCharacter()->ReqEnableInventoryLockInputMsg( strpin.c_str() );
	CInnerInterface::GetInstance().HideGroup( GetWndID() );
}
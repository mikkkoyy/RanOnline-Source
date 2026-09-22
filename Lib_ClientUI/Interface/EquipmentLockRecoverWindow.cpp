#include "StdAfx.h"
#include "EquipmentLockRecoverWindow.h"

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

int	CEquipmentLockRecoverWindow::nLIMIT_PIN		= 4;
int	CEquipmentLockRecoverWindow::nLIMIT_AGREE	= 7;

CEquipmentLockRecoverWindow::CEquipmentLockRecoverWindow(void)
	: m_pInfoDesc(NULL)
	, m_pInfoPin1(NULL)
	, m_pInfoPin2(NULL)
	, m_pInfoOrigPin1(NULL)
	, m_pInfoOrigPin2(NULL)
	, m_pInfoConfirm(NULL)
	, m_pEditBoxMan(NULL)
	, m_pButtonChange(NULL)
	, m_pButtonDelete(NULL)
	, m_pButtonClose(NULL)
{
}

CEquipmentLockRecoverWindow::~CEquipmentLockRecoverWindow(void)
{
}

void CEquipmentLockRecoverWindow::CreateSubControl()
{
	CD3DFontPar* pFont08 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_SHADOW_FLAG );
	CD3DFontPar* pFont09 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_INFO_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_INPUT_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_LINE_ORIG_PIN" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_LINE_NEW_PIN1" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_LINE_NEW_PIN2" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_LINE_CONFIRM" );
	RegisterControl ( pBasicLineBox );

	m_pInfoDesc = new CBasicTextBox;
	m_pInfoDesc->CreateSub ( this, "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_INFO_TEXT", UI_FLAG_DEFAULT );
	m_pInfoDesc->SetFont ( pFont08 );
	m_pInfoDesc->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pInfoDesc->SetText( ID2GAMEINTEXT( "RAN_EQUIPMENT_LOCK_RECOVER_INFO" ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pInfoDesc );

	m_pInfoOrigPin1 = new CBasicTextBox;
	m_pInfoOrigPin1->CreateSub ( this, "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_INFO_ORIG_PIN1", UI_FLAG_DEFAULT );
	m_pInfoOrigPin1->SetFont ( pFont09 );
	m_pInfoOrigPin1->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pInfoOrigPin1->SetText( ID2GAMEWORD( "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW", 1 ), NS_UITEXTCOLOR::ORNAGERED );
	RegisterControl ( m_pInfoOrigPin1 );

	m_pInfoOrigPin2 = new CBasicTextBox;
	m_pInfoOrigPin2->CreateSub ( this, "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_INFO_ORIG_PIN2", UI_FLAG_DEFAULT );
	m_pInfoOrigPin2->SetFont ( pFont09 );
	m_pInfoOrigPin2->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pInfoOrigPin2->SetText( "xxxx", NS_UITEXTCOLOR::ORNAGERED );
	RegisterControl ( m_pInfoOrigPin2 );

	m_pInfoPin1 = new CBasicTextBox;
	m_pInfoPin1->CreateSub ( this, "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_INFO_PIN1", UI_FLAG_DEFAULT );
	m_pInfoPin1->SetFont ( pFont09 );
	m_pInfoPin1->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pInfoPin1->SetText( ID2GAMEWORD( "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW", 2 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pInfoPin1 );

	m_pInfoPin2 = new CBasicTextBox;
	m_pInfoPin2->CreateSub ( this, "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_INFO_PIN2", UI_FLAG_DEFAULT );
	m_pInfoPin2->SetFont ( pFont09 );
	m_pInfoPin2->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pInfoPin2->SetText( ID2GAMEWORD( "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW", 3 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pInfoPin2 );

	m_pInfoConfirm = new CBasicTextBox;
	m_pInfoConfirm->CreateSub ( this, "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_INFO_CONFIRM", UI_FLAG_DEFAULT );
	m_pInfoConfirm->SetFont ( pFont09 );
	m_pInfoConfirm->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pInfoConfirm->SetText( ID2GAMEWORD( "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW", 4 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pInfoConfirm );

	m_pEditBoxMan = new CUIEditBoxMan;
	m_pEditBoxMan->CreateSub ( this, "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_EDITMAN", UI_FLAG_DEFAULT, EQUIPMENT_LOCK_RECOVER_WINDOW_EDIT_MAN );
	m_pEditBoxMan->CreateEditBox ( EQUIPMENT_LOCK_RECOVER_WINDOW_EDIT_PIN1, "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_TEXT_PIN1", "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_CARRAT", TRUE, UINT_MAX, pFont09, nLIMIT_PIN );
	m_pEditBoxMan->CreateEditBox ( EQUIPMENT_LOCK_RECOVER_WINDOW_EDIT_PIN2, "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_TEXT_PIN2", "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_CARRAT", TRUE, UINT_MAX, pFont09, nLIMIT_PIN );
	m_pEditBoxMan->CreateEditBox ( EQUIPMENT_LOCK_RECOVER_WINDOW_EDIT_CONFIRM, "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_TEXT_CONFIRM", "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_CARRAT", TRUE, UINT_MAX, pFont09, nLIMIT_AGREE );
	m_pEditBoxMan->SetHide ( EQUIPMENT_LOCK_RECOVER_WINDOW_EDIT_PIN1, TRUE );
	m_pEditBoxMan->SetHide ( EQUIPMENT_LOCK_RECOVER_WINDOW_EDIT_PIN2, TRUE );
	RegisterControl ( m_pEditBoxMan );

	m_pButtonChange = new CBasicTextButton;
	m_pButtonChange->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, EQUIPMENT_LOCK_ENABLE_WINDOW_BUTTON_CHANGE );
	m_pButtonChange->CreateBaseButton ( "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_BUTTON_CHANGE", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_EQUIPMENT_LOCK_RECOVER_WINDOW", 5 ) );
	RegisterControl ( m_pButtonChange );

	m_pButtonDelete = new CBasicTextButton;
	m_pButtonDelete->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, EQUIPMENT_LOCK_ENABLE_WINDOW_BUTTON_DELETE );
	m_pButtonDelete->CreateBaseButton ( "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_BUTTON_DELETE", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_EQUIPMENT_LOCK_RECOVER_WINDOW", 6 ) );
	RegisterControl ( m_pButtonDelete );

	m_pButtonClose = new CBasicTextButton;
	m_pButtonClose->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, EQUIPMENT_LOCK_ENABLE_WINDOW_BUTTON_CLOSE );
	m_pButtonClose->CreateBaseButton ( "RAN_EQUIPMENT_LOCK_RECOVER_WINDOW_BUTTON_CANCEL", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_EQUIPMENT_LOCK_RECOVER_WINDOW", 7 ) );
	RegisterControl ( m_pButtonClose );
}

void CEquipmentLockRecoverWindow::TranslateUIMessage( UIGUID cID, DWORD dwMsg )
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

	case EQUIPMENT_LOCK_ENABLE_WINDOW_BUTTON_CHANGE:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			{
				ActionChangeKey();
			}
		}break;

	case EQUIPMENT_LOCK_ENABLE_WINDOW_BUTTON_DELETE:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			{
				ActionDeleteKey();
			}
		}break;

	case EQUIPMENT_LOCK_ENABLE_WINDOW_BUTTON_CLOSE:
		{
			if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			{
				ActionCloseWindow();
			}
		}break;
	};

}

void CEquipmentLockRecoverWindow::Update ( int x ,int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
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

void CEquipmentLockRecoverWindow::SetVisibleSingle( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		if ( m_pEditBoxMan )
		{
			m_pEditBoxMan->EndEdit ();
			m_pEditBoxMan->ClearEdit ( EQUIPMENT_LOCK_RECOVER_WINDOW_EDIT_PIN1 );
			m_pEditBoxMan->ClearEdit ( EQUIPMENT_LOCK_RECOVER_WINDOW_EDIT_PIN2 );
			m_pEditBoxMan->ClearEdit ( EQUIPMENT_LOCK_RECOVER_WINDOW_EDIT_CONFIRM );

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

void CEquipmentLockRecoverWindow::ActionChangeKey()
{
	if ( !m_pEditBoxMan )	return;

	std::string strpin = m_pEditBoxMan->GetEditString( EQUIPMENT_LOCK_RECOVER_WINDOW_EDIT_PIN1 );
	std::string strpin2 = m_pEditBoxMan->GetEditString( EQUIPMENT_LOCK_RECOVER_WINDOW_EDIT_PIN2 );
	std::string stragree = m_pEditBoxMan->GetEditString( EQUIPMENT_LOCK_RECOVER_WINDOW_EDIT_CONFIRM );

	if ( strpin.empty() || strpin2.empty() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_KEY_EMPTY") );
		return;
	}

	if ( strpin.size() < nLIMIT_PIN || strpin.size() > nLIMIT_PIN || strpin2.size() < nLIMIT_PIN || strpin2.size() > nLIMIT_PIN )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_KEY_SIZE") );
		return;
	}

	if ( strcmp( strpin.c_str(), strpin2.c_str() ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_KEY_NOT_SAME") );
		return;
	}

	if ( strpin.find_first_not_of( "0123456789" ) != string::npos || strpin2.find_first_not_of( "0123456789" ) != string::npos )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_KEY_NUMBER_ONLY") );
		return;
	}

	if ( stragree.empty() || stragree.size() < nLIMIT_AGREE || stragree.size() > nLIMIT_AGREE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_AGREE_MISMATCH") );
		return;
	}

	if ( strcmp( stragree.c_str(), "CONFIRM" ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_AGREE_MISMATCH") );
		return;
	}

	GLGaeaClient::GetInstance().GetCharacter()->ReqRecoverEquipmentLockChange( strpin.c_str(), strpin2.c_str(), stragree.c_str() );
	CInnerInterface::GetInstance().HideGroup( GetWndID() );
}

void CEquipmentLockRecoverWindow::ActionDeleteKey()
{
	if ( !m_pEditBoxMan )	return;

	std::string stragree = m_pEditBoxMan->GetEditString( EQUIPMENT_LOCK_RECOVER_WINDOW_EDIT_CONFIRM );

	if ( stragree.empty() || stragree.size() < nLIMIT_AGREE || stragree.size() > nLIMIT_AGREE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_DELETE_AGREE_MISMATCH") );
		return;
	}

	if ( strcmp( stragree.c_str(), "CONFIRM" ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_DELETE_AGREE_MISMATCH") );
		return;
	}

	GLGaeaClient::GetInstance().GetCharacter()->ReqRecoverEquipmentLockDelete( stragree.c_str() );
	CInnerInterface::GetInstance().HideGroup( GetWndID() );
}

void CEquipmentLockRecoverWindow::ActionCloseWindow()
{
	if ( !m_pEditBoxMan )	return;

	std::string stragree = m_pEditBoxMan->GetEditString( EQUIPMENT_LOCK_RECOVER_WINDOW_EDIT_CONFIRM );

	if ( stragree.empty() || stragree.size() < nLIMIT_AGREE || stragree.size() > nLIMIT_AGREE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_CLOSE_AGREE_MISMATCH") );
		return;
	}

	if ( strcmp( stragree.c_str(), "CONFIRM" ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_CLOSE_AGREE_MISMATCH") );
		return;
	}

	CInnerInterface::GetInstance().HideGroup( GetWndID() );
}

void CEquipmentLockRecoverWindow::setCurrentKey( std::string strKey )
{
	if ( m_pInfoOrigPin2 )
		m_pInfoOrigPin2->SetText( strKey.c_str(), NS_UITEXTCOLOR::ORNAGERED );
}

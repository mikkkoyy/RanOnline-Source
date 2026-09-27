#include "StdAfx.h"
#include "SecurityWindow.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "d3dfont.h"
#include "GameTextControl.h"
#include "UITextControl.h"

#include "GLGaeaClient.h"

#include "BasicButton.h"
#include "InnerInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSecurityWindow::CSecurityWindow ()
	: m_bCHECK_MOUSE_STATE(FALSE)
	, m_bFirstGap(FALSE)
	, m_nPosX(0)
	, m_nPosY(0)
	, m_pEquipmentButtonLock(NULL)
	, m_pEquipmentButtonUnlock(NULL)
	, m_pEquipmentButtonActivate(NULL)
	, m_pStorageButtonLock(NULL)
	, m_pStorageButtonUnlock(NULL)
	, m_pStorageButtonActivate(NULL)
	, m_pInventoryButtonLock(NULL)
	, m_pInventoryButtonUnlock(NULL)
	, m_pInventoryButtonActivate(NULL)
	, m_pStorageMouseOver(NULL)
	, m_pEquipmentMouseOver(NULL)
	, m_pInventoryMouseOver(NULL)
{
}

CSecurityWindow::~CSecurityWindow ()
{
}

void CSecurityWindow::CreateSubControl ()
{
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, "SECURITY_WINDOW_BG", UI_FLAG_DEFAULT, SECURITY_WINDOW_BG );
	RegisterControl ( pControl );

	m_pStorageButtonLock = new CBasicButton;
	m_pStorageButtonLock->CreateSub ( this, "SECURITY_WINDOW_STORAGE_LOCK", UI_FLAG_DEFAULT, STORAGE_BUTTON_LOCK );
	m_pStorageButtonLock->SetAlignFlag(UI_FLAG_DEFAULT);
	m_pStorageButtonLock->SetUseDynamic(TRUE);
	m_pStorageButtonLock->SetVisibleSingle( FALSE );
	RegisterControl ( m_pStorageButtonLock );

	m_pStorageButtonUnlock = new CBasicButton;
	m_pStorageButtonUnlock->CreateSub ( this, "SECURITY_WINDOW_STORAGE_UNLOCK", UI_FLAG_DEFAULT, STORAGE_BUTTON_UNLOCK );
	m_pStorageButtonUnlock->SetAlignFlag(UI_FLAG_DEFAULT);
	m_pStorageButtonUnlock->SetUseDynamic(TRUE);
	m_pStorageButtonUnlock->SetVisibleSingle( FALSE );
	RegisterControl ( m_pStorageButtonUnlock );

	m_pStorageButtonActivate = new CBasicButton;
	m_pStorageButtonActivate->CreateSub ( this, "SECURITY_WINDOW_STORAGE_ACTIVATE", UI_FLAG_DEFAULT, STORAGE_BUTTON_ACTIVATE );
	m_pStorageButtonActivate->SetAlignFlag(UI_FLAG_DEFAULT);
	m_pStorageButtonActivate->SetUseDynamic(TRUE);
	m_pStorageButtonActivate->SetVisibleSingle( FALSE );
	RegisterControl ( m_pStorageButtonActivate );

	m_pEquipmentButtonLock = new CBasicButton;
	m_pEquipmentButtonLock->CreateSub ( this, "SECURITY_WINDOW_EQUIPMENT_LOCK", UI_FLAG_DEFAULT, EQUIPMENT_BUTTON_LOCK );
	m_pEquipmentButtonLock->CreateMouseOver("SECURITY_WINDOW_EQUIPMENT_LOCK_OVER");
	m_pEquipmentButtonLock->SetAlignFlag(UI_FLAG_DEFAULT);
	m_pEquipmentButtonLock->SetUseDynamic(TRUE);
	m_pEquipmentButtonLock->SetVisibleSingle( FALSE );
	RegisterControl ( m_pEquipmentButtonLock );

	m_pEquipmentButtonUnlock = new CBasicButton;
	m_pEquipmentButtonUnlock->CreateSub ( this, "SECURITY_WINDOW_EQUIPMENT_UNLOCK", UI_FLAG_DEFAULT, EQUIPMENT_BUTTON_UNLOCK );
	m_pEquipmentButtonUnlock->SetAlignFlag(UI_FLAG_DEFAULT);
	m_pEquipmentButtonUnlock->SetUseDynamic(TRUE);
	m_pEquipmentButtonUnlock->SetVisibleSingle( FALSE );
	RegisterControl ( m_pEquipmentButtonUnlock );

	m_pEquipmentButtonActivate = new CBasicButton;
	m_pEquipmentButtonActivate->CreateSub ( this, "SECURITY_WINDOW_EQUIPMENT_ACTIVATE", UI_FLAG_DEFAULT, EQUIPMENT_BUTTON_ACTIVATE );
	m_pEquipmentButtonActivate->SetAlignFlag(UI_FLAG_DEFAULT);
	m_pEquipmentButtonActivate->SetUseDynamic(TRUE);
	m_pEquipmentButtonActivate->SetVisibleSingle( FALSE );
	RegisterControl ( m_pEquipmentButtonActivate );

	m_pInventoryButtonLock = new CBasicButton;
	m_pInventoryButtonLock->CreateSub ( this, "SECURITY_WINDOW_INVENTORY_LOCK", UI_FLAG_DEFAULT, INVENTORY_BUTTON_LOCK );
	m_pInventoryButtonLock->SetAlignFlag(UI_FLAG_DEFAULT);
	m_pInventoryButtonLock->SetUseDynamic(TRUE);
	m_pInventoryButtonLock->SetVisibleSingle( FALSE );
	RegisterControl ( m_pInventoryButtonLock );

	m_pInventoryButtonUnlock = new CBasicButton;
	m_pInventoryButtonUnlock->CreateSub ( this, "SECURITY_WINDOW_INVENTORY_UNLOCK", UI_FLAG_DEFAULT, INVENTORY_BUTTON_UNLOCK );
	m_pInventoryButtonUnlock->SetAlignFlag(UI_FLAG_DEFAULT);
	m_pInventoryButtonUnlock->SetUseDynamic(TRUE);
	m_pInventoryButtonUnlock->SetVisibleSingle( FALSE );
	RegisterControl ( m_pInventoryButtonUnlock );

	m_pInventoryButtonActivate = new CBasicButton;
	m_pInventoryButtonActivate->CreateSub ( this, "SECURITY_WINDOW_INVENTORY_ACTIVATE", UI_FLAG_DEFAULT, INVENTORY_BUTTON_ACTIVATE );
	m_pInventoryButtonActivate->SetAlignFlag(UI_FLAG_DEFAULT);
	m_pInventoryButtonActivate->SetUseDynamic(TRUE);
	m_pInventoryButtonActivate->SetVisibleSingle( FALSE );
	RegisterControl ( m_pInventoryButtonActivate );
	
	m_pStorageMouseOver = new CUIControl;
	m_pStorageMouseOver->CreateSub ( this, "SECURITY_WINDOW_STORAGE_MOUSE_OVER" );
	RegisterControl ( m_pStorageMouseOver );

	m_pEquipmentMouseOver = new CUIControl;
	m_pEquipmentMouseOver->CreateSub ( this, "SECURITY_WINDOW_EQUIPMENT_MOUSE_OVER" );
	RegisterControl ( m_pEquipmentMouseOver );

	m_pInventoryMouseOver = new CUIControl;
	m_pInventoryMouseOver->CreateSub ( this, "SECURITY_WINDOW_INVENTORY_MOUSE_OVER" );
	RegisterControl ( m_pInventoryMouseOver );
}

CBasicButton* CSecurityWindow::CreateFlipButton(char* szButton, char* szButtonFlip, UIGUID ControlID, WORD wFlipType)
{
	CBasicButton* pButton = new CBasicButton;
	pButton->CreateSub(this, szButton, UI_FLAG_DEFAULT, ControlID);
	pButton->CreateFlip(szButtonFlip, wFlipType);
	RegisterControl(pButton);
	return pButton;
}

void CSecurityWindow::CheckMouseState ()
{
	const DWORD dwMsg = GetMessageEx ();
	if ( CHECK_MOUSE_IN ( dwMsg ) )
	{
		if( UIMSG_LB_DUP & dwMsg )
		{
			AddMessageEx( UIMSG_SECURITY_WINDOW_LBDUP );
			SetDiffuse ( D3DCOLOR_ARGB ( 0xFF, 0xFF, 0xFF, 0xFF ) );
			return ;
		}

		if ( dwMsg & UIMSG_LB_DOWN )
		{
			SetExclusiveControl();	

			if ( !m_bFirstGap )
			{
				UIRECT rcPos = GetGlobalPos ();
				m_vGap.x = m_nPosX - rcPos.left;
				m_vGap.y = m_nPosY - rcPos.top;
				m_bFirstGap = TRUE;

				SetDiffuse ( D3DCOLOR_ARGB ( 0xB4, 0xFF, 0xFF, 0xFF ) );
			}
		}
		else if ( CHECK_LB_UP_LIKE ( dwMsg ) )
		{
			ResetExclusiveControl();
			m_bFirstGap = FALSE;				

			SetDiffuse ( D3DCOLOR_ARGB ( 0xFF, 0xFF, 0xFF, 0xFF ) );
		}
	}
	else if ( CHECK_LB_UP_LIKE ( dwMsg ) )		
	{								
		ResetExclusiveControl();
		m_bFirstGap = FALSE;					

		SetDiffuse ( D3DCOLOR_ARGB ( 0xFF, 0xFF, 0xFF, 0xFF ) );
	}
}

void CSecurityWindow::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{	
	m_bCHECK_MOUSE_STATE = false;
	m_nPosX = x;
	m_nPosY = y;
	
	if ( m_pStorageMouseOver )	 m_pStorageMouseOver->SetVisibleSingle ( FALSE );
	if ( m_pEquipmentMouseOver ) m_pEquipmentMouseOver->SetVisibleSingle ( FALSE );
	if ( m_pInventoryMouseOver ) m_pInventoryMouseOver->SetVisibleSingle ( FALSE );
	
	BOOL bStorageEnabled = GLGaeaClient::GetInstance().GetCharacter()->m_bEnableStorageLock;
	BOOL bStorageLocked = GLGaeaClient::GetInstance().GetCharacter()->m_bStorageLockStatus;
	if ( m_pStorageButtonLock )	m_pStorageButtonLock->SetVisibleSingle( bStorageEnabled && bStorageLocked );
	if ( m_pStorageButtonUnlock )	m_pStorageButtonUnlock->SetVisibleSingle( bStorageEnabled && !bStorageLocked );
	if ( m_pStorageButtonActivate )	m_pStorageButtonActivate->SetVisibleSingle( !bStorageEnabled && !bStorageLocked );

	BOOL bEquipmentEnabled = GLGaeaClient::GetInstance().GetCharacter()->m_bEnableEquipmentLock;
	BOOL bEquipmentLocked = GLGaeaClient::GetInstance().GetCharacter()->m_bEquipmentLockStatus;
	if ( m_pEquipmentButtonLock )	m_pEquipmentButtonLock->SetVisibleSingle( bEquipmentEnabled && bEquipmentLocked );
	if ( m_pEquipmentButtonUnlock )	m_pEquipmentButtonUnlock->SetVisibleSingle( bEquipmentEnabled && !bEquipmentLocked );
	if ( m_pEquipmentButtonActivate )	m_pEquipmentButtonActivate->SetVisibleSingle( !bEquipmentEnabled && !bEquipmentLocked );

	BOOL bInventoryEnabled = GLGaeaClient::GetInstance().GetCharacter()->m_bEnableInventoryLock;
	BOOL bInventoryLocked = GLGaeaClient::GetInstance().GetCharacter()->m_bInventoryLockStatus;
	if ( m_pInventoryButtonLock )	m_pInventoryButtonLock->SetVisibleSingle( bInventoryEnabled && bInventoryLocked );
	if ( m_pInventoryButtonUnlock )	m_pInventoryButtonUnlock->SetVisibleSingle( bInventoryEnabled && !bInventoryLocked );
	if ( m_pInventoryButtonActivate )	m_pInventoryButtonActivate->SetVisibleSingle( !bInventoryEnabled && !bInventoryLocked );

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( IsExclusiveSelfControl() ){
		SetGlobalPos ( D3DXVECTOR2 ( x - m_vGap.x, y - m_vGap.y ) );
	}
}

void CSecurityWindow::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	if ( !m_bCHECK_MOUSE_STATE )
	{
		CheckMouseState ();
		m_bCHECK_MOUSE_STATE = true;
	}

	switch ( ControlID )
	{
	case STORAGE_BUTTON_LOCK:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( m_pStorageMouseOver )
				{
					m_pStorageMouseOver->SetVisibleSingle(TRUE);
				}

				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEWORD ( "RAN_STORAGE_LOCK_INPUT_WINDOW",0 ), NS_UITEXTCOLOR::WHITE  );				
				if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
				{
					CInnerInterface::GetInstance().OpenStorageLockInput();
				}
			}
		}break;
	case STORAGE_BUTTON_UNLOCK:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( m_pStorageMouseOver )
				{
					m_pStorageMouseOver->SetVisibleSingle(TRUE);
				}
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEWORD ( "RAN_STORAGE_LOCK_INPUT_WINDOW",0 ), NS_UITEXTCOLOR::WHITE  );				
				if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
					CInnerInterface::GetInstance().OpenStorageLockInput();
			}
		}break;
	case EQUIPMENT_BUTTON_LOCK:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( m_pEquipmentMouseOver )
				{
					m_pEquipmentMouseOver->SetVisibleSingle(TRUE);
				}
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEWORD ( "RAN_EQUIPMENT_LOCK_INPUT_WINDOW",0 ), NS_UITEXTCOLOR::WHITE  );
				if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
					CInnerInterface::GetInstance().OpenEquipmentLockInput();
			}
		}break;
	case EQUIPMENT_BUTTON_UNLOCK:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( m_pEquipmentMouseOver )
				{
					m_pEquipmentMouseOver->SetVisibleSingle(TRUE);
				}
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEWORD ( "RAN_EQUIPMENT_LOCK_INPUT_WINDOW",0 ), NS_UITEXTCOLOR::WHITE  );
				if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
					CInnerInterface::GetInstance().OpenEquipmentLockInput();
			}
		}break;
	case INVENTORY_BUTTON_LOCK:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( m_pInventoryMouseOver )
				{
					m_pInventoryMouseOver->SetVisibleSingle(TRUE);
				}
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEWORD ( "RAN_INVENTORY_LOCK_INPUT_WINDOW",0 ), NS_UITEXTCOLOR::WHITE  );
				if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
					CInnerInterface::GetInstance().OpenInventoryLockInput();
			}
		}break;
	case INVENTORY_BUTTON_UNLOCK:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( m_pInventoryMouseOver )
				{
					m_pInventoryMouseOver->SetVisibleSingle(TRUE);
				}
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEWORD ( "RAN_INVENTORY_LOCK_INPUT_WINDOW",0 ), NS_UITEXTCOLOR::WHITE  );
				if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
					CInnerInterface::GetInstance().OpenInventoryLockInput();
			}
		}break;

	case EQUIPMENT_BUTTON_ACTIVATE:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( m_pEquipmentMouseOver )
				{
					m_pEquipmentMouseOver->SetVisibleSingle(TRUE);
				}
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEWORD ( "RAN_EQUIPMENT_LOCK_INPUT_WINDOW",0 ), NS_UITEXTCOLOR::WHITE  );
				if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
					CInnerInterface::GetInstance().PrintMsgTextDlg(NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_INPUT_NOT_ENABLED"));
			}
		}break;

	case STORAGE_BUTTON_ACTIVATE:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( m_pStorageMouseOver )
				{
					m_pStorageMouseOver->SetVisibleSingle(TRUE);
				}
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEWORD ( "RAN_STORAGE_LOCK_INPUT_WINDOW",0 ), NS_UITEXTCOLOR::WHITE  );	
				if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
					CInnerInterface::GetInstance().PrintMsgTextDlg(NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_INPUT_NOT_ENABLED"));
			}
		}break;

	case INVENTORY_BUTTON_ACTIVATE:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( m_pInventoryMouseOver )
				{
					m_pInventoryMouseOver->SetVisibleSingle(TRUE);
				}
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEWORD ( "RAN_INVENTORY_LOCK_INPUT_WINDOW",0 ), NS_UITEXTCOLOR::WHITE  );
				if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
					CInnerInterface::GetInstance().PrintMsgTextDlg(NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_INPUT_NOT_ENABLED"));
			}
		}break;
	}
}
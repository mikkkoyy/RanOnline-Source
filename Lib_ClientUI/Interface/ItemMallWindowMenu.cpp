#include "StdAfx.h"
#include "ItemMallWindowMenu.h"
#include "ItemMallWindowMenuButton.h"

#include "BasicComboBox.h"
#include "BasicComboBoxRollOver.h"

#include "GameTextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CItemMallWindowMenu::CItemMallWindowMenu ()
	: m_wType( 0 )
	, m_nType( 2 )
{
	for(int i=0; i < 13; i++ )
		m_pMenuButton[i] = NULL;
}

CItemMallWindowMenu::~CItemMallWindowMenu ()
{
}

CItemMallWindowMenuButton* CItemMallWindowMenu::CreateMenu ( CString strKeyword, UIGUID ControlID, CString strOn, CString strOff )
{
	CItemMallWindowMenuButton* pSlot = new CItemMallWindowMenuButton;
	pSlot->CreateSub ( this, strKeyword.GetString (), UI_FLAG_DEFAULT, ControlID );
	pSlot->CreateSubControl( strOff, strOn );
	pSlot->SetVisibleSingle(TRUE);
	RegisterControl ( pSlot );

	return pSlot;
}

void CItemMallWindowMenu::CreateSubControl()
{
	for(int i=0; i < 13; i++ )
	{
		CString strKeyword, strKeyOff, strKeyOn;
		strKeyword.Format( "ITEM_COMPOUND_WINDOW_MENU_BUTTON0%d", i );
		strKeyOff.Format( "ITEM_COMPOUND_WINDOW_MENU_BUTTON_OFF0%d", i );
		strKeyOn.Format( "ITEM_COMPOUND_WINDOW_MENU_BUTTON_ON0%d", i );

		m_pMenuButton[i] = CreateMenu ( strKeyword, MENU_00 + i, strKeyOn, strKeyOff );
	}

	CBasicComboBox* pComboBox = NULL;
	CBasicComboBoxRollOver*	pComboBoxRollOver = NULL;

	pComboBox = new CBasicComboBox;
	pComboBox->CreateSub ( this, "BASIC_COMBOBOX", UI_FLAG_XSIZE, ITEMMALL_COMBO_OPEN );
	pComboBox->CreateBaseComboBox ( "ITEMMALL_COMBO_OPEN" );			
	RegisterControl ( pComboBox );
	m_pComboBoxTypeOpen = pComboBox;

	pComboBoxRollOver = new CBasicComboBoxRollOver;
	pComboBoxRollOver->CreateSub ( this, "BASIC_COMBOBOX_ROLLOVER", UI_FLAG_XSIZE | UI_FLAG_YSIZE, ITEMMALL_COMBO_ROLLOVER );
	pComboBoxRollOver->CreateBaseComboBoxRollOver ( "ITEMMALL_COMBO_ROLLOVER" );			
	pComboBoxRollOver->SetVisibleSingle ( FALSE );
	RegisterControl ( pComboBoxRollOver );
	m_pComboBoxTypeRollOver = pComboBoxRollOver;

	m_pComboBoxTypeRollOver->ClearText ();
	for ( int i = 0; i < 3; i++ )
	{
		m_pComboBoxTypeRollOver->AddText( (char*)ID2GAMEWORD("ITEMMALL_COMBO_OPTION", i) );
	}

	SetItemType(2);
	m_pComboBoxTypeOpen->SetText ( (char*)ID2GAMEWORD("ITEMMALL_COMBO_OPTION", m_nType) );
	
	SetType(0);
	m_pMenuButton[0]->SetOn();
}

void CItemMallWindowMenu::TranslateUIMessage( UIGUID ControlID, DWORD dwMsg )
{
	switch( ControlID )
	{
		case ITEMMALL_COMBO_OPEN:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( dwMsg & UIMSG_LB_DOWN )
				{
					if( !m_pComboBoxTypeRollOver->IsVisible() )	m_pComboBoxTypeRollOver->SetVisibleSingle ( TRUE );
					else										m_pComboBoxTypeRollOver->SetVisibleSingle ( FALSE );
					m_pComboBoxTypeRollOver->SetScrollPercent ( 0.0f );				
				}
			}
		}
		break;

	case ITEMMALL_COMBO_ROLLOVER:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{				
				int nIndex = m_pComboBoxTypeRollOver->GetSelectIndex ();
				if ( nIndex < 0 ) return ;

				m_pComboBoxTypeRollOver->SetVisibleSingle ( FALSE );
				m_pComboBoxTypeOpen->SetText ( (char*)ID2GAMEWORD("ITEMMALL_COMBO_OPTION", nIndex) );
				//set type here

				SetItemType( nIndex );
			}
		}
		break;

	case MENU_00:
	case MENU_01:
	case MENU_02:
	case MENU_03:
	case MENU_04:
	case MENU_05:
	case MENU_06:
	case MENU_07:
	case MENU_08:
	case MENU_09:
	case MENU_10:
	case MENU_11:
	case MENU_12:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE( dwMsg ))
			{
				int nIndex = ControlID - MENU_00;
				for(int i = 0; i < 13; i++ )
				{
					if ( i == nIndex ) 
						continue;

					m_pMenuButton[i]->SetOff();
				}

				SetType( nIndex );
			}
		}
		break;

	}
}
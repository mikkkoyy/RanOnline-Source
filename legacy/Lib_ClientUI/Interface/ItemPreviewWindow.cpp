#include "StdAfx.h"
#include "ItemPreviewWindow.h"
#include "ItemPreviewWindowRender.h"

#include "InnerInterface.h"
#include "BasicLineBox.h"
#include "ItemImage.h"
#include "BasicTextBox.h"
#include "BasicButton.h"

#include "UITextControl.h"
#include "GameTextControl.h"
#include "../Lib_Engine/DxCommon/d3dfont.h"

#include "GLItemMan.h"
#include "GLGaeaClient.h"
#include "DxGlobalStage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CItemPreviewWindow::CItemPreviewWindow () 
	: m_pRender(NULL)
	, m_pButtonLeft(NULL)
	, m_pButtonRight(NULL)
	, m_pStanceButton(NULL)
	, m_pStanceText(NULL)
{
	for( int i=0; i<EMITEM_PREVIEW_SLOT_TOTAL; ++ i )
	{
		m_pSlotItem[i] = NULL;
		m_pSlotText[i] = NULL;
	}
}

CItemPreviewWindow::~CItemPreviewWindow ()
{
}

void CItemPreviewWindow::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "ITEMPREVIEW_WINDOW_BKLINE" );
	RegisterControl ( pBasicLineBox );

	std::string strSlot[EMITEM_PREVIEW_SLOT_TOTAL] =
	{
		"ITEMPREVIEW_SLOT_HEAD",
		"ITEMPREVIEW_SLOT_UPPER",
		"ITEMPREVIEW_SLOT_LOWER",
		"ITEMPREVIEW_SLOT_HAND",
		"ITEMPREVIEW_SLOT_FOOT",
		"ITEMPREVIEW_SLOT_WEAPON",
		//new item preview
		"ITEMPREVIEW_SLOT_WAIST",
		"ITEMPREVIEW_SLOT_WRIST",
		"ITEMPREVIEW_SLOT_NECK",
		"ITEMPREVIEW_SLOT_FACE",
		"ITEMPREVIEW_SLOT_MISC"
	};

	std::string strText[EMITEM_PREVIEW_SLOT_TOTAL] =
	{
		"ITEMPREVIEW_TEXTBOX_HEAD",
		"ITEMPREVIEW_TEXTBOX_UPPER",
		"ITEMPREVIEW_TEXTBOX_LOWER",
		"ITEMPREVIEW_TEXTBOX_HAND",
		"ITEMPREVIEW_TEXTBOX_FOOT",
		"ITEMPREVIEW_TEXTBOX_WEAPON",
		//new item preview
		"ITEMPREVIEW_TEXTBOX_WAIST",
		"ITEMPREVIEW_TEXTBOX_WRIST",
		"ITEMPREVIEW_TEXTBOX_NECK",
		"ITEMPREVIEW_TEXTBOX_FACE",
		"ITEMPREVIEW_TEXTBOX_MISC"
	};

	for( int i=0; i<EMITEM_PREVIEW_SLOT_TOTAL; ++ i )
	{
		m_pSlotItem[i] = new CItemImage;
		m_pSlotItem[i]->CreateSub ( this, strSlot[i].c_str(), UI_FLAG_DEFAULT, ITEM_PREVIEW_ITEMSLOT_HEAD + i );
		m_pSlotItem[i]->CreateSubControl ();
		RegisterControl ( m_pSlotItem[i] );

		m_pSlotText[i] = new CBasicTextBox;
		m_pSlotText[i]->CreateSub ( this, strText[i].c_str() );
		m_pSlotText[i]->SetFont ( pFont9 );
		m_pSlotText[i]->SetText( ID2GAMEWORD( "ITEMPREVIEW_TEXTBOX", i + 1 ), NS_UITEXTCOLOR::YELLOW );
		RegisterControl ( m_pSlotText[i] );
	}

	m_pButtonLeft = new CBasicButton;
	m_pButtonLeft->CreateSub ( this, "ITEMPREVIEW_WINDOW_ARROW_TURN_LEFT_DEFAULT", UI_FLAG_DEFAULT, ITEM_PREVIEW_BUTTON_LEFT );
	m_pButtonLeft->CreateFlip ( "ITEMPREVIEW_WINDOW_ARROW_TURN_LEFT_CLICK", CBasicButton::CLICK_FLIP );
	m_pButtonLeft->CreateMouseOver ( "ITEMPREVIEW_WINDOW_ARROW_TURN_LEFT_OVER" );
	m_pButtonLeft->SetVisibleSingle( TRUE );
	RegisterControl ( m_pButtonLeft );

	m_pButtonRight = new CBasicButton;
	m_pButtonRight->CreateSub ( this, "ITEMPREVIEW_WINDOW_ARROW_TURN_RIGHT_DEFAULT", UI_FLAG_DEFAULT, ITEM_PREVIEW_BUTTON_RIGHT );
	m_pButtonRight->CreateFlip ( "ITEMPREVIEW_WINDOW_ARROW_TURN_RIGHT_CLICK", CBasicButton::CLICK_FLIP );
	m_pButtonRight->CreateMouseOver ( "ITEMPREVIEW_WINDOW_ARROW_TURN_RIGHT_OVER" );
	m_pButtonRight->SetVisibleSingle( TRUE );
	RegisterControl ( m_pButtonRight );

	m_pStanceButton = new CBasicButton;
	m_pStanceButton->CreateSub ( this, "ITEMPREVIEW_STANCE_BUTTON", UI_FLAG_DEFAULT, ITEM_PREVIEW_STANCE_BUTTON );
	m_pStanceButton->CreateFlip ( "ITEMPREVIEW_STANCE_BUTTON_ON", CBasicButton::RADIO_FLIP );
	m_pStanceButton->SetControlNameEx ( "ITEMPREVIEW_STANCE_BUTTON" );
	m_pStanceButton->SetFlip( RANPARAM::bItemPreviewStance );
	RegisterControl ( m_pStanceButton );

	m_pStanceText = new CBasicTextBox;
	m_pStanceText->CreateSub ( this, "ITEMPREVIEW_STANCE_TEXT" );
	m_pStanceText->SetFont ( pFont9 );
	m_pStanceText->SetText( ID2GAMEWORD( "ITEMPREVIEW_TEXTBOX", 0 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pStanceText );

	m_pRender = new CItemPreviewWindowRender;
	m_pRender->CreateSub ( this, "ITEMPREVIEW_WINDOW_MESHRENDER" );
	m_pRender->CreateSubControl ();
	RegisterControl ( m_pRender );
}

void CItemPreviewWindow::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( m_pRender )
	{
		for( int i=0; i<EMITEM_PREVIEW_SLOT_TOTAL; ++ i )
		{
			SITEM* pItem = GLItemMan::GetInstance().GetItem( m_pRender->m_sPreviewData.sPutOn[i].sNativeID );
			if ( pItem )
			{
				m_pSlotItem[i]->SetItem( m_pRender->m_sPreviewData.sPutOn[i].sNativeID );
			}else{
				m_pSlotItem[i]->ResetItem();
			}
		}
	}
}

void CItemPreviewWindow::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	switch( ControlID )
	{
	case ITEM_PREVIEW_BUTTON_LEFT:
		{
			if ( CHECK_MOUSE_IN_LBDOWNLIKE ( dwMsg ) )
			{
				if ( m_pRender )	m_pRender->m_vRot.x -=0.1f;
			}
		}break;
	case ITEM_PREVIEW_BUTTON_RIGHT:
		{
			if ( CHECK_MOUSE_IN_LBDOWNLIKE ( dwMsg ) )
			{
				if ( m_pRender )	m_pRender->m_vRot.x +=0.1f;
				
			}
		}break;

	case ITEM_PREVIEW_ITEMSLOT_HEAD:
	case ITEM_PREVIEW_ITEMSLOT_UPPER:
	case ITEM_PREVIEW_ITEMSLOT_LOWER:
	case ITEM_PREVIEW_ITEMSLOT_HAND:
	case ITEM_PREVIEW_ITEMSLOT_FOOT:
	case ITEM_PREVIEW_ITEMSLOT_WEAPON:
	//new item preview
	case ITEM_PREVIEW_ITEMSLOT_WAIST:
	case ITEM_PREVIEW_ITEMSLOT_WRIST:
	case ITEM_PREVIEW_ITEMSLOT_NECK:
	case ITEM_PREVIEW_ITEMSLOT_FACE:
	case ITEM_PREVIEW_ITEMSLOT_MISC:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{	
				int nIndex = ControlID - ITEM_PREVIEW_ITEMSLOT_HEAD;

				if ( nIndex < 0 || nIndex >= EMITEM_PREVIEW_SLOT_TOTAL )	return;

				if ( m_pRender )
				{
					SITEMCUSTOM sItem = m_pRender->m_sPreviewData.sPutOn[nIndex];

					if( sItem.sNativeID != NATIVEID_NULL () )
					{
						//CInnerInterface::GetInstance().SHOW_ITEM_INFO ( sItem, FALSE, FALSE, FALSE, USHRT_MAX, USHRT_MAX );
						CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW ( sItem, FALSE, FALSE, FALSE, FALSE, FALSE, USHRT_MAX, USHRT_MAX );
					}
				}
			}
		}break;

	case ITEM_PREVIEW_STANCE_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				RANPARAM::bItemPreviewStance = !RANPARAM::bItemPreviewStance;
				m_pStanceButton->SetFlip( RANPARAM::bItemPreviewStance );

				if ( m_pRender )
					m_pRender->ChangeStance();
			}
		}break;
	};

	CUIWindowEx::TranslateUIMessage ( ControlID, dwMsg );
}

void CItemPreviewWindow::SetVisibleSingle ( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle( bVisible );

	if( bVisible )
	{
	}else{
		ClearRender();
	}
}

void CItemPreviewWindow::ClearRender()
{
	if ( m_pRender )
		m_pRender->ClearRender();
}

void CItemPreviewWindow::StartPreview( SITEMCUSTOM sItem )
{
	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( !pCharacter )		return;

	if ( !m_pRender )	return;

	m_pRender->m_sPreviewData.wFace = pCharacter->m_wFace;
	m_pRender->m_sPreviewData.wHair = pCharacter->m_wHair;
	m_pRender->m_sPreviewData.wHairColor = pCharacter->m_wHairColor;

	m_pRender->m_sPreviewData.sPutOn[EMITEM_PREVIEW_SLOT_HEAD] = pCharacter->m_PutOnItems[SLOT_HEADGEAR]; //head
	m_pRender->m_sPreviewData.sPutOn[EMITEM_PREVIEW_SLOT_UPPER] = pCharacter->m_PutOnItems[SLOT_UPPER]; //upper
	m_pRender->m_sPreviewData.sPutOn[EMITEM_PREVIEW_SLOT_LOWER] = pCharacter->m_PutOnItems[SLOT_LOWER]; //lower
	m_pRender->m_sPreviewData.sPutOn[EMITEM_PREVIEW_SLOT_HAND] = pCharacter->m_PutOnItems[SLOT_HAND]; //hand
	m_pRender->m_sPreviewData.sPutOn[EMITEM_PREVIEW_SLOT_FOOT] = pCharacter->m_PutOnItems[SLOT_FOOT]; //foot
	m_pRender->m_sPreviewData.sPutOn[EMITEM_PREVIEW_SLOT_WEAPON] = pCharacter->m_PutOnItems[ pCharacter->IsUseArmSub()? SLOT_RHAND_S : SLOT_RHAND ]; //weapon

	//new item preview
	m_pRender->m_sPreviewData.sPutOn[EMITEM_PREVIEW_SLOT_WAIST] = pCharacter->m_PutOnItems[SLOT_WAIST]; //foot
	m_pRender->m_sPreviewData.sPutOn[EMITEM_PREVIEW_SLOT_WRIST] = pCharacter->m_PutOnItems[SLOT_WRIST]; //foot
	m_pRender->m_sPreviewData.sPutOn[EMITEM_PREVIEW_SLOT_NECK] = pCharacter->m_PutOnItems[SLOT_NECK]; //foot
	m_pRender->m_sPreviewData.sPutOn[EMITEM_PREVIEW_SLOT_FACE] = pCharacter->m_PutOnItems[SLOT_FACE]; //foot
	m_pRender->m_sPreviewData.sPutOn[EMITEM_PREVIEW_SLOT_MISC] = pCharacter->m_PutOnItems[SLOT_MISC]; //foot

	SITEM* pItemCheck = GLItemMan::GetInstance().GetItem( sItem.sNativeID );
	if ( !pItemCheck )	return;

	if ( pItemCheck->sBasicOp.emItemType == ITEM_SUIT )
	{
		bool bCan = CInnerInterface::GetInstance().PreviewItemCheckWear( pItemCheck->sBasicOp.sNativeID );
		if ( !bCan )	return;

		int nSlotNum = -1;
		switch ( pItemCheck->sSuitOp.emSuit )
		{
		case SUIT_HEADGEAR:		nSlotNum = EMITEM_PREVIEW_SLOT_HEAD;	break;
		case SUIT_UPPER:		nSlotNum = EMITEM_PREVIEW_SLOT_UPPER;	break;
		case SUIT_LOWER:		nSlotNum = EMITEM_PREVIEW_SLOT_LOWER;	break;
		case SUIT_HAND:			nSlotNum = EMITEM_PREVIEW_SLOT_HAND;	break;
		case SUIT_FOOT:			nSlotNum = EMITEM_PREVIEW_SLOT_FOOT;	break;
		case SUIT_HANDHELD:		nSlotNum = EMITEM_PREVIEW_SLOT_WEAPON;	break;
		//new item preview
		case SUIT_BELT:			nSlotNum = EMITEM_PREVIEW_SLOT_WAIST;	break;
		case SUIT_WRIST:		nSlotNum = EMITEM_PREVIEW_SLOT_WRIST;	break;
		case SUIT_NECK:			nSlotNum = EMITEM_PREVIEW_SLOT_NECK;	break;
		case SUIT_FACE:			nSlotNum = EMITEM_PREVIEW_SLOT_FACE;	break;
		case SUIT_MISC:			nSlotNum = EMITEM_PREVIEW_SLOT_MISC;	break;
		};

		if ( nSlotNum >= 0 )	m_pRender->m_sPreviewData.sPutOn[nSlotNum] = sItem;
	}
	else if ( pItemCheck->sBasicOp.emItemType == ITEM_BOX )
	{
		for ( int i=0; i<ITEM::SBOX::ITEM_SIZE; ++i )
		{
			SITEMCUSTOM sItemCustomBox;
			sItemCustomBox.sNativeID = pItemCheck->sBox.sITEMS[i].nidITEM;

			SITEM* pItemBox = GLItemMan::GetInstance().GetItem( sItemCustomBox.sNativeID );
			if( pItemBox )
			{
				if ( pItemBox->sBasicOp.emItemType != ITEM_SUIT )	continue;

				bool bCan = CInnerInterface::GetInstance().PreviewItemCheckWear( pItemBox->sBasicOp.sNativeID );
				if ( !bCan )	continue;

				int nSlotNum = -1;
				switch ( pItemBox->sSuitOp.emSuit )
				{
				case SUIT_HEADGEAR:		nSlotNum = EMITEM_PREVIEW_SLOT_HEAD;	break;
				case SUIT_UPPER:		nSlotNum = EMITEM_PREVIEW_SLOT_UPPER;	break;
				case SUIT_LOWER:		nSlotNum = EMITEM_PREVIEW_SLOT_LOWER;	break;
				case SUIT_HAND:			nSlotNum = EMITEM_PREVIEW_SLOT_HAND;	break;
				case SUIT_FOOT:			nSlotNum = EMITEM_PREVIEW_SLOT_FOOT;	break;
				case SUIT_HANDHELD:		nSlotNum = EMITEM_PREVIEW_SLOT_WEAPON;	break;
				//new item preview

				case SUIT_BELT:			nSlotNum = EMITEM_PREVIEW_SLOT_WAIST;	break;
				case SUIT_WRIST:		nSlotNum = EMITEM_PREVIEW_SLOT_WRIST;	break;
				case SUIT_NECK:			nSlotNum = EMITEM_PREVIEW_SLOT_NECK;	break;
				case SUIT_FACE:			nSlotNum = EMITEM_PREVIEW_SLOT_FACE;	break;
				case SUIT_MISC:			nSlotNum = EMITEM_PREVIEW_SLOT_MISC;	break;
				}

				if ( nSlotNum >= 0 )	m_pRender->m_sPreviewData.sPutOn[nSlotNum] = sItemCustomBox;
			}
		}
	}

	ClearRender();
	m_pRender->m_bReady = true;
}
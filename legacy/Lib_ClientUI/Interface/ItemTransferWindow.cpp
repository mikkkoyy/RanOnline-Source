#include "StdAfx.h"
#include "ItemTransferWindow.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "ItemImage.h"
#include "ItemMove.h"
#include "BasicTextButton.h"

#include "d3dfont.h"
#include "UITextControl.h"
#include "GameTextControl.h"
#include "InnerInterface.h"
#include "GLGaeaClient.h"
#include "GLItemMan.h"

#include "RANPARAM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CItemTransferWindow::CItemTransferWindow()
	: m_pTextInfo(NULL)
	, m_pItemImageOld(NULL)
	, m_pItemImageNew(NULL)
	, m_pItemImageResult(NULL)
	, m_pArrowBad(NULL)
	, m_pArrowGood(NULL)
	, m_pTextItemOld(NULL)
	, m_pTextItemNew(NULL)
	, m_pTextItemResult(NULL)
	, m_pTextCost(NULL)
	, m_pButtonStart(NULL)
	, m_pButtonCancel(NULL)
{
}

CItemTransferWindow::~CItemTransferWindow()
{
}


void CItemTransferWindow::CreateSubControl()
{
	CD3DFontPar* pFont08 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "ITEM_TRANSFER_WINDOW_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "ITEM_TRANSFER_WINDOW_INFO_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "ITEM_TRANSFER_WINDOW_TEXT_INFO_OLD" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "ITEM_TRANSFER_WINDOW_TEXT_INFO_NEW" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "ITEM_TRANSFER_WINDOW_TEXT_INFO_RESULT" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "ITEM_TRANSFER_WINDOW_TEXT_INFO_COST" );
	RegisterControl ( pBasicLineBox );

	m_pTextInfo = new CBasicTextBox;
	m_pTextInfo->CreateSub ( this, "ITEM_TRANSFER_WINDOW_INFO_TEXT", UI_FLAG_DEFAULT );
	m_pTextInfo->SetFont ( pFont08 );
	m_pTextInfo->SetTextAlign ( TEXT_ALIGN_LEFT );
	m_pTextInfo->SetText( ID2GAMEINTEXT( "ITEM_TRANSFER_WINDOW_INFO" ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextInfo );

	m_pItemImageOld = new CItemImage;
	m_pItemImageOld->CreateSub( this, "ITEM_TRANSFER_WINDOW_ITEM_SLOT_OLD", UI_FLAG_DEFAULT, ITEM_TRANSFER_ITEM_IMAGE_ITEM_OLD );
	m_pItemImageOld->CreateSubControl();
	RegisterControl( m_pItemImageOld );

	m_pItemImageNew = new CItemImage;
	m_pItemImageNew->CreateSub( this, "ITEM_TRANSFER_WINDOW_ITEM_SLOT_NEW", UI_FLAG_DEFAULT, ITEM_TRANSFER_ITEM_IMAGE_ITEM_NEW );
	m_pItemImageNew->CreateSubControl();
	RegisterControl( m_pItemImageNew );

	m_pItemImageResult = new CItemImage;
	m_pItemImageResult->CreateSub( this, "ITEM_TRANSFER_WINDOW_ITEM_SLOT_RESULT", UI_FLAG_DEFAULT, ITEM_TRANSFER_ITEM_IMAGE_ITEM_RESULT );
	m_pItemImageResult->CreateSubControl();
	RegisterControl( m_pItemImageResult );

	m_pArrowBad = new CUIControl;
	m_pArrowBad->CreateSub ( this, "ITEM_TRANSFER_WINDOW_ARROW_BAD", UI_FLAG_DEFAULT );	
	m_pArrowBad->SetVisibleSingle( FALSE );
	RegisterControl ( m_pArrowBad );

	m_pArrowGood = new CUIControl;
	m_pArrowGood->CreateSub ( this, "ITEM_TRANSFER_WINDOW_ARROW_GOOD", UI_FLAG_DEFAULT );	
	m_pArrowGood->SetVisibleSingle( FALSE );
	RegisterControl ( m_pArrowGood );

	m_pTextItemOld = new CBasicTextBox;
	m_pTextItemOld->CreateSub ( this, "ITEM_TRANSFER_WINDOW_TEXT_INFO_OLD", UI_FLAG_DEFAULT );
	m_pTextItemOld->SetFont ( pFont08 );
	m_pTextItemOld->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextItemOld->SetText( ID2GAMEWORD( "ITEM_TRANSFER_WINDOW", 1 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextItemOld );

	m_pTextItemNew = new CBasicTextBox;
	m_pTextItemNew->CreateSub ( this, "ITEM_TRANSFER_WINDOW_TEXT_INFO_NEW", UI_FLAG_DEFAULT );
	m_pTextItemNew->SetFont ( pFont08 );
	m_pTextItemNew->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextItemNew->SetText( ID2GAMEWORD( "ITEM_TRANSFER_WINDOW", 2 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextItemNew );

	m_pTextItemResult = new CBasicTextBox;
	m_pTextItemResult->CreateSub ( this, "ITEM_TRANSFER_WINDOW_TEXT_INFO_RESULT", UI_FLAG_DEFAULT );
	m_pTextItemResult->SetFont ( pFont08 );
	m_pTextItemResult->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextItemResult->SetText( ID2GAMEWORD( "ITEM_TRANSFER_WINDOW", 3 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextItemResult );

	m_pTextCost = new CBasicTextBox;
	m_pTextCost->CreateSub ( this, "ITEM_TRANSFER_WINDOW_TEXT_INFO_COST", UI_FLAG_DEFAULT );
	m_pTextCost->SetFont ( pFont08 );
	m_pTextCost->SetTextAlign ( TEXT_ALIGN_RIGHT | TEXT_ALIGN_CENTER_Y );
	m_pTextCost->SetText( "", NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextCost );


	m_pButtonStart = new CBasicTextButton;
	m_pButtonStart->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, ITEM_TRANSFER_BUTTON_START );
	m_pButtonStart->CreateBaseButton ( "ITEM_TRANSFER_WINDOW_BUTTON_START", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, ID2GAMEWORD("ITEM_TRANSFER_WINDOW", 4 ) );
	RegisterControl ( m_pButtonStart );

	m_pButtonCancel = new CBasicTextButton;
	m_pButtonCancel->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, ITEM_TRANSFER_BUTTON_CANCEL );
	m_pButtonCancel->CreateBaseButton ( "ITEM_TRANSFER_WINDOW_BUTTON_CANCEL", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, ID2GAMEWORD("ITEM_TRANSFER_WINDOW", 5 ) );
	RegisterControl ( m_pButtonCancel );
}

void CItemTransferWindow::Update( INT x, INT y, BYTE LB, BYTE MB, BYTE RB, INT nScroll, FLOAT fElapsedTime, BOOL bFirstControl )
{
	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( !pCharacter )	return;
	
	if ( m_pItemImageOld )
	{
		const SITEMCUSTOM sitemcustom = pCharacter->GetItemTransferItemOld();

		if( sitemcustom.sNativeID == NATIVEID_NULL() ){
			m_pItemImageOld->ResetItem();
		}else{
			SITEM* pItem = GLItemMan::GetInstance().GetItem( sitemcustom.sNativeID );
			if( pItem )
				m_pItemImageOld->SetItem( sitemcustom );
			else
				m_pItemImageOld->ResetItem();
		}
	}
	
	if ( m_pItemImageNew )
	{
		const SITEMCUSTOM sitemcustom = pCharacter->GetItemTransferItemNew();

		if( sitemcustom.sNativeID == NATIVEID_NULL() ){
			m_pItemImageNew->ResetItem();
		}else{
			SITEM* pItem = GLItemMan::GetInstance().GetItem( sitemcustom.sNativeID );
			if( pItem )
				m_pItemImageNew->SetItem( sitemcustom );
			else
				m_pItemImageNew->ResetItem();
		}
	}

	if ( m_pItemImageResult )
	{
		const SITEMCUSTOM sitemcustom_old = pCharacter->GetItemTransferItemOld();
		const SITEMCUSTOM sitemcustom_new = pCharacter->GetItemTransferItemNew();

		SITEM* pitem_data_old = GLItemMan::GetInstance().GetItem( sitemcustom_old.sNativeID );
		SITEM* pitem_data_new = GLItemMan::GetInstance().GetItem( sitemcustom_new.sNativeID );
		if ( pitem_data_old && pitem_data_new )
		{
			m_pItemImageResult->SetItem( sitemcustom_new );

			if ( m_pArrowBad )	m_pArrowBad->SetVisibleSingle( FALSE );
			if ( m_pArrowGood ) m_pArrowGood->SetVisibleSingle( TRUE );
		}
		else
		{
			m_pItemImageResult->ResetItem();

			if ( m_pArrowBad )	m_pArrowBad->SetVisibleSingle( TRUE );
			if ( m_pArrowGood ) m_pArrowGood->SetVisibleSingle( FALSE );
		}
	}

	if ( m_pTextCost )
	{
		m_pTextCost->ClearText();

		if ( RANPARAM::dwFeatureItemTransferCost )
		{
			CString strTemp = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( RANPARAM::dwFeatureItemTransferCost, 3, "," );
			int nIndex = m_pTextCost->AddText( ID2GAMEWORD( "ITEM_TRANSFER_WINDOW", 6 ), NS_UITEXTCOLOR::WHITE );
			m_pTextCost->AddString( nIndex, strTemp.GetString(), NS_UITEXTCOLOR::GOLD );
		}
	}
}

void CItemTransferWindow::TranslateUIMessage( UIGUID ControlID, DWORD dwMsg )
{
	switch( ControlID )
	{
	case ITEM_TRANSFER_ITEM_IMAGE_ITEM_OLD:
		{
			if( CHECK_MOUSE_IN( dwMsg ) )
			{
				CItemMove* pItemMove = CInnerInterface::GetInstance().GetItemMove();
				if( !pItemMove )	break;

				SNATIVEID sItem = pItemMove->GetItem();
				if( sItem != NATIVEID_NULL() )
				{
					const UIRECT& rcSlotPos = m_pItemImageOld->GetGlobalPos();
					pItemMove->SetGlobalPos( rcSlotPos );
					CInnerInterface::GetInstance().SetSnapItem();
				}

				if( dwMsg & UIMSG_LB_UP )
				{
					GLGaeaClient::GetInstance().GetCharacter()->SetItemTransferMoveItemOld();
				}

				if( dwMsg & UIMSG_RB_UP )
				{
					GLGaeaClient::GetInstance().GetCharacter()->ReSetItemTransferItemOld();
				}

				SITEMCUSTOM sItemCustom = GLGaeaClient::GetInstance().GetCharacter()->GetItemTransferItemOld();
				if( sItemCustom.sNativeID != NATIVEID_NULL() )
					CInnerInterface::GetInstance().SHOW_ITEM_INFO( sItemCustom, FALSE, FALSE, FALSE, 0, 0 );

				/*item preview, Juver, 2017/07/27 */
				if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LMENU  ) & DXKEY_DOWNED )
				{
					if ( dwMsg & UIMSG_LB_UP )
					{
						CInnerInterface::GetInstance().PreviewItem( sItemCustom );
						return;
					}
				}

				/*box contents, Juver, 2017/08/30 */
				if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LMENU  ) & DXKEY_DOWNED )
				{
					if ( dwMsg & UIMSG_RB_UP )
					{
						CInnerInterface::GetInstance().ShowBoxContents( sItemCustom.sNativeID );
						return;
					}
				}
			}
		}break;

	case ITEM_TRANSFER_ITEM_IMAGE_ITEM_NEW:
		{
			if( CHECK_MOUSE_IN( dwMsg ) )
			{
				CItemMove* pItemMove = CInnerInterface::GetInstance().GetItemMove();
				if( !pItemMove )	break;

				SNATIVEID sItem = pItemMove->GetItem();
				if( sItem != NATIVEID_NULL() )
				{
					const UIRECT& rcSlotPos = m_pItemImageNew->GetGlobalPos();
					pItemMove->SetGlobalPos( rcSlotPos );
					CInnerInterface::GetInstance().SetSnapItem();
				}

				if( dwMsg & UIMSG_LB_UP )
				{
					GLGaeaClient::GetInstance().GetCharacter()->SetItemTransferMoveItemNew();
				}

				if( dwMsg & UIMSG_RB_UP )
				{
					GLGaeaClient::GetInstance().GetCharacter()->ReSetItemTransferItemNew();
				}

				SITEMCUSTOM sItemCustom = GLGaeaClient::GetInstance().GetCharacter()->GetItemTransferItemNew();
				if( sItemCustom.sNativeID != NATIVEID_NULL() )
					CInnerInterface::GetInstance().SHOW_ITEM_INFO( sItemCustom, FALSE, FALSE, FALSE, 0, 0 );

				/*item preview, Juver, 2017/07/27 */
				if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LMENU  ) & DXKEY_DOWNED )
				{
					if ( dwMsg & UIMSG_LB_UP )
					{
						CInnerInterface::GetInstance().PreviewItem( sItemCustom );
						return;
					}
				}

				/*box contents, Juver, 2017/08/30 */
				if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LMENU  ) & DXKEY_DOWNED )
				{
					if ( dwMsg & UIMSG_RB_UP )
					{
						CInnerInterface::GetInstance().ShowBoxContents( sItemCustom.sNativeID );
						return;
					}
				}
			}
		}break;

	case ITEM_TRANSFER_ITEM_IMAGE_ITEM_RESULT:
		{
			if( CHECK_MOUSE_IN( dwMsg ) )
			{
				SITEMCUSTOM sItemCustom_OLD = GLGaeaClient::GetInstance().GetCharacter()->GetItemTransferItemOld();
				SITEMCUSTOM sItemCustom_NEW = GLGaeaClient::GetInstance().GetCharacter()->GetItemTransferItemNew();
				if( sItemCustom_OLD.sNativeID != NATIVEID_NULL() && sItemCustom_NEW.sNativeID != NATIVEID_NULL() )
				{
					sItemCustom_OLD.sNativeID = sItemCustom_NEW.sNativeID;
					CInnerInterface::GetInstance().SHOW_ITEM_INFO( sItemCustom_OLD, FALSE, FALSE, FALSE, 0, 0 );

					/*item preview, Juver, 2017/07/27 */
					if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LMENU  ) & DXKEY_DOWNED )
					{
						if ( dwMsg & UIMSG_LB_UP )
						{
							CInnerInterface::GetInstance().PreviewItem( sItemCustom_OLD );
							return;
						}
					}

					/*box contents, Juver, 2017/08/30 */
					if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LMENU  ) & DXKEY_DOWNED )
					{
						if ( dwMsg & UIMSG_RB_UP )
						{
							CInnerInterface::GetInstance().ShowBoxContents( sItemCustom_OLD.sNativeID );
							return;
						}
					}
				}
			}
		}break;

	case ITEM_TRANSFER_BUTTON_CANCEL:
		if( CHECK_KEYFOCUSED( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			CInnerInterface::GetInstance().CloseItemTransferWindow();
		break;

	case ET_CONTROL_BUTTON:
		if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			CInnerInterface::GetInstance().CloseItemTransferWindow();
		break;

	case ITEM_TRANSFER_BUTTON_START:
		if( CHECK_KEYFOCUSED( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			TransferItemStats();
		break;
	}

	CUIWindowEx::TranslateUIMessage( ControlID, dwMsg );
}

void CItemTransferWindow::SetVisibleSingle( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle( bVisible );
}

void CItemTransferWindow::TransferItemStats()
{
	const SITEMCUSTOM sitemcustom_old = GLGaeaClient::GetInstance().GetCharacter()->GetItemTransferItemOld();
	const SITEMCUSTOM sitemcustom_new = GLGaeaClient::GetInstance().GetCharacter()->GetItemTransferItemNew();

	SITEM* pitem_data_old = GLItemMan::GetInstance().GetItem( sitemcustom_old.sNativeID );
	SITEM* pitem_data_new = GLItemMan::GetInstance().GetItem( sitemcustom_new.sNativeID );
	if ( !pitem_data_old || !pitem_data_new )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_INVALID_ITEM") );
		return;
	}

	if ( pitem_data_old->sBasicOp.emItemType != ITEM_SUIT || pitem_data_new->sBasicOp.emItemType != ITEM_SUIT )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_INVALID_ITEM_TYPE") );
		return;
	}

	if ( pitem_data_old->IsTIMELMT() || pitem_data_new->IsTIMELMT() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_TIME_LIMIT") );
		return;
	}

	if ( pitem_data_old->sBasicOp.IsDISGUISE() || pitem_data_new->sBasicOp.IsDISGUISE() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_DISGUISE") );
		return;
	}

	if ( pitem_data_old->sSuitOp.emSuit != pitem_data_new->sSuitOp.emSuit )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_SUIT_MISMATCH") );
		return;
	}

	if ( pitem_data_old->sSuitOp.emSuit == SUIT_HANDHELD && pitem_data_old->sSuitOp.emAttack != pitem_data_new->sSuitOp.emAttack )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_ATTACK_MISMATCH") );
		return;
	}

	if ( RANPARAM::dwFeatureItemTransferCost != 0 && GLGaeaClient::GetInstance().GetCharacter()->m_lnMoney < RANPARAM::dwFeatureItemTransferCost ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_COST") );
		return;
	}

	if ( sitemcustom_old.nidDISGUISE != NATIVEID_NULL() || sitemcustom_new.nidDISGUISE != NATIVEID_NULL() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_HAVE_DISGUISE") );
		return;
	}

	GLGaeaClient::GetInstance().GetCharacter()->ItemTransferStart();
}

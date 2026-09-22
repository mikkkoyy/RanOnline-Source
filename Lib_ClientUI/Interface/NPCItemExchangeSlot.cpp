#include "StdAfx.h"
#include "NPCItemExchangeSlot.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "ItemImage.h"
#include "BasicTextButton.h"

#include "UITextControl.h"
#include "GameTextControl.h"
#include "GLItemMan.h"
#include "GLItem.h"
#include "ModalWindow.h"
#include "ModalCallerID.h"

#include "GLGaeaClient.h"
#include "InnerInterface.h"

#include "../Lib_Engine/DxCommon/d3dfont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CNPCItemExchangeSlot::CNPCItemExchangeSlot ()
	: m_pTextName(NULL)
	, m_pItemImageResult(NULL)
	, m_sNPCID( NATIVEID_NULL() )
	, m_sDATA( SNPC_ITEM_EXCHANGE_DATA() )
	, m_pButton(NULL)
{
	for ( int i=0; i<NPC_ITEM_EXCHANGE_MAX_REQ; ++i )
		m_pItemImageRequire[i] = NULL;
}

CNPCItemExchangeSlot::~CNPCItemExchangeSlot ()
{
}

void CNPCItemExchangeSlot::CreateSubControl ()
{
	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList( "NPC_ITEM_EXCHANGE_NODE_BACK" );
	RegisterControl ( pBasicLineBox );

	m_pTextName = new CBasicTextBox;
	m_pTextName->CreateSub ( this, "NPC_ITEM_EXCHANGE_NODE_NAME" );
	m_pTextName->SetFont ( pFont8 );
	m_pTextName->SetTextAlign( TEXT_ALIGN_LEFT );
	RegisterControl ( m_pTextName );
	
	std::string stritem_req[NPC_ITEM_EXCHANGE_MAX_REQ] = 
	{
		"NPC_ITEM_EXCHANGE_NODE_ITEM_IMAGE_REQ_0",
		"NPC_ITEM_EXCHANGE_NODE_ITEM_IMAGE_REQ_1",
		"NPC_ITEM_EXCHANGE_NODE_ITEM_IMAGE_REQ_2",
		"NPC_ITEM_EXCHANGE_NODE_ITEM_IMAGE_REQ_3",
		"NPC_ITEM_EXCHANGE_NODE_ITEM_IMAGE_REQ_4",
	};

	for( int i=0; i<NPC_ITEM_EXCHANGE_MAX_REQ; ++i )
	{
		m_pItemImageRequire[i] = new CItemImage;
		m_pItemImageRequire[i]->CreateSub ( this, stritem_req[i].c_str(), UI_FLAG_DEFAULT, NPC_ITEM_EXCHANGE_SLOT_ITEM_IMAGE_REQ_0 + i );
		m_pItemImageRequire[i]->CreateSubControl ();
		m_pItemImageRequire[i]->CreateTextBoxDownRight();
		RegisterControl( m_pItemImageRequire[i] );
	}

	m_pItemImageResult = new CItemImage;
	m_pItemImageResult->CreateSub ( this, "NPC_ITEM_EXCHANGE_NODE_ITEM_IMAGE_RES", UI_FLAG_DEFAULT, NPC_ITEM_EXCHANGE_SLOT_ITEM_IMAGE_RES );
	m_pItemImageResult->CreateSubControl ();
	m_pItemImageResult->CreateTextBoxDownRight();
	RegisterControl ( m_pItemImageResult );

	m_pButton = new CBasicTextButton;
	m_pButton->CreateSub ( this, "BASIC_TEXT_BUTTON18", UI_FLAG_XSIZE, NPC_ITEM_EXCHANGE_SLOT_BUTTON );
	m_pButton->CreateBaseButton ( "NPC_ITEM_EXCHANGE_NODE_BUTTON", CBasicTextButton::SIZE18, CBasicButton::CLICK_FLIP, ID2GAMEWORD("NPC_ITEM_EXCHANGE_TEXT", 3) );
	RegisterControl ( m_pButton );
}

void CNPCItemExchangeSlot::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case NPC_ITEM_EXCHANGE_SLOT_ITEM_IMAGE_RES:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				SITEMCUSTOM sitemcustom;
				sitemcustom.sNativeID = m_sDATA.sidItemResult.sidItem;
				sitemcustom.wTurnNum = m_sDATA.sidItemResult.wItemNum;

				sitemcustom.cDAMAGE = m_sDATA.cDAMAGE;
				sitemcustom.cDEFENSE = m_sDATA.cDEFENSE;
				sitemcustom.cRESIST_FIRE = m_sDATA.cRESIST_FIRE;
				sitemcustom.cRESIST_ICE = m_sDATA.cRESIST_ICE;
				sitemcustom.cRESIST_ELEC = m_sDATA.cRESIST_ELEC;
				sitemcustom.cRESIST_POISON = m_sDATA.cRESIST_POISON;
				sitemcustom.cRESIST_SPIRIT = m_sDATA.cRESIST_SPIRIT;

				if( sitemcustom.sNativeID == NATIVEID_NULL() ) return;

				SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sitemcustom.sNativeID );
				if ( pitem_data && pitem_data->ISPILE() )	sitemcustom.wTurnNum = pitem_data->GETAPPLYNUM() * m_sDATA.sidItemResult.wItemNum;

				CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW( sitemcustom, FALSE, FALSE, FALSE,FALSE,FALSE, NULL, NULL, NATIVEID_NULL(), FALSE, 0 );

				/*item preview, Juver, 2017/07/27 */
				if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LMENU  ) & DXKEY_DOWNED )
				{
					if ( dwMsg & UIMSG_LB_UP )
					{
						CInnerInterface::GetInstance().PreviewItem( sitemcustom );
						return;
					}
				}

				/*box contents, Juver, 2017/08/30 */
				if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LMENU  ) & DXKEY_DOWNED )
				{
					if ( dwMsg & UIMSG_RB_UP )
					{
						CInnerInterface::GetInstance().ShowBoxContents( sitemcustom.sNativeID );
						return;
					}
				}
			}
		}break;

	case NPC_ITEM_EXCHANGE_SLOT_ITEM_IMAGE_REQ_0:
	case NPC_ITEM_EXCHANGE_SLOT_ITEM_IMAGE_REQ_1:
	case NPC_ITEM_EXCHANGE_SLOT_ITEM_IMAGE_REQ_2:
	case NPC_ITEM_EXCHANGE_SLOT_ITEM_IMAGE_REQ_3:
	case NPC_ITEM_EXCHANGE_SLOT_ITEM_IMAGE_REQ_4:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				int nSelect = ControlID - NPC_ITEM_EXCHANGE_SLOT_ITEM_IMAGE_REQ_0;

				if ( nSelect >= NPC_ITEM_EXCHANGE_MAX_REQ )	return;

				SITEMCUSTOM sitemcustom;
				sitemcustom.sNativeID = m_sDATA.sidItemRequire[nSelect].sidItem;
				sitemcustom.wTurnNum = m_sDATA.sidItemRequire[nSelect].wItemNum;

				if( sitemcustom.sNativeID == NATIVEID_NULL() ) return;

				CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW( sitemcustom, FALSE, FALSE, FALSE,FALSE,FALSE, NULL, NULL, NATIVEID_NULL(), FALSE, 0 );

				/*item preview, Juver, 2017/07/27 */
				if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LMENU  ) & DXKEY_DOWNED )
				{
					if ( dwMsg & UIMSG_LB_UP )
					{
						CInnerInterface::GetInstance().PreviewItem( sitemcustom );
						return;
					}
				}

				/*box contents, Juver, 2017/08/30 */
				if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LMENU  ) & DXKEY_DOWNED )
				{
					if ( dwMsg & UIMSG_RB_UP )
					{
						CInnerInterface::GetInstance().ShowBoxContents( sitemcustom.sNativeID );
						return;
					}
				}
			}
		}break;

	case NPC_ITEM_EXCHANGE_SLOT_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				SITEM* pitem_data_req = GLItemMan::GetInstance().GetItem( m_sDATA.sidItemResult.sidItem );
				if ( pitem_data_req )
				{
					CString strQuestion = CInnerInterface::GetInstance().MakeString( ID2GAMEINTEXT("NPCEXCHANGE_QUESTION"), pitem_data_req->GetName() );
					DoModal( strQuestion.GetString(),  MODAL_INFOMATION, YESNO, MODAL_NPCEXCHANGE );	
				}
				//DoExchange();
			}
		}break;
	};
}

void CNPCItemExchangeSlot::DataSet( SNATIVEID snpc_id, SNPC_ITEM_EXCHANGE_DATA sdata )
{
	DataReset();

	m_sNPCID = snpc_id;
	m_sDATA = sdata;

	for ( int i=0; i<NPC_ITEM_EXCHANGE_MAX_REQ; ++i )
	{
		SNPC_ITEM_EXCHANGE_ITEM& sitem_exchange_data_req = m_sDATA.sidItemRequire[i];
		SITEM* pitem_data_req = GLItemMan::GetInstance().GetItem( sitem_exchange_data_req.sidItem );
		if ( pitem_data_req )
		{
			if ( m_pItemImageRequire[i] )
			{
				m_pItemImageRequire[i]->SetItem( sitem_exchange_data_req.sidItem );

				if ( pitem_data_req->ISPILE() )
				{
					CString strNum;
					strNum.Format( "%d", sitem_exchange_data_req.wItemNum );
					m_pItemImageRequire[i]->SetTextDownRight( strNum.GetString(), NS_UITEXTCOLOR::GOLD );
				}
			}
		}	
	}

	SNPC_ITEM_EXCHANGE_ITEM& sitem_exchange_data_result = m_sDATA.sidItemResult;
	SITEM* pitem_data_result = GLItemMan::GetInstance().GetItem( sitem_exchange_data_result.sidItem );
	if ( pitem_data_result )
	{
		if ( m_pTextName )		
			m_pTextName->SetText( pitem_data_result->GetName(), COMMENT::ITEMCOLOR[pitem_data_result->sBasicOp.emLevel] );

		if ( m_pItemImageResult )
		{
			m_pItemImageResult->SetItem( sitem_exchange_data_result.sidItem );

			if ( pitem_data_result->ISPILE() )
			{
				CString strNum;
				strNum.Format( "%d", pitem_data_result->GETAPPLYNUM() * sitem_exchange_data_result.wItemNum );
				m_pItemImageResult->SetTextDownRight( strNum.GetString(), NS_UITEXTCOLOR::GOLD );
			}
		}
	}	
}

void CNPCItemExchangeSlot::DataReset()
{
	for ( int i=0; i<NPC_ITEM_EXCHANGE_MAX_REQ; ++i )
	{
		if ( m_pItemImageRequire[i] )
		{
			m_pItemImageRequire[i]->ResetItem();
			m_pItemImageRequire[i]->SetTextDownRight( "", NS_UITEXTCOLOR::GOLD );
		}
	}

	if ( m_pItemImageResult )	
	{
		m_pItemImageResult->ResetItem();
		m_pItemImageResult->SetTextDownRight( "", NS_UITEXTCOLOR::GOLD );
	}

	if ( m_pTextName )	
		m_pTextName->ClearText();
}

void CNPCItemExchangeSlot::DoExchange()
{
	GLCharacter* pcharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( !pcharacter )	return;

	pcharacter->ReqNPCItemExchange( m_sNPCID, m_sDATA.wID );
}
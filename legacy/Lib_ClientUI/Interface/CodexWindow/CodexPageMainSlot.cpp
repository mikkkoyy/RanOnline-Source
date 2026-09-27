#include "StdAfx.h"
#include "CodexPageMainSlot.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "UITextControl.h"
#include "GameTextControl.h"
#include "../BasicLineBox.h"
#include "ItemImage.h"
#include "GLItemMan.h"

#include "GLCodex.h"
#include "GLGaeaClient.h"
#include "InnerInterface.h"

#include "../Lib_ClientUI/Interface/ModalCallerID.h"
#include "../Lib_ClientUI/Interface/ModalWindow.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CCodexPageMainSlot::CCodexPageMainSlot ()	
	: m_pStatusComplete(NULL)
	, m_pProgressComplete(NULL)
	, m_pTextComplete(NULL)
	, m_pTextTitle(NULL)
	, m_pTextDesc(NULL)
	, m_pTextProgress(NULL)
	, m_pImageBadge(NULL)
	, m_pImagePoint(NULL)
	, m_pTextBadge(NULL)
	, m_pTextPoint(NULL)
	, m_fUpdateTime(0.0f)
	, m_dwCodexID(UINT_MAX)
	
	, m_pItemImage0(NULL)
	, m_pItemImage1(NULL)
	, m_pItemImage2(NULL)
	, m_pItemImage3(NULL)
	, m_pItemImage4(NULL)

	, m_sidItem0(NATIVEID_NULL())
	, m_sidItem1(NATIVEID_NULL())
	, m_sidItem2(NATIVEID_NULL())
	, m_sidItem3(NATIVEID_NULL())
	, m_sidItem4(NATIVEID_NULL())
	
	, bItem0(false)
	, bItem1(false)
	, bItem2(false)
	, bItem3(false)
	, bItem4(false)
	
	, m_wGradeItem0(0)
	, m_wGradeItem1(0)
	, m_wGradeItem2(0)
	, m_wGradeItem3(0)
	, m_wGradeItem4(0)

	//select highlight
	, m_pSelectImage(NULL)
{
	for( int i=0; i<MAX_CODEX_ITEM; ++i )
	{
		m_pItemImageRequire[i] = NULL;
		m_pTextRegistered[i] = NULL;
		m_pImageAvailable[i] = NULL;
	}
}

CCodexPageMainSlot::~CCodexPageMainSlot ()
{
}

void CCodexPageMainSlot::CreateSubControl ()
{
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );
	CD3DFontPar* pFont2 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 12, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_CODEX_SLOT_ARRIVAL_BACK" );
	RegisterControl ( pLineBox );
	
	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxWhiteBlankBody ( "RAN_SPEC_CODEX_SLOT_ARRIVAL_BACK" );
	RegisterControl ( pLineBox );

	/*pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_CODEX_SLOT_PRESENT_BACK" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_CODEX_SLOT_REWARD_BACK" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_CODEX_SLOT_REWARD_BACK_INNER1" );
	RegisterControl ( pLineBox );

	pLineBox = new CBasicLineBox;
	pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pLineBox->CreateBaseBoxQuestList ( "RAN_SPEC_CODEX_SLOT_REWARD_BACK_INNER2" );
	RegisterControl ( pLineBox );

	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, "RAN_SPEC_CODEX_SLOT_PRESENT_BACK_TOP",  UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "RAN_SPEC_CODEX_SLOT_REWARD_BACK_TOP",  UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	RegisterControl ( pControl );

	pControl = new CUIControl;
	pControl->CreateSub ( this, "RAN_SPEC_CODEX_SLOT_ALL",  UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	RegisterControl ( pControl );*/

	/*m_pStatusComplete = new CUIControl;
	m_pStatusComplete->CreateSub ( this, "RAN_SPEC_CODEX_SLOT_ARRIVAL",  UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pStatusComplete->SetVisibleSingle( FALSE );
	RegisterControl ( m_pStatusComplete );

	m_pProgressComplete = new CUIControl;
	m_pProgressComplete->CreateSub ( this, "RAN_SPEC_CODEX_SLOT_COMP_IMAGE",  UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pProgressComplete->SetVisibleSingle( FALSE );
	RegisterControl ( m_pProgressComplete );*/

	/*m_pImageBadge = new CUIControl;
	m_pImageBadge->CreateSub ( this, "RAN_SPEC_CODEX_SLOT_REWARD_TITLE_IMAGE",  UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pImageBadge->SetVisibleSingle( FALSE );
	RegisterControl ( m_pImageBadge );

	m_pImagePoint = new CUIControl;
	m_pImagePoint->CreateSub ( this, "RAN_SPEC_CODEX_SLOT_REWARD_POINT_IMAGE",  UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pImagePoint->SetVisibleSingle( FALSE );
	RegisterControl ( m_pImagePoint );

	m_pTextComplete = new CBasicTextBox;
	m_pTextComplete->CreateSub ( this, "RAN_SPEC_CODEX_SLOT_COMP_TEXT" );
	m_pTextComplete->SetFont ( pFont );
	m_pTextComplete->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextComplete->SetText( "--" , NS_UITEXTCOLOR::WHITE );
	m_pTextComplete->SetVisibleSingle( FALSE );
	RegisterControl ( m_pTextComplete );*/

	m_pTextTitle = new CBasicTextBox;
	m_pTextTitle->CreateSub ( this, "RAN_SPEC_CODEX_SLOT_TITLE_TEXT" );
	m_pTextTitle->SetFont ( pFont2 );
	m_pTextTitle->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextTitle->SetText( "--" , NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextTitle );

	m_pTextDesc = new CBasicTextBox;
	m_pTextDesc->CreateSub ( this, "RAN_SPEC_CODEX_SLOT_DESC_TEXT" );
	m_pTextDesc->SetFont ( pFont );
	m_pTextDesc->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextDesc->SetText( "--" , NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextDesc );

	m_pTextProgress = new CBasicTextBox;
	m_pTextProgress->CreateSub ( this, "RAN_SPEC_CODEX_SLOT_PROGRESS_TEXT" );
	m_pTextProgress->SetFont ( pFont );
	m_pTextProgress->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextProgress->SetText( "--" , NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextProgress );

	/*m_pTextBadge = new CBasicTextBox;
	m_pTextBadge->CreateSub ( this, "RAN_SPEC_CODEX_SLOT_REWARD_TITLE_TEXT" );
	m_pTextBadge->SetFont ( pFont );
	m_pTextBadge->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextBadge->SetText( "--" , NS_UITEXTCOLOR::WHITE );
	m_pTextBadge->SetVisibleSingle( FALSE );
	RegisterControl ( m_pTextBadge );

	m_pTextPoint = new CBasicTextBox;
	m_pTextPoint->CreateSub ( this, "RAN_SPEC_CODEX_SLOT_REWARD_POINT_TEXT" );
	m_pTextPoint->SetFont ( pFont );
	m_pTextPoint->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextPoint->SetText( "--" , NS_UITEXTCOLOR::WHITE );
	m_pTextPoint->SetVisibleSingle( FALSE );
	RegisterControl ( m_pTextPoint );*/
	
	/*m_pItemImage0 = CreateItemImage0();
	m_pItemImage1 = CreateItemImage1();
	m_pItemImage2 = CreateItemImage2();
	m_pItemImage3 = CreateItemImage3();
	m_pItemImage4 = CreateItemImage4();*/
	
	m_pItemImage0 = new CItemImage;
	m_pItemImage0->CreateSub ( this, "CODEX_SLOT_IMAGE_00", UI_FLAG_DEFAULT, ITEM_SLOT_IMAGE0 );
	m_pItemImage0->CreateSubControl ();
	m_pItemImage0->SetUseRender ( TRUE );
	m_pItemImage0->CreateTextBoxDownRight();
    RegisterControl ( m_pItemImage0 );

	m_pItemImage1 = new CItemImage;
	m_pItemImage1->CreateSub ( this, "CODEX_SLOT_IMAGE_01", UI_FLAG_DEFAULT, ITEM_SLOT_IMAGE1 );
	m_pItemImage1->CreateSubControl ();
	m_pItemImage1->SetUseRender ( TRUE );
	m_pItemImage1->CreateTextBoxDownRight();
    RegisterControl ( m_pItemImage1 );
	
	m_pItemImage2 = new CItemImage;
	m_pItemImage2->CreateSub ( this, "CODEX_SLOT_IMAGE_02", UI_FLAG_DEFAULT, ITEM_SLOT_IMAGE2 );
	m_pItemImage2->CreateSubControl ();
	m_pItemImage2->SetUseRender ( TRUE );
	m_pItemImage2->CreateTextBoxDownRight();
    RegisterControl ( m_pItemImage2 );
	
	m_pItemImage3 = new CItemImage;
	m_pItemImage3->CreateSub ( this, "CODEX_SLOT_IMAGE_03", UI_FLAG_DEFAULT, ITEM_SLOT_IMAGE3 );
	m_pItemImage3->CreateSubControl ();
	m_pItemImage3->SetUseRender ( TRUE );
	m_pItemImage3->CreateTextBoxDownRight();
    RegisterControl ( m_pItemImage3 );
	
	m_pItemImage4 = new CItemImage;
	m_pItemImage4->CreateSub ( this, "CODEX_SLOT_IMAGE_04", UI_FLAG_DEFAULT, ITEM_SLOT_IMAGE4 );
	m_pItemImage4->CreateSubControl ();
	m_pItemImage4->SetUseRender ( TRUE );
	m_pItemImage4->CreateTextBoxDownRight();
    RegisterControl ( m_pItemImage4 );
	
	
	char* stritem_req[MAX_CODEX_ITEM] = 
	{
		"CODEX_SLOT_IMAGE_00",
		"CODEX_SLOT_IMAGE_01",
		"CODEX_SLOT_IMAGE_02",
		"CODEX_SLOT_IMAGE_03",
		"CODEX_SLOT_IMAGE_04",
	};
	
	
	for( int i=0; i<MAX_CODEX_ITEM; ++i )
	{
		m_pItemImageRequire[i] = new CBasicLineBox;
		m_pItemImageRequire[i]->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		m_pItemImageRequire[i]->CreateBaseBoxQuestList ( stritem_req[i] );
		m_pItemImageRequire[i]->SetVisibleSingle(TRUE);
		RegisterControl ( m_pItemImageRequire[i] );
	}
	
	char* stritem_req2[MAX_CODEX_ITEM] = 
	{
		"CODEX_SLOT_IMAGE_000",
		"CODEX_SLOT_IMAGE_010",
		"CODEX_SLOT_IMAGE_020",
		"CODEX_SLOT_IMAGE_030",
		"CODEX_SLOT_IMAGE_040",
	};
	for( int i=0; i<MAX_CODEX_ITEM; ++i )
	{
		m_pTextRegistered[i] = new CBasicTextBox;
		m_pTextRegistered[i]->CreateSub ( this, stritem_req2[i] );//"RAN_SPEC_CODEX_SLOT_REWARD_TITLE_TEXT" );
		m_pTextRegistered[i]->SetFont ( pFont );
		m_pTextRegistered[i]->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
		m_pTextRegistered[i]->SetText( "" , NS_UITEXTCOLOR::GREEN );
		m_pTextRegistered[i]->SetVisibleSingle( FALSE );
		RegisterControl ( m_pTextRegistered[i] );
	}
	
	char* stritem_req3[MAX_CODEX_ITEM] = 
	{
		"CODEX_AVAILABLE_IMAGE_000",
		"CODEX_AVAILABLE_IMAGE_010",
		"CODEX_AVAILABLE_IMAGE_020",
		"CODEX_AVAILABLE_IMAGE_030",
		"CODEX_AVAILABLE_IMAGE_040",
	};
	for( int i=0; i<MAX_CODEX_ITEM; ++i )
	{
		m_pImageAvailable[i] = new CBasicTextBox;
		m_pImageAvailable[i]->CreateSub ( this, stritem_req3[i] );//"RAN_SPEC_CODEX_SLOT_REWARD_TITLE_TEXT" );
		m_pImageAvailable[i]->SetFont ( pFont );
		m_pImageAvailable[i]->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
		m_pImageAvailable[i]->SetText( "" , NS_UITEXTCOLOR::GREEN );
		m_pImageAvailable[i]->SetVisibleSingle( FALSE );
		RegisterControl ( m_pImageAvailable[i] );
	}



	//Select Highlight

	m_pSelectImage = new CUIControl;
	m_pSelectImage->CreateSub ( this, "CODEX_LIST_NODE_SELECT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pSelectImage->SetVisibleSingle( FALSE );
	RegisterControl ( m_pSelectImage );
}

void CCodexPageMainSlot::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );
	
	switch ( ControlID )
	{
	case ITEM_SLOT_IMAGE0:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				SITEMCUSTOM sitemcustom;
				sitemcustom.sNativeID = m_sidItem0;
				//sItemCustom.cDAMAGE = pItem->sBasicOp.wGradeAttack;
				//sItemCustom.cDEFENSE = pItem->sBasicOp.wGradeDefense;

				if( sitemcustom.sNativeID == NATIVEID_NULL() ) return;

				SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sitemcustom.sNativeID );
				if ( pitem_data && pitem_data->ISPILE() )	sitemcustom.wTurnNum = pitem_data->GETAPPLYNUM();
				
				sitemcustom.cDAMAGE = 0;
				sitemcustom.cDEFENSE = 0;

				switch ( pitem_data->sSuitOp.emSuit )
				{
					case SUIT_HEADGEAR:
					case SUIT_UPPER:
					case SUIT_LOWER:
					case SUIT_HAND:
					case SUIT_FOOT:
					{
						sitemcustom.cDEFENSE = m_wGradeItem0;
					}
					break;

					case SUIT_HANDHELD:
					{
						sitemcustom.cDAMAGE = m_wGradeItem0;
					}
					break;
				};

				CInnerInterface::GetInstance().SHOW_ITEM_INFO( sitemcustom, FALSE, FALSE, FALSE, NULL, NULL, NATIVEID_NULL(), FALSE, 0 );

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
				if ( dwMsg & UIMSG_LB_UP )
				{				
					//if ( bItem0 == true )
					//	CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_REGISTER_CODEX_FB_DONE") );
					//else				
						//GLGaeaClient::GetInstance().GetCharacter()->ReqCodexRegister(m_sidItem0, m_dwCodexID, m_wGradeItem0);
					
					CString strCombine = CInnerInterface::GetInstance().MakeString ( "Are you sure you want to register this item?" );
					SetCodexModalData ( m_sidItem0, m_dwCodexID, m_wGradeItem0, m_wQuantity0 );
					DoModal ( strCombine, MODAL_QUESTION, YESNO, MODAL_REGISTER_CODEX );
				}
			}
		}
		break;
	case ITEM_SLOT_IMAGE1:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				SITEMCUSTOM sitemcustom;
				sitemcustom.sNativeID = m_sidItem1;
				//sItemCustom.cDAMAGE = pItem->sBasicOp.wGradeAttack;
				//sItemCustom.cDEFENSE = pItem->sBasicOp.wGradeDefense;

				if( sitemcustom.sNativeID == NATIVEID_NULL() ) return;

				SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sitemcustom.sNativeID );
				if ( pitem_data && pitem_data->ISPILE() )	sitemcustom.wTurnNum = pitem_data->GETAPPLYNUM();
				
				sitemcustom.cDAMAGE = 0;
				sitemcustom.cDEFENSE = 0;

				switch ( pitem_data->sSuitOp.emSuit )
				{
					case SUIT_HEADGEAR:
					case SUIT_UPPER:
					case SUIT_LOWER:
					case SUIT_HAND:
					case SUIT_FOOT:
					{
						sitemcustom.cDEFENSE = m_wGradeItem1;
					}
					break;

					case SUIT_HANDHELD:
					{
						sitemcustom.cDAMAGE = m_wGradeItem1;
					}
					break;
				};

				CInnerInterface::GetInstance().SHOW_ITEM_INFO( sitemcustom, FALSE, FALSE, FALSE, NULL, NULL, NATIVEID_NULL(), FALSE, 0 );

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
				if ( dwMsg & UIMSG_LB_UP )
				{				
					//if ( bItem1 == true )
					//	CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_REGISTER_CODEX_FB_DONE") );
					//else				
					//	GLGaeaClient::GetInstance().GetCharacter()->ReqCodexRegister(m_sidItem1, m_dwCodexID, m_wGradeItem1);
				
					CString strCombine = CInnerInterface::GetInstance().MakeString ( "Are you sure you want to register this item?" );
					SetCodexModalData ( m_sidItem1, m_dwCodexID, m_wGradeItem1, m_wQuantity1);
					DoModal ( strCombine, MODAL_QUESTION, YESNO, MODAL_REGISTER_CODEX );
				}
			}
		}
		break;
	case ITEM_SLOT_IMAGE2:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				SITEMCUSTOM sitemcustom;
				sitemcustom.sNativeID = m_sidItem2;
				//sItemCustom.cDAMAGE = pItem->sBasicOp.wGradeAttack;
				//sItemCustom.cDEFENSE = pItem->sBasicOp.wGradeDefense;

				if( sitemcustom.sNativeID == NATIVEID_NULL() ) return;

				SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sitemcustom.sNativeID );
				if ( pitem_data && pitem_data->ISPILE() )	sitemcustom.wTurnNum = pitem_data->GETAPPLYNUM();
				
				sitemcustom.cDAMAGE = 0;
				sitemcustom.cDEFENSE = 0;

				switch ( pitem_data->sSuitOp.emSuit )
				{
					case SUIT_HEADGEAR:
					case SUIT_UPPER:
					case SUIT_LOWER:
					case SUIT_HAND:
					case SUIT_FOOT:
					{
						sitemcustom.cDEFENSE = m_wGradeItem2;
					}
					break;

					case SUIT_HANDHELD:
					{
						sitemcustom.cDAMAGE = m_wGradeItem2;
					}
					break;
				};

				CInnerInterface::GetInstance().SHOW_ITEM_INFO( sitemcustom, FALSE, FALSE, FALSE, NULL, NULL, NATIVEID_NULL(), FALSE, 0 );

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
				if ( dwMsg & UIMSG_LB_UP )
				{				
					//if ( bItem2 == true )
					//	CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_REGISTER_CODEX_FB_DONE") );
					//else				
					//	GLGaeaClient::GetInstance().GetCharacter()->ReqCodexRegister(m_sidItem2, m_dwCodexID, m_wGradeItem2);
				
					CString strCombine = CInnerInterface::GetInstance().MakeString ( "Are you sure you want to register this item?" );
					SetCodexModalData ( m_sidItem2, m_dwCodexID, m_wGradeItem2, m_wQuantity2);
					DoModal ( strCombine, MODAL_QUESTION, YESNO, MODAL_REGISTER_CODEX );
				}
			}
		}
		break;
	case ITEM_SLOT_IMAGE3:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				SITEMCUSTOM sitemcustom;
				sitemcustom.sNativeID = m_sidItem3;
				//sItemCustom.cDAMAGE = pItem->sBasicOp.wGradeAttack;
				//sItemCustom.cDEFENSE = pItem->sBasicOp.wGradeDefense;

				if( sitemcustom.sNativeID == NATIVEID_NULL() ) return;

				SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sitemcustom.sNativeID );
				if ( pitem_data && pitem_data->ISPILE() )	sitemcustom.wTurnNum = pitem_data->GETAPPLYNUM();
				
				sitemcustom.cDAMAGE = 0;
				sitemcustom.cDEFENSE = 0;

				switch ( pitem_data->sSuitOp.emSuit )
				{
					case SUIT_HEADGEAR:
					case SUIT_UPPER:
					case SUIT_LOWER:
					case SUIT_HAND:
					case SUIT_FOOT:
					{
						sitemcustom.cDEFENSE = m_wGradeItem3;
					}
					break;

					case SUIT_HANDHELD:
					{
						sitemcustom.cDAMAGE = m_wGradeItem3;
					}
					break;
				};

				CInnerInterface::GetInstance().SHOW_ITEM_INFO( sitemcustom, FALSE, FALSE, FALSE, NULL, NULL, NATIVEID_NULL(), FALSE, 0 );

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
				if ( dwMsg & UIMSG_LB_UP )
				{				
					//if ( bItem3 == true )
					//	CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_REGISTER_CODEX_FB_DONE") );
					//else				
					//	GLGaeaClient::GetInstance().GetCharacter()->ReqCodexRegister(m_sidItem3, m_dwCodexID, m_wGradeItem3);
					
					CString strCombine = CInnerInterface::GetInstance().MakeString ( "Are you sure you want to register this item?" );
					SetCodexModalData ( m_sidItem3, m_dwCodexID, m_wGradeItem3, m_wQuantity3);
					DoModal ( strCombine, MODAL_QUESTION, YESNO, MODAL_REGISTER_CODEX );
				}
			}
		}
		break;
	case ITEM_SLOT_IMAGE4:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				SITEMCUSTOM sitemcustom;
				sitemcustom.sNativeID = m_sidItem4;
				//sItemCustom.cDAMAGE = pItem->sBasicOp.wGradeAttack;
				//sItemCustom.cDEFENSE = pItem->sBasicOp.wGradeDefense;

				if( sitemcustom.sNativeID == NATIVEID_NULL() ) return;

				SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sitemcustom.sNativeID );
				if ( pitem_data && pitem_data->ISPILE() )	sitemcustom.wTurnNum = pitem_data->GETAPPLYNUM();
				
				sitemcustom.cDAMAGE = 0;
				sitemcustom.cDEFENSE = 0;

				switch ( pitem_data->sSuitOp.emSuit )
				{
					case SUIT_HEADGEAR:
					case SUIT_UPPER:
					case SUIT_LOWER:
					case SUIT_HAND:
					case SUIT_FOOT:
					{
						sitemcustom.cDEFENSE = m_wGradeItem4;
					}
					break;

					case SUIT_HANDHELD:
					{
						sitemcustom.cDAMAGE = m_wGradeItem4;
					}
					break;
				};
				
				CInnerInterface::GetInstance().SHOW_ITEM_INFO( sitemcustom, FALSE, FALSE, FALSE, NULL, NULL, NATIVEID_NULL(), FALSE, 0 );

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
				if ( dwMsg & UIMSG_LB_UP )
				{	
					//if ( bItem4 == true )
					//	CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_REGISTER_CODEX_FB_DONE") );
					//else				
					//	GLGaeaClient::GetInstance().GetCharacter()->ReqCodexRegister(m_sidItem4, m_dwCodexID, m_wGradeItem4);
				
					CString strCombine = CInnerInterface::GetInstance().MakeString ( "Are you sure you want to register this item?" );
					SetCodexModalData ( m_sidItem4, m_dwCodexID, m_wGradeItem4, m_wQuantity4);
					DoModal ( strCombine, MODAL_QUESTION, YESNO, MODAL_REGISTER_CODEX );
				}
			}
		}
		break;
	}
}

void CCodexPageMainSlot::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	m_fUpdateTime += fElapsedTime;
	if ( m_fUpdateTime >= 0.2f )
	{
		UpdateInfo();
	}
}

void CCodexPageMainSlot::ResetData()
{
	m_fUpdateTime = 0.0f;
	m_dwCodexID = UINT_MAX;

	//if ( m_pStatusComplete )	m_pStatusComplete->SetVisibleSingle( FALSE );
	//if ( m_pProgressComplete )	m_pProgressComplete->SetVisibleSingle( FALSE );
	//if ( m_pImageBadge )		m_pImageBadge->SetVisibleSingle( FALSE );
	//if ( m_pImagePoint )		m_pImagePoint->SetVisibleSingle( FALSE );

	//if ( m_pTextComplete )		m_pTextComplete->ClearText();
	if ( m_pTextTitle )			m_pTextTitle->ClearText();
	if ( m_pTextDesc )			m_pTextDesc->ClearText();
	if ( m_pTextProgress )		m_pTextProgress->ClearText();
	//if ( m_pTextBadge )			m_pTextBadge->ClearText();
	//if ( m_pTextPoint )			m_pTextPoint->ClearText();
}

void CCodexPageMainSlot::SetData( DWORD dwID )
{
	ResetData();

	m_dwCodexID = dwID;

	UpdateInfo();
}

void CCodexPageMainSlot::UpdateInfo()
{
	/*if ( m_pStatusComplete )	m_pStatusComplete->SetVisibleSingle( FALSE );
	if ( m_pProgressComplete )	m_pProgressComplete->SetVisibleSingle( FALSE );
	if ( m_pImageBadge )		m_pImageBadge->SetVisibleSingle( FALSE );
	if ( m_pImagePoint )		m_pImagePoint->SetVisibleSingle( FALSE );

	if ( m_pTextComplete )		m_pTextComplete->ClearText();*/
	if ( m_pTextTitle )			m_pTextTitle->ClearText();
	if ( m_pTextDesc )			m_pTextDesc->ClearText();
	if ( m_pTextProgress )		m_pTextProgress->ClearText();
	//if ( m_pTextBadge )			m_pTextBadge->ClearText();
	//if ( m_pTextPoint )			m_pTextPoint->ClearText();

	CString strTemp;
	
	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( !pCharacter )	return;
	GLInventory& ref_Inventory = pCharacter->m_cInventory;

	SCODEX_FILE_DATA* pcodex_data = GLCodex::GetInstance().GetCodex( m_dwCodexID );
	if ( pcodex_data )
	{
		/*First Image*/
		SNATIVEID sNativeID0;
		sNativeID0.wMainID = pcodex_data->sidMobKill.wMainID;
		sNativeID0.wSubID = pcodex_data->sidMobKill.wSubID;
		SITEM* pItemData0 = GLItemMan::GetInstance().GetItem ( sNativeID0 );
		if( pItemData0 )
		{
			m_sidItem0 = sNativeID0;
			m_pItemImage0->SetSkill ( pItemData0->sBasicOp.sICONID, pItemData0->GetInventoryFile() );
			m_pItemImage0->SetVisibleSingle( TRUE );
			DWORD dwGrade = pcodex_data->wItemGrade1;
			DWORD dwQuantity = pcodex_data->wProgressMobKill;
			m_wGradeItem0 = dwGrade;
			m_wQuantity0 = dwQuantity;
			if ( dwGrade > 0 )
			{
				CString strNum;
				strNum.Format( "+%d", dwGrade );
				m_pItemImage0->SetTextDownRight( strNum.GetString(), NS_UITEXTCOLOR::GOLD );
			}
			/*else
			{
				m_pItemImage0->SetTextDownRight( "", NS_UITEXTCOLOR::GOLD );
			}*/

			if (dwQuantity > 1) 
			{
				CString strNum;
				strNum.Format("%d", dwQuantity);
				m_pItemImage0->SetTextDownRight(strNum.GetString(), NS_UITEXTCOLOR::GOLD);
			}
			/*else
			{
				CString strNum;
				strNum.Format("%d", dwQuantity);
				m_pItemImage0->SetTextDownRight("", NS_UITEXTCOLOR::GOLD);
			}*/
			
			SINVENITEM* pinven_item = ref_Inventory.FindItem( sNativeID0, dwGrade, dwQuantity );
			if ( pinven_item )
			{				
				if (pinven_item->sItemCustom.wTurnNum == dwQuantity)
				//if ( pinven_item->sItemCustom.GETGRADE ( EMGRINDING_NONE ) == pcodex_data->wItemGrade1 )
				{
					m_pItemImageRequire[0]->SetVisibleSingle(FALSE);
					m_pImageAvailable[0]->SetVisibleSingle(TRUE);
				}
				else 
				{
					m_pItemImageRequire[0]->SetVisibleSingle(TRUE);
					m_pImageAvailable[0]->SetVisibleSingle(FALSE);
				}
			}
			else
			{
				m_pItemImageRequire[0]->SetVisibleSingle(TRUE);
				m_pImageAvailable[0]->SetVisibleSingle(FALSE);
			}
		}
		else
		{
			m_pItemImage0->SetVisibleSingle( FALSE );
			m_pItemImageRequire[0]->SetVisibleSingle(FALSE);
			m_pImageAvailable[0]->SetVisibleSingle(FALSE);
		}
		/*Second Image*/
		SNATIVEID sNativeID1;
		sNativeID1.wMainID = pcodex_data->sidMapKill.wMainID;
		sNativeID1.wSubID = pcodex_data->sidMapKill.wSubID;
		SITEM* pItemData1 = GLItemMan::GetInstance().GetItem ( sNativeID1 );
		if( pItemData1 )
		{
			m_sidItem1 = sNativeID1;
			m_pItemImage1->SetSkill ( pItemData1->sBasicOp.sICONID, pItemData1->GetInventoryFile() );
			m_pItemImage1->SetVisibleSingle( TRUE );
			
			DWORD dwGrade = pcodex_data->wItemGrade2;
			DWORD dwQuantity = pcodex_data->wProgressMapKill;
			m_wQuantity1 = dwQuantity;
			m_wGradeItem1 = dwGrade;
			if ( dwGrade > 0 )
			{
				CString strNum;
				strNum.Format( "+%d", dwGrade );
				m_pItemImage1->SetTextDownRight( strNum.GetString(), NS_UITEXTCOLOR::GOLD );
			}
			/*else
			{
				m_pItemImage1->SetTextDownRight( "", NS_UITEXTCOLOR::GOLD );
			}*/

			if (dwQuantity > 1)
			{
				CString strNum;
				strNum.Format("%d", dwQuantity);
				m_pItemImage1->SetTextDownRight(strNum.GetString(), NS_UITEXTCOLOR::GOLD);
			}
			/*else
			{
				CString strNum;
				strNum.Format("%d", dwQuantity);
				m_pItemImage1->SetTextDownRight("", NS_UITEXTCOLOR::GOLD);
			}*/

			SINVENITEM* pinven_item = ref_Inventory.FindItem( sNativeID1, dwGrade, dwQuantity);
			if ( pinven_item )
			{
				if (pinven_item->sItemCustom.wTurnNum == dwQuantity)
					//if ( pinven_item->sItemCustom.GETGRADE ( EMGRINDING_NONE ) == pcodex_data->wItemGrade2 )
				{
					m_pItemImageRequire[1]->SetVisibleSingle(FALSE);
					m_pImageAvailable[1]->SetVisibleSingle(TRUE);
				}
				else
				{
					m_pItemImageRequire[1]->SetVisibleSingle(TRUE);
					m_pImageAvailable[1]->SetVisibleSingle(FALSE);
				}
			}
			else
			{
				m_pItemImageRequire[1]->SetVisibleSingle(TRUE);
				m_pImageAvailable[1]->SetVisibleSingle(FALSE);
			}
		}
		else
		{
			m_pItemImage1->SetVisibleSingle( FALSE );
			m_pItemImageRequire[1]->SetVisibleSingle(FALSE);
			m_pImageAvailable[1]->SetVisibleSingle(FALSE);
		}
		/*Third Image*/
		SNATIVEID sNativeID2;
		sNativeID2.wMainID = pcodex_data->sidMapReach.wMainID;
		sNativeID2.wSubID = pcodex_data->sidMapReach.wSubID;
		SITEM* pItemData2 = GLItemMan::GetInstance().GetItem ( sNativeID2 );
		if( pItemData2 )
		{
			m_sidItem2 = sNativeID2;
			m_pItemImage2->SetSkill ( pItemData2->sBasicOp.sICONID, pItemData2->GetInventoryFile() );
			m_pItemImage2->SetVisibleSingle( TRUE );
			
			DWORD dwGrade = pcodex_data->wItemGrade3;
			DWORD dwQuantity = pcodex_data->wProgressMapReach;
			m_wGradeItem2 = dwGrade;
			m_wQuantity2 = dwQuantity;
			if ( dwGrade > 0 )
			{
				CString strNum;
				strNum.Format( "+%d", dwGrade );
				m_pItemImage2->SetTextDownRight( strNum.GetString(), NS_UITEXTCOLOR::GOLD );
			}
			/*else
			{
				m_pItemImage2->SetTextDownRight( "", NS_UITEXTCOLOR::GOLD );
			}*/

			if (dwQuantity > 1)
			{
				CString strNum;
				strNum.Format("%d", dwQuantity);
				m_pItemImage2->SetTextDownRight(strNum.GetString(), NS_UITEXTCOLOR::GOLD);
			}
			/*else
			{
				CString strNum;
				strNum.Format("%d", dwQuantity);
				m_pItemImage2->SetTextDownRight("", NS_UITEXTCOLOR::GOLD);
			}*/

			SINVENITEM* pinven_item = ref_Inventory.FindItem( sNativeID2, dwGrade, dwQuantity);
			if ( pinven_item )
			{
				if (pinven_item->sItemCustom.wTurnNum == dwQuantity)
					//if ( pinven_item->sItemCustom.GETGRADE ( EMGRINDING_NONE ) == pcodex_data->wItemGrade3 )
				{
					m_pItemImageRequire[2]->SetVisibleSingle(FALSE);
					m_pImageAvailable[2]->SetVisibleSingle(TRUE);
				}
				else
				{
					m_pItemImageRequire[2]->SetVisibleSingle(TRUE);
					m_pImageAvailable[2]->SetVisibleSingle(FALSE);
				}
			}
			else
			{
				m_pItemImageRequire[2]->SetVisibleSingle(TRUE);
				m_pImageAvailable[2]->SetVisibleSingle(FALSE);
			}
				
		}
		else
		{
			m_pItemImage2->SetVisibleSingle( FALSE );
			m_pItemImageRequire[2]->SetVisibleSingle(FALSE);
			m_pImageAvailable[2]->SetVisibleSingle(FALSE);
		}
		/*Fourth Image*/
		SNATIVEID sNativeID3;
		sNativeID3.wMainID = pcodex_data->sidItemGet.wMainID;
		sNativeID3.wSubID = pcodex_data->sidItemGet.wSubID;
		SITEM* pItemData3 = GLItemMan::GetInstance().GetItem ( sNativeID3 );
		if( pItemData3 )
		{
			m_sidItem3 = sNativeID3;
			m_pItemImage3->SetSkill ( pItemData3->sBasicOp.sICONID, pItemData3->GetInventoryFile() );
			m_pItemImage3->SetVisibleSingle( TRUE );
			
			DWORD dwGrade = pcodex_data->wItemGrade4;
			DWORD dwQuantity = pcodex_data->wProgressItemGet;
			m_wGradeItem3 = dwGrade;
			m_wQuantity3 = dwQuantity;
			if ( dwGrade > 0 )
			{
				CString strNum;
				strNum.Format( "+%d", dwGrade );
				m_pItemImage3->SetTextDownRight( strNum.GetString(), NS_UITEXTCOLOR::GOLD );
			}
			/*else
			{
				m_pItemImage3->SetTextDownRight( "", NS_UITEXTCOLOR::GOLD );
			}*/

			if (dwQuantity > 1)
			{
				CString strNum;
				strNum.Format("%d", dwQuantity);
				m_pItemImage3->SetTextDownRight(strNum.GetString(), NS_UITEXTCOLOR::GOLD);
			}
			/*else
			{
				CString strNum;
				strNum.Format("%d", dwQuantity);
				m_pItemImage3->SetTextDownRight("", NS_UITEXTCOLOR::GOLD);
			}*/

			SINVENITEM* pinven_item = ref_Inventory.FindItem( sNativeID3, dwGrade, dwQuantity);
			if ( pinven_item )
			{
				if (pinven_item->sItemCustom.wTurnNum == dwQuantity)
					//if ( pinven_item->sItemCustom.GETGRADE ( EMGRINDING_NONE ) == pcodex_data->wItemGrade4 )
				{
					m_pItemImageRequire[3]->SetVisibleSingle(FALSE);
					m_pImageAvailable[3]->SetVisibleSingle(TRUE);
				}
				else
				{
					m_pItemImageRequire[3]->SetVisibleSingle(TRUE);
					m_pImageAvailable[3]->SetVisibleSingle(FALSE);
				}
			}
			else
			{
				m_pItemImageRequire[3]->SetVisibleSingle(TRUE);
				m_pImageAvailable[3]->SetVisibleSingle(FALSE);
			}
		}
		else
		{
			m_pItemImage3->SetVisibleSingle( FALSE );
			m_pItemImageRequire[3]->SetVisibleSingle(FALSE);
			m_pImageAvailable[3]->SetVisibleSingle(FALSE);
		}
		/*Fifth Image*/
		SNATIVEID sNativeID4;
		sNativeID4.wMainID = pcodex_data->sidItemUse.wMainID;
		sNativeID4.wSubID = pcodex_data->sidItemUse.wSubID;
		SITEM* pItemData4 = GLItemMan::GetInstance().GetItem ( sNativeID4 );
		if( pItemData4 )
		{
			m_sidItem4 = sNativeID4;
			m_pItemImage4->SetSkill ( pItemData4->sBasicOp.sICONID, pItemData4->GetInventoryFile() );
			m_pItemImage4->SetVisibleSingle( TRUE );
			
			DWORD dwGrade = pcodex_data->wItemGrade5;
			DWORD dwQuantity = pcodex_data->wProgressItemUse;
			m_wGradeItem4 = dwGrade;
			m_wQuantity4 = dwQuantity;
			if ( dwGrade > 0 )
			{
				CString strNum;
				strNum.Format( "+%d", dwGrade );
				m_pItemImage4->SetTextDownRight( strNum.GetString(), NS_UITEXTCOLOR::GOLD );
			}
			/*else
			{
				m_pItemImage4->SetTextDownRight( "", NS_UITEXTCOLOR::GOLD );
			}*/

			if (dwQuantity > 1)
			{
				CString strNum;
				strNum.Format("%d", dwQuantity);
				m_pItemImage4->SetTextDownRight(strNum.GetString(), NS_UITEXTCOLOR::GOLD);
			}
			/*else
			{
				CString strNum;
				strNum.Format("%d", dwQuantity);
				m_pItemImage4->SetTextDownRight("", NS_UITEXTCOLOR::GOLD);
			}*/

			SINVENITEM* pinven_item = ref_Inventory.FindItem( sNativeID4, dwGrade, dwQuantity);
			if ( pinven_item )
			{
				if (pinven_item->sItemCustom.wTurnNum == dwQuantity)
					//if ( pinven_item->sItemCustom.GETGRADE ( EMGRINDING_NONE ) == pcodex_data->wItemGrade5 )
				{
					m_pItemImageRequire[4]->SetVisibleSingle(FALSE);
					m_pImageAvailable[4]->SetVisibleSingle(TRUE);
				}
				else
				{
					m_pItemImageRequire[4]->SetVisibleSingle(TRUE);
					m_pImageAvailable[4]->SetVisibleSingle(FALSE);
				}
			}
			else
			{
				m_pItemImageRequire[4]->SetVisibleSingle(TRUE);
				m_pImageAvailable[4]->SetVisibleSingle(FALSE);
			}
		}
		else
		{
			m_pItemImage4->SetVisibleSingle( FALSE );
			m_pItemImageRequire[4]->SetVisibleSingle(FALSE);
			m_pImageAvailable[4]->SetVisibleSingle(FALSE);
		}
		
		
		if ( m_pTextTitle )
		{
			strTemp.Format( "%s", pcodex_data->strCodexTitle.c_str() );
			m_pTextTitle->AddText( strTemp.GetString(), NS_UITEXTCOLOR::ORANGE );
		}

		if ( m_pTextDesc )			
		{
			strTemp.Format( "%s +%u", COMMENT::CODEX_MID[pcodex_data->emType].c_str(), pcodex_data->dwRewardPoint );
			//m_pTextDesc->AddText( pcodex_data->strDescription.c_str(), NS_UITEXTCOLOR::WHITE );
			m_pTextDesc->AddText( strTemp.GetString(), NS_UITEXTCOLOR::WHITE );
			
			//pcodex_data->dwData.emType;
		}

		if ( pcodex_data->bRewardBadge && pcodex_data->strBadgeString.size() )
		{
			if ( m_pImageBadge )		m_pImageBadge->SetVisibleSingle( FALSE );
			if ( m_pTextBadge )			m_pTextBadge->AddText( pcodex_data->strBadgeString.c_str(), NS_UITEXTCOLOR::WHITE );
		}

		/*if ( pcodex_data->dwRewardPoint )
		{
			strTemp.Format( "%u", pcodex_data->dwRewardPoint );
			if ( m_pImagePoint )		m_pImagePoint->SetVisibleSingle( FALSE );
			if ( m_pTextPoint )			m_pTextPoint->AddText( strTemp.GetString(), NS_UITEXTCOLOR::GOLD );
		}*/


		SCODEX_CHAR_DATA* pchar_codex_data_done = GLGaeaClient::GetInstance().GetCharacter()->GetCodexDone( m_dwCodexID );
		if ( pchar_codex_data_done )
		{
			//if ( m_pStatusComplete )	m_pStatusComplete->SetVisibleSingle( TRUE );
			//if ( m_pProgressComplete )	m_pProgressComplete->SetVisibleSingle( TRUE );

			if ( m_pTextProgress )		m_pTextProgress->AddText( ID2GAMEWORD( "RAN_SPEC_CODEX_PROGRESS_TEXT", 1 ), NS_UITEXTCOLOR::GREENYELLOW );
			
			if ( pchar_codex_data_done->dwProgressItemDone1 == 1 )
			{
				m_pTextRegistered[0]->SetVisibleSingle( TRUE );
				m_pItemImage0->SetVisibleSingle( TRUE );
				m_pItemImageRequire[0]->SetVisibleSingle(TRUE);
				m_pImageAvailable[0]->SetVisibleSingle(FALSE);
				bItem0 = true;
			}
			else
			{
				m_pTextRegistered[0]->SetVisibleSingle( FALSE );
				bItem0 = false;
			}
			if ( pchar_codex_data_done->dwProgressItemDone2 == 1 )
			{
				m_pTextRegistered[1]->SetVisibleSingle( TRUE );
				m_pItemImage1->SetVisibleSingle( TRUE );
				m_pItemImageRequire[1]->SetVisibleSingle(TRUE);
				m_pImageAvailable[1]->SetVisibleSingle(FALSE);
				bItem1 = true;
			}
			else
			{
				m_pTextRegistered[1]->SetVisibleSingle( FALSE );
				bItem1 = false;
			}
			if ( pchar_codex_data_done->dwProgressItemDone3 == 1 )
			{
				m_pTextRegistered[2]->SetVisibleSingle( TRUE );
				m_pItemImage2->SetVisibleSingle( TRUE );
				m_pItemImageRequire[2]->SetVisibleSingle(TRUE);
				m_pImageAvailable[2]->SetVisibleSingle(FALSE);
				bItem2 = true;
			}
			else
			{
				m_pTextRegistered[2]->SetVisibleSingle( FALSE );
				bItem2 = false;
			}
			if ( pchar_codex_data_done->dwProgressItemDone4 == 1 )
			{
				m_pTextRegistered[3]->SetVisibleSingle( TRUE );
				m_pItemImage3->SetVisibleSingle( TRUE );
				m_pItemImageRequire[3]->SetVisibleSingle(TRUE);
				m_pImageAvailable[3]->SetVisibleSingle(FALSE);
				bItem3 = true;
			}
			else
			{
				m_pTextRegistered[3]->SetVisibleSingle( FALSE );
				bItem3 = false;
			}
			if ( pchar_codex_data_done->dwProgressItemDone5 == 1 )
			{
				m_pTextRegistered[4]->SetVisibleSingle( TRUE );
				m_pItemImage4->SetVisibleSingle( TRUE );
				m_pItemImageRequire[4]->SetVisibleSingle(TRUE);
				m_pImageAvailable[4]->SetVisibleSingle(FALSE);
				bItem4 = true;
			}
			else
			{
				m_pTextRegistered[4]->SetVisibleSingle( FALSE );
				bItem4 = false;
			}

			return;
		}

		SCODEX_CHAR_DATA* pchar_codex_data_prog = GLGaeaClient::GetInstance().GetCharacter()->GetCodexProg( m_dwCodexID );
		if ( pchar_codex_data_prog )
		{
			strTemp.Format( "%s %u/%u", ID2GAMEWORD( "RAN_SPEC_CODEX_PROGRESS_TEXT", 0 ), pchar_codex_data_prog->dwProgressNow, pchar_codex_data_prog->dwProgressMax );
			if ( m_pTextProgress )		m_pTextProgress->AddText( "", NS_UITEXTCOLOR::WHITE );
			//if ( m_pTextProgress )		m_pTextProgress->AddText( strTemp.GetString(), NS_UITEXTCOLOR::WHITE );
			//if ( m_pTextComplete )		m_pTextComplete->AddText( ID2GAMEWORD( "RAN_SPEC_CODEX_PROGRESS_TEXT", 2 ), NS_UITEXTCOLOR::AQUA );
			
			if ( pchar_codex_data_prog->dwProgressItemDone1 == 1 )
			{
				m_pTextRegistered[0]->SetVisibleSingle( TRUE );
				m_pItemImage0->SetVisibleSingle( TRUE );
				m_pItemImageRequire[0]->SetVisibleSingle(TRUE);
				m_pImageAvailable[0]->SetVisibleSingle(FALSE);
				bItem0 = true;
			}
			else
			{
				m_pTextRegistered[0]->SetVisibleSingle( FALSE );
				bItem0 = false;
			}
			if ( pchar_codex_data_prog->dwProgressItemDone2 == 1 )
			{
				m_pTextRegistered[1]->SetVisibleSingle( TRUE );
				m_pItemImage1->SetVisibleSingle( TRUE );
				m_pItemImageRequire[1]->SetVisibleSingle(TRUE);
				m_pImageAvailable[1]->SetVisibleSingle(FALSE);
				bItem1 = true;
			}
			else
			{
				m_pTextRegistered[1]->SetVisibleSingle( FALSE );
				bItem1 = false;
			}
			if ( pchar_codex_data_prog->dwProgressItemDone3 == 1 )
			{
				m_pTextRegistered[2]->SetVisibleSingle( TRUE );
				m_pItemImage2->SetVisibleSingle( TRUE );
				m_pItemImageRequire[2]->SetVisibleSingle(TRUE);
				m_pImageAvailable[2]->SetVisibleSingle(FALSE);
				bItem2 = true;
			}
			else
			{
				m_pTextRegistered[2]->SetVisibleSingle( FALSE );
				bItem2 = false;
			}
			if ( pchar_codex_data_prog->dwProgressItemDone4 == 1 )
			{
				m_pTextRegistered[3]->SetVisibleSingle( TRUE );
				m_pItemImage3->SetVisibleSingle( TRUE );
				m_pItemImageRequire[3]->SetVisibleSingle(TRUE);
				m_pImageAvailable[3]->SetVisibleSingle(FALSE);
				bItem3 = true;
			}
			else
			{
				m_pTextRegistered[3]->SetVisibleSingle( FALSE );
				bItem3 = false;
			}
			if ( pchar_codex_data_prog->dwProgressItemDone5 == 1 )
			{
				m_pTextRegistered[4]->SetVisibleSingle( TRUE );
				m_pItemImage4->SetVisibleSingle( TRUE );
				m_pItemImageRequire[4]->SetVisibleSingle(TRUE);
				m_pImageAvailable[4]->SetVisibleSingle(FALSE);
				bItem4 = true;
			}
			else
			{
				m_pTextRegistered[4]->SetVisibleSingle( FALSE );
				bItem4 = false;
			}
		}
	}
}
void CCodexPageMainSlot::SetOverVisible()
{
	m_pSelectImage->SetVisibleSingle( TRUE );
}
void CCodexPageMainSlot::SetOverVisibleOFF()
{
	m_pSelectImage->SetVisibleSingle( FALSE );
}
/*CItemImage* CCodexPageMainSlot::CreateItemImage0 ()
{
	CItemImage* pItemImage = new CItemImage;
	pItemImage->CreateSub ( this, "CODEX_SLOT_IMAGE", UI_FLAG_DEFAULT, ITEM_SLOT_IMAGE );
	pItemImage->CreateSubControl ();
	pItemImage->SetUseRender ( TRUE );
    RegisterControl ( pItemImage );
	return pItemImage;
}*/

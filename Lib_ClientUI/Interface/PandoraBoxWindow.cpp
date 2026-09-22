#include "StdAfx.h"
#include "PandoraBoxWindow.h"
#include "PandoraBoxItemSlot.h"
#include "PandoraBoxResultsItemSlot.h"

#include "./DxFontMan.h"
#include "./d3dfont.h"

#include "./DxGlobalStage.h"
#include "./BasicButton.h"
#include "./BasicTextBox.h"
#include "./BasicTextBoxEx.h"
#include "./BasicTextButton.h"
#include "./BasicLineBox.h"
#include "./UITextControl.h"
#include "./GameTextControl.h"
#include "./BasicScrollBarEx.h"
#include "./BasicScrollThumbFrame.h"
#include "./SwapImage.h"
#include "./Util/CheckBox.h"
#include "./InnerInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int CPandoraBoxWindow::nSTARTLINE = 0;
const int CPandoraBoxWindow::nOUTOFRANGE = -1;

CPandoraBoxWindow::CPandoraBoxWindow ( GLGaeaClient* pGaeaClient, CInnerInterface* pInterface )
	: m_pGaeaClient(pGaeaClient)
	, m_pInterface ( pInterface )
	, m_pListTextBox(NULL)
	, m_pListScrollBar(NULL)
	, m_pItemListScrollBar(NULL)
	, m_pButtonPurchase(NULL)
	, m_pTextCharEPoint(NULL)
	, m_pTextCharGoldPoint(NULL)
	, m_pButtonClear(NULL)
	, m_pTextEPoint(NULL)
	, m_pTextGoldPoint(NULL)
	, m_sSelectItem(NATIVEID_NULL())
	, m_nCurPos ( -1 )
{
	for( int i=0; i<PANDORA_BOX_MAX_ITEM; ++i )
		m_pItemSlot[i] = NULL;

	for( int i=0; i<PANDORA_BOX_MAX_RESULT_ITEM; ++i )
		m_pItemResultsSlot[i] = NULL;

	memset ( m_pItemSlotArrayDummy, 0, sizeof( m_pItemSlotArrayDummy ) );

	for( int i=0; i<PANDORA_BOX_OPEN_TYPE_SIZE; ++i )
		m_pOpenType[i] = NULL;
}

CPandoraBoxWindow::~CPandoraBoxWindow ()
{
}

void CPandoraBoxWindow::CreateSubControl ()
{
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );
	CD3DFontPar* pFont20 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 12, _DEFAULT_FONT_SHADOW_FLAG );

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxWhite ( "PANDORA_WINDOW_WHITE_BG" );
	//RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "PANDORA_WINDOW_ITEMLIST" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "PANDORA_WINDOW_ITEMRESULT" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "PANDORA_WINDOW_OPENBOX" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "PANDORA_ITEM_RESULT_CURR_1" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "PANDORA_ITEM_RESULT_CURR_2" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "PANDORA_OPENBOX_NAME" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "PANDORA_ITEM_RESULT_NAME" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "PANDORA_ITEM_LIST_NAME" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "PANDORA_OPENBOX_CURR_1_BACK" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "PANDORA_OPENBOX_CURR_2_BACK" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "PANDORA_INFO_BACK_BOX" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "PANDORA_ITEM_LIST_BG" );
	RegisterControl ( pBasicLineBox );

	m_pListTextBox = new CBasicTextBoxEx;
	m_pListTextBox->CreateSub ( this, "PANDORA_INFO_BOX", UI_FLAG_DEFAULT, PANDORA_BOX_SELECTION_TEXTBOX );
	m_pListTextBox->SetFont ( pFont9 );		
	m_pListTextBox->SetLineInterval ( 3.0f );
	m_pListTextBox->SetSensitive ( true );
	m_pListTextBox->SetLimitLine ( 10000 );
	RegisterControl ( m_pListTextBox );

	int nTotalLine = m_pListTextBox->GetVisibleLine ();

	m_pListScrollBar = new CBasicScrollBarEx;
	m_pListScrollBar->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, PANDORA_BOX_SELECTION_SCROLLBAR );
	m_pListScrollBar->CreateBaseScrollBar ( "PANDORA_INFO_SCROLLBAR" );
	m_pListScrollBar->GetThumbFrame()->SetState ( 1, nTotalLine );
	RegisterControl ( m_pListScrollBar );

	m_pTextCharGoldPoint = new CBasicTextBox;
	m_pTextCharGoldPoint->CreateSub ( this, "PANDORA_ITEM_RESULT_CURR_1" );
	m_pTextCharGoldPoint->SetFont ( pFont9 );
	m_pTextCharGoldPoint->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextCharGoldPoint->SetText( "--" );
	RegisterControl ( m_pTextCharGoldPoint );

	m_pTextCharEPoint = new CBasicTextBox;
	m_pTextCharEPoint->CreateSub ( this, "PANDORA_ITEM_RESULT_CURR_2" );
	m_pTextCharEPoint->SetFont ( pFont9 );
	m_pTextCharEPoint->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextCharEPoint->SetText( "--" );
	RegisterControl ( m_pTextCharEPoint );

	m_pTextGoldPoint = new CBasicTextBox;
	m_pTextGoldPoint->CreateSub ( this, "PANDORA_OPENBOX_CURR_1_BACK" );
	m_pTextGoldPoint->SetFont ( pFont9 );
	m_pTextGoldPoint->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextGoldPoint->SetText( "8,000,000" );
	RegisterControl ( m_pTextGoldPoint );

	m_pTextEPoint = new CBasicTextBox;
	m_pTextEPoint->CreateSub ( this, "PANDORA_OPENBOX_CURR_2_BACK" );
	m_pTextEPoint->SetFont ( pFont9 );
	m_pTextEPoint->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	m_pTextEPoint->SetText( "0", NS_UITEXTCOLOR::DODGERBLUE );
	RegisterControl ( m_pTextEPoint );

	std::string strSlot[PANDORA_BOX_MAX_ITEM] =
	{
		"PANDORA_ITEM_SLOT_00",
		"PANDORA_ITEM_SLOT_01",
		"PANDORA_ITEM_SLOT_02",
		"PANDORA_ITEM_SLOT_03",
		"PANDORA_ITEM_SLOT_04",
		"PANDORA_ITEM_SLOT_05",
		"PANDORA_ITEM_SLOT_06",
		"PANDORA_ITEM_SLOT_07",
		"PANDORA_ITEM_SLOT_08",
		"PANDORA_ITEM_SLOT_09",
		"PANDORA_ITEM_SLOT_10",
		"PANDORA_ITEM_SLOT_11",
	};

	//	���� ����
	for ( int i = 0; i < PANDORA_BOX_MAX_ITEM; i++ )
	{
		m_pItemSlotArrayDummy[i] = CreateControl ( strSlot[i].c_str () );
	}

	for( int i=0; i<PANDORA_BOX_MAX_ITEM; ++i )
	{
		m_pItemSlot[i] = new CPandoraBoxItemSlot(m_pGaeaClient);
		m_pItemSlot[i]->CreateSub ( this, strSlot[0].c_str(), UI_FLAG_DEFAULT, PANDORA_BOX_ITEM00 + i );
		m_pItemSlot[i]->CreateSubControl ();
		m_pItemSlot[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pItemSlot[i] );
	}

	std::string strSlotResult[PANDORA_BOX_MAX_RESULT_ITEM] =
	{
		"PANDORA_ITEM_RESULT_SLOT_0",
		"PANDORA_ITEM_RESULT_SLOT_1",
		"PANDORA_ITEM_RESULT_SLOT_2",
		"PANDORA_ITEM_RESULT_SLOT_3",
		"PANDORA_ITEM_RESULT_SLOT_4",
		"PANDORA_ITEM_RESULT_SLOT_5",
		"PANDORA_ITEM_RESULT_SLOT_6",
		"PANDORA_ITEM_RESULT_SLOT_7",
		"PANDORA_ITEM_RESULT_SLOT_8",
		"PANDORA_ITEM_RESULT_SLOT_9",
	};

	for( int i=0; i<PANDORA_BOX_MAX_RESULT_ITEM; ++i )
	{
		m_pItemResultsSlot[i] = new CPandoraBoxResultsItemSlot(m_pGaeaClient);
		m_pItemResultsSlot[i]->CreateSub ( this, strSlotResult[i].c_str(), UI_FLAG_DEFAULT, PANDORA_BOX_RESULT_ITEM00 + i );
		m_pItemResultsSlot[i]->CreateSubControl ();
		m_pItemResultsSlot[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pItemResultsSlot[i] );
	}

	m_pItemListScrollBar = new CBasicScrollBarEx;
	m_pItemListScrollBar->CreateSub ( this, "BASIC_SCROLLBAR", UI_FLAG_RIGHT | UI_FLAG_YSIZE, PANDORA_BOX_LIST_SCROLLBAR );
	m_pItemListScrollBar->CreateBaseScrollBar ( "PANDORA_ITEM_LIST_SCROLLBAR" );
	m_pItemListScrollBar->GetThumbFrame()->SetState ( GLCONST_CHAR::vecPandoraBoxes.size(), PANDORA_BOX_MAX_ITEM );
	RegisterControl ( m_pItemListScrollBar );
	
	m_pButtonPurchase = new CBasicTextButton;
	m_pButtonPurchase->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, PANDORA_BOX_PURCHASE_BUTTON );
	m_pButtonPurchase->CreateBaseButton ( "PANDORA_OPENBOX_BTN", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, "Open" );
	RegisterControl ( m_pButtonPurchase );
	
	m_pButtonClear = new CBasicTextButton;
	m_pButtonClear->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, PANDORA_BOX_CLEAR_BUTTON );
	m_pButtonClear->CreateBaseButton ( "PANDORA_OPENBOX_RELOAD", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, "Clear All" );
	RegisterControl ( m_pButtonClear );

	/*m_pButtonClose = new CBasicTextButton;
	m_pButtonClose->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, PANDORA_BOX_CLOSE_BUTTON );
	m_pButtonClose->CreateBaseButton ( "PANDORA_WINDOW_CLOSE_BTN", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, "Close" );
	RegisterControl ( m_pButtonClose );*/

	m_pOpenType[0] = new CCheckBox;
	m_pOpenType[0]->CreateSub ( this, "PANDORA_BOX_OPEN_CHECKBOX_MONEY", UI_FLAG_DEFAULT, PANDORA_BOX_TYPE1_BUTTON );
	m_pOpenType[0]->CreateSubControl ( "PANDORA_CHECKBOX_OFF", "PANDORA_CHECKBOX_ON" );
	m_pOpenType[0]->SetCheck( TRUE );
	RegisterControl ( m_pOpenType[0] );

	m_pOpenType[1] = new CCheckBox;
	m_pOpenType[1]->CreateSub ( this, "PANDORA_BOX_OPEN_CHECKBOX_EPOINT", UI_FLAG_DEFAULT, PANDORA_BOX_TYPE2_BUTTON );
	m_pOpenType[1]->CreateSubControl ( "PANDORA_CHECKBOX_OFF", "PANDORA_CHECKBOX_ON" );
	m_pOpenType[1]->SetCheck( FALSE );
	RegisterControl ( m_pOpenType[1] );

	CSwapImage* pSwapImage = NULL;

	pSwapImage = new CSwapImage;
	pSwapImage->CreateSub   ( this, "PANDORA_BOX_OPEN_ICON", UI_FLAG_DEFAULT );
	pSwapImage->SetImage    ( "PANDORA_BOX_OPEN_ICON0" );
	RegisterControl ( pSwapImage );
	m_pImageBOX = pSwapImage;


	CBasicTextBox*		pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, "PANDORA_ITEM_RESULT_NAME" );
	pTextBox->SetFont ( pFont20 );
	pTextBox->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	pTextBox->SetText( "Results", NS_UITEXTCOLOR::GOLD );
	RegisterControl ( pTextBox );

	pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, "PANDORA_ITEM_LIST_NAME" );
	pTextBox->SetFont ( pFont20 );
	pTextBox->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	pTextBox->SetText( "Possible To Obtain", NS_UITEXTCOLOR::GOLD );
	RegisterControl ( pTextBox );
	
	pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, "PANDORA_OPENBOX_NAME" );
	pTextBox->SetFont ( pFont20 );
	pTextBox->SetTextAlign ( TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y );
	pTextBox->SetText( "Open Box", NS_UITEXTCOLOR::GOLD );
	RegisterControl ( pTextBox );
	m_pTextBoxStatus = pTextBox;

	CreateControl ( "PANDORA_CHECKBOX_GOLD" );
	CreateControl ( "PANDORA_CHECKBOX_CONTRIB" );

	CBasicButton* pButton = new CBasicButton;
	pButton->CreateSub ( this, "PANDORA_RES_INVENTORY_MONEY_BUTTON", UI_FLAG_DEFAULT );
	pButton->CreateFlip ( "PANDORA_RES_INVENTORY_MONEY_BUTTON_F", CBasicButton::CLICK_FLIP );
	pButton->SetControlNameEx ( "PANDORA_RES_INVENTORY_MONEY_BUTTON" );
	RegisterControl ( pButton );

	pButton = new CBasicButton;
	pButton->CreateSub ( this, "PANDORA_RES_INVENTORY_POINT_BUTTON", UI_FLAG_DEFAULT );
	pButton->CreateFlip ( "PANDORA_RES_INVENTORY_POINT_BUTTON_F", CBasicButton::CLICK_FLIP );
	pButton->SetControlNameEx ( "PANDORA_RES_INVENTORY_POINT_BUTTON" );
	RegisterControl ( pButton );

	pButton = new CBasicButton;
	pButton->CreateSub ( this, "PANDORA_BUTTON_RES_INVENTORY_MONEY_BUTTON", UI_FLAG_DEFAULT );
	pButton->CreateFlip ( "PANDORA_BUTTON_RES_INVENTORY_MONEY_BUTTON_F", CBasicButton::CLICK_FLIP );
	pButton->SetControlNameEx ( "PANDORA_BUTTON_RES_INVENTORY_MONEY_BUTTON" );
	RegisterControl ( pButton );

	pButton = new CBasicButton;
	pButton->CreateSub ( this, "PANDORA_BUTTON_RES_INVENTORY_POINT_BUTTON", UI_FLAG_DEFAULT );
	pButton->CreateFlip ( "PANDORA_BUTTON_RES_INVENTORY_POINT_BUTTON_F", CBasicButton::CLICK_FLIP );
	pButton->SetControlNameEx ( "PANDORA_BUTTON_RES_INVENTORY_POINT_BUTTON" );
	RegisterControl ( pButton );

	ResetWindow();

	SetItemSlotRender ( nSTARTLINE, PANDORA_BOX_MAX_ITEM );
}

void CPandoraBoxWindow::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( m_pTextCharEPoint )
	{
		m_pTextCharEPoint->ClearText();
		CString strMoney;
		strMoney = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( m_pGaeaClient->GetCharacter()->m_dwPremiumPoints, 3, "," );
		m_pTextCharEPoint->SetText( strMoney.GetString(), NS_UITEXTCOLOR::DODGERBLUE );
	}

	if ( m_pTextCharGoldPoint )
	{
		m_pTextCharGoldPoint->ClearText();
		CString strMoney;
		strMoney = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( m_pGaeaClient->GetCharacter()->m_lnMoney, 3, "," );
		m_pTextCharGoldPoint->SetText( strMoney.GetString(), NS_UITEXTCOLOR::GOLD );
	}

	if ( m_pTextGoldPoint )
	{
		m_pTextGoldPoint->ClearText();
		CString strMoney;
		strMoney = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( GLCONST_CHAR::sPandoraSetting.llGold, 3, "," );
		m_pTextGoldPoint->SetText( strMoney.GetString() );
	}

	if ( m_pTextEPoint )
	{
		m_pTextEPoint->ClearText();
		CString strMoney;
		strMoney = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( GLCONST_CHAR::sPandoraSetting.wEP, 3, "," );
		int nIndex = m_pTextEPoint->AddText(  strMoney.GetString(), NS_UITEXTCOLOR::DODGERBLUE );
		strMoney.Format( "(Chance : %.2f%%)", GLCONST_CHAR::sPandoraSetting.fPremChance );
		m_pTextEPoint->AddString( nIndex, strMoney.GetString(), NS_UITEXTCOLOR::GOLD );
	}

	if ( m_pListScrollBar )
	{
		CBasicScrollThumbFrame* const pThumbFrame = m_pListScrollBar->GetThumbFrame ();

		const int nTotalLine = m_pListTextBox->GetTotalLine ();
		const int nLinePerOneView = m_pListTextBox->GetVisibleLine ();
		CDebugSet::ToView ( 1, 10, "���̴� ���� %d", nLinePerOneView );
		pThumbFrame->SetState ( nTotalLine, nLinePerOneView );
		if ( nLinePerOneView < nTotalLine )
		{
			const int nMovableLine = nTotalLine - nLinePerOneView;
			float fPercent = pThumbFrame->GetPercent ();
			int nPos = (int)floor(fPercent * nMovableLine);
			m_pListTextBox->SetCurLine ( nPos );	
		}
	}


	if ( m_pItemListScrollBar )
	{
		CBasicScrollThumbFrame* pThumbFrame = m_pItemListScrollBar->GetThumbFrame ();
		int nTotal = pThumbFrame->GetTotal ();

		if ( nTotal < PANDORA_BOX_MAX_ITEM ) return ;

		const int nViewPerPage = pThumbFrame->GetViewPerPage ();
		if ( nViewPerPage <= nTotal )
		{
			int nCurPos = nSTARTLINE;
			const int nMovableLine = nTotal - nViewPerPage;
			float fPercent = pThumbFrame->GetPercent ();

			nCurPos = (int)floor(fPercent * nMovableLine);
			if ( nCurPos < nSTARTLINE ) nCurPos = nSTARTLINE;

			if ( m_nCurPos == nCurPos ) return;

			m_nCurPos = nCurPos;

			ResetAllItemSlotRender ();
			SetItemSlotRender ( nCurPos, nCurPos + nViewPerPage );
		}
	}
}

void CPandoraBoxWindow::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{

	switch( ControlID )
	{
	case ET_CONTROL_TITLE:
	case ET_CONTROL_TITLE_F:
		{
			if ( (dwMsg & UIMSG_LB_DUP) && CHECK_MOUSE_IN ( dwMsg ) )
			{
				m_pInterface->SetDefaultPosInterface( ITEMSHOP_WINDOW );
			}
		}break;
	case ET_CONTROL_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				m_pGaeaClient->GetCharacter()->ReqItemShopOpen( false );
				m_pInterface->HideGroup ( GetWndID () );
			}
		}
		break;

	case PANDORA_BOX_TYPE1_BUTTON:
		{
			if( dwMsg & UIMSG_CHECKBOX_PUSH )
			{
				if ( m_pOpenType[1] ) 
					m_pOpenType[1]->SetCheck( !m_pOpenType[0]->IsChecked() );
			}
		}
		break;
	case PANDORA_BOX_TYPE2_BUTTON:
		{
			if( dwMsg & UIMSG_CHECKBOX_PUSH )
			{
				if ( m_pOpenType[0] ) 
					m_pOpenType[0]->SetCheck( !m_pOpenType[1]->IsChecked() );
			}
		}
		break;

	case PANDORA_BOX_RESULT_ITEM00:
	case PANDORA_BOX_RESULT_ITEM01:
	case PANDORA_BOX_RESULT_ITEM02:
	case PANDORA_BOX_RESULT_ITEM03:
	case PANDORA_BOX_RESULT_ITEM04:
	case PANDORA_BOX_RESULT_ITEM05:
	case PANDORA_BOX_RESULT_ITEM06:
	case PANDORA_BOX_RESULT_ITEM07:
	case PANDORA_BOX_RESULT_ITEM08:
	case PANDORA_BOX_RESULT_ITEM09:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				int nSelect = ControlID - PANDORA_BOX_RESULT_ITEM00;
				if ( m_pItemResultsSlot[nSelect] )
				{
					GLPANDORA_BOX sshop_item = m_pItemResultsSlot[nSelect]->ItemGet();
					SITEMCUSTOM sitemcustom(sshop_item.sItemID);
					SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sitemcustom.sNativeID );
					if ( pitem_data )
					{
						if ( pitem_data->ISPILE() )
							sitemcustom.wTurnNum = pitem_data->GETAPPLYNUM();

						sitemcustom.cDAMAGE = (BYTE)pitem_data->sBasicOp.wGradeAttack;
						sitemcustom.cDEFENSE = (BYTE)pitem_data->sBasicOp.wGradeDefense;

						if ( UIMSG_PANDORA_BOX_RESULT_SLOT_ITEM_IMAGE & dwMsg )
						{
							m_pInterface->SHOW_ITEM_INFO( sitemcustom, FALSE, FALSE, FALSE, NULL, NULL );
						}
					}

					if ( UIMSG_PANDORA_BOX_RESULT_SLOT_ITEM_REMOVE & dwMsg )
					{
						GLMSG::SNET_PANDORA_BOX_BUY_ITEM NetMsg;
						NetMsg.emReq = EMPANDORA_ITEM_REMOVE_REQ;
						NetMsg.nIndex = sshop_item.nIndex;
						NETSENDTOFIELD(&NetMsg);
						return;
					}

					if ( UIMSG_PANDORA_BOX_RESULT_SLOT_ITEM_BUY & dwMsg )
					{
						GLMSG::SNET_PANDORA_BOX_BUY_ITEM NetMsg;
						NetMsg.emReq = (sshop_item.llGold != 0) ? EMPANDORA_ITEM_BUY_GOLD_REQ : EMPANDORA_ITEM_BUY_PREM_REQ;
						NetMsg.nIndex = sshop_item.nIndex;
						NETSENDTOFIELD(&NetMsg);
						return;
					}
				}
			}
		}
		break;

	case PANDORA_BOX_ITEM00:
	case PANDORA_BOX_ITEM01:
	case PANDORA_BOX_ITEM02:
	case PANDORA_BOX_ITEM03:
	case PANDORA_BOX_ITEM04:
	case PANDORA_BOX_ITEM05:
	case PANDORA_BOX_ITEM06:
	case PANDORA_BOX_ITEM07:
	case PANDORA_BOX_ITEM08:
	case PANDORA_BOX_ITEM09:
	case PANDORA_BOX_ITEM010:
	case PANDORA_BOX_ITEM011:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				int nSelect = ControlID - PANDORA_BOX_ITEM00;
				if ( m_pItemSlot[nSelect] )
				{
					GLPANDORA_BOX sshop_item = m_pItemSlot[nSelect]->ItemGet();
					SITEMCUSTOM sitemcustom(sshop_item.sItemID);
					SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sitemcustom.sNativeID );
					if ( pitem_data )
					{
						if ( pitem_data->ISPILE() )
						sitemcustom.wTurnNum = pitem_data->GETAPPLYNUM();

						sitemcustom.cDAMAGE = (BYTE)pitem_data->sBasicOp.wGradeAttack;
						sitemcustom.cDEFENSE = (BYTE)pitem_data->sBasicOp.wGradeDefense;

						if ( UIMSG_PANDORA_BOX_SLOT_ITEM_IMAGE & dwMsg )
						{
							m_pInterface->SHOW_ITEM_INFO( sitemcustom, FALSE, FALSE, FALSE, NULL, NULL );
						}
					}

					if ( CHECK_LB_UP_LIKE ( dwMsg ) )	
					{
						SelectItem(nSelect);
					}
				}
			}
		}break;

	case PANDORA_BOX_CLEAR_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				GLMSG::SNET_PANDORA_BOX_CLEAR_RESULTS_REQ NetMsg;
				NETSENDTOFIELD(&NetMsg);
			}
		}
		break;

	case PANDORA_BOX_PURCHASE_BUTTON:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				if ( !m_pOpenType[0]->IsChecked() && !m_pOpenType[1]->IsChecked() )
					return;

				if ( m_pOpenType[0]->IsChecked() && m_pOpenType[1]->IsChecked() )
					return;

				GLMSG::SNET_PANDORA_BOX_OPEN_REQ NetMsg;
				if ( m_pOpenType[0]->IsChecked() )
					NetMsg.emReq = EMPANDORA_OPEN_BOX_GOLD;
				else if ( m_pOpenType[1]->IsChecked() )
					NetMsg.emReq = EMPANDORA_OPEN_BOX_PREM;
				NETSENDTOFIELD(&NetMsg);
			}
		}break;
	};
	CUIWindowEx::TranslateUIMessage ( ControlID, dwMsg );
}

void CPandoraBoxWindow::SetVisibleSingle ( BOOL bVisible )
{
	CUIWindowEx::SetVisibleSingle( bVisible );

	if( bVisible )
	{
		InitWindow();
	}
	else
	{
		ResetWindow();
	}
}

void CPandoraBoxWindow::InitWindow()
{
	m_mapItem = m_pGaeaClient->GetCharacter()->m_mapPandoraResults;
	if (m_mapItem.empty()) {
		GLMSG::SNET_PANDORA_BOX_REFRESH_RESULT NetMsgRes;
		NETSENDTOFIELD(&NetMsgRes);
	}
	ResetResultsSlotRender();
}

void CPandoraBoxWindow::ResetWindow()
{
	m_mapItem.clear();
	m_vecItem.clear();

	m_sSelectItem = NATIVEID_NULL();

	for ( int i=0; i<PANDORA_BOX_MAX_ITEM; ++i )
	{
		m_pItemSlot[i]->ItemReset();
		m_pItemSlot[i]->SetVisibleSingle( FALSE );
	}

	for ( int i=0; i<PANDORA_BOX_MAX_RESULT_ITEM; ++i )
	{
		m_pItemResultsSlot[i]->ItemReset();
		m_pItemResultsSlot[i]->SetVisibleSingle( FALSE );
	}

	std::sort(  GLCONST_CHAR::vecPandoraBoxes.begin(),GLCONST_CHAR::vecPandoraBoxes.end() );


	for ( PANDORABOXMAP_ITER iter = m_mapItem.begin(); iter != m_mapItem.end(); ++iter )
		m_vecItem.push_back(iter->second);

	std::sort(m_vecItem.begin(),m_vecItem.end());

	ShowItem ();
}

void CPandoraBoxWindow::ShowItem()
{
	for ( int i=0; i< PANDORA_BOX_MAX_ITEM; ++ i )
	{
		m_pItemSlot[i]->ItemReset();
		m_pItemSlot[i]->SetVisibleSingle( FALSE );
	}

	for ( int i=0; i< PANDORA_BOX_MAX_RESULT_ITEM; ++ i )
	{
		m_pItemResultsSlot[i]->ItemReset();
		m_pItemResultsSlot[i]->SetVisibleSingle( FALSE );
	}

	for ( int i=0; i<PANDORA_BOX_MAX_ITEM && !GLCONST_CHAR::vecPandoraBoxes.empty(); ++i )
	{
		if ( m_pItemSlot[i] )
		{
			m_pItemSlot[i]->SetVisibleSingle( TRUE );
			m_pItemSlot[i]->ItemSet( GLCONST_CHAR::vecPandoraBoxes[i] );
		}
	}
}

void CPandoraBoxWindow::SelectItem( int nSelect )
{
	for ( int i=0; i< PANDORA_BOX_MAX_ITEM; ++ i )
	{
		if ( m_pItemSlot[i] )
			m_pItemSlot[i]->ItemSelect( FALSE );
	}

	if ( nSelect >= 0 && nSelect < PANDORA_BOX_MAX_ITEM )
	{
		if ( m_pItemSlot[nSelect] )
		{
			if ( m_pItemSlot[nSelect]->IsVisible() )
				m_pItemSlot[nSelect]->ItemSelect( TRUE );

			m_sSelectItem = m_pItemSlot[nSelect]->ItemGet().sItemID;
		}
	}
	else
	{
		m_sSelectItem = NATIVEID_NULL();
	}

}

void CPandoraBoxWindow::ResetResultsSlotRender()
{
	for ( int i=0; i<PANDORA_BOX_MAX_RESULT_ITEM; ++i )
	{
		m_pItemResultsSlot[i]->ItemReset();
		m_pItemResultsSlot[i]->SetVisibleSingle( FALSE );
	}

	m_vecItem.clear();

	for ( PANDORABOXMAP_ITER iter = m_mapItem.begin(); iter != m_mapItem.end(); ++iter )
		m_vecItem.push_back(iter->second);

	std::sort(m_vecItem.begin(),m_vecItem.end());

	for ( int i=0, j=0; i<m_vecItem.size(); ++i )
	{
		if ( m_vecItem[i].sItemID != NATIVEID_NULL() )
		{
			if ( m_pItemResultsSlot[j] )
			{
				m_pItemResultsSlot[j]->ItemSet( m_vecItem[i] );
				m_pItemResultsSlot[j++]->SetVisibleSingle( TRUE );
			}
		}
	}

	switch ( m_pGaeaClient->GetCharacter()->m_sPandoraStatus.emStatus )
	{
	case EMPANDORA_STATUS_RARE:
		m_pImageBOX->SetImage( "PANDORA_BOX_OPEN_ICON1" );
		m_pTextBoxStatus->SetText( "Open Box", NS_UITEXTCOLOR::GOLD );
		break;
	case EMPANDORA_STATUS_FREE:
		m_pImageBOX->SetImage( "PANDORA_BOX_OPEN_ICON2" );
		m_pTextBoxStatus->SetText( "Free Box", NS_UITEXTCOLOR::LIGHTSKYBLUE );
		break;
	case EMPANDORA_STATUS_OPEN:
		m_pImageBOX->SetImage( "PANDORA_BOX_OPEN_ICON0" );
		m_pTextBoxStatus->SetText( "Open Box", NS_UITEXTCOLOR::GOLD );
		break;
	}
}

void CPandoraBoxWindow::ResetAllItemSlotRender ()
{
	for ( int i = 0; i < PANDORA_BOX_MAX_ITEM; i++ )
	{
		if ( m_pItemSlot[i] )
		{
			m_pItemSlot[i]->ItemReset();
			m_pItemSlot[i]->SetVisibleSingle ( FALSE );
		}
	}
}

void CPandoraBoxWindow::SetItemSlotRender ( int nStartIndex, int nTotal )
{
	if ( nTotal < 0 ) return ;

	const UIRECT& rcParentPos = GetGlobalPos ();

	for ( int i = nStartIndex; i < nTotal; i++ )
	{
		int nAbsoluteIndex = i - nStartIndex;
		CPandoraBoxItemSlot* pItemSlot = m_pItemSlot[nAbsoluteIndex];
		if ( pItemSlot )
		{
			CUIControl* pDummyControl = m_pItemSlotArrayDummy[nAbsoluteIndex];			
			const UIRECT& rcSlotPos = pDummyControl->GetGlobalPos ();
			const UIRECT& rcSlotLocalPos = pDummyControl->GetLocalPos ();

			pItemSlot->SetLocalPos ( rcSlotLocalPos );
			pItemSlot->SetGlobalPos ( rcSlotPos );

			pItemSlot->SetVisibleSingle ( TRUE );
			pItemSlot->ItemSet( GLCONST_CHAR::vecPandoraBoxes[i] );
		}
	}
}

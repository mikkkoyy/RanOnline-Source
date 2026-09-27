#include "StdAfx.h"
#include "ItemMallWindowMenuSlot.h"
#include "GameTextControl.h"
#include "UITextControl.h"
#include "ItemImage.h"
#include "GLGaeaClient.h"
#include "InnerInterface.h"

#include "ModalWindow.h"
#include "ModalCallerID.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "BasicButton.h"
#include "BasicTextBox.h"
#include "BasicLineBox.h"
#include "BasicLineBoxEx.h"
#include "BasicTextButton.h"

#include "GLItemMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CItemMallWindowMenuSlot::CItemMallWindowMenuSlot () 
	: m_pItemImage ( NULL )
	, m_pTextBox ( NULL )
	, m_pTextBoxQty ( NULL ) 
	, m_pTextBoxDesc ( NULL )
	, m_pBuyButton ( NULL )
{
	memset( m_pImage, 0, sizeof ( CUIControl* ) * 2 );
}

CItemMallWindowMenuSlot::~CItemMallWindowMenuSlot ()
{
}

void CItemMallWindowMenuSlot::CreateSubControl ()
{
	D3DCOLOR dwColor = NS_UITEXTCOLOR::DEFAULT;
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_FLAG );
	int	nTextAlign = TEXT_ALIGN_LEFT;

	CBasicLineBox* m_pLineBox = new CBasicLineBox;
	m_pLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	m_pLineBox->CreateBaseBoxQuestList( "ITEMMALL_SLOT_BACKLINE" );
	m_pLineBox->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pLineBox );

	CString strMark[] = 
	{
		"ITEMMALL_IMAGE_HOT",
		"ITEMMALL_IMAGE_NEW"
	};

	for( int i=0;i<2;i++)
	{
		m_pImage[i] =  new CUIControl;
		m_pImage[i]->CreateSub ( this, strMark[i].GetString(), UI_FLAG_DEFAULT );	
		m_pImage[i]->SetVisibleSingle( FALSE );
		RegisterControl ( m_pImage[i] );
	}
	

	m_pTextBox = CreateStaticControl ( "ITEMMALL_SLOT_TEXT", pFont, dwColor, nTextAlign );			
	m_pTextBox->SetLineInterval ( 2.0f );

	m_pTextBoxQty = CreateStaticControl ( "ITEMMALL_SLOT_QTY_TEXT", pFont, dwColor, nTextAlign );			
	m_pTextBoxQty->SetLineInterval ( 2.0f );

	m_pTextBoxDesc = CreateStaticControl ( "ITEMMALL_SLOT_DESC_TEXT", pFont, dwColor, nTextAlign );	
	m_pTextBoxDesc->SetLineInterval ( 2.0f );

	m_pItemImage = CreateItemImage();

	const int nBUTTONSIZE = CBasicTextButton::SIZE19;
	m_pBuyButton = new CBasicTextButton;
	m_pBuyButton->CreateSub ( this, "BASIC_TEXT_BUTTON19", UI_FLAG_XSIZE, ITEM_BUTTON_BUY );
	m_pBuyButton->CreateBaseButton ( "ITEMMALL_SLOT_BUY_BUTTON", nBUTTONSIZE, CBasicButton::RADIO_FLIP, "BUY" );
	m_pBuyButton->SetFlip( FALSE );
	RegisterControl ( m_pBuyButton );
}

CItemImage* CItemMallWindowMenuSlot::CreateItemImage ()
{
	CItemImage* pItemImage = new CItemImage;
	pItemImage->CreateSub ( this, "ITEMMALL_SLOT_IMAGE", UI_FLAG_DEFAULT, ITEM_SLOT_IMAGE );
	pItemImage->CreateSubControl ();
	pItemImage->SetUseRender ( TRUE );
    RegisterControl ( pItemImage );
	return pItemImage;
}

CUIControl*	CItemMallWindowMenuSlot::CreateControl ( char* szControl )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl );
	RegisterControl ( pControl );
	return pControl;
}

CBasicButton* CItemMallWindowMenuSlot::CreateFlipButton ( char* szButton, char* szButtonFlip, char* szMouseOver, UIGUID ControlID )
{
	CBasicButton* pButton = new CBasicButton;
	pButton->CreateSub ( this, szButton, UI_FLAG_DEFAULT, ControlID );
	pButton->CreateFlip ( szButtonFlip, CBasicButton::RADIO_FLIP );
	pButton->CreateMouseOver ( szMouseOver );
	RegisterControl ( pButton );
	return pButton;
}

CBasicTextBox* CItemMallWindowMenuSlot::CreateStaticControl ( char* szControlKeyword, CD3DFontPar* pFont, D3DCOLOR dwColor, int nAlign )
{
	CBasicTextBox* pStaticText = new CBasicTextBox;
	pStaticText->CreateSub ( this, szControlKeyword );
	pStaticText->SetFont ( pFont );
	pStaticText->SetTextAlign ( nAlign );
	RegisterControl ( pStaticText );
	return pStaticText;
}

void CItemMallWindowMenuSlot::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{	
	if ( !IsVisible () ) return ;

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}
void CItemMallWindowMenuSlot::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage( ControlID, dwMsg );

	switch( ControlID )
	{
	case ITEM_SLOT_IMAGE:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				SITEM* pItem = GLItemMan::GetInstance().GetItem( GetNativeItemID() );
				if( pItem )
				{
					SITEMCUSTOM sItemCustom;
					sItemCustom.sNativeID = pItem->sBasicOp.sNativeID;
					sItemCustom.cDAMAGE = pItem->sBasicOp.wGradeAttack;
					sItemCustom.cDEFENSE = pItem->sBasicOp.wGradeDefense;
					sItemCustom.wTurnNum = pItem->sDrugOp.wPileNum;

					CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW ( sItemCustom, TRUE, TRUE, FALSE, FALSE, FALSE, USHRT_MAX, USHRT_MAX );

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
			}
		}
		break;
	case ITEM_BUTTON_BUY:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( dwMsg & UIMSG_LB_DOWN )
				{	
					m_pBuyButton->SetFlip( TRUE );
					SITEM* pItem = GLItemMan::GetInstance().GetItem( GetNativeItemID() );
					if( pItem )
					{
						CString strType[2]={"Premium Points", 
							"Game Points"};
						const std::string& strItemName = pItem->GetName();
						CString strCombine = CInnerInterface::GetInstance().MakeString ( ID2GAMEINTEXT("BUY_ITEMMALL_CONFIRM"), strItemName.c_str(), m_sItemMallData.dwItemPrice, strType[m_sItemMallData.wItemCtg].GetString() );
						DoModal ( strCombine, MODAL_QUESTION, YESNO, MODAL_ITEMMALL_BUY_ITEM );
					}
				}				
			}
		}
		break;
	}
}

void CItemMallWindowMenuSlot::SetItem( ITEMMALLDATA sItemData )
{
	m_sItemMallData = sItemData;

	SITEM* pItemData = GLItemMan::GetInstance().GetItem ( m_sItemMallData.sNativeID );
	if( pItemData )
	{
		if( sItemData.dwIndex < 3 ) m_pImage[0]->SetVisibleSingle( TRUE );
		else if ( sItemData.dwIndex < 10 ) m_pImage[1]->SetVisibleSingle( TRUE );
		m_pItemImage->SetItem ( m_sItemMallData.sNativeID );
		m_pItemImage->SetVisibleSingle( TRUE );
		m_pBuyButton->SetVisibleSingle( TRUE );
		m_pBuyButton->SetFlip( FALSE );

		CString strType[2]={"Premium Points", 
							"Game Points"};
		m_strName.Format( "%s", pItemData->GetName());
		m_strDesc.Format( "Type: %s", strType[m_sItemMallData.wItemCtg].GetString() );
		m_strPrice.Format( "Price: %d\t", m_sItemMallData.dwItemPrice );
		m_strStocks.Format( "Stocks: %d", m_sItemMallData.dwItemStocks);
		if( m_pTextBox && m_pTextBoxDesc && m_pTextBoxQty )
		{
			m_pTextBox->ClearText ();
			m_pTextBoxDesc->ClearText ();
			m_pTextBoxQty->ClearText();

			m_pTextBox->AddTextNoSplit ( m_strName );
			m_pTextBox->AddTextNoSplit ( m_strPrice );
			m_pTextBoxQty->AddTextNoSplit ( " " );
			m_pTextBoxQty->AddTextNoSplit ( m_strStocks );

			m_pTextBoxDesc->AddTextNoSplit ( m_strDesc, NS_UITEXTUTIL::ENABLE );
		}
	}
	else
	{
		this->SetVisibleSingle(FALSE);
	}
}
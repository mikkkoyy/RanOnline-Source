#include "StdAfx.h"
#include "ProductWindowProduct.h"

#include "BasicLineBox.h"
#include "BasicTextBox.h"
#include "BasicTextButton.h"
#include "ItemImage.h"
#include "BasicProgressBar.h"
#include "BasicLineBoxSmart.h"

#include "../Lib_Engine/Common/StringFormat.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "UITextControl.h"
#include "GameTextControl.h"

#include "GLItemMixMan.h"
#include "GLItemMan.h"

#include "InnerInterface.h"
#include "GLGaeaClient.h"

#include "Util/RnButton.h"

#include "./UIEditBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int nLIMIT_PRODUCT_CNT = 2;

CProductWindowProduct::CProductWindowProduct ()
	: m_pReqLevelText(NULL)
	, m_pTextProductChance(NULL)
	, m_pProductTimeText(NULL)
	, m_pResulteItemImage(NULL)
	, m_pCostSumText(NULL)
	, m_pResultItemText(NULL)
	, m_pTextResultNum(NULL)
	, m_pTextResultCost(NULL)
	, m_pProductMessageText(NULL)
	, m_pButtonProduction(NULL)
	, m_pTimer(NULL)
	, m_fUpdateTime(0.0f)
	, m_pProductEditBox(NULL)
	, m_nProductCount(1)
	, m_nProductCountLast(1)
	, m_dwProductID(UINT_MAX)
{
	for( int i=0; i<ITEMMIX_ITEMNUM; ++i )
	{
		m_pItemImageArray[i] = NULL;
		m_pMaterialText[i] = NULL;
	}

	m_sItemMix = ITEM_MIX();
}

CProductWindowProduct::~CProductWindowProduct ()
{
}

void CProductWindowProduct::CreateSubControl ()
{
    CBasicLineBoxSmart::CreateSubControl( "PRODUCT_WINDOW_BASE_SMARTLINEBOX_DARKGRAY" );
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );

	
    CString	strItemSlot[LIMIT_COLUMN_CNT] = 
    {
        "PRODUCT_WINDOW_GOODS_ITEMSLOT_0",
        "PRODUCT_WINDOW_GOODS_ITEMSLOT_1",
        "PRODUCT_WINDOW_GOODS_ITEMSLOT_2",
        "PRODUCT_WINDOW_GOODS_ITEMSLOT_3",
        "PRODUCT_WINDOW_GOODS_ITEMSLOT_4",
    };
    CString	strItemSlotText[LIMIT_COLUMN_CNT] = 
    {
        "PRODUCT_WINDOW_GOODS_ITEMSLOT_TEXT_0",
        "PRODUCT_WINDOW_GOODS_ITEMSLOT_TEXT_1",
        "PRODUCT_WINDOW_GOODS_ITEMSLOT_TEXT_2",
        "PRODUCT_WINDOW_GOODS_ITEMSLOT_TEXT_3",
        "PRODUCT_WINDOW_GOODS_ITEMSLOT_TEXT_4",
    };

    { // ��� �ڽ�
        CBasicLineBoxSmart* pLinBoxSmartItemNameBox = new CBasicLineBoxSmart(  );
        pLinBoxSmartItemNameBox->CreateSub( this, "PRODUCT_WINDOW_GOODS_ITEMNAME", UI_FLAG_XSIZE | UI_FLAG_YSIZE);
        pLinBoxSmartItemNameBox->CreateSubControl( "PRODUCT_WINDOW_BASE_SMARTLINEBOX_GRAYLINE" );
        RegisterControl( pLinBoxSmartItemNameBox );

        { 
			//������ �ܰ���;
			CBasicLineBoxSmart* pLinBoxSmartMaterial = new CBasicLineBoxSmart(  );
			pLinBoxSmartMaterial->CreateSub( this, "PRODUCT_WINDOW_GOODS_RESULT_ITEMSLOT", UI_FLAG_XSIZE | UI_FLAG_YSIZE);
			pLinBoxSmartMaterial->CreateSubControl( "PRODUCT_WINDOW_GOODS_ITEMSLOT_BACK" );
			RegisterControl( pLinBoxSmartMaterial );
			

			// ��� ������;
            CItemImage* pItemImage = new CItemImage();
            pItemImage->CreateSub( this, "PRODUCT_WINDOW_GOODS_RESULT_ITEMSLOT", UI_FLAG_DEFAULT , ID_PRODUCT_GOODS_RESULT_ITEMSLOT );
            pItemImage->CreateSubControl();
			//pItemImage->CreateTextBoxDownRight();
            RegisterControl( pItemImage );
            m_pResulteItemImage = pItemImage;
		}
		

		{
			// ��� ������ �ؽ�Ʈ���� �ڽ�;
			CBasicLineBoxSmart* pLinBoxSmartMaterialText = new CBasicLineBoxSmart(  );
			pLinBoxSmartMaterialText->CreateSub( this, "PRODUCT_WINDOW_GOODS_RESULT_ITEMSLOT_TEXT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
			pLinBoxSmartMaterialText->CreateSubControl( "PRODUCT_WINDOW_GOODS_ITEMSLOT_TEXT_BACK" );
			RegisterControl( pLinBoxSmartMaterialText );

			// ��� ������ ���� �ؽ�Ʈ;
			CBasicTextBox* pTextBox = new CBasicTextBox();
			pTextBox->CreateSub(this, "PRODUCT_WINDOW_GOODS_RESULT_ITEMSLOT_TEXT", UI_FLAG_DEFAULT );
			pTextBox->SetFont ( pFont9 );	
			pTextBox->SetTextAlign( UI_FLAG_CENTER_X | UI_FLAG_CENTER_Y  );
			RegisterControl ( pTextBox );
			m_pResultItemText = pTextBox;
		}

        { // �����ð� ����? �ؽ�Ʈ
            CBasicTextBox* pTextBox = new CBasicTextBox();
            pTextBox->CreateSub(this, "PRODUCT_WINDOW_GOODS_PRODUCTTIME_TEXT", UI_FLAG_DEFAULT );
            pTextBox->SetFont ( pFont9 );
            RegisterControl ( pTextBox );
            m_pProductTimeText = pTextBox;
        }

        { // �ʿ买ǰ Ÿ��Ʋ �ؽ�Ʈ
            CBasicTextBox* pTextBox = new CBasicTextBox();
            pTextBox->CreateSub(this, "PRODUCT_WINDOW_GOODS_NEEDMATERIAL_TEXT", UI_FLAG_DEFAULT );
            pTextBox->SetFont ( pFont9 );
            pTextBox->SetTextAlign( UI_FLAG_CENTER_X | UI_FLAG_CENTER_Y  );
            RegisterControl ( pTextBox );

            std::string strNeedMaterial = sc::string::format( "- %s -" , ID2GAMEWORD("PRODUCT_WINDOW_TEXT", 6 ) );
            pTextBox->SetText( strNeedMaterial.c_str(), NS_UITEXTCOLOR::GREENYELLOW );
        }

		//hsshin ���õ� �ý��� �߰�
		{ //�ʿ����۷��� �ؽ�Ʈ
			CBasicTextBox* pTextBox = new CBasicTextBox();
			pTextBox->CreateSub(this, "PRODUCT_WINDOW_GOODS_REQ_LEVEL", UI_FLAG_DEFAULT );
			pTextBox->SetFont ( pFont9 );
			RegisterControl ( pTextBox );
			m_pReqLevelText = pTextBox;
			pTextBox = NULL;
		}

	}

	{ // ����
        for ( int nCol = 0; nCol < LIMIT_COLUMN_CNT ; nCol++ )
        {
            //������ �ܰ���
            CBasicLineBoxSmart* pLinBoxSmartMaterial = new CBasicLineBoxSmart(  );
            pLinBoxSmartMaterial->CreateSub( this, strItemSlot[nCol].GetString(), UI_FLAG_XSIZE | UI_FLAG_YSIZE);
            pLinBoxSmartMaterial->CreateSubControl( "PRODUCT_WINDOW_GOODS_ITEMSLOT_BACK" );
            RegisterControl( pLinBoxSmartMaterial );
            //������ ����
            CItemImage* pItemImage = new CItemImage();
            pItemImage->CreateSub( this, strItemSlot[nCol].GetString(), UI_FLAG_DEFAULT, ID_PRODUCT_GOODS_ITEMSLOT_START+nCol );
            pItemImage->CreateSubControl();
            RegisterControl( pItemImage );
            m_pItemImageArray[nCol] = pItemImage;

            //�������� �ؽ�Ʈ���� �ڽ�
            CBasicLineBoxSmart* pLinBoxSmartMaterialText = new CBasicLineBoxSmart(  );
            pLinBoxSmartMaterialText->CreateSub( this, strItemSlotText[nCol].GetString(), UI_FLAG_XSIZE | UI_FLAG_YSIZE);
            pLinBoxSmartMaterialText->CreateSubControl( "PRODUCT_WINDOW_GOODS_ITEMSLOT_TEXT_BACK" );
            RegisterControl( pLinBoxSmartMaterialText );
            //�������� ���� �ؽ�Ʈ
            CBasicTextBox* pTextBox = new CBasicTextBox();
            pTextBox->CreateSub(this, strItemSlotText[nCol].GetString(), UI_FLAG_DEFAULT, ID_PRODUCT_GOODS_ITEMSLOT_TEXT_START+nCol );
            pTextBox->SetFont ( pFont9 );	
            pTextBox->SetTextAlign( UI_FLAG_CENTER_X | UI_FLAG_CENTER_Y  );
            RegisterControl ( pTextBox );
            m_pMaterialText[nCol] = pTextBox;
        }
    }
	

    { // �ϴ� ���ιڽ�, �ؽ�Ʈ�ڽ�
        CBasicLineBoxSmart* pLinBoxSmartItemNameBox = new CBasicLineBoxSmart(  );
        pLinBoxSmartItemNameBox->CreateSub( this, "PRODUCT_WINDOW_GOODS_MIDDLE", UI_FLAG_XSIZE | UI_FLAG_YSIZE);
        pLinBoxSmartItemNameBox->CreateSubControl( "PRODUCT_WINDOW_BASE_SMARTLINEBOX_GRAYLINE" );
        RegisterControl( pLinBoxSmartItemNameBox );

        //���������Ѱ�? �ƴѰ� �޼���
        CBasicTextBox* pTextBox = new CBasicTextBox();
        pTextBox->CreateSub(this, "PRODUCT_WINDOW_GOODS_MIDDLE", UI_FLAG_DEFAULT );
        pTextBox->SetFont ( pFont9 );	
        pTextBox->SetTextAlign( UI_FLAG_CENTER_X | UI_FLAG_CENTER_Y  );
        RegisterControl ( pTextBox );
        m_pProductMessageText = pTextBox;

    }
	

    { // �� ���� ���
        CBasicLineBoxSmart* pLinBoxSmartItemNameBox = new CBasicLineBoxSmart(  );
        pLinBoxSmartItemNameBox->CreateSub( this, "PRODUCT_WINDOW_GOODS_MIDDLE_RIGHT", UI_FLAG_XSIZE | UI_FLAG_YSIZE);
        pLinBoxSmartItemNameBox->CreateSubControl( "PRODUCT_WINDOW_BASE_SMARTLINEBOX_GRAYLINE" );
        RegisterControl( pLinBoxSmartItemNameBox );

        //�� ���ۺ�� �ؽ�Ʈ
        CBasicTextBox* pTextBox = new CBasicTextBox();
        pTextBox->CreateSub(this, "PRODUCT_WINDOW_GOODS_MIDDLE_RIGHT", UI_FLAG_DEFAULT );
        pTextBox->SetFont ( pFont9 );	
        pTextBox->SetTextAlign( UI_FLAG_LEFT | UI_FLAG_CENTER_Y  );
        RegisterControl ( pTextBox );
        m_pCostSumText = pTextBox;
    }
	
    { // �����ݾ�
        CBasicLineBoxSmart* pLinBoxSmartItemNameBox = new CBasicLineBoxSmart(  );
        pLinBoxSmartItemNameBox->CreateSub( this, "PRODUCT_WINDOW_GOODS_BOTTOM", UI_FLAG_XSIZE | UI_FLAG_YSIZE);
        pLinBoxSmartItemNameBox->CreateSubControl( "PRODUCT_WINDOW_BASE_SMARTLINEBOX_GRAYLINE" );
        RegisterControl( pLinBoxSmartItemNameBox );

        CBasicButton* pButton = new CBasicButton(  );
        pButton->CreateSub( this, "PRODUCT_WINDOW_GOODS_MONEY_BUTTON", UI_FLAG_DEFAULT );
        RegisterControl( pButton );

        CBasicTextBox* pTextBox = new CBasicTextBox();
        pTextBox->CreateSub(this, "PRODUCT_WINDOW_GOODS_MONEY_TEXT" , UI_FLAG_DEFAULT);
        pTextBox->SetFont ( pFont9 );	
        pTextBox->SetTextAlign( UI_FLAG_LEFT | UI_FLAG_CENTER_Y );
        RegisterControl ( pTextBox );
        m_pMoneyText = pTextBox;
    }

	m_pTextProductChance = new CBasicTextBox;
	m_pTextProductChance->CreateSub ( this, "RAN_PRODUCT_WINDOW_GOODS_PRODUCTCHANCE_TEXT" );
	m_pTextProductChance->SetFont ( pFont9 );
	m_pTextProductChance->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextProductChance->SetText( "--" );
	RegisterControl ( m_pTextProductChance );

	CBasicLineBox* pLinBoxItemNameBox = new CBasicLineBox;
    pLinBoxItemNameBox->CreateSub( this, "BASIC_LINE_BOX_EDIT", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
    pLinBoxItemNameBox->CreateBaseBoxYellow( "RAN_PRODUCT_WINDOW_GOODS_EDITBOX_BACK" );
    RegisterControl( pLinBoxItemNameBox );

	m_pTextResultNum = new CBasicTextBox;
	m_pTextResultNum->CreateSub ( this, "RAN_PRODUCT_WINDOW_GOODS_ITEM_NUM_TEXT" );
	m_pTextResultNum->SetFont ( pFont9 );
	m_pTextResultNum->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextResultNum->SetText( ID2GAMEWORD( "RAN_PRODUCT_WINDOW_TEXT", 7 ), NS_UITEXTCOLOR::WHITE );
	RegisterControl ( m_pTextResultNum );

	m_pTextResultCost = new CBasicTextBox;
	m_pTextResultCost->CreateSub ( this, "RAN_PRODUCT_WINDOW_GOODS_ITEM_COST_TEXT" );
	m_pTextResultCost->SetFont ( pFont9 );
	m_pTextResultCost->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	m_pTextResultCost->SetText( "--" );
	RegisterControl ( m_pTextResultCost );

	{   // �����ϱ� ��ư
        CD3DFontPar* pFont10Shadow = DxFontMan::GetInstance().LoadDxFont( _DEFAULT_FONT, 10, _DEFAULT_FONT_SHADOW_FLAG );

        RnButton* pRnButton = NULL;
        RnButton::CreateArg arg;
        //////////////////////////////////////////////////////////////////////////
        arg.defaultTextureKey = "RNBUTTON_DEFAULT_TEXTURE";
        arg.mouseOverTextureKey = "RNBUTTON_OVER_TEXTURE";
        arg.mouseClickTextureKey = "RNBUTTON_CLICK_TEXTURE";
        //////////////////////////////////////////////////////////////////////////
        arg.pFont = pFont10Shadow;
        arg.text = ID2GAMEWORD( "PRODUCT_BUTTON", 0 );
        arg.dwColor = NS_UITEXTCOLOR::WHITE;

        pRnButton = new RnButton();
        pRnButton->CreateSub(this,"PRODUCT_WINDOW_PRODUCTION_BUTTON",UI_FLAG_DEFAULT,PRODUCT_BUTTON_PRODUCTION);
        pRnButton->CreateSubControl(arg);
        RegisterControl(pRnButton);
        m_pProductionButton = pRnButton;
    }

	//m_pButtonProduction = new CBasicTextButton;
	//m_pButtonProduction->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, PRODUCT_BUTTON_PRODUCTION );
	//m_pButtonProduction->CreateBaseButton ( "RAN_PRODUCT_WINDOW_PRODUCTION_BUTTON", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, ID2GAMEWORD("RAN_PRODUCT_BUTTON", 0) );
	//RegisterControl ( m_pButtonProduction );

	m_pTimer = new CBasicProgressBar;
	m_pTimer->CreateSub ( this, "RAN_PRODUCT_PROGRESS" );
	m_pTimer->CreateOverImage ( "RAN_PRODUCT_PROGRESS_OVER" );
	m_pTimer->SetVisibleSingle( FALSE );
	RegisterControl ( m_pTimer );

	CUIEditBox* pEditBox = new CUIEditBox;
	pEditBox->CreateSub( this, "RAN_PRODUCT_WINDOW_GOODS_EDITBOX", UI_FLAG_DEFAULT, PRODUCT_GOODS_EDITBOX_MANAGER );
	pEditBox->CreateCarrat( "MODAL_CARRAT", TRUE, UINT_MAX );
	pEditBox->SetLimitInput( nLIMIT_PRODUCT_CNT );
	pEditBox->SetFont( pFont9 );
	pEditBox->DoUSE_NUMBER( true );
	RegisterControl( pEditBox );
	m_pProductEditBox = pEditBox;	
}

void CProductWindowProduct::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	m_fUpdateTime += fElapsedTime;
	if ( m_fUpdateTime >= PRODUCT_INFO_UPDATE_TIME )
	{
		m_fUpdateTime = 0.0f;
		UpdateInfo();
	}

	/*GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( pCharacter )
	{
		float fPer = 0.0f;
		if ( pCharacter->m_bItemCompoundTask )
			fPer = pCharacter->m_fItemCompoundTaskTimer / pCharacter->m_fItemCompoundTaskTime;

		if ( m_pTimer )	
		{
			m_pTimer->SetVisibleSingle(TRUE);
			m_pTimer->SetPercent( fPer );
		}
	}*/
}


void CProductWindowProduct::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );

	switch ( ControlID )
	{
	case PRODUCT_ITEM_IMAGE_RES:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				SITEMCUSTOM sitemcustom;
				sitemcustom.sNativeID = m_sItemMix.sResultItem.sNID;
				sitemcustom.wTurnNum = m_sItemMix.sResultItem.nNum;

				sitemcustom.cDAMAGE = m_sItemMix.cDAMAGE;
				sitemcustom.cDEFENSE = m_sItemMix.cDEFENSE;
				sitemcustom.cRESIST_FIRE = m_sItemMix.cRESIST_FIRE;
				sitemcustom.cRESIST_ICE = m_sItemMix.cRESIST_ICE;
				sitemcustom.cRESIST_ELEC = m_sItemMix.cRESIST_ELEC;
				sitemcustom.cRESIST_POISON = m_sItemMix.cRESIST_POISON;
				sitemcustom.cRESIST_SPIRIT = m_sItemMix.cRESIST_SPIRIT;
				
				if( sitemcustom.sNativeID == NATIVEID_NULL() ) return;

				SITEM* pitem_data = GLItemMan::GetInstance().GetItem( sitemcustom.sNativeID );
				if ( pitem_data && pitem_data->ISPILE() )	sitemcustom.wTurnNum = pitem_data->GETAPPLYNUM() * m_sItemMix.sResultItem.nNum;

				CInnerInterface::GetInstance().SHOW_ITEM_INFO( sitemcustom, FALSE, FALSE, FALSE, NULL, NULL, NATIVEID_NULL(), FALSE, 0, FALSE, TRUE, m_sItemMix.bGenerateRandomValue );

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

    case ID_PRODUCT_GOODS_ITEMSLOT_START:
    case ID_PRODUCT_GOODS_ITEMSLOT_START + 1:
    case ID_PRODUCT_GOODS_ITEMSLOT_START + 2:
    case ID_PRODUCT_GOODS_ITEMSLOT_START + 3:
    case ID_PRODUCT_GOODS_ITEMSLOT_START + 4:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				int nSelect = ControlID - ID_PRODUCT_GOODS_ITEMSLOT_START;

				if ( nSelect >= ITEMMIX_ITEMNUM )	return;

				SITEMCUSTOM sitemcustom;
				sitemcustom.sNativeID = m_sItemMix.sMeterialItem[nSelect].sNID;
				sitemcustom.wTurnNum = m_sItemMix.sMeterialItem[nSelect].nNum;
				
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

	case PRODUCT_GOODS_EDITBOX_MANAGER:
		{
			if( m_pProductEditBox )
			{
				if( CHECK_MOUSE_IN_LBDOWNLIKE( dwMsg ) )
				{
					m_pProductEditBox->BeginEdit();
				}
				else if( CHECK_LB_DOWN_LIKE( dwMsg ) )
				{
					m_pProductEditBox->EndEdit();

					if( m_nProductCount == 0 )	m_nProductCount = 1;
					else m_nProductCount = _ttoi( m_pProductEditBox->GetEditString() );

					if( m_nProductCount != m_nProductCountLast )
					{
						m_nProductCountLast = m_nProductCount;
						SetData( m_dwProductID );
					}
				}
			}
		}
		break;
	case PRODUCT_BUTTON_PRODUCTION:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
				if ( !pCharacter )	break;

				if( !pCharacter->m_bItemCompoundTask )
					GLGaeaClient::GetInstance().GetCharacter()->ItemCompound( m_sItemMix.dwKey, m_nProductCount );
				else 
					GLGaeaClient::GetInstance().GetCharacter()->ItemCompoundCancel();
			}
		}break;
	};
	
	CString strTemp = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( GLGaeaClient::GetInstance().GetCharacter()->GetInvenMoney(), 3, "," );
    m_pMoneyText->SetText( strTemp.GetString(), NS_UITEXTCOLOR::WHITE );
}

void CProductWindowProduct::ResetData()
{
	m_sItemMix = ITEM_MIX();
	m_nProductCount = 1;
	m_nProductCountLast = 1;
	m_dwProductID = UINT_MAX;

	m_pTimer->SetVisibleSingle(FALSE);

	if ( m_pReqLevelText )			m_pReqLevelText->ClearText();
	if ( m_pTextProductChance )		m_pTextProductChance->ClearText();
	if ( m_pProductTimeText )		m_pProductTimeText->ClearText();
	if ( m_pCostSumText )			m_pCostSumText->ClearText();
	if ( m_pResultItemText )		m_pResultItemText->ClearText();
	if ( m_pTextResultCost )		m_pTextResultCost->ClearText();
	//if ( m_pTextResultNum )			m_pTextResultNum->ClearText();
	if ( m_pProductMessageText )	m_pProductMessageText->ClearText();
	if ( m_pMoneyText )				m_pMoneyText->ClearText();

	if ( m_pResulteItemImage )
	{
		m_pResulteItemImage->ResetItem();
		//m_pResulteItemImage->SetTextDownRight( "", NS_UITEXTCOLOR::WHITE );
	}

	for ( int i=0; i<ITEMMIX_ITEMNUM; ++i )
	{
		if ( m_pItemImageArray[i] )
			m_pItemImageArray[i]->ResetItem();

		if ( m_pMaterialText[i] )
		{
			m_pMaterialText[i]->ClearText();
			//m_pMaterialText[i]->SetText( "0/0", NS_UITEXTCOLOR::WHITE );
		}
	}

	/*if ( m_pButtonProduction )
		m_pButtonProduction->SetVisibleSingle( FALSE );*/

	if ( m_pTimer )
		m_pTimer->SetVisibleSingle( FALSE );
}

void CProductWindowProduct::SetData( DWORD dwID )
{
	const ITEM_MIX* pitem_mix = GLItemMixMan::GetInstance().GetProduct( dwID );
	if ( !pitem_mix )	return;
	if ( m_dwProductID != dwID )	m_dwProductID = dwID;

	m_sItemMix = *pitem_mix;

	CString strTemp;

	if ( m_pReqLevelText )
	{
		m_pReqLevelText->ClearText();

		if ( m_sItemMix.wLevelReq )
		{
			int nIndex = m_pReqLevelText->AddText( ID2GAMEWORD( "RAN_PRODUCT_WINDOW_TEXT", 11 ), NS_UITEXTCOLOR::WHITE );
			strTemp.Format( " %d", m_sItemMix.wLevelReq );
			m_pReqLevelText->AddString( nIndex, strTemp.GetString(), NS_UITEXTCOLOR::ORANGE );
		}
	}

	if ( m_pTextProductChance )
	{
		m_pTextProductChance->ClearText();

		int nIndex = m_pTextProductChance->AddText( ID2GAMEWORD( "RAN_PRODUCT_WINDOW_TEXT", 15 ), NS_UITEXTCOLOR::WHITE );
		strTemp.Format( " %d %%", m_sItemMix.dwRate );
		m_pTextProductChance->AddString( nIndex, strTemp.GetString(), NS_UITEXTCOLOR::ORANGE );
	}

	if ( m_pProductTimeText )
	{
		m_pProductTimeText->ClearText();
		int nIndex = m_pProductTimeText->AddText( ID2GAMEWORD( "RAN_PRODUCT_WINDOW_TEXT", 5 ), NS_UITEXTCOLOR::WHITE );
		strTemp.Format( " %g Second", m_sItemMix.fTime );
		m_pProductTimeText->AddString( nIndex, strTemp.GetString(), NS_UITEXTCOLOR::ORANGE );
	}

	if ( m_pResulteItemImage )
	{
		m_pResulteItemImage->ResetItem();
		//m_pResulteItemImage->SetTextDownRight( "", NS_UITEXTCOLOR::WHITE );

		SITEM* pitem_data_result = GLItemMan::GetInstance().GetItem( m_sItemMix.sResultItem.sNID );
		if ( pitem_data_result )
		{
			m_pResulteItemImage->SetItem( m_sItemMix.sResultItem.sNID );

			/*if ( pitem_data_result->ISPILE() )
			{
				strTemp.Format( "%d", pitem_data_result->GETAPPLYNUM() * m_sItemMix.sResultItem.nNum );
				m_pResulteItemImage->SetTextDownRight( strTemp.GetString(), NS_UITEXTCOLOR::WHITE );
			}*/
		}
	}

	for ( int i=0; i<ITEMMIX_ITEMNUM; ++i )
	{
		if ( m_pItemImageArray[i] )
		{
			m_pItemImageArray[i]->ResetItem();

			SITEM* pitem_data_require = GLItemMan::GetInstance().GetItem( m_sItemMix.sMeterialItem[i].sNID );
			if ( pitem_data_require )
			{
				m_pItemImageArray[i]->SetItem( m_sItemMix.sMeterialItem[i].sNID );
			}
		}	
	}

	
	if ( m_pResultItemText )
	{
		m_pResultItemText->ClearText();
		//int nIndex = m_pResultItemText->AddText( ID2GAMEWORD( "RAN_PRODUCT_WINDOW_TEXT", 7 ), NS_UITEXTCOLOR::WHITE );
		strTemp.Format( "%d", m_sItemMix.sResultItem.nNum );
		m_pResultItemText->SetText( strTemp.GetString(), NS_UITEXTCOLOR::WHITE );
	}
	

	if ( m_pTextResultCost )
	{
		m_pTextResultCost->ClearText();
		int nIndex = m_pTextResultCost->AddText( ID2GAMEWORD( "RAN_PRODUCT_WINDOW_TEXT", 8 ), NS_UITEXTCOLOR::WHITE );
		CString strTempPrice = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( (LONGLONG)m_sItemMix.dwPrice, 3, "," );
		strTemp.Format( " : %s", ( strTempPrice ) );
		m_pTextResultCost->AddString( nIndex, strTemp.GetString(), NS_UITEXTCOLOR::ORANGE );
	}

	if ( m_pProductEditBox )
	{
		m_pProductEditBox->BeginEdit();
		CString strCount;
		strCount.Format( _T( "%d" ), m_nProductCount );
		m_pProductEditBox->SetEditString( strCount.GetString() );
	}
}

void CProductWindowProduct::UpdateInfo()
{
	GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
	if ( !pCharacter )	return;

	CString strTemp;

	BOOL bitem_cost_fail = FALSE;
	BOOL bitem_num_fail = FALSE;
	BOOL bchar_level_fail = FALSE;

	if( pCharacter->m_lnMoney < ( m_sItemMix.dwPrice * m_nProductCount ) )
		bitem_cost_fail = TRUE;

	if ( pCharacter->m_wLevel < m_sItemMix.wLevelReq )
		bchar_level_fail = TRUE;

	if ( m_pMoneyText )
	{
		m_pMoneyText->ClearText();
		strTemp = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( pCharacter->m_lnMoney, 3, "," );
		m_pMoneyText->SetOneLineText( strTemp, NS_UITEXTCOLOR::WHITE );
	}

	GLInventory& ref_Inventory = pCharacter->m_cInventory;

	for ( int i=0; i<ITEMMIX_ITEMNUM; ++i )
	{	
		DWORD dwitem_num = 0;
		BOOL bitem_num_complete = FALSE;

		SITEM* pitem_data_require = GLItemMan::GetInstance().GetItem( m_sItemMix.sMeterialItem[i].sNID );
		if ( pitem_data_require )
		{
			if ( pitem_data_require->ISPILE() ){
				dwitem_num = ref_Inventory.CountTurnItem( pitem_data_require->sBasicOp.sNativeID );
			}else{
				SINVENITEM* pinven_item = ref_Inventory.FindItem( pitem_data_require->sBasicOp.sNativeID );
				if ( pinven_item ) dwitem_num = 1;
			}
		}

		if ( dwitem_num >= DWORD( m_sItemMix.sMeterialItem[i].nNum * m_nProductCount) )
			bitem_num_complete = TRUE;
		else if( dwitem_num >= m_sItemMix.sMeterialItem[i].nNum ) 
		{
			m_nProductCount = int ( dwitem_num/m_sItemMix.sMeterialItem[i].nNum );
			if( m_nProductCountLast < m_nProductCount )
			{
				m_nProductCount = m_nProductCountLast;
			}
			else
			{
				m_nProductCountLast = m_nProductCount;
			}

			SetData( m_dwProductID );
			bitem_num_complete = TRUE;			
		}

		if ( !bitem_num_complete )
			bitem_num_fail = TRUE;

		if ( m_pMaterialText[i] )
		{
			m_pMaterialText[i]->ClearText();
			strTemp.Format( "%d/%d", dwitem_num, ( m_sItemMix.sMeterialItem[i].nNum * m_nProductCount ) );
			m_pMaterialText[i]->AddText( strTemp.GetString(), bitem_num_complete? NS_UITEXTCOLOR::WHITE:NS_UITEXTCOLOR::RED );
		}
	}

	if ( m_pProductMessageText )
	{
		m_pProductMessageText->ClearText();

		if ( m_sItemMix.dwKey != UINT_MAX )
		{
			if ( bitem_cost_fail )
				m_pProductMessageText->AddText( ID2GAMEWORD( "RAN_PRODUCT_GOODS_MESSAGE", 4 ), NS_UITEXTCOLOR::ORANGE );
			else if ( bitem_num_fail )
				m_pProductMessageText->AddText( ID2GAMEWORD( "RAN_PRODUCT_GOODS_MESSAGE", 0 ), NS_UITEXTCOLOR::ORANGE );
			else if ( bchar_level_fail )
				m_pProductMessageText->AddText( ID2GAMEWORD( "RAN_PRODUCT_GOODS_MESSAGE", 5 ), NS_UITEXTCOLOR::ORANGE );
			else
				m_pProductMessageText->AddText( ID2GAMEWORD( "RAN_PRODUCT_GOODS_MESSAGE", 2 ), NS_UITEXTCOLOR::ORANGE );
		}
	}

	if ( pCharacter )
	{
		float fPer = 0.0f;
		if ( pCharacter->m_bItemCompoundTask )
		{
			fPer = pCharacter->m_fItemCompoundTaskTimer / pCharacter->m_fItemCompoundTaskTime;

			if ( m_pTimer )	
			{
				m_pTimer->SetVisibleSingle(TRUE);
				m_pTimer->SetPercent( fPer );
			}
		}
		else
		{
			m_pTimer->SetVisibleSingle(FALSE);
		}
	}

	if ( m_pProductionButton )
	{
		if ( pCharacter->m_bItemCompoundTask )
			m_pProductionButton->SetOneLineText( ID2GAMEWORD("RAN_PRODUCT_BUTTON", 1), NS_UITEXTCOLOR::WHITE );
		else
			m_pProductionButton->SetOneLineText( ID2GAMEWORD("RAN_PRODUCT_BUTTON", 0), NS_UITEXTCOLOR::WHITE );
	}
}
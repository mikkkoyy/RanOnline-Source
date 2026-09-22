/*dmk14 reform*/
#include "StdAfx.h"
#include "ItemRebuild.h"

#include "D3DFont.h"
#include "ItemImage.h"
#include "ItemMove.h"
#include "BasicTextButton.h"
#include "UITextControl.h"
#include "GameTextControl.h"
#include "InnerInterface.h"
#include "GLItemMan.h"
#include "GLGaeaClient.h"

#include "BasicLineBoxEx.h"
#include "UIInfoloader.h"
#include "BasicTextBox.h"
#include "ItemRebuildOption.h"
#include "GameTextControl.h"

#include "./BasicTextBox.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "../Lib_ClientUI/Interface/ModalWindow.h"
#include "../Lib_ClientUI/Interface/ModalCallerID.h"

#include "../Lib_Client/G-Logic/GLogicData.h"

#include "Util/RnButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//--------------------------------------------------------------------
// ItemRebuild
//--------------------------------------------------------------------
CItemRebuild::CItemRebuild()	// ITEMREBUILD_MARK
	: m_pItemImage( NULL )
	, m_pItemSealImage( NULL )
	, m_pMoneyTextBox( NULL )
	, m_pOkButton(NULL)
	, m_pPrevButton(NULL)
	, m_pOkBlankButton(NULL)
	, m_pPrevBlankButton(NULL)
	, m_pHelpTextBox(NULL)
	, nNumSelOpt( 0 )
	, m_wMaxRemodel( 0 )
	, sRebuildBack( NATIVEID_NULL() )
	, sSealedBack( NATIVEID_NULL() ) 
	, m_bSortOpt( false )
{
	for( int i=0;i<SRANDOM_GEN::MAX_OPT;i++)	
	{
		m_emLockOpt[i]=EMR_OPT_NULL;
		m_pOptionBefore[i]=NULL;
	}
	memset ( m_pOption, 0, sizeof ( CItemRebuildOption* ) * SRANDOM_GEN::MAX_OPT );
}

CItemRebuild::~CItemRebuild()
{
}

CItemRebuildOption* CItemRebuild::CreateCheckBox ( CString strKeyword, UIGUID ControlID )
{
	CItemRebuildOption* pCheck = new CItemRebuildOption;
	pCheck->CreateSub ( this, strKeyword.GetString (), UI_FLAG_XSIZE | UI_FLAG_YSIZE, ControlID );
	pCheck->CreateSubControl();
	pCheck->SetVisibleSingle( FALSE );
	RegisterControl ( pCheck );

	return pCheck;
}

VOID CItemRebuild::CreateSubControl()
{
	CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont( _DEFAULT_FONT, 9 );
	CD3DFontPar* pFont12 = DxFontMan::GetInstance().LoadDxFont( _DEFAULT_FONT, 15, _DEFAULT_FONT_SHADOW_FLAG  );
	CD3DFontPar* pFontBefore = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );
	
	{	// 텍스트 박스 배경
		CBasicLineBox* pBasicLinBox = new CBasicLineBox;
		pBasicLinBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLinBox->CreateBaseBoxQuestList ( "ITEM_REBUILD_RENEWAL_BASE_REGION" );
		RegisterControl ( pBasicLinBox );		
	}


	//linebox
	{
		CBasicLineBoxEx* pBasicLineBox = new CBasicLineBoxEx;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxEditBox ( "ITEM_REBUILD_RENEWAL_HELP_REGION" );
		RegisterControl ( pBasicLineBox );

		pBasicLineBox = new CBasicLineBoxEx;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxEditBox ( "ITEM_REBUILD_RENEWAL_BASIC_REGION_BOX_CURRENT" );
		RegisterControl ( pBasicLineBox );

		pBasicLineBox = new CBasicLineBoxEx;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxEditBox ( "ITEM_REBUILD_RENEWAL_MONEY_BACK" );
		RegisterControl ( pBasicLineBox );
		
		pBasicLineBox = new CBasicLineBoxEx;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxEditBox ( "ITEM_REBUILD_RENEWAL_BASIC_REGION_BOX_BEFORE" );
		RegisterControl ( pBasicLineBox );

		pBasicLineBox = new CBasicLineBoxEx;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxEditBox ( "ITEM_REBUILD_RENEWAL_OPTION_REGION_BOX_CURRENT" );
		RegisterControl ( pBasicLineBox );

		pBasicLineBox = new CBasicLineBoxEx;
		pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_OUTER", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
		pBasicLineBox->CreateBaseBoxEditBox ( "ITEM_REBUILD_RENEWAL_OPTION_REGION_BOX_BEFORE" );
		RegisterControl ( pBasicLineBox );
	}

	{
		m_pItemImage = new CItemImage;
		m_pItemImage->CreateSub( this, "ITEM_REBUILD_ITEM_SLOT", UI_FLAG_DEFAULT, ITEM_REBUILD_ITEM_SLOT );
		m_pItemImage->CreateSubControl();
		RegisterControl( m_pItemImage );

		m_pItemSealImage = new CItemImage;
		m_pItemSealImage->CreateSub( this, "ITEM_REBUILD_ITEM_SEAL_SLOT", UI_FLAG_DEFAULT, ITEM_REBUILD_ITEM_SEAL_SLOT );
		m_pItemSealImage->CreateSubControl();
		RegisterControl( m_pItemSealImage );
	}
	{
	}

	// 개조비용 : 0
	{
		m_pHelpTextBox = CreateStaticControl( "ITEM_REBUILD_HELP_TEXT_BOX", pFont, NS_UITEXTCOLOR::DEFAULT, TEXT_ALIGN_LEFT );

		CBasicTextBox* pBasicText = new CBasicTextBox;
		pBasicText = CreateStaticControl( "ITEM_REBUILD_CURRENT_TEXT_STATIC", pFont12, NS_UITEXTCOLOR::DEFAULT, TEXT_ALIGN_CENTER_X );
		pBasicText->AddText( ID2GAMEWORD("REBUILD_ITEMTAG_APPLYRESULT",0), NS_UITEXTCOLOR::DEFAULT );

		pBasicText = new CBasicTextBox;
		pBasicText = CreateStaticControl( "ITEM_REBUILD_BEFORE_TEXT_STATIC", pFont12, NS_UITEXTCOLOR::DEFAULT, TEXT_ALIGN_CENTER_X );
		pBasicText->AddText( ID2GAMEWORD("REBUILD_ITEMTAG_APPLYRESULT",1) , NS_UITEXTCOLOR::DEFAULT );
	}
	{
		m_pBasicOption = new CItemRebuildOption;
		m_pBasicOption->CreateSub(this, "ITEM_REBUILD_RENEWAL_OPTION_REGION", UI_FLAG_DEFAULT);
		m_pBasicOption->CreateSubControl();
		m_pBasicOption->SetVisibleSingle( FALSE );	
		RegisterControl( m_pBasicOption );

		m_pBasicOptionBefore = new CItemRebuildOption;
		m_pBasicOptionBefore->CreateSub(this, "ITEM_REBUILD_RENEWAL_OPTION_BEFORE_REGION", UI_FLAG_DEFAULT);
		m_pBasicOptionBefore->CreateSubControl();
		m_pBasicOptionBefore->SetVisibleSingle( FALSE );	
		RegisterControl( m_pBasicOptionBefore );
	}

	// 내야되는돈?
	{
		//CUIControl* pControl = CreateControl( "ITEM_REBUILD_MONEY_BACK" );
		//pControl->SetAlignFlag( UI_FLAG_BOTTOM );

		CUIControl* pControl = CreateControl( "ITEM_REBUILD_MONEY_IMAGE" );
		pControl->SetAlignFlag( UI_FLAG_BOTTOM );

		CD3DFontPar* pFont = DxFontMan::GetInstance().LoadDxFont( _DEFAULT_FONT, 8, D3DFONT_SHADOW | D3DFONT_ASCII );
		m_pMoneyTextBox = CreateStaticControl( "ITEM_REBUILD_MONEY_TEXT", pFont, NS_UITEXTCOLOR::DEFAULT, TEXT_ALIGN_LEFT );
		m_pMoneyTextBox->SetAlignFlag( UI_FLAG_BOTTOM );

		m_pOptionBefore[0] = CreateStaticControl( "ITEM_REBUILD_RENEWAL_OPTION_BEFORE_0", pFontBefore, NS_UITEXTCOLOR::dwAddAbility, TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP  );
		m_pOptionBefore[1] = CreateStaticControl( "ITEM_REBUILD_RENEWAL_OPTION_BEFORE_1", pFontBefore, NS_UITEXTCOLOR::dwAddAbility, TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP  );
		m_pOptionBefore[2] = CreateStaticControl( "ITEM_REBUILD_RENEWAL_OPTION_BEFORE_2", pFontBefore, NS_UITEXTCOLOR::dwAddAbility, TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP  );
		m_pOptionBefore[3] = CreateStaticControl( "ITEM_REBUILD_RENEWAL_OPTION_BEFORE_3", pFontBefore, NS_UITEXTCOLOR::dwAddAbility, TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP  );
		m_pOptionBefore[4] = CreateStaticControl( "ITEM_REBUILD_RENEWAL_OPTION_BEFORE_4", pFontBefore, NS_UITEXTCOLOR::dwAddAbility, TEXT_ALIGN_LEFT );
		
		m_pOptionBefore[0]->ClearText();
		m_pOptionBefore[1]->ClearText();
		m_pOptionBefore[2]->ClearText();
		m_pOptionBefore[3]->ClearText();
		m_pOptionBefore[4]->ClearText();
	}

	// 승인, 거절
	{
		RnButton::CreateArg argblankapply;
		argblankapply.defaultTextureKey = "RNBUTTON_BLANK_DEFAULT_TEXTURE";
		argblankapply.pFont = pFont;
		argblankapply.text = ID2GAMEINTEXT( "ITEM_REBUILD_OK" );
		argblankapply.dwColor = NS_UITEXTCOLOR::WHITE;

		m_pOkBlankButton = new RnButton();
		m_pOkBlankButton->CreateSub(this,"ITEM_REBUILD_OK_BUTTON",UI_FLAG_DEFAULT);
		m_pOkBlankButton->CreateSubControl(argblankapply);
		RegisterControl(m_pOkBlankButton);

		RnButton::CreateArg argblankreset;
		argblankreset.defaultTextureKey = "RNBUTTON_BLANK_DEFAULT_TEXTURE";
		argblankreset.pFont = pFont;
		argblankreset.text = ID2GAMEINTEXT( "ITEM_REBUILD_PREV" );
		argblankreset.dwColor = NS_UITEXTCOLOR::WHITE;

		m_pPrevBlankButton = new RnButton();
		m_pPrevBlankButton->CreateSub(this,"ITEM_REBUILD_PREV_BUTTON",UI_FLAG_DEFAULT);
		m_pPrevBlankButton->CreateSubControl(argblankreset);
		RegisterControl(m_pPrevBlankButton);

		RnButton::CreateArg argapply;
		argapply.defaultTextureKey = "RNBUTTON_DEFAULT_TEXTURE";
		argapply.mouseOverTextureKey = "RNBUTTON_OVER_TEXTURE";
		argapply.mouseClickTextureKey = "RNBUTTON_CLICK_TEXTURE";
		argapply.pFont = pFont;
		argapply.text = ID2GAMEINTEXT( "ITEM_REBUILD_OK" );
		argapply.dwColor = NS_UITEXTCOLOR::WHITE;

		m_pOkButton = new RnButton();
		m_pOkButton->CreateSub(this,"ITEM_REBUILD_OK_BUTTON",UI_FLAG_DEFAULT,ITEM_REBUILD_OK_BUTTON);
		m_pOkButton->CreateSubControl(argapply);
		m_pOkButton->SetVisibleSingle( FALSE );
		RegisterControl(m_pOkButton);

		RnButton::CreateArg argreset;
		argreset.defaultTextureKey = "RNBUTTON_DEFAULT_TEXTURE";
		argreset.mouseOverTextureKey = "RNBUTTON_OVER_TEXTURE";
		argreset.mouseClickTextureKey = "RNBUTTON_CLICK_TEXTURE";
		argreset.pFont = pFont;
		argreset.text = ID2GAMEINTEXT( "ITEM_REBUILD_PREV" );
		argreset.dwColor = NS_UITEXTCOLOR::WHITE;

		m_pPrevButton = new RnButton();
		m_pPrevButton->CreateSub(this,"ITEM_REBUILD_PREV_BUTTON",UI_FLAG_DEFAULT,ITEM_REBUILD_PREV_BUTTON);
		m_pPrevButton->CreateSubControl(argreset);
		m_pPrevButton->SetVisibleSingle( FALSE );
		RegisterControl(m_pPrevButton);

		RnButton::CreateArg argcancel;
		argcancel.defaultTextureKey = "RNBUTTON_DEFAULT_TEXTURE";
		argcancel.mouseOverTextureKey = "RNBUTTON_OVER_TEXTURE";
		argcancel.mouseClickTextureKey = "RNBUTTON_CLICK_TEXTURE";
		argcancel.pFont = pFont;
		argcancel.text = ID2GAMEINTEXT( "ITEM_REBUILD_CANCEL" );
		argcancel.dwColor = NS_UITEXTCOLOR::WHITE;

		m_pCancelButton = new RnButton();
		m_pCancelButton->CreateSub(this,"ITEM_REBUILD_CANCEL_BUTTON",UI_FLAG_DEFAULT,ITEM_REBUILD_CANCEL_BUTTON);
		m_pCancelButton->CreateSubControl(argcancel);
		//m_pCancelButton->SetShortcutKey( DIK_ESCAPE );
		RegisterControl(m_pCancelButton);


		CString strKeyword[ SRANDOM_GEN::MAX_OPT ] =
		{
			"ITEM_REBUILD_RENEWAL_OPTION_CURRENT_0",
			"ITEM_REBUILD_RENEWAL_OPTION_CURRENT_1",
			"ITEM_REBUILD_RENEWAL_OPTION_CURRENT_2",
			"ITEM_REBUILD_RENEWAL_OPTION_CURRENT_3"
		};

		for( int i=0;i<SRANDOM_GEN::MAX_OPT;++i )
		{
			m_pOption[i] = CreateCheckBox(  strKeyword[i].GetString(), ITEM_REBUILD_ITEM_OPTION00 + i );
		}
	}
}

VOID CItemRebuild::Update( INT x, INT y, BYTE LB, BYTE MB, BYTE RB, INT nScroll, FLOAT fElapsedTime, BOOL bFirstControl )
{
	CUIWindowEx::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	GLCHARLOGIC& ref_CharData = GLGaeaClient::GetInstance().GetCharacterLogic();
	
	if( m_pHelpTextBox ) 
	{
		m_pHelpTextBox->ClearText();
		m_pHelpTextBox->AddText( ID2GAMEINTEXT( "ITEM_REBUILD_USE_REBUILD" ), NS_UITEXTCOLOR::DEFAULT );
	}

	SITEMCUSTOM sItemCustom = GLGaeaClient::GetInstance().GetCharacter()->GET_REBUILD_ITEM();
	if( m_pItemImage )
	{
		if( sItemCustom.sNativeID == NATIVEID_NULL() )
		{
			m_pItemImage->ResetItem();

			Init( true );
			ResetPrevDATA();
		}
		else
		{
			if( sRebuildBack != sItemCustom.sNativeID )
			{
				sRebuildBack = sItemCustom.sNativeID;
				Init( true );
				ResetPrevDATA();
			}

			SITEM* pItem = GLItemMan::GetInstance().GetItem( sItemCustom.sNativeID );
			if( pItem )
			{
				m_pItemImage->SetItem( sItemCustom );
				m_pBasicOption->SetItemInformation(sItemCustom);
				m_pBasicOption->SetVisibleSingle ( TRUE );

				if( m_pHelpTextBox ) 
				{
					m_pHelpTextBox->ClearText();
					m_pHelpTextBox->AddText( ID2GAMEINTEXT( "ITEM_REBUILD_USE_ITEM" ), NS_UITEXTCOLOR::DEFAULT );
				}
			}
			else 
			{
				m_pItemImage->ResetItem();
				//m_pBasicOption->SetVisibleSingle ( FALSE );
			}

			int nIndex=0;
			for( int i=0;i<SRANDOM_GEN::MAX_OPT;i++)
			{
				m_pOption[i]->ResetData();

				if ( m_pOption[nIndex]->SetData( sItemCustom, i) ) 
				{
					nIndex++;
				}
			}

			for( int i=0;i<SRANDOM_GEN::MAX_OPT;i++)
			{
				if( m_pOption[i]->IsRandOptNull() )	
					m_pOption[i]->SetVisibleSingle( FALSE );
				else 
					m_pOption[i]->SetVisibleSingle( TRUE );
			}

			if( nNumSelOpt == m_wMaxRemodel )
			{
				for( int i=0;i<SRANDOM_GEN::MAX_OPT;i++)
				{
					if( !m_pOption[ i ]->IsCheck() ) m_pOption[ i ]->SetVisibleCheckBox( FALSE );
					if( m_pOption[ i ]->GetOpt() != m_emLockOpt[ i ] && m_emLockOpt[ i ] != EMR_OPT_NULL )	m_bSortOpt = true;
				}
			}
			else
			{
				for( int i=0;i<SRANDOM_GEN::MAX_OPT;i++)
				{
					m_pOption[ i ]->SetVisibleCheckBox( TRUE );
				}
			}

			if( m_bSortOpt )
			{
				EMRANDOM_OPT		emLockOpt[ SRANDOM_GEN::MAX_OPT ];
				for( int i=0;i<SRANDOM_GEN::MAX_OPT;i++)	
				{
					emLockOpt[ i ] = EMR_OPT_NULL;
					if( m_pOption[ i ]->GetOpt() == m_emLockOpt[ i ] )			continue;
					for( int j=0;j<SRANDOM_GEN::MAX_OPT;j++)	
					{
						if( m_emLockOpt[ j ] == EMR_OPT_NULL )					continue;
						if( m_pOption[ i ]->GetOpt() == m_emLockOpt[ j ] )
						{
							emLockOpt[ i ] = m_emLockOpt[ j ];
							m_pOption[ i ]->SetFlip( true );
							m_pOption[ i ]->SetVisibleCheckBox( true );
							m_pOption[ j ]->SetFlip( false );
						}
					}
					m_emLockOpt[ i ] = emLockOpt[ i ];
				}
				m_bSortOpt = false;
			}
		}
	}

	SITEMCUSTOM sItemCustomSealed = GLGaeaClient::GetInstance().GetCharacter()->GET_SEALED_ITEM();
	if( m_pItemSealImage )
	{
		if( sItemCustomSealed.sNativeID == NATIVEID_NULL() )
		{
			m_pItemSealImage->ResetItem();
			Init( false );
		}
		else
		{
			if( sSealedBack != sItemCustomSealed.sNativeID )
			{
				sSealedBack = sItemCustomSealed.sNativeID;
				Init( true );
				ResetPrevDATA();
			}

			SITEM* pItem = GLItemMan::GetInstance().GetItem( sItemCustomSealed.sNativeID );
			if( pItem )
			{
				m_wMaxRemodel = pItem->sSuitOp.wReModelNum;
				m_pItemSealImage->SetItem( sItemCustomSealed );

				if( m_pHelpTextBox ) 
				{
					CString strFormat;
					strFormat.Format( ID2GAMEINTEXT( "ITEM_REBUILD_USE_ONLY_STAMP_CARD" ), pItem->sSuitOp.wReModelNum );
					m_pHelpTextBox->ClearText();
					m_pHelpTextBox->AddText( strFormat, NS_UITEXTCOLOR::DEFAULT );
				}
			}
			else
				m_pItemSealImage->ResetItem();
		}
	}

	if( ref_CharData.m_sItemPrevCopy.sNativeID != SNATIVEID(false) )
	{
		ResetPrevDATA();
		for( int i=0;i<SRANDOM_GEN::MAX_OPT;i++)
		{
			SetBeforeData( ref_CharData.m_sItemPrevCopy, i );
		}
		m_pBasicOptionBefore->SetItemInformation(ref_CharData.m_sItemPrevCopy);
		m_pBasicOptionBefore->SetVisibleSingle ( TRUE );

		m_pPrevButton->SetVisibleSingle( TRUE );
	}
	else
	{
		m_pPrevButton->SetVisibleSingle( FALSE );
		m_pBasicOptionBefore->SetVisibleSingle ( FALSE );
	}
	
	LONGLONG i64RebuildCost = GLGaeaClient::GetInstance().GetCharacter()->GetRebuildInput();

	if( i64RebuildCost > 0 || sItemCustom.sNativeID != NATIVEID_NULL() ) 
		m_pOkButton->SetVisibleSingle( TRUE );
	else
		m_pOkButton->SetVisibleSingle( FALSE );

	if( m_pMoneyTextBox )
	{
		CString strTemp = NS_UITEXTCONTROL::MAKE_MONEY_FORMAT( i64RebuildCost, 3, "," );
		m_pMoneyTextBox->SetOneLineText( strTemp );
	}
}

VOID CItemRebuild::TranslateUIMessage( UIGUID ControlID, DWORD dwMsg )
{
	switch( ControlID )
	{
	case ITEM_REBUILD_ITEM_SLOT:
		if( CHECK_MOUSE_IN( dwMsg ) )
		{
			CItemMove* pItemMove = CInnerInterface::GetInstance().GetItemMove();
			if( !pItemMove )
			{
				GASSERT( 0 && "CItemRebuild::TranslateUIMessage, pItemMove == NULL" );
				break;
			}

			SNATIVEID sItem = pItemMove->GetItem();
			if( sItem != NATIVEID_NULL() )
			{
				const UIRECT& rcSlotPos = m_pItemImage->GetGlobalPos();
				pItemMove->SetGlobalPos( rcSlotPos );
				CInnerInterface::GetInstance().SetSnapItem();
			}

			if( dwMsg & UIMSG_LB_UP )
				GLGaeaClient::GetInstance().GetCharacter()->ReqRebuildMoveItem();

			SITEMCUSTOM sItemCustom = GLGaeaClient::GetInstance().GetCharacter()->GET_REBUILD_ITEM();
			if( sItemCustom.sNativeID != NATIVEID_NULL() )
			{
				CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW( sItemCustom, FALSE, FALSE, FALSE, FALSE, FALSE, USHRT_MAX, USHRT_MAX );
			}
			else
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEINTEXT( "ITEM_REBUILD_ITEM_SLOT_TOOLTIP" ) , NS_UITEXTCOLOR::WHITE );
		}
		break;

	case ITEM_REBUILD_ITEM_SEAL_SLOT:
		if( CHECK_MOUSE_IN( dwMsg ) )
		{
			CItemMove* pItemMove = CInnerInterface::GetInstance().GetItemMove();
			if( !pItemMove )
			{
				GASSERT( 0 && "CItemRebuild::TranslateUIMessage, pItemMove == NULL" );
				break;
			}

			SNATIVEID sItem = pItemMove->GetItem();
			if( sItem != NATIVEID_NULL() )
			{
				const UIRECT& rcSlotPos = m_pItemSealImage->GetGlobalPos();
				pItemMove->SetGlobalPos( rcSlotPos );
				CInnerInterface::GetInstance().SetSnapItem();
				m_pBasicOptionBefore->SetVisibleSingle(FALSE);
			}

			if( dwMsg & UIMSG_LB_UP )
				GLGaeaClient::GetInstance().GetCharacter()->ReqRebuildMoveItemSealed();

			SITEMCUSTOM sItemCustom = GLGaeaClient::GetInstance().GetCharacter()->GET_SEALED_ITEM();
			if( sItemCustom.sNativeID != NATIVEID_NULL() )
				CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW( sItemCustom, FALSE, FALSE, FALSE,FALSE, FALSE, 0, 0 );
			else
				CInnerInterface::GetInstance().SHOW_COMMON_LINEINFO( ID2GAMEINTEXT( "ITEM_REBUILD_STAMP_SLOT_TOOLTIP" ) , NS_UITEXTCOLOR::WHITE );
		}
		break;
	case ITEM_REBUILD_OK_BUTTON:
		if( CHECK_KEYFOCUSED( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
		{
			DoModal( ID2GAMEINTEXT("ITEM_REBUILD_OKQUESTION"), MODAL_QUESTION, YESNO, MODAL_ITEMREBUILD_OKQUESTION );
			SetMessageEx( dwMsg &= ~UIMSG_KEY_FOCUSED ); 
		}
		break;
	case ITEM_REBUILD_PREV_BUTTON:
		if( CHECK_KEYFOCUSED( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
		{
			DoModal( ID2GAMEINTEXT("ITEM_REBUILD_PREVQUESTION"), MODAL_QUESTION, YESNO, MODAL_ITEMREBUILD_PREVQUESTION );
			SetMessageEx( dwMsg &= ~UIMSG_KEY_FOCUSED ); 
		}
		break;
	case ITEM_REBUILD_CANCEL_BUTTON:
		{
			if( CHECK_KEYFOCUSED( dwMsg ) || CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
				CInnerInterface::GetInstance().CloseItemRebuildWindow();
		}
		break;
	case ITEM_REBUILD_ITEM_OPTION00:
	case ITEM_REBUILD_ITEM_OPTION01:
	case ITEM_REBUILD_ITEM_OPTION02:
	case ITEM_REBUILD_ITEM_OPTION03:
		{
			if( CHECK_MOUSE_IN( dwMsg ) )
			{
				int nIndex = ControlID - ITEM_REBUILD_ITEM_OPTION00;
				if( dwMsg & UIMSG_LB_UP )
				{
					if( m_pOption[ nIndex ]->IsRandOptNull() )								break;
					if( !m_pOption[ nIndex ]->IsCheck() && nNumSelOpt >= m_wMaxRemodel )	break; 

					m_pOption[ nIndex ]->IsClicked();

					if( m_pOption[ nIndex ]->IsCheck() ) 
					{
						m_emLockOpt[ nIndex ] = m_pOption[ nIndex ]->GetOpt();
						nNumSelOpt++;
					}
					else 
					{
						m_emLockOpt[ nIndex ] = EMR_OPT_NULL;
						nNumSelOpt--;
					}
				}
			}
		}
		break;
	case ET_CONTROL_BUTTON:
		if( CHECK_MOUSEIN_LBUPLIKE( dwMsg ) )
			CInnerInterface::GetInstance().CloseItemRebuildWindow();
		break;
	}

	CUIWindowEx::TranslateUIMessage( ControlID, dwMsg );
}

void CItemRebuild::Init( bool bClear )
{
	nNumSelOpt = 0;
	m_wMaxRemodel = 0;

	for( int i=0;i<SRANDOM_GEN::MAX_OPT;i++)	
	{
		if( bClear ) 
		{
			m_pBasicOption->SetVisibleSingle(FALSE);
			m_pOption[ i ]->SetVisibleCheckBox( FALSE );
			m_pOptionBefore[ i ]->ClearText();
			m_emLockOpt[ i ] = EMR_OPT_NULL;
		}
		m_pOption[ i ]->Init( bClear );
	}
	
	//m_pBasicOption->SetVisibleSingle(FALSE);
}

void CItemRebuild::SetBeforeData( SITEMCUSTOM sItemCust, int nIndex )
{
	EMRANDOM_OPT m_emOpt = EMR_OPT_NULL;
	float m_fVal = 0.0f;
	//DWORD dwRndOptColor = NS_UITEXTCOLOR::dwInchant1;
	DWORD dwRndOptColor = ITEM_INFOR_COLOR::dwRandomAbility;

	switch( nIndex )
	{
	case 0:
		{
			m_emOpt = sItemCust.GETOptTYPE1();
			m_fVal = sItemCust.GETOptVALUE1();
		}
		break;
	case 1:
		{
			m_emOpt = sItemCust.GETOptTYPE2();
			m_fVal = sItemCust.GETOptVALUE2();
		}
		break;
	case 2:
		{
			m_emOpt = sItemCust.GETOptTYPE3();
			m_fVal = sItemCust.GETOptVALUE3();
		}
		break;
	case 3:
		{
			m_emOpt = sItemCust.GETOptTYPE4();
			m_fVal = sItemCust.GETOptVALUE4();
		}
		break;
	}

	if( m_fVal != 0.0f )	
	{
		CString strDescText;
		strDescText.Format( "%s: ", ID2GAMEWORD("ITEM_RANDOM_OPTION", m_emOpt ) );
		if( m_fVal > 0.0f) strDescText += _T('+');
		strDescText.AppendFormat( "%.2f", m_fVal );
		if( sItemCust.IsPerRandOpt( m_emOpt ) ) strDescText += _T('%');

		m_pOptionBefore[nIndex]->SetText( strDescText, dwRndOptColor );
	}
}

void CItemRebuild::ResetPrevDATA()
{
	m_pBasicOptionBefore->SetVisibleSingle(FALSE);
	m_pPrevButton->SetVisibleSingle(FALSE);
	for( int i=0;i<SRANDOM_GEN::MAX_OPT;i++)
	{
		m_pOptionBefore[i]->ClearText();
	}
}
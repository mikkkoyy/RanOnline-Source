#include "StdAfx.h"
#include "InfoBoxContents.h"
#include "InnerInterface.h"
#include "GLItemMan.h"

#include "BasicButton.h"
#include "BasicTextBox.h"
#include "BasicLineBox.h"
#include "ItemImage.h"

#include "UITextControl.h"
#include "GameTextControl.h"

#include "BasicTextButton.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"

#include "GLGaeaClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define EVENTNOTICE_RENDERTIME (float)20.0f

CInfoBoxContents::CInfoBoxContents ()	
	: m_bCHECK_MOUSE_STATE(FALSE)
	, m_bFirstGap(FALSE)
	, m_nPosX(0)
	, m_nPosY(0)
	, m_sItemID(NATIVEID_NULL())
	, m_pCloseButton(NULL)
	, m_pButtonSelect(NULL)
	, m_pItemImageMain(NULL)
	, m_pTextName(NULL)
	, m_pTextDesc(NULL)
	, m_pMouseOver(NULL)
	, m_nSelectCount( 0 )
	, m_nMaxSelect( 0 )
	, m_bSpecialBox( FALSE )
	, m_wInvenPosX( 0 )
	, m_wInvenPosY( 0 )
{
	for( int i=0; i<ITEM::SBOX::ITEM_SIZE; ++i )
	{
		m_pItemImageBox[i] = NULL;
		m_sItemIDBox[i] = NATIVEID_NULL();

		m_pSelectedImageBox[i] = NULL;
		m_bSelectIndex[i] = FALSE;
	}
}

CInfoBoxContents::~CInfoBoxContents ()
{
}

void CInfoBoxContents::CreateSubControl ()
{
	CD3DFontPar* pFont11 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 11, _DEFAULT_FONT_SHADOW_FLAG );
	CD3DFontPar* pFont9 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 9, _DEFAULT_FONT_SHADOW_FLAG );
	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, _DEFAULT_FONT_FLAG );

	const int nAlignLeft = TEXT_ALIGN_LEFT;
	const int nAlignCenter = TEXT_ALIGN_CENTER_X;
	const int nAlignCenterBoth = TEXT_ALIGN_CENTER_X | TEXT_ALIGN_CENTER_Y;

	CBasicLineBox* pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_QUEST_LIST", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxQuestList ( "INFO_BOX_CONTENTS_LINE" );
	RegisterControl ( pBasicLineBox );

	pBasicLineBox = new CBasicLineBox;
	pBasicLineBox->CreateSub ( this, "BASIC_LINE_BOX_WHITE", UI_FLAG_XSIZE | UI_FLAG_YSIZE );
	pBasicLineBox->CreateBaseBoxWhiteBlankBody ( "INFO_BOX_CONTENTS_LINE" );
	RegisterControl ( pBasicLineBox );

	m_pCloseButton = new CBasicButton;
	m_pCloseButton->CreateSub ( this, "INFO_BOX_CONTENTS_CLOSE" , UI_FLAG_DEFAULT, INFOBOX_CONTENTS_BUTTON_CLOSE );
	m_pCloseButton->CreateFlip ( "INFO_BOX_CONTENTS_CLOSE_F", CBasicButton::CLICK_FLIP );
	m_pCloseButton->CreateMouseOver( "INFO_BOX_CONTENTS_CLOSE_OVER" );
	m_pCloseButton->SetVisibleSingle ( TRUE );
	RegisterControl ( m_pCloseButton );

	m_pButtonSelect = new CBasicTextButton;
	m_pButtonSelect->CreateSub ( this, "BASIC_TEXT_BUTTON22", UI_FLAG_XSIZE, INFOBOX_CONTENTS_BUTTON_SELECT );
	m_pButtonSelect->CreateBaseButton ( "INFO_BOX_CONTENTS_BUTTON_COMPLETE", CBasicTextButton::SIZE22, CBasicButton::CLICK_FLIP, ID2GAMEWORD("INFO_BOX_CONTENTS_BUTTON_TEXT", 0 ) );
	RegisterControl ( m_pButtonSelect );

	m_pItemImageMain = new CItemImage;
	m_pItemImageMain->CreateSub ( this, "INFO_BOX_CONTENTS_ITEM_MAIN", UI_FLAG_DEFAULT, INFOBOX_CONTENTS_ITEM_MAIN );
	m_pItemImageMain->CreateSubControl ();
	RegisterControl ( m_pItemImageMain );

	m_pTextName = new CBasicTextBox;
	m_pTextName->CreateSub ( this, "INFO_BOX_CONTENTS_ITEM_NAME" );
	m_pTextName->SetFont ( pFont11 );
	m_pTextName->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	RegisterControl ( m_pTextName );

	m_pTextDesc = new CBasicTextBox;
	m_pTextDesc->CreateSub ( this, "INFO_BOX_CONTENTS_ITEM_DESC" );
	m_pTextDesc->SetFont ( pFont9 );
	m_pTextDesc->SetTextAlign ( TEXT_ALIGN_LEFT | TEXT_ALIGN_CENTER_Y );
	RegisterControl ( m_pTextDesc );

	std::string strBoxImage[ITEM::SBOX::ITEM_SIZE] = 
	{
		"INFO_BOX_CONTENTS_ITEM_0",
		"INFO_BOX_CONTENTS_ITEM_1",
		"INFO_BOX_CONTENTS_ITEM_2",
		"INFO_BOX_CONTENTS_ITEM_3",
		"INFO_BOX_CONTENTS_ITEM_4",
		"INFO_BOX_CONTENTS_ITEM_5",
		"INFO_BOX_CONTENTS_ITEM_6",
		"INFO_BOX_CONTENTS_ITEM_7",
		"INFO_BOX_CONTENTS_ITEM_8",
		"INFO_BOX_CONTENTS_ITEM_9",
		"INFO_BOX_CONTENTS_ITEM_10",
		"INFO_BOX_CONTENTS_ITEM_11",
		"INFO_BOX_CONTENTS_ITEM_12",
		"INFO_BOX_CONTENTS_ITEM_13",
		"INFO_BOX_CONTENTS_ITEM_14",
		"INFO_BOX_CONTENTS_ITEM_15",
		"INFO_BOX_CONTENTS_ITEM_16",
		"INFO_BOX_CONTENTS_ITEM_17",
		"INFO_BOX_CONTENTS_ITEM_18",
		"INFO_BOX_CONTENTS_ITEM_19",
	};

	for( int i=0; i<ITEM::SBOX::ITEM_SIZE; ++i )
	{
		m_pItemImageBox[i] = new CItemImage;
		m_pItemImageBox[i]->CreateSub ( this, strBoxImage[i].c_str(), UI_FLAG_DEFAULT, INFOBOX_CONTENTS_ITEM_BOX_0 + i );
		m_pItemImageBox[i]->CreateSubControl ();
		m_pItemImageBox[i]->CreateTextBoxDownRight();
		RegisterControl ( m_pItemImageBox[i] );

		m_pSelectedImageBox[i] = new CUIControl;
		m_pSelectedImageBox[i]->CreateSub ( this, "INFO_BOX_CONTENTS_ITEM_MOUSE_OVER" );
		RegisterControl ( m_pSelectedImageBox[i] );
	}

	m_pMouseOver = new CUIControl;
	m_pMouseOver->CreateSub ( this, "INFO_BOX_CONTENTS_ITEM_MOUSE_OVER" );
	RegisterControl ( m_pMouseOver );
}

void CInfoBoxContents::CheckMouseState ()
{
	const DWORD dwMsg = GetMessageEx ();
	if ( CHECK_MOUSE_IN ( dwMsg ) )
	{
		bool bBlockAction = false;
		if ( m_pCloseButton && CHECK_MOUSE_IN( m_pCloseButton->GetMessageEx() ) )	bBlockAction = true;
		if ( m_pItemImageMain && CHECK_MOUSE_IN( m_pItemImageMain->GetMessageEx() ) )	bBlockAction = true;
		if ( m_pButtonSelect && CHECK_MOUSE_IN( m_pButtonSelect->GetMessageEx() ))	bBlockAction = true;
		
		for( int i=0; i<ITEM::SBOX::ITEM_SIZE; ++i )
		{
			if ( m_pItemImageBox[i] && CHECK_MOUSE_IN(  m_pItemImageBox[i]->GetMessageEx() ) )	bBlockAction = true;
		}

		if ( dwMsg & UIMSG_LB_DOWN && !bBlockAction )
		{
			ResetSelected();
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
		ResetSelected();

		ResetExclusiveControl();
		m_bFirstGap = FALSE;					

		SetDiffuse ( D3DCOLOR_ARGB ( 0xFF, 0xFF, 0xFF, 0xFF ) );
	}
}

void CInfoBoxContents::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{	
	m_bCHECK_MOUSE_STATE = false;
	m_nPosX = x;
	m_nPosY = y;

	if ( m_pMouseOver ) m_pMouseOver->SetVisibleSingle ( FALSE );

	for( int i=0; i<ITEM::SBOX::ITEM_SIZE; ++i )
	{
		if( !m_bSpecialBox )	break;
		if( m_pSelectedImageBox[i] && m_pItemImageBox[i] )
		{
			const UIRECT& rcImagePos = m_pItemImageBox[ i ]->GetGlobalPos();
			m_pSelectedImageBox[i]->SetGlobalPos ( rcImagePos );
			m_pSelectedImageBox[i]->SetVisibleSingle( m_bSelectIndex[i] );
		}
	}

	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( IsExclusiveSelfControl() ){
		SetGlobalPos ( D3DXVECTOR2 ( x - m_vGap.x, y - m_vGap.y ) );
	}
}

void CInfoBoxContents::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	if ( !m_bCHECK_MOUSE_STATE )
	{
		CheckMouseState ();
		m_bCHECK_MOUSE_STATE = true;
	}

	switch( ControlID )
	{
	case INFOBOX_CONTENTS_BUTTON_CLOSE:
		{
			if ( CHECK_MOUSEIN_LBUPLIKE ( dwMsg ) )
			{
				CInnerInterface::GetInstance().HideGroup( GetWndID() );
				return;
			}
		}break;

	case INFOBOX_CONTENTS_ITEM_MAIN:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if( m_sItemID != NATIVEID_NULL() )
				{
					SITEMCUSTOM sitemcustom;
					sitemcustom.sNativeID = m_sItemID;

					CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW( sitemcustom, FALSE, FALSE, FALSE,FALSE,FALSE, NULL, NULL, NATIVEID_NULL(), FALSE );

					/*item preview, Juver, 2017/07/27 */
					if ( DxInputDevice::GetInstance().GetKeyState ( DIK_LMENU  ) & DXKEY_DOWNED )
					{
						if ( dwMsg & UIMSG_LB_UP )
						{
							CInnerInterface::GetInstance().PreviewItem( sitemcustom );
							return;
						}
					}
				}
			}
		}break;
	case INFOBOX_CONTENTS_BUTTON_SELECT:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				if ( dwMsg & UIMSG_LB_UP )
				{
					GLCharacter* pCharacter = GLGaeaClient::GetInstance().GetCharacter();
					if( !pCharacter )
						return;

					if( m_nSelectCount == m_nMaxSelect )
					pCharacter->ReqBoxOpen( m_wInvenPosX, m_wInvenPosY );
				}
			}
		}
		break;
	case INFOBOX_CONTENTS_ITEM_BOX_0:
	case INFOBOX_CONTENTS_ITEM_BOX_1:
	case INFOBOX_CONTENTS_ITEM_BOX_2:
	case INFOBOX_CONTENTS_ITEM_BOX_3:
	case INFOBOX_CONTENTS_ITEM_BOX_4:
	case INFOBOX_CONTENTS_ITEM_BOX_5:
	case INFOBOX_CONTENTS_ITEM_BOX_6:
	case INFOBOX_CONTENTS_ITEM_BOX_7:
	case INFOBOX_CONTENTS_ITEM_BOX_8:
	case INFOBOX_CONTENTS_ITEM_BOX_9:
	case INFOBOX_CONTENTS_ITEM_BOX_10:
	case INFOBOX_CONTENTS_ITEM_BOX_11:
	case INFOBOX_CONTENTS_ITEM_BOX_12:
	case INFOBOX_CONTENTS_ITEM_BOX_13:
	case INFOBOX_CONTENTS_ITEM_BOX_14:
	case INFOBOX_CONTENTS_ITEM_BOX_15:
	case INFOBOX_CONTENTS_ITEM_BOX_16:
	case INFOBOX_CONTENTS_ITEM_BOX_17:
	case INFOBOX_CONTENTS_ITEM_BOX_18:
	case INFOBOX_CONTENTS_ITEM_BOX_19:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				int nIndex = ControlID - INFOBOX_CONTENTS_ITEM_BOX_0;

				if ( nIndex < ITEM::SBOX::ITEM_SIZE )
				{
					if( m_sItemIDBox[nIndex] != NATIVEID_NULL() )
					{
						SITEMCUSTOM sitemcustom;
						sitemcustom.sNativeID = m_sItemIDBox[nIndex];

						CInnerInterface::GetInstance().SHOW_ITEM_INFO_NEW( sitemcustom, FALSE, FALSE, FALSE,FALSE,FALSE, NULL, NULL, NATIVEID_NULL(), FALSE );

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

						if ( m_pMouseOver && m_pItemImageBox[nIndex] )
						{
							const UIRECT& rcImagePos = m_pItemImageBox[nIndex]->GetGlobalPos();
							m_pMouseOver->SetGlobalPos ( rcImagePos );
							m_pMouseOver->SetVisibleSingle ( TRUE );
							
							//select box content open
							if ( m_bSpecialBox )
							{
								if ( dwMsg & UIMSG_LB_UP )
								{
									if( m_nSelectCount >= m_nMaxSelect )	
									{
										ResetSelected();
										if( m_bSelectIndex[nIndex] ) 
										{
											m_bSelectIndex[nIndex] = FALSE;
											m_nSelectCount--;
										}
										return;
									}

									m_bSelectIndex[nIndex] = ( m_bSelectIndex[nIndex] ) ? FALSE:TRUE;

									if( m_bSelectIndex[nIndex] ) m_nSelectCount++;
									else m_nSelectCount--;
								}	
							}
						}
					}
				}
			}
		}break;

	};

	CUIGroup::TranslateUIMessage ( ControlID, dwMsg );
}

void CInfoBoxContents::DataReset()
{
	m_sItemID = NATIVEID_NULL();

	m_bSpecialBox = FALSE;
	m_nMaxSelect = 0;
	m_wInvenPosX = 0;
	m_wInvenPosY = 0;

	if ( m_pItemImageMain )
		m_pItemImageMain->ResetItem();

	if ( m_pTextName )
		m_pTextName->ClearText();

	if ( m_pTextDesc )
		m_pTextDesc->ClearText();

	for( int i=0; i<ITEM::SBOX::ITEM_SIZE; ++i )
	{
		m_sItemIDBox[i] = NATIVEID_NULL();

		if ( m_pItemImageBox[i] )	
		{
			m_pItemImageBox[i]->ResetItem();
			m_pItemImageBox[i]->SetTextDownRight( "", NS_UITEXTCOLOR::ORANGE );
		}
	}
	ResetSelected();
}

void CInfoBoxContents::ResetSelected()
{
	m_nSelectCount = 0;

	for( int i=0; i<ITEM::SBOX::ITEM_SIZE; ++i )
	{
		if(	m_pSelectedImageBox[i] ) m_pSelectedImageBox[i]->SetVisibleSingle( FALSE );
		m_bSelectIndex[i] = FALSE;
	}
}

void CInfoBoxContents::DataShow( SITEM* pItem, BOOL bBoxSelect, WORD wPosX, WORD wPosY )
{
	DataReset();

	if ( !pItem )	return;

	m_sItemID = pItem->sBasicOp.sNativeID;
	m_nMaxSelect = pItem->sBox.m_nMaxSelect;
	m_bSpecialBox = bBoxSelect;
	m_wInvenPosX = wPosX;
	m_wInvenPosY = wPosY;

	if ( m_pItemImageMain )		
		m_pItemImageMain->SetItem( m_sItemID );

	if ( m_pTextName )
		m_pTextName->AddText( pItem->GetName(), COMMENT::ITEMCOLOR[pItem->sBasicOp.emLevel] );

	if( m_pButtonSelect )
		m_pButtonSelect->SetVisibleSingle( m_bSpecialBox );

	if ( m_bSpecialBox )
	{
		if ( m_pTextDesc )
		{
			CString strText;
			strText.Format (ID2GAMEWORD("INFO_BOX_CONTENTS_BUTTON_TEXT",1),m_nMaxSelect);
			m_pTextDesc->AddText(strText, NS_UITEXTCOLOR::WHITE );
		}
	}

	for( int i=0; i<ITEM::SBOX::ITEM_SIZE; ++i )
	{
		m_sItemIDBox[i] = pItem->sBox.sITEMS[i].nidITEM;
		DWORD dwNum = pItem->sBox.sITEMS[i].dwAMOUNT;

		SITEM* pitembox = GLItemMan::GetInstance().GetItem( m_sItemIDBox[i] );
		if ( !pitembox )	continue;;

		if ( m_pItemImageBox[i] )
		{
			CString strText;
			strText.Format( "%u", dwNum );
			m_pItemImageBox[i]->SetItem( m_sItemIDBox[i] );
			m_pItemImageBox[i]->SetTextDownRight( strText.GetString(), NS_UITEXTCOLOR::ORANGE );
		}
	}

}

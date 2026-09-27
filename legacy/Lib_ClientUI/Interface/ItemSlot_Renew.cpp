#include "StdAfx.h"
#include "ItemSlot_Renew.h"
#include "ItemImage.h"
#include "InnerInterface.h"
#include "ItemMove.h"
#include "BasicTextBox.h"
#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "UITextControl.h"
#include "d3dfont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int CItemSlot_Renew::nOUTOFRANGE = -1;

CItemSlot_Renew::CItemSlot_Renew () :
	m_pMouseOver ( NULL ),
	m_nMaxColumn ( 0 ),
	m_bBLOCK ( false )
{
	SecureZeroMemory ( m_pItemImageArray, sizeof( m_pItemImageArray ) );	
	SecureZeroMemory ( m_pNumberBoxArray, sizeof( m_pNumberBoxArray ) );
	memset ( m_pBlock, 0, sizeof ( CUIControl* ) * nBLOCK_MAX );
}

CItemSlot_Renew::~CItemSlot_Renew ()
{
}

void CItemSlot_Renew::CreateSubControl ( int nMaxColumn, BOOL bNumberUse )
{
	m_nMaxColumn = nMaxColumn;

	{
		CString	strSlotBack[nLIMIT_COLUMN] = 
		{
			"ITEM_IMAGE_BACK0",
			"ITEM_IMAGE_BACK1",
			"ITEM_IMAGE_BACK2",
			"ITEM_IMAGE_BACK3",
			"ITEM_IMAGE_BACK4",
			"ITEM_IMAGE_BACK5",
			"ITEM_IMAGE_BACK6",
			"ITEM_IMAGE_BACK7",
			"ITEM_IMAGE_BACK8",
			"ITEM_IMAGE_BACK9"
		};

		for ( int i = 0; i < m_nMaxColumn; i++ )
		{
			m_pItemImageArray[i] = CreateItemImage ( strSlotBack[i].GetString (), ITEM_IMAGE0 + i );			
		}
	}

	if ( bNumberUse )
	{
		CreateNumberBox ();
	}

	m_pMouseOver = new CUIControl;
	m_pMouseOver->CreateSub ( this, "ITEM_MOUSE_OVER" );
	RegisterControl ( m_pMouseOver );

	CString	strSlotBlock[nBLOCK_MAX] = 
	{
		"ITEM_SLOT_BLOCK_X0",
		"ITEM_SLOT_BLOCK_X1",
		"ITEM_SLOT_BLOCK_X2",
		"ITEM_SLOT_BLOCK_X3",
		"ITEM_SLOT_BLOCK_X4",
		"ITEM_SLOT_BLOCK_X5"
	};
	
	for (int i = 0 ; i < nBLOCK_MAX ; i++ )
	{
		m_pBlock[i] = new CUIControl;
		m_pBlock[i]->CreateSub ( this, strSlotBlock[i].GetString () );	
		m_pBlock[i]->SetVisibleSingle ( FALSE );
		RegisterControl ( m_pBlock[i] );
	}
}

CItemImage*	CItemSlot_Renew::CreateItemImage ( const char* szControl, UIGUID ControlID )
{
	CItemImage* pItemImage = new CItemImage;
	pItemImage->CreateSub ( this, szControl, UI_FLAG_DEFAULT, ControlID );
	pItemImage->CreateSubControl ();
	RegisterControl ( pItemImage );

	return pItemImage;
}

void CItemSlot_Renew::ResetItemImage ( int nIndex )
{
	if ( nIndex < 0 || m_nMaxColumn <= nIndex )
	{
		return ;
	}

	CItemImage* pItem = m_pItemImageArray[nIndex];
	pItem->ResetItem ();
	
	m_InvenItem[nIndex].sItemCustom.sNativeID = NATIVEID_NULL ();
}

void CItemSlot_Renew::SetItemImage ( int nIndex, SINVENITEM& ref_InvenItem )
{
	if ( nIndex < 0 || m_nMaxColumn <= nIndex )
	{
		return ;
	}

	SITEMCUSTOM &sItemCustom = ref_InvenItem.sItemCustom;
	SITEM* pItem = GLItemMan::GetInstance().GetItem ( sItemCustom.sNativeID );
	if ( !pItem )
	{
		GASSERT ( 0 && "등록되지 않은 ID입니다." );
		return ;
	}
    
	CItemImage* pItemImage = m_pItemImageArray[nIndex];
	pItemImage->SetItem ( sItemCustom );

	/*item wrapper, Juver, 2018/01/12 */
	/*if ( pItem->sBasicOp.emItemType == ITEM_WRAPPER_BOX )
	{
		SITEM* pitem_disguise = GLItemMan::GetInstance().GetItem( sItemCustom.nidDISGUISE );
		if ( pitem_disguise )
			pItemImage->SetBackItem( pitem_disguise->sBasicOp.sICONID, pitem_disguise->GetInventoryFile() );
	}*/

	m_InvenItem[nIndex] = ref_InvenItem;
}

void CItemSlot_Renew::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
    SetItemIndex ( nOUTOFRANGE );
	m_pMouseOver->SetVisibleSingle ( FALSE );
    
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );

	if ( m_pNumberBoxArray[0] )
	{
		for ( int i = 0; i < m_nMaxColumn; i++ )
		{
			ResetNumber ( i );

			SNATIVEID sNativeID = m_InvenItem[i].sItemCustom.sNativeID;
			if ( sNativeID != NATIVEID_NULL () )
			{	
				SITEM* pItemData = GLItemMan::GetInstance().GetItem ( sNativeID );
				if ( pItemData )
				{
					const WORD wPileNum = pItemData->sDrugOp.wPileNum;
					const WORD wTurnNum = m_InvenItem[i].sItemCustom.wTurnNum;						
					const WORD wGradeDefNum = m_InvenItem[i].sItemCustom.grade_defense();
					const WORD wGradeDamNum = m_InvenItem[i].sItemCustom.grade_damage();

					if ( wPileNum>1 || wTurnNum>1 )
					{
						SetNumber ( i, wTurnNum, wPileNum );
					}

					WORD Higher;
					if( wGradeDefNum > wGradeDamNum )
						Higher = wGradeDefNum;
					else
						Higher = wGradeDamNum;

					if ( Higher > 1 )
					{
						SetNumber2 ( i,  Higher );
					}
				}
			}
		}
	}
}

void CItemSlot_Renew::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
{
	switch ( ControlID )
	{
	case ITEM_IMAGE0:
	case ITEM_IMAGE1:
	case ITEM_IMAGE2:
	case ITEM_IMAGE3:
	case ITEM_IMAGE4:
	case ITEM_IMAGE5:
	case ITEM_IMAGE6:
	case ITEM_IMAGE7:
	case ITEM_IMAGE8:
	case ITEM_IMAGE9:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				m_nIndex = ControlID - ITEM_IMAGE0;			

				if ( !CInnerInterface::GetInstance().IsFirstItemSlot () ) return ;								

				if ( m_pItemImageArray[m_nIndex]->GetItem () != NATIVEID_NULL () )
				{
					const UIRECT& rcImagePos = m_pItemImageArray[m_nIndex]->GetGlobalPos ();
					const UIRECT& rcImageLocalPos = m_pItemImageArray[m_nIndex]->GetLocalPos ();
					m_pMouseOver->SetLocalPos ( rcImageLocalPos );
					m_pMouseOver->SetGlobalPos ( rcImagePos );
					m_pMouseOver->SetVisibleSingle ( TRUE );
				}

				CItemMove* pItemMove = CInnerInterface::GetInstance().GetItemMove ();
				if ( !pItemMove )
				{
					GASSERT ( 0 && "CItemMove가 널입니다." );
					return ;
				}

				if ( pItemMove->GetItem () != NATIVEID_NULL () )
				{
					const UIRECT& rcSlotPos = m_pItemImageArray[m_nIndex]->GetGlobalPos ();
					pItemMove->SetGlobalPos ( rcSlotPos );						
					AddMessageEx ( UIMSG_MOUSEIN_ITEMSLOT_RENEW | UIMSG_TOTOPPARENT );
				}
			}
		}
		break;
	};
}

CBasicTextBox* CItemSlot_Renew::CreateNumberBox ( const char* szControl )
{
	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, D3DFONT_SHADOW | D3DFONT_ASCII );
	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, szControl );
    pTextBox->SetFont ( pFont8 );
	pTextBox->SetTextAlign ( TEXT_ALIGN_RIGHT );
	RegisterControl ( pTextBox );
	return pTextBox;
}

void CItemSlot_Renew::SetNumber ( int nIndex, int nNumber, int nMaxNumber )
{
	CString strNumber;
	strNumber.Format ( "%d", nNumber );
	DWORD dwColor = NS_UITEXTCOLOR::GREENYELLOW;
	if ( nNumber == nMaxNumber ) dwColor = NS_UITEXTCOLOR::ORANGE;
	m_pNumberBoxArray[nIndex]->SetOneLineText ( strNumber, dwColor );
	m_pNumberBoxArray[nIndex]->SetVisibleSingle ( TRUE );
}

void CItemSlot_Renew::SetNumber2 ( int nIndex, int nNumber )
{
	CString strNumber;
	strNumber.Format ( "+%d", nNumber );
	DWORD dwColor = NS_UITEXTCOLOR::WHITE;
	if ( nNumber >= 1 && nNumber <= 4 )
		dwColor = NS_UITEXTCOLOR::RED;
	else if ( nNumber == 5 ){
		dwColor = NS_UITEXTCOLOR::PALEGOLDENROD;
	}else if ( nNumber == 6 ){
		dwColor = NS_UITEXTCOLOR::TOMATO;
	}else if ( nNumber >= 7 && nNumber <= 8 ){
		dwColor = NS_UITEXTCOLOR::DARKORANGE;
	}else if ( nNumber == 9 ){
		dwColor = NS_UITEXTCOLOR::GOLD;
	}else if ( nNumber == 10 ){
		dwColor = NS_UITEXTCOLOR::DARKTURQUOISE;
	}else if ( nNumber == 11 ){
		dwColor = NS_UITEXTCOLOR::DARKVIOLET;
	}
	else
		dwColor = NS_UITEXTCOLOR::WHITE;
	m_pNumberBoxArray[nIndex]->SetOneLineText ( strNumber, dwColor );
	m_pNumberBoxArray[nIndex]->SetVisibleSingle ( TRUE );
}

void CItemSlot_Renew::ResetNumber ( int nIndex )
{
	m_pNumberBoxArray[nIndex]->ClearText ();
	m_pNumberBoxArray[nIndex]->SetVisibleSingle ( FALSE );
}

void CItemSlot_Renew::CreateNumberBox ()
{
	CString	strSlotBack[nLIMIT_COLUMN] = 
	{
		"ITEM_NUMBER0",
		"ITEM_NUMBER1",
		"ITEM_NUMBER2",
		"ITEM_NUMBER3",
		"ITEM_NUMBER4",
		"ITEM_NUMBER5",
		"ITEM_NUMBER6",
		"ITEM_NUMBER7",
		"ITEM_NUMBER8",
		"ITEM_NUMBER9"
	};

	for ( int i = 0; i < m_nMaxColumn; i++ )
	{
		m_pNumberBoxArray[i] = CreateNumberBox ( strSlotBack[i].GetString () );			
	}
}

void	CItemSlot_Renew::SetBLOCK ()
{
	m_bBLOCK = true;
	for (int i = 0 ; i < nBLOCK_MAX ; i++ )
	{
		m_pBlock[i]->SetUseRender ( TRUE );
		m_pBlock[i]->SetVisibleSingle ( TRUE );
		m_pBlock[i]->SetDiffuse (D3DCOLOR_ARGB(125,0,0,0));
	}
}

void	CItemSlot_Renew::ResetBLOCK ()
{
	m_bBLOCK = false;
	for (int i = 0 ; i < nBLOCK_MAX ; i++ )
	{
		m_pBlock[i]->SetUseRender ( FALSE );
		m_pBlock[i]->SetVisibleSingle ( FALSE );
	}
}
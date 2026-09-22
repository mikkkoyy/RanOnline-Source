#include "StdAfx.h"
#include "BasicQuickPotionSlot.h"

#include "../Lib_Engine/DxCommon/DxFontMan.h"
#include "./BasicTextBox.h"
#include "GLGaeaClient.h"
#include "ItemImage.h"
#include "GLItemMan.h"
#include "d3dfont.h"
#include "UITextControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const D3DCOLOR CBasicQuickPotionSlot::dwDEFAULT_TRANSPARENCY = D3DCOLOR_ARGB ( 160, 255, 255, 255 );;
const D3DCOLOR CBasicQuickPotionSlot::dwFULL_TRANSPARENCY = D3DCOLOR_ARGB ( 255, 255, 255, 255 );;

CBasicQuickPotionSlot::CBasicQuickPotionSlot () :
	m_pPotionMouseOver ( NULL ),
	m_pBlockImage ( NULL ), /*prohibit potion skill logic, Juver, 2017/06/06 */
	m_pMiniText ( NULL ),
	m_pd3dDevice ( NULL ),
	m_nIndex ( 0 ),
	m_pItemImage ( NULL )
{
}

CBasicQuickPotionSlot::~CBasicQuickPotionSlot ()
{
}

void  CBasicQuickPotionSlot::CreateSubControl ()
{
	CreatePotionImage ( "BASIC_QUICK_POTION_IMAGE" );	
	CreateMouseOver ( "BASIC_QUICK_POTION_SLOT_MOUSE_OVER" );
	m_pMiniText = CreateTextBox ( "BASIC_QUICK_POTION_SLOT_AMOUNT_SINGLE" );	
	m_pMiniText->SetTextAlign ( TEXT_ALIGN_RIGHT );
	SetSlotIndex ( 0 );
}

void CBasicQuickPotionSlot::CreatePotionImage ( char* szImage )
{
	CItemImage* pItemImage = new CItemImage;
	pItemImage->CreateSub ( this, szImage );
	pItemImage->CreateSubControl ();
	pItemImage->SetUseRender ( TRUE );
	pItemImage->SetVisibleSingle ( FALSE );
	RegisterControl ( pItemImage );
    m_pItemImage = pItemImage;
}

HRESULT CBasicQuickPotionSlot::InitDeviceObjects ( LPDIRECT3DDEVICEQ pd3dDevice )
{
	HRESULT hr = S_OK;
	hr = CUIGroup::InitDeviceObjects ( pd3dDevice );
	if ( FAILED ( hr ) ) return hr;

	m_pd3dDevice = pd3dDevice;

	return S_OK;
}

void CBasicQuickPotionSlot::CreateMouseOver ( char* szMouseOver )
{
	CUIControl* pPotionMouseOver = new CUIControl;
	pPotionMouseOver->CreateSub ( this, szMouseOver, UI_FLAG_DEFAULT, QUICK_POTION_MOUSE_OVER );
	pPotionMouseOver->SetVisibleSingle ( TRUE );
    RegisterControl ( pPotionMouseOver );
	m_pPotionMouseOver = pPotionMouseOver;
}

/*prohibit potion skill logic, Juver, 2017/06/06 */
void CBasicQuickPotionSlot::CreateBlockImage ( char* szBlock )
{
	CUIControl* pBlock = new CUIControl;
	pBlock->CreateSub ( this, szBlock, UI_FLAG_DEFAULT, QUICK_POTION_BLOCK );
	pBlock->SetVisibleSingle ( TRUE );
	RegisterControl ( pBlock );
	m_pBlockImage = pBlock;
}

CBasicTextBox* CBasicQuickPotionSlot::CreateTextBox ( char* szTextBox )
{
	CD3DFontPar* pFont8 = DxFontMan::GetInstance().LoadDxFont ( _DEFAULT_FONT, 8, D3DFONT_SHADOW | D3DFONT_ASCII );

	CBasicTextBox* pTextBox = new CBasicTextBox;
	pTextBox->CreateSub ( this, szTextBox );
    pTextBox->SetFont ( pFont8 );	
	RegisterControl ( pTextBox );
	return pTextBox;
}

void CBasicQuickPotionSlot::SetItem ( SNATIVEID sItemID, int nAmount )
{
	if ( m_pItemImage )
	{
		m_pItemImage->SetItem ( sItemID );
		m_pItemImage->SetVisibleSingle ( TRUE );

		CString strNumber;
		strNumber.Format ( "%d", nAmount );
		if ( m_pMiniText ) m_pMiniText->SetOneLineText ( strNumber, NS_UITEXTCOLOR::WHITE );

		if ( nAmount <= 0 )	m_pItemImage->SetDiffuse ( dwDEFAULT_TRANSPARENCY );
		else				m_pItemImage->SetDiffuse ( dwFULL_TRANSPARENCY );
	}
}

void CBasicQuickPotionSlot::ResetItem ()
{
	if ( m_pItemImage )
	{
		m_pItemImage->SetVisibleSingle ( FALSE );
		m_pItemImage->ResetItem ();

		if ( m_pMiniText ) m_pMiniText->ClearText ();
	}
}

void CBasicQuickPotionSlot::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
	
	const int nIndex = GetSlotIndex ();
	SACTION_SLOT sSlot = GLGaeaClient::GetInstance().GetCharacter()->m_sACTIONQUICK[nIndex];
	bool bProhibitPotion = GLGaeaClient::GetInstance().GetCharacter()->m_bProhibitPotion; /*prohibit potion skill logic, Juver, 2017/06/06 */
	if ( sSlot.sNID != NATIVEID_NULL () )
	{
		SITEM* pItem = GLItemMan::GetInstance().GetItem ( sSlot.sNID );
		if ( !pItem )
		{
			GASSERT ( 0 && "등록되지 않은 ID입니다." );
			return ;
		}

		//	NOTE
		//		이전 프레임과 ID가 다를경우 날림
		if ( GetItemID () != sSlot.sNID ) ResetItem ();

		//	NOTE
		//		슬롯에 등록된 ID 등록
		SetItemNativeID ( sSlot.sNID );

		int nAmountNumber = GLGaeaClient::GetInstance().GetCharacter()->GetAmountActionQ ( nIndex );
		SetItem ( sSlot.sNID, nAmountNumber );

		/*prohibit potion skill logic, Juver, 2017/06/06 */
		/*if ( m_pBlockImage )
		{
			BOOL bPotion = FALSE;
			if ( bProhibitPotion && pItem->sBasicOp.emItemType == ITEM_CURE )
			{
				if ( pItem->sDrugOp.emDrug == ITEM_DRUG_HP || 
					pItem->sDrugOp.emDrug == ITEM_DRUG_MP || 
					pItem->sDrugOp.emDrug == ITEM_DRUG_SP || 
					pItem->sDrugOp.emDrug == ITEM_DRUG_HP_MP || 
					pItem->sDrugOp.emDrug == ITEM_DRUG_MP_SP || 
					pItem->sDrugOp.emDrug == ITEM_DRUG_HP_MP_SP || 
					pItem->sDrugOp.emDrug == ITEM_DRUG_CURE || 
					pItem->sDrugOp.emDrug == ITEM_DRUG_HP_CURE || 
					pItem->sDrugOp.emDrug == ITEM_DRUG_HP_MP_SP_CURE || 
					pItem->sDrugOp.emDrug == ITEM_DRUG_CP ){
						bPotion = TRUE;
				}
			}

			m_pBlockImage->SetVisibleSingle ( bPotion );
		}*/
	}
	else
	{
		ResetItem ();
	}
}
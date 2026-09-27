#include "StdAfx.h"
#include "CharacterWindowCharWear.h"
#include "ItemImageCharWear.h"
#include "GLGaeaClient.h"
#include "GLItemMan.h"
#include "InnerInterface.h"
#include "BasicButton.h"
#include "ItemMove.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int CCharacterWindowCharWear::nOUTOFRANGE = -1;

CCharacterWindowCharWear::CCharacterWindowCharWear () 
	: m_pMouseOver ( NULL )
{
}

CCharacterWindowCharWear::~CCharacterWindowCharWear ()
{
}

void CCharacterWindowCharWear::CreateSubControl ()
{
	CString strInvenWearItem[ITEM_IMAGE_SIZE] = 
	{
		"CHARACTER_WINDOW_WEAR_SLOT_HEAD",
		"CHARACTER_WINDOW_WEAR_SLOT_UPPER",
		"CHARACTER_WINDOW_WEAR_SLOT_WAIST",
		"CHARACTER_WINDOW_WEAR_SLOT_LOWER",
		"CHARACTER_WINDOW_WEAR_SLOT_HAND",
		"CHARACTER_WINDOW_WEAR_SLOT_FOOT",
		"CHARACTER_WINDOW_WEAR_SLOT_LEFT_ACCESSORY",
		"CHARACTER_WINDOW_WEAR_SLOT_LEFT_EARRING",
		"CHARACTER_WINDOW_WEAR_SLOT_RIGHT_EARRING",
		"CHARACTER_WINDOW_WEAR_SLOT_NECK",
		"CHARACTER_WINDOW_WEAR_SLOT_WRIST",
		"CHARACTER_WINDOW_WEAR_SLOT_LEFT_FINGER",
		"CHARACTER_WINDOW_WEAR_SLOT_RIGHT_FINGER",
		"CHARACTER_WINDOW_WEAR_SLOT_RIGHT_ACCESSORY",
		"CHARACTER_WINDOW_WEAR_SLOT_RIGHT_WEAPON",
		"CHARACTER_WINDOW_WEAR_SLOT_LEFT_WEAPON",
		"CHARACTER_WINDOW_WEAR_SLOT_VEHICLE",
		"CHARACTER_WINDOW_WEAR_SLOT_FACE",
		"CHARACTER_WINDOW_WEAR_SLOT_MISC",
	};

	CString strInvenWearOver[ITEM_IMAGE_SIZE] = 
	{
		"CHARACTER_WINDOW_WEAR_SLOT_HEAD_OVER",
		"CHARACTER_WINDOW_WEAR_SLOT_UPPER_OVER",
		"CHARACTER_WINDOW_WEAR_SLOT_WAIST_OVER",
		"CHARACTER_WINDOW_WEAR_SLOT_LOWER_OVER",
		"CHARACTER_WINDOW_WEAR_SLOT_HAND_OVER",
		"CHARACTER_WINDOW_WEAR_SLOT_FOOT_OVER",
		"CHARACTER_WINDOW_WEAR_SLOT_LEFT_ACCESSORY_OVER",
		"CHARACTER_WINDOW_WEAR_SLOT_LEFT_EARRING_OVER",
		"CHARACTER_WINDOW_WEAR_SLOT_RIGHT_EARRING_OVER",
		"CHARACTER_WINDOW_WEAR_SLOT_NECK_OVER",
		"CHARACTER_WINDOW_WEAR_SLOT_WRIST_OVER",
		"CHARACTER_WINDOW_WEAR_SLOT_LEFT_FINGER_OVER",
		"CHARACTER_WINDOW_WEAR_SLOT_RIGHT_FINGER_OVER",
		"CHARACTER_WINDOW_WEAR_SLOT_RIGHT_ACCESSORY_OVER",
		"CHARACTER_WINDOW_WEAR_SLOT_RIGHT_WEAPON_OVER",
		"CHARACTER_WINDOW_WEAR_SLOT_LEFT_WEAPON_OVER",
		"CHARACTER_WINDOW_WEAR_SLOT_VEHICLE_OVER",
		"CHARACTER_WINDOW_WEAR_SLOT_FACE_OVER",
		"CHARACTER_WINDOW_WEAR_SLOT_MISC_OVER",
	};

	for ( int i = 0; i < ITEM_IMAGE_SIZE; i++ )
	{
		m_pItemImage[i] = CreateItemImage ( strInvenWearItem[i].GetString (), ITEM_IMAGE0 + i );
		m_pSlotDisplay[i] = CreateControl ( strInvenWearOver[i].GetString () );
		m_pSlotDisplay[i]->SetVisibleSingle ( FALSE );
	}

	m_pMouseOver = CreateControl ( "ITEM_MOUSE_OVER" );	
}

CUIControl*	CCharacterWindowCharWear::CreateControl ( const char* szControl )
{
	CUIControl* pControl = new CUIControl;
	pControl->CreateSub ( this, szControl );
	RegisterControl ( pControl );

	return pControl;
}

CItemImageCharWear*	CCharacterWindowCharWear::CreateItemImage ( const char* szControl, UIGUID ControlID )
{
	CItemImageCharWear* pItemImage = new CItemImageCharWear;
	pItemImage->CreateSub ( this, szControl, UI_FLAG_DEFAULT, ControlID );
	pItemImage->CreateSubControl ();
	RegisterControl ( pItemImage );

	return pItemImage;
}

EMSLOT CCharacterWindowCharWear::IMAGE2EMSLOT ( int nIndex )
{
	switch ( nIndex )
	{	
	case ITEM_IMAGE0:	return SLOT_HEADGEAR;//HEAD
	case ITEM_IMAGE1:	return SLOT_UPPER;//UPPER
	case ITEM_IMAGE2:	return SLOT_WAIST;//WAIST
	case ITEM_IMAGE3:	return SLOT_LOWER;//LOWER
	case ITEM_IMAGE4:	return SLOT_HAND;//HAND
	case ITEM_IMAGE5:	return SLOT_FOOT;//FOOT
	case ITEM_IMAGE6:	return SLOT_LACCESSORY;//LEFT ACCESSORY
	case ITEM_IMAGE7:	return SLOT_LEARRING;//LEFT EARRING
	case ITEM_IMAGE8:	return SLOT_ORNAMENT;//ORNAMENT
	case ITEM_IMAGE9:	return SLOT_NECK;//NECK
	case ITEM_IMAGE10:	return SLOT_WRIST;//WRIST
	case ITEM_IMAGE11:	return SLOT_LFINGER;//LFINGER
	case ITEM_IMAGE12:	return SLOT_RFINGER;//RFINGER
	case ITEM_IMAGE13:	return SLOT_RACCESSORY;//RIGHT ACCESSORY
	case ITEM_IMAGE14:	return SLOT_RHAND;//RHAND
	case ITEM_IMAGE15:	return SLOT_LHAND;//LHAND
	case ITEM_IMAGE16:	return SLOT_VEHICLE;//VEHICLE
	case ITEM_IMAGE17:	return SLOT_FACE;
	case ITEM_IMAGE18:	return SLOT_MISC;
	}

	GASSERT ( 0 && "범위를 벗어납니다." );
	return SLOT_TSIZE;
}

void CCharacterWindowCharWear::Update ( int x, int y, BYTE LB, BYTE MB, BYTE RB, int nScroll, float fElapsedTime, BOOL bFirstControl )
{
	SetItemIndex ( nOUTOFRANGE );
	if ( m_pMouseOver ) m_pMouseOver->SetVisibleSingle ( FALSE );

	CItemMove* pItemMove = CInnerInterface::GetInstance().GetItemMove ();
	SNATIVEID sHOLD_ITEM_ID(false);
	if ( pItemMove ) sHOLD_ITEM_ID = pItemMove->GetItem();

	for ( int i = 0; i < SLOT_NSIZE_2; i++ )
	{
		if ( i >= ITEM_IMAGE_SIZE )
			continue;

		const SITEMCUSTOM& sItemCustomOld = GetItem ( i );

		EMSLOT emSlot = IMAGE2EMSLOT ( i + ITEM_IMAGE0 );
		SITEMCUSTOM sItemCustom = GLGaeaClient::GetInstance().GetCharacter()->GET_SLOT_ITEM ( emSlot );

		if ( sItemCustom != sItemCustomOld )
		{		
			if ( sItemCustom.sNativeID != NATIVEID_NULL () )
			{
				LoadItem ( i, sItemCustom );
			}else{
				UnLoadItem ( i );
			}
		}

		m_pSlotDisplay[i]->SetVisibleSingle ( FALSE );

		if ( sHOLD_ITEM_ID != NATIVEID_NULL () )
		{
			if ( GLGaeaClient::GetInstance().GetCharacter()->CHECKSLOT_ITEM ( sHOLD_ITEM_ID, emSlot ) )
			{
				const UIRECT& rcImagePos = m_pItemImage[i]->GetGlobalPos ();
				D3DXVECTOR2 vPos ( rcImagePos.left, rcImagePos.top );

				if ( GLGaeaClient::GetInstance().GetCharacter()->ACCEPT_ITEM ( sHOLD_ITEM_ID ) )
				{
					m_pSlotDisplay[i]->SetVisibleSingle ( TRUE );
				}
			}
		}
	}
	
	CUIGroup::Update ( x, y, LB, MB, RB, nScroll, fElapsedTime, bFirstControl );
}

void CCharacterWindowCharWear::TranslateUIMessage ( UIGUID ControlID, DWORD dwMsg )
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
	case ITEM_IMAGE10:
	case ITEM_IMAGE11:
	case ITEM_IMAGE12:
	case ITEM_IMAGE13:
	case ITEM_IMAGE14:
	case ITEM_IMAGE15:
	case ITEM_IMAGE16:
	case ITEM_IMAGE17:
	case ITEM_IMAGE18:
		{
			if ( CHECK_MOUSE_IN ( dwMsg ) )
			{
				int nIndex = ControlID - ITEM_IMAGE0;
				SetItemIndex ( nIndex );

				if ( !CInnerInterface::GetInstance().IsFirstItemSlot () ) return ;

				CItemMove* pItemMove = CInnerInterface::GetInstance().GetItemMove ();
				if ( !pItemMove )
				{
					GASSERT ( 0 && "CItemMove가 널입니다." );
					return ;
				}

				if ( pItemMove->GetItem () != NATIVEID_NULL () )
				{
					const UIRECT& rcSlotPos = m_pItemImage[nIndex]->GetGlobalPos ();
					pItemMove->SetGlobalPos ( rcSlotPos );						
					AddMessageEx ( UIMSG_MOUSEIN_NEWWEARSLOT | UIMSG_TOTOPPARENT );
				}				

				if ( pItemMove->GetItem () == NATIVEID_NULL () && m_pItemImage[m_nIndex]->GetItem () != NATIVEID_NULL () )
				{
					const UIRECT& rcImagePos = m_pItemImage[nIndex]->GetGlobalPos ();
					m_pMouseOver->SetGlobalPos ( rcImagePos );
					m_pMouseOver->SetVisibleSingle ( TRUE );
				}
			}
		}
		break;
	}
}

EMSLOT CCharacterWindowCharWear::GetItemSlot ()
{
	return IMAGE2EMSLOT ( GetItemIndex () + ITEM_IMAGE0 );
}

void CCharacterWindowCharWear::LoadItem ( int nIndex, SITEMCUSTOM& ref_sItemCustom )
{
	m_ItemCustomArray[nIndex] = ref_sItemCustom;

	SITEM* pItemData = GLItemMan::GetInstance().GetItem ( ref_sItemCustom.sNativeID );
	m_pItemImage[nIndex]->SetItem ( pItemData->sBasicOp.sICONID, pItemData->GetInventoryFile(), ref_sItemCustom.GETGRADE( EMGRINDING_NONE ), pItemData->sDrugOp.wPileNum, ref_sItemCustom.wTurnNum, pItemData->sBasicOp.sNativeID );
}

SITEMCUSTOM& CCharacterWindowCharWear::GetItem ( int nIndex )
{
	return m_ItemCustomArray[nIndex];
}

void CCharacterWindowCharWear::UnLoadItem ( int nIndex )
{
	m_ItemCustomArray[nIndex].sNativeID = NATIVEID_NULL ();
	m_pItemImage[nIndex]->ResetItem ();
}

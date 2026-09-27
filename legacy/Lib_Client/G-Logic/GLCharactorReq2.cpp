#include "stdafx.h"

#include "DxGlobalStage.h"

#include "GLCharacter.h"
#include "GLItemMan.h"
#include "GLItemMixMan.h"
#include "GLActivity.h"
#include "RanFilter.h"

#include "../Lib_ClientUI/Interface/UITextControl.h"
#include "../Lib_ClientUI/Interface/GameTextControl.h"
#include "../Lib_ClientUI/Interface/InnerInterface.h"
#include "../Lib_ClientUI/Interface/ModalWindow.h"
#include "../Lib_ClientUI/Interface/ModalCallerID.h"

#include "../Lib_ClientUI/Interface/EquipmentLockEnableWindow.h"
#include "../Lib_ClientUI/Interface/EquipmentLockInputWindow.h"
#include "../Lib_ClientUI/Interface/EquipmentLockRecoverWindow.h"

#include "../Lib_ClientUI/Interface/StorageLockEnableWindow.h"
#include "../Lib_ClientUI/Interface/StorageLockInputWindow.h"
#include "../Lib_ClientUI/Interface/StorageLockRecoverWindow.h"

#include "../Lib_ClientUI/Interface/InventoryLockEnableWindow.h"
#include "../Lib_ClientUI/Interface/InventoryLockInputWindow.h"
#include "../Lib_ClientUI/Interface/InventoryLockRecoverWindow.h"

#include "../Lib_Engine/Common/StringUtils.h"

#include "GLGaeaClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*itemfood system, Juver, 2017/05/25 */
HRESULT GLCharacter::ReqConsumeFood ( WORD wPosX, WORD wPosY )
{
	SINVENITEM* pINVENITEM = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !pINVENITEM )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMCONSUME_FOOD_FB_NOITEM") );
		return E_FAIL;
	}

	wPosX = pINVENITEM->wPosX;
	wPosY = pINVENITEM->wPosY;

	SITEM* pITEM = GLItemMan::GetInstance().GetItem ( pINVENITEM->sItemCustom.sNativeID );
	if ( !pITEM )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMCONSUME_FOOD_FB_NOITEM") );
		return E_FAIL;
	}

	if ( pITEM->sBasicOp.emItemType != ITEM_FOOD )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMCONSUME_FOOD_FB_NOITEM") );
		return E_FAIL;
	}

	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( pITEM->sSkillBookOp.sSkill_ID );
	if ( !pSKILL )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMCONSUME_FOOD_FB_NONDATA") );
		return E_FAIL;
	}

	GLMSG::SNET_INVEN_CONSUME_FOOD NetMsg;
	NetMsg.wPosX = wPosX;
	NetMsg.wPosY = wPosY;
	NETSENDTOFIELD ( &NetMsg );

	return S_OK;
}

/*itemfood system, Juver, 2017/05/26 */
HRESULT GLCharacter::ReqUnlockFoodSlot ( WORD wPosX, WORD wPosY )
{
	SINVENITEM* pINVENITEM = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !pINVENITEM )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMUNLOCK_FOOD_FB_NOITEM") );
		return E_FAIL;
	}

	wPosX = pINVENITEM->wPosX;
	wPosY = pINVENITEM->wPosY;

	SITEM* pITEM = GLItemMan::GetInstance().GetItem ( pINVENITEM->sItemCustom.sNativeID );
	if ( !pITEM )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMUNLOCK_FOOD_FB_NOITEM") );
		return E_FAIL;
	}

	if ( pITEM->sBasicOp.emItemType != ITEM_CARD_FOODUNLOCK )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMUNLOCK_FOOD_FB_NOITEM") );
		return E_FAIL;
	}

	BOOL bUNLOCKED = TRUE;
	for( int i=0; i<FITEMFACT_SIZE; ++i ){
		if ( !m_sFITEMFACT[i].bENABLE ){
			bUNLOCKED = FALSE;
			break;
		}
	}

	if ( bUNLOCKED )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMUNLOCK_FOOD_FB_UNLOCKED") );
		return E_FAIL;
	}

	GLMSG::SNET_INVEN_UNLOCK_FOOD NetMsg;
	NetMsg.wPosX = wPosX;
	NetMsg.wPosY = wPosY;
	NETSENDTOFIELD ( &NetMsg );

	return S_OK;
}

/*rightclick wear/unwear, Juver, 2017/06/23 */
HRESULT GLCharacter::ReqInvenToWear ( WORD wPosX, WORD wPosY )
{
	SINVENITEM* pInvenItem = m_cInventory.FindPosItem ( wPosX, wPosY );

	if ( !pInvenItem )						return E_FAIL;
	if ( m_sCONFTING.IsCONFRONTING() )		return E_FAIL;
	if ( ValidWindowOpen() )				return E_FAIL;	
	if ( VALID_HOLD_ITEM() )				return E_FAIL;
	if ( IsACTION(GLAT_ATTACK) || IsACTION(GLAT_SKILL) )	return E_FAIL;

	SITEM* pITEM = GLItemMan::GetInstance().GetItem( pInvenItem->sItemCustom.sNativeID );
	if ( !pITEM )		return E_FAIL;

	//check accept item
	if( pInvenItem->sItemCustom.nidDISGUISE!=SNATIVEID(false) )
	{
		if ( !ACCEPT_ITEM ( pInvenItem->sItemCustom.sNativeID, pInvenItem->sItemCustom.nidDISGUISE ) )
			return E_FAIL;
	}
	else
	{
		if ( !ACCEPT_ITEM ( pInvenItem->sItemCustom.sNativeID ) )
			return E_FAIL;
	}

	if ( m_fWearDelay < 1.0f )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("INVENTOWEAR_REQUIRE_TIME") );
		return E_FAIL;
	}

	//find compatible slot
	EMSLOT emUseSlot = SLOT_TSIZE;

	switch( pITEM->sSuitOp.emSuit )
	{
	case SUIT_HEADGEAR:	emUseSlot = SLOT_HEADGEAR;	break;
	case SUIT_UPPER:	emUseSlot = SLOT_UPPER;		break;
	case SUIT_LOWER:	emUseSlot = SLOT_LOWER;		break;
	case SUIT_HAND:		emUseSlot = SLOT_HAND;		break;
	case SUIT_FOOT:		emUseSlot = SLOT_FOOT;		break;

	case SUIT_HANDHELD:
		{
			if ( pITEM->sBasicOp.emItemType == ITEM_SUIT )
				emUseSlot = GetCurRHand();
			else emUseSlot = GetCurLHand();
		}break;

	case SUIT_NECK:		emUseSlot = SLOT_NECK;		break;
	case SUIT_WRIST:	emUseSlot = SLOT_WRIST;		break;
	case SUIT_BELT:		emUseSlot = SLOT_WAIST;		break;

	case SUIT_ACCESSORY:	
		emUseSlot = m_bAccessorySlotFirst? SLOT_LACCESSORY:SLOT_RACCESSORY;		
		break;

	case SUIT_EARRING:	emUseSlot = SLOT_LEARRING;	break;

	case SUIT_ORNAMENT:	emUseSlot = SLOT_ORNAMENT;	break;

	case SUIT_FACE:	emUseSlot = SLOT_FACE;	break;
	case SUIT_MISC:	emUseSlot = SLOT_MISC;	break;

	case SUIT_FINGER:	
		emUseSlot = m_bRingSlotFirst? SLOT_LFINGER:SLOT_RFINGER;		
		break;
	};

	if ( emUseSlot == SLOT_TSIZE )	return E_FAIL;

	/*equipment lock, Juver, 2018/01/18 */
	if ( m_bEnableEquipmentLock && m_bEquipmentLockStatus && !( emUseSlot == SLOT_LHAND || emUseSlot == SLOT_LHAND_S ) )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("ACTION_EQUIPMENT_LOCKED") );
		return E_FAIL;
	}

	if ( !CHECKSLOT_ITEM ( pITEM->sBasicOp.sNativeID, emUseSlot ) )	return E_FAIL;

	//check inven free space
	if ( pITEM->sSuitOp.emSuit==SUIT_HANDHELD && pITEM->sSuitOp.IsBOTHHAND() )
	{
		EMSLOT emRHand = GetCurRHand();
		EMSLOT emLHand = GetCurLHand();

		if ( VALID_SLOT_ITEM(emLHand) && VALID_SLOT_ITEM(emRHand) )
		{
			SITEM *pItemLeft = GLItemMan::GetInstance().GetItem(GET_SLOT_ITEM(emLHand).sNativeID);

			WORD wInvenPosX, wInvenPosY;
			BOOL bOk = m_cInventory.FindInsrtable ( pItemLeft->sBasicOp.wInvenSizeX, pItemLeft->sBasicOp.wInvenSizeY, wInvenPosX, wInvenPosY );
			if ( !bOk )
			{
				CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("INVENTOWEAR_NO_SPACE_LEFTITEM") );
				return E_FAIL;
			}
		}
	}

	m_bRingSlotFirst = !m_bRingSlotFirst;
	m_bAccessorySlotFirst = !m_bAccessorySlotFirst;

	GLMSG::SNETPC_REQ_INVEN_TO_WEAR NetMsg;
	NetMsg.wPosX = pInvenItem->wPosX;
	NetMsg.wPosY = pInvenItem->wPosY;
	NetMsg.emSlot = emUseSlot;

	NETSENDTOFIELD ( &NetMsg );

	m_fWearDelay = 0.0f;

	return S_OK;
}

/*rightclick wear/unwear, Juver, 2017/06/24 */
HRESULT GLCharacter::ReqWearToInven ( EMSLOT emSLOT )
{
	if ( !VALID_SLOT_ITEM(emSLOT) )	return E_FAIL;
	if ( ValidWindowOpen() )		return E_FAIL;	

	if ( IsACTION(GLAT_ATTACK) || IsACTION(GLAT_SKILL) )
		return E_FAIL;

	/*equipment lock, Juver, 2018/01/18 */
	if ( m_bEnableEquipmentLock && m_bEquipmentLockStatus && !( emSLOT == SLOT_LHAND || emSLOT == SLOT_LHAND_S ) )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("ACTION_EQUIPMENT_LOCKED") );
		return E_FAIL;
	}

	SITEM *pItemSlot = GLItemMan::GetInstance().GetItem(GET_SLOT_ITEM(emSLOT).sNativeID);
	if ( !pItemSlot )	return E_FAIL;

	WORD wInvenPosX, wInvenPosY;
	BOOL bOk = m_cInventory.FindInsrtable ( pItemSlot->sBasicOp.wInvenSizeX, pItemSlot->sBasicOp.wInvenSizeY, wInvenPosX, wInvenPosY );
	if ( !bOk )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("WEARTOINVEN_NO_SPACE") );
		return E_FAIL;
	}

	GLMSG::SNETPC_REQ_WEAR_TO_INVEN NetMsg;
	NetMsg.emSlot = emSLOT;
	NETSENDTOFIELD ( &NetMsg );

	return S_OK;
}

/*npc shop, Juver, 2017/07/27 */
HRESULT GLCharacter::ReqNpcShopPurchaseReset()
{
	m_sNPCShopCrow = NATIVEID_NULL();
	m_sNPCShopItem = NATIVEID_NULL();
	m_wNPCShopType = 0;

	return S_OK;
}

/*npc shop, Juver, 2017/07/27 */
HRESULT GLCharacter::ReqNpcShopPurchase( SNATIVEID sidCrow, SNATIVEID sidItem, WORD wShopType )
{
	if ( m_fNPCShopDelay < NPC_SHOP_DELAY )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMNPCSHOP_PURCHASE_FB_DELAY") );
		return E_FAIL;
	}

	SITEM* pitem = GLItemMan::GetInstance().GetItem ( sidItem );
	if ( !pitem )	
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMNPCSHOP_PURCHASE_FB_INVALID_ITEM") );
		return E_FAIL;
	}

	SCROWDATA*	pcrow = GLCrowDataMan::GetInstance().GetCrowData( sidCrow );
	if ( !pcrow )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMNPCSHOP_PURCHASE_FB_INVALID_CROW") );
		return E_FAIL;
	}

	SNPC_SHOP& npc_shop = pcrow->m_sNPCShop;

	if ( npc_shop.wShopType != wShopType )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMNPCSHOP_PURCHASE_FB_INVALID_SHOPTYPE") );
		return E_FAIL;
	}

	SNPC_SHOP_ITEM* pshopitem = npc_shop.findShopItem( sidItem.dwID );
	if ( !pshopitem )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMNPCSHOP_PURCHASE_FB_INVALID_SHOPITEM") );
		return E_FAIL;
	}

	//player money check
	LONGLONG dwPrice = pitem->sBasicOp.dwBuyPrice;
	if ( m_lnMoney < (LONGLONG)dwPrice )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMNPCSHOP_PURCHASE_FB_NOT_MONEY") );
		return E_FAIL;
	}

	/*contribution point, Juver, 2017/08/23 */
	if ( pitem->sBasicOp.dwReqContributionPoint != 0 &&
		m_llContributionPoint < (LONGLONG)pitem->sBasicOp.dwReqContributionPoint )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("BUYITEM_NOCONTRIBUTIONPOINT") );
		return E_FAIL;
	}

	/*activity point, Juver, 2017/08/23 */
	if ( pitem->sBasicOp.dwReqActivityPoint != 0 && 
		m_dwActivityPoint < pitem->sBasicOp.dwReqActivityPoint )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("BUYITEM_NOPOINT") );
		return E_FAIL;
	}

	WORD winvensizex = pitem->sBasicOp.wInvenSizeX;
	WORD winvensizey = pitem->sBasicOp.wInvenSizeY;

	//inventory space check
	BOOL bITEM_SPACE(FALSE);
	if ( pitem->ISPILE() )
	{	
		WORD wPILENUM = pitem->sDrugOp.wPileNum;
		WORD wREQINSRTNUM = ( 1 * pitem->GETAPPLYNUM() );
		bITEM_SPACE = m_cInventory.ValidPileInsrt ( wREQINSRTNUM, sidItem, wPILENUM, winvensizex, winvensizey );
	}else{
		WORD wInsertPosX(0), wInsertPosY(0);
		bITEM_SPACE = m_cInventory.FindInsrtable ( winvensizex, winvensizey, wInsertPosX, wInsertPosY );
	}

	if ( !bITEM_SPACE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMNPCSHOP_PURCHASE_FB_NOT_INVEN_SPACE") );
		return E_FAIL;
	}


	m_sNPCShopCrow = sidCrow;
	m_sNPCShopItem = sidItem;
	m_wNPCShopType = wShopType;

	if ( RANPARAM::bNPCShopConfirm )
	{
		CString strQuestion = CInnerInterface::GetInstance().MakeString( ID2GAMEINTEXT("NPCSHOP_PURCHASE_QUESTION"), pitem->GetName() );
		DoModal( strQuestion.GetString(),  MODAL_INFOMATION, YESNO, MODAL_NPCSHOP_PURCHASE );		
	}
	else
	{
		ReqNpcShopDoPurchase();
	}
	
	return S_OK;
}

/*npc shop, Juver, 2017/07/27 */
HRESULT GLCharacter::ReqNpcShopDoPurchase()
{
	if ( m_wNPCShopType == 0 )
	{
		GLMSG::SNETPC_REQ_NPCSHOP_PURCHASE_MONEY NetMsg;
		NetMsg.sidCrow = m_sNPCShopCrow;
		NetMsg.sidItem = m_sNPCShopItem;
		NetMsg.wShopType = m_wNPCShopType;
		NETSENDTOFIELD ( &NetMsg );
	}

	m_fNPCShopDelay = 0.0f;

	return S_OK;
}

/*vehicle booster system, Juver, 2017/08/10 */
void GLCharacter::ReqVehicleEnableBooster( WORD wPosX, WORD wPosY )
{
	SINVENITEM* pInvenItem = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !pInvenItem ) return;

	wPosX = pInvenItem->wPosX;
	wPosY = pInvenItem->wPosY;

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) return;

	if ( pItem->sBasicOp.emItemType != ITEM_VEHICLE )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_ITEM") );
		return;
	}

	SITEM* pHold = GET_SLOT_ITEMDATA ( SLOT_HOLD );
	if ( !pHold ) return;

	if ( pHold->sBasicOp.emItemType != ITEM_CARD_BIKEBOOST )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_CARD") );
		return;
	}

	if ( pInvenItem->sItemCustom.dwVehicleID == 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_ITEM") );
		return;
	}

	if ( pHold->sVehicle.emVehicleType != pItem->sVehicle.emVehicleType )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_TYPE") );
		return;
	}

	if ( pHold->sVehicle.emVehicleSubType != pItem->sVehicle.emVehicleSubType )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_TYPE") );
		return;
	}

	SVEHICLEITEMINFO sVehicle;

	if ( !DxGlobalStage::GetInstance().IsEmulator() )
	{
		VEHICLEITEMINFO_MAP_ITER iter = m_mapVEHICLEItemInfo.find ( pInvenItem->sItemCustom.dwVehicleID );
		if ( iter==m_mapVEHICLEItemInfo.end() ) return;
		sVehicle = (*iter).second;
	}
	else
	{
		sVehicle.m_bBooster = m_sVehicle.m_bBooster;
	}

	if ( sVehicle.m_bBooster ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMVEHICLE_REQ_ENABLE_BOOSTER_FB_ALREADY_ENABLED") );
		return;
	}

	GLMSG::SNET_VEHICLE_REQ_ENABLE_BOOSTER NetMsg;
	NetMsg.wPosX = wPosX;
	NetMsg.wPosY = wPosY;

	NETSENDTOFIELD ( &NetMsg );
}

/*item exchange, Juver, 2017/10/13 */
HRESULT GLCharacter::ReqNPCItemExchange( SNATIVEID sidNPC, WORD wExchangeID )
{
	if ( VALID_HOLD_ITEM() )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMNPC_ITEM_EXCHANGE_FB_INVALID_ITEM") );
		return E_FAIL;
	}

	if ( m_fNPCItemExchangeDelay < NPC_ITEM_EXCHANGE_DELAY )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMNPC_ITEM_EXCHANGE_FB_DELAY") );
		return E_FAIL;
	}

	PCROWDATA pcrow_data = GLCrowDataMan::GetInstance().GetCrowData( sidNPC );
	if ( !pcrow_data )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMNPC_ITEM_EXCHANGE_FB_INVALID_NPC") );
		return E_FAIL;
	}

	SNPC_ITEM_EXCHANGE& npc_item_exchange = pcrow_data->m_sNPCItemExchange;
	SNPC_ITEM_EXCHANGE_DATA* pnpc_item_exchange_data = npc_item_exchange.findData( wExchangeID );
	if ( !pnpc_item_exchange_data )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMNPC_ITEM_EXCHANGE_FB_INVALID_ID") );
		return E_FAIL;
	}

	SITEM* pitem_data_result = GLItemMan::GetInstance().GetItem( pnpc_item_exchange_data->sidItemResult.sidItem );
	if ( !pitem_data_result )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMNPC_ITEM_EXCHANGE_FB_INVALID_ITEM") );
		return E_FAIL;
	}

	/*contribution point, Juver, 2017/08/23 */
	/*if ( pitem_data_result->sBasicOp.dwReqContributionPoint != 0 &&
		m_llContributionPoint < (LONGLONG)pitem_data_result->sBasicOp.dwReqContributionPoint )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMNPC_ITEM_EXCHANGE_FB_NOCONTRIBUTIONPOINT") );
		return E_FAIL;
	}*/

	/*activity point, Juver, 2017/08/23 */
	if ( pitem_data_result->sBasicOp.dwReqActivityPoint != 0 && 
		m_dwActivityPoint < pitem_data_result->sBasicOp.dwReqActivityPoint )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMNPC_ITEM_EXCHANGE_FB_NOPOINT") );
		return E_FAIL;
	}

	WORD winvensizex = pitem_data_result->sBasicOp.wInvenSizeX;
	WORD winvensizey = pitem_data_result->sBasicOp.wInvenSizeY;

	//inventory space check
	BOOL bITEM_SPACE(FALSE);
	if ( pitem_data_result->ISPILE() )
	{	
		WORD wPILENUM = pitem_data_result->sDrugOp.wPileNum;
		WORD wREQINSRTNUM = ( pnpc_item_exchange_data->sidItemResult.wItemNum * pitem_data_result->GETAPPLYNUM() );
		bITEM_SPACE = m_cInventory.ValidPileInsrt ( wREQINSRTNUM, pnpc_item_exchange_data->sidItemResult.sidItem, wPILENUM, winvensizex, winvensizey );
	}else{
		WORD wInsertPosX(0), wInsertPosY(0);
		bITEM_SPACE = m_cInventory.FindInsrtable ( winvensizex, winvensizey, wInsertPosX, wInsertPosY );
	}

	if ( !bITEM_SPACE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMNPC_ITEM_EXCHANGE_FB_NOT_INVEN_SPACE") );
		return E_FAIL;
	}


	//requirements check
	for( int i=0; i<NPC_ITEM_EXCHANGE_MAX_REQ; ++i )
	{
		SITEM* pitem_data_require = GLItemMan::GetInstance().GetItem( pnpc_item_exchange_data->sidItemRequire[i].sidItem );
		if ( pitem_data_require )
		{
			if ( pitem_data_require->ISPILE() )
			{
				WORD wNum = pnpc_item_exchange_data->sidItemRequire[i].wItemNum;
				WORD witem_num = m_cInventory.CountTurnItem( pitem_data_require->sBasicOp.sNativeID );
				if ( witem_num < wNum )
				{
					CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMNPC_ITEM_EXCHANGE_FB_REQUIRE_MISSING") );
					return E_FAIL;
				}
			}
			else
			{
				//not pile just check if exist
				SINVENITEM* pinvenitem = m_cInventory.FindItem( pitem_data_require->sBasicOp.sNativeID );
				if ( !pinvenitem )
				{
					CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMNPC_ITEM_EXCHANGE_FB_REQUIRE_MISSING") );
					return E_FAIL;
				}
			}
		}
	}
	
	m_fNPCItemExchangeDelay = 0.0f;

	GLMSG::SNETPC_REQ_NPC_ITEM_EXCHANGE_TRADE NetMsg;
	NetMsg.sid_NPC = sidNPC;
	NetMsg.wTradeID = wExchangeID;;
	NETSENDTOFIELD ( &NetMsg );

	return S_OK;
}

/*product item, Juver, 2017/10/17 */
bool GLCharacter::ValidItemCompoundOpen()
{
	if ( CInnerInterface::GetInstance().IsVisibleGroup( PRODUCT_WINDOW ) )
		return true;
	return false;
}

/*product item, Juver, 2017/10/18 */
void GLCharacter::ItemCompound( DWORD dwID, int nProductCount )
{
	m_dwProductID = dwID;
	m_nProductCount = nProductCount;

	DoModal( ID2GAMEINTEXT("RAN_PPRODUCT_BUTTON_PRODUCE_QUESTION"),  MODAL_INFOMATION, YESNO, MODAL_ITEM_PRODUCT_PRODUCE );		
}

void GLCharacter::ItemCompoundCancel()
{
	GLMSG::SNETPC_REQ_ITEM_COMPOUND_CANCEL NetMsg;
	NETSENDTOFIELD ( &NetMsg );
}

/*product item, Juver, 2017/10/17 */
HRESULT GLCharacter::ReqItemCompound( DWORD dwID, int nProductCount )
{
	if ( m_bItemCompoundTask )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("RAN_PRODUCT_FB_TASK_RUNNING") );
		return E_FAIL;
	}

	const ITEM_MIX* pitem_mix = GLItemMixMan::GetInstance().GetProduct( dwID );
	if ( !pitem_mix )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("RAN_PRODUCT_FB_INVALID_ID") );
		return E_FAIL;
	}

	if ( VALID_HOLD_ITEM() )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("RAN_PRODUCT_FB_RESULT_ITEM_INVALID") );
		return E_FAIL;
	}

	//money check
	if( m_lnMoney < ( pitem_mix->dwPrice * nProductCount ) )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("RAN_PRODUCT_FB_INVALID_COST") );
		return E_FAIL;
	}

	//level check
	if ( m_wLevel < pitem_mix->wLevelReq )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("RAN_PRODUCT_FB_INVALID_CHAR_LEVEL") );
		return E_FAIL;
	}

	//result item check
	SITEM* pitem_data_result = GLItemMan::GetInstance().GetItem( pitem_mix->sResultItem.sNID );
	if ( !pitem_data_result )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("RAN_PRODUCT_FB_RESULT_ITEM_INVALID") );
		return E_FAIL;
	}

	WORD winvensizex = pitem_data_result->sBasicOp.wInvenSizeX;
	WORD winvensizey = pitem_data_result->sBasicOp.wInvenSizeY;

	//inventory space check
	BOOL bITEM_SPACE(FALSE);
	if ( pitem_data_result->ISPILE() )
	{	
		WORD wPILENUM = pitem_data_result->sDrugOp.wPileNum;
		int nNum = pitem_mix->sResultItem.nNum * nProductCount;
		WORD wREQINSRTNUM = ( nNum * pitem_data_result->GETAPPLYNUM() );
		bITEM_SPACE = m_cInventory.ValidPileInsrt ( wREQINSRTNUM, pitem_mix->sResultItem.sNID, wPILENUM, winvensizex, winvensizey );
	}else{
		WORD wInsertPosX(0), wInsertPosY(0);
		bITEM_SPACE = m_cInventory.FindInsrtable ( winvensizex, winvensizey, wInsertPosX, wInsertPosY );
	}

	if ( !bITEM_SPACE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("RAN_PRODUCT_FB_RESULT_ITEM_NO_SPACE") );
		return E_FAIL;
	}

	//requirements check
	for ( int i=0; i<ITEMMIX_ITEMNUM; ++i )
	{	
		SITEM* pitem_data_require = GLItemMan::GetInstance().GetItem( pitem_mix->sMeterialItem[i].sNID );
		if ( pitem_data_require )
		{
			if ( pitem_data_require->ISPILE() )
			{
				WORD wNum = ( pitem_mix->sMeterialItem[i].nNum * nProductCount );
				WORD witem_num = m_cInventory.CountTurnItem( pitem_data_require->sBasicOp.sNativeID );
				if ( witem_num < wNum )
				{
					CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("RAN_PRODUCT_FB_MATERIAL") );
					return E_FAIL;
				}
			}
			else
			{
				//not pile just check if exist
				SINVENITEM* pinvenitem = m_cInventory.FindItem( pitem_data_require->sBasicOp.sNativeID );
				if ( !pinvenitem )
				{
					CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("RAN_PRODUCT_FB_MATERIAL") );
					return E_FAIL;
				}

				if ( pinvenitem->sItemCustom.nidDISGUISE !=SNATIVEID(false) )
				{
					CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("RAN_PRODUCT_FB_FAIL_COSTUME") );
					return E_FAIL;
				}
			}
		}
	}

	GLMSG::SNETPC_REQ_ITEM_COMPOUND_START NetMsg;
	NetMsg.dwProductID = dwID;
	NetMsg.nProductCount = nProductCount;
	NETSENDTOFIELD ( &NetMsg );

	return S_OK;
}

/*activity system, Juver, 2017/11/05 */
void GLCharacter::ReqChangeBadge( DWORD dwBadgeID )
{
	if ( m_fCharTitleDelay < CHARACTER_BADGE_TIME )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_CHAR_TITLE_DELAY") );
		return;
	}

	SACTIVITY_FILE_DATA* pactivity_file_data = GLActivity::GetInstance().GetActivity( dwBadgeID );
	if ( pactivity_file_data )
	{
		if ( !pactivity_file_data->bRewardBadge )
		{
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_CHAR_TITLE_FAIL") );
			return;
		}

		if ( strcmp(m_szBadge, pactivity_file_data->strBadgeString.c_str() ) == 0 )
		{
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_CHAR_TITLE_SAME") );
			return;
		}
	}

	m_fCharTitleDelay = 0.0f;

	GLMSG::SNETPC_REQ_CHARACTER_BADGE_CHANGE NetMsg;
	NetMsg.dwBadgeID = dwBadgeID;
	NETSENDTOFIELD ( &NetMsg );
}

/*charinfoview , Juver, 2017/11/11 */
void GLCharacter::RequestCharacterInfo( DWORD dwGaeaID )
{
	if ( !RANPARAM::bFeatureViewCharInfo )	return;

	PGLCHARCLIENT pChar = GLGaeaClient::GetInstance().GetChar( dwGaeaID );
	if ( !pChar )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_SHOW_CHARACTER_INVALID_TARGET") );
		return;
	}

	if ( m_fReqCharInfoTimer < REQ_CHARINFO_DELAY )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_SHOW_CHARACTER_TIMEDELAY") );
		return;
	}

	m_fReqCharInfoTimer = 0.0f;

	GLMSG::SNETPC_REQ_CHARINFO NetMsg;
	NetMsg.dwGaeaID = dwGaeaID;
	NETSENDTOFIELD ( &NetMsg );
}

/*bike color , Juver, 2017/11/12 */
void GLCharacter::ReqVehicleColor( WORD wPosX, WORD wPosY )
{
	SINVENITEM* pInvenItem = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !pInvenItem )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_BIKE_COLOR_NOT_CARD") );
		return;
	}

	wPosX = pInvenItem->wPosX;
	wPosY = pInvenItem->wPosY;

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_BIKE_COLOR_NOT_CARD") );
		return;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_CARD_BIKECOLOR )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_BIKE_COLOR_NOT_CARD") );
		return;
	}

	if ( !m_bVehicle )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_BIKE_COLOR_NOT_VEHICLE") );
		return;
	}

	SITEM* pVehicle = GET_SLOT_ITEMDATA( SLOT_VEHICLE );
	if ( !pVehicle )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_BIKE_COLOR_NOT_VEHICLE") );
		return;
	}

	if ( pVehicle->sVehicle.emVehicleType != VEHICLE_TYPE_BIKE )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_BIKE_COLOR_NOT_BIKE") );
		return;
	}

	for( int i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
		m_wBikeColor[i] = m_sVehicle.m_wColor[i];

	m_bEnableBikeColor = TRUE;

	CInnerInterface::GetInstance().ShowGroupFocus( BIKE_COLOR_WINDOW );
}

/*bike color , Juver, 2017/11/13 */
void GLCharacter::ReqBikeColorChange()
{
	SINVENITEM*	 pInvenItem = m_cInventory.FindItem( ITEM_CARD_BIKECOLOR );
	if ( !pInvenItem )	return;

	if ( !m_bVehicle )	return;
	if ( !m_bEnableBikeColor )	return;

	GLMSG::SNET_VEHICLE_REQ_CHANGE_COLOR NetMsg;
	for( int i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
		NetMsg.wColor[i] = m_wBikeColor[i];

	NETSENDTOFIELD ( &NetMsg );
}


/*rv card, Juver, 2017/11/25 */
HRESULT GLCharacter::ReqRandomOptionChange ( WORD wPosX, WORD wPosY )
{
	if ( !IsValidBody() )						return E_FAIL;
	if ( GLTradeClient::GetInstance().Valid() )	return E_FAIL;

	SINVENITEM* ptarget_inven_item = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !ptarget_inven_item )	
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EM_RANDOM_OPTION_CHANGE_INVALID_TARGET_ITEM") );
		return S_FALSE;
	}

	wPosX = ptarget_inven_item->wPosX;
	wPosY = ptarget_inven_item->wPosY;

	SITEM* pitem_data_target = GLItemMan::GetInstance().GetItem ( ptarget_inven_item->sItemCustom.sNativeID );
	if ( !pitem_data_target )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EM_RANDOM_OPTION_CHANGE_INVALID_TARGET_ITEM") );
		return S_FALSE;
	}

	SITEM* pitem_data_hold = GET_SLOT_ITEMDATA ( SLOT_HOLD );
	if ( !pitem_data_hold )	return S_FALSE;

	if ( pitem_data_hold->sBasicOp.emItemType != ITEM_RANDOM_OPTION_CARD )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EM_RANDOM_OPTION_CHANGE_INVALID_CARD_ITEM") );
		return S_FALSE;
	}

	if ( pitem_data_target->sBasicOp.emItemType != ITEM_SUIT )
	{	
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EM_RANDOM_OPTION_CHANGE_TARGET_NOT_SUIT") );
		return S_FALSE;
	}

	if ( !pitem_data_target->sRandomOpt.bPVPItem )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EM_RANDOM_OPTION_CHANGE_SUIT_MISMATCH") );
		return S_FALSE;
	}

	if ( !pitem_data_hold->sRvCard.bUseSuit[pitem_data_target->sSuitOp.emSuit] )
	{	
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EM_RANDOM_OPTION_CHANGE_SUIT_MISMATCH") );
		return S_FALSE;
	}
	
	if ( pitem_data_hold->sRvCard.bReplaceOpt )
	{
		if ( ptarget_inven_item->sItemCustom.cOptTYPE1 == (BYTE) pitem_data_hold->sRvCard.emOption 
			&& ptarget_inven_item->sItemCustom.nOptVALUE1 >= (short) pitem_data_hold->sRvCard.wValue )
		{
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EM_RANDOM_OPTION_CHANGE_TARGET_OPTION_ALREADY_SET") );
			return S_FALSE;
		}

		if ( ptarget_inven_item->sItemCustom.cOptTYPE2 == (BYTE) pitem_data_hold->sRvCard.emOption 
			&& ptarget_inven_item->sItemCustom.nOptVALUE2 >= (short) pitem_data_hold->sRvCard.wValue )
		{
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EM_RANDOM_OPTION_CHANGE_TARGET_OPTION_ALREADY_SET") );
			return S_FALSE;
		}

		if ( ptarget_inven_item->sItemCustom.cOptTYPE3 == (BYTE) pitem_data_hold->sRvCard.emOption 
			&& ptarget_inven_item->sItemCustom.nOptVALUE3 >= (short) pitem_data_hold->sRvCard.wValue )
		{
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EM_RANDOM_OPTION_CHANGE_TARGET_OPTION_ALREADY_SET") );
			return S_FALSE;
		}

		if ( ptarget_inven_item->sItemCustom.cOptTYPE4 == (BYTE) pitem_data_hold->sRvCard.emOption 
			&& ptarget_inven_item->sItemCustom.nOptVALUE4 >= (short) pitem_data_hold->sRvCard.wValue )
		{
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EM_RANDOM_OPTION_CHANGE_TARGET_OPTION_ALREADY_SET") );
			return S_FALSE;
		}
	}


	if ( pitem_data_hold->sRvCard.bCheckExist )
	{
		BOOL bfound_option = FALSE;

		if ( ptarget_inven_item->sItemCustom.cOptTYPE1 == (BYTE) pitem_data_hold->sRvCard.emOption )	bfound_option = TRUE;
		if ( ptarget_inven_item->sItemCustom.cOptTYPE2 == (BYTE) pitem_data_hold->sRvCard.emOption )	bfound_option = TRUE;
		if ( ptarget_inven_item->sItemCustom.cOptTYPE3 == (BYTE) pitem_data_hold->sRvCard.emOption )	bfound_option = TRUE;
		if ( ptarget_inven_item->sItemCustom.cOptTYPE4 == (BYTE) pitem_data_hold->sRvCard.emOption )	bfound_option = TRUE;

		if ( !bfound_option )
		{
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EM_RANDOM_OPTION_CHANGE_TARGET_OPTION_NOT_EXIST") );
			return S_FALSE;
		}
	}
	else
	{
		BOOL bfound_free = FALSE;

		if ( ptarget_inven_item->sItemCustom.cOptTYPE1 == EMR_OPT_NULL )	bfound_free = TRUE;
		if ( ptarget_inven_item->sItemCustom.cOptTYPE2 == EMR_OPT_NULL )	bfound_free = TRUE;
		if ( ptarget_inven_item->sItemCustom.cOptTYPE3 == EMR_OPT_NULL )	bfound_free = TRUE;
		if ( ptarget_inven_item->sItemCustom.cOptTYPE4 == EMR_OPT_NULL )	bfound_free = TRUE;

		if ( ptarget_inven_item->sItemCustom.cOptTYPE1 == (BYTE) pitem_data_hold->sRvCard.emOption )	bfound_free = TRUE;
		if ( ptarget_inven_item->sItemCustom.cOptTYPE2 == (BYTE) pitem_data_hold->sRvCard.emOption )	bfound_free = TRUE;
		if ( ptarget_inven_item->sItemCustom.cOptTYPE3 == (BYTE) pitem_data_hold->sRvCard.emOption )	bfound_free = TRUE;
		if ( ptarget_inven_item->sItemCustom.cOptTYPE4 == (BYTE) pitem_data_hold->sRvCard.emOption )	bfound_free = TRUE;

		if ( !bfound_free )
		{
			CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EM_RANDOM_OPTION_CHANGE_TARGET_OPTION_NOT_FREE") );
			return S_FALSE;
		}
	}


	GLMSG::SNET_INVEN_RANDOM_OPTION_CHANGE net_msg_field;
	net_msg_field.wPosX = wPosX;
	net_msg_field.wPosY = wPosY;

	NETSENDTOFIELD ( &net_msg_field );

	return S_OK;
}

/*nondrop card, Juver, 2017/11/26 */
HRESULT GLCharacter::ReqItemNonDropCard ( WORD wPosX, WORD wPosY )
{
	if ( !IsValidBody() )						return E_FAIL;
	if ( GLTradeClient::GetInstance().Valid() )	return E_FAIL;

	SINVENITEM* ptarget_inven_item = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !ptarget_inven_item )	
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EM_NONDROP_CARD_INVALID_TARGET_ITEM") );
		return S_FALSE;
	}

	wPosX = ptarget_inven_item->wPosX;
	wPosY = ptarget_inven_item->wPosY;

	SITEM* pitem_data_target = GLItemMan::GetInstance().GetItem ( ptarget_inven_item->sItemCustom.sNativeID );
	if ( !pitem_data_target )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EM_NONDROP_CARD_INVALID_TARGET_ITEM") );
		return S_FALSE;
	}

	SITEM* pitem_data_hold = GET_SLOT_ITEMDATA ( SLOT_HOLD );
	if ( !pitem_data_hold )	return S_FALSE;

	if ( pitem_data_hold->sBasicOp.emItemType != ITEM_NONDROP_CARD )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EM_NONDROP_CARD_INVALID_CARD_ITEM") );
		return S_FALSE;
	}

	if ( pitem_data_target->sBasicOp.emItemType != ITEM_SUIT )
	{	
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EM_NONDROP_CARD_TARGET_NOT_SUIT") );
		return S_FALSE;
	}

	if ( ptarget_inven_item->sItemCustom.IsGM_GENITEM() )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EM_NONDROP_CARD_TARGET_ALREADY") );
		return S_FALSE;
	}
	
	GLMSG::SNET_INVEN_NONDROP_CARD net_msg_field;
	net_msg_field.wPosX = wPosX;
	net_msg_field.wPosY = wPosY;

	NETSENDTOFIELD ( &net_msg_field );

	return S_OK;
}

/*regional chat, Juver, 2017/12/06 */
BOOL GLCharacter::RegionalChatPay ()
{
	if ( !IsValidBody() )		return FALSE;

	DWORD dwCost = RANPARAM::dwFeatureRegionalChatCost;
	if ( dwCost <= 0 )	return TRUE;
	
	if( m_lnMoney < dwCost )	return FALSE;

	GLMSG::SNET_REGIONAL_CHAT_PAY net_msg_field;
	net_msg_field.dwCost = dwCost;
	NETSENDTOFIELD ( &net_msg_field );

	return TRUE;
}

/*change scale card, Juver, 2018/01/03 */
void GLCharacter::ReqChangeScale( WORD wPosX, WORD wPosY )
{
	SINVENITEM* pInvenItem = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !pInvenItem )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_CHANGE_SCALE_NOT_CARD") );
		return;
	}

	wPosX = pInvenItem->wPosX;
	wPosY = pInvenItem->wPosY;

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_CHANGE_SCALE_NOT_CARD") );
		return;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_CHANGE_SCALE_CARD )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_CHANGE_SCALE_NOT_CARD") );
		return;
	}

	m_fActiveCharScale = m_fScaleRange;
	m_bChangeCharScale = TRUE;

	CInnerInterface::GetInstance().ShowGroupFocus( CHANGE_SCALE_WINDOW );
}

/*change scale card, Juver, 2018/01/03 */
void GLCharacter::ChangeScaleCancel()
{
	m_bChangeCharScale = FALSE;
	m_fActiveCharScale = 1.0f;
}

/*change scale card, Juver, 2018/01/04 */
void GLCharacter::ChangeScaleSave()
{
	SINVENITEM*	 pInvenItem = m_cInventory.FindItem( ITEM_CHANGE_SCALE_CARD );
	if ( !pInvenItem )	return;

	if ( !m_bChangeCharScale )	return;

	GLMSG::SNETPC_INVEN_SCALE_CHANGE NetMsg;
	NetMsg.fScale = m_fActiveCharScale;
	NETSENDTOFIELD ( &NetMsg );
}

BOOL GLCharacter::InvenUseToPutOn( EMSLOT emSlot )
{
	if ( !IsValidBody() )		return FALSE;
	if ( ValidWindowOpen() )	return FALSE;	
	if ( !VALID_HOLD_ITEM() )	return FALSE;	
	if ( emSlot >= SLOT_TSIZE )	return FALSE;
	
	SITEM* pitem_data_hold = GET_SLOT_ITEMDATA( SLOT_HOLD );
	if ( !pitem_data_hold )	return FALSE;

	SITEM* pitem_data_slot = GET_SLOT_ITEMDATA( emSlot );
	if ( !pitem_data_slot )	return FALSE;

	if ( !SIMPLE_CHECK_ITEM( pitem_data_hold->sBasicOp.sNativeID ) )	return TRUE;
	if ( CheckCoolTime( pitem_data_hold->sBasicOp.sNativeID ) )			return TRUE;

	switch( pitem_data_hold->sBasicOp.emItemType )
	{
	case ITEM_COSTUME_COLOR:
		{
			/*item color, Juver, 2018/01/10 */
			ReqItemColorChange( emSlot );
		}break;

	default:
		{
			return FALSE;
		}break;
	};


	WORD wInvenPosX(0);
	WORD wInvenPosY(0);
	BOOL bOk = m_cInventory.FindInsrtable ( pitem_data_hold->sBasicOp.wInvenSizeX, pitem_data_hold->sBasicOp.wInvenSizeY, wInvenPosX, wInvenPosY );
	if ( !bOk )	return FALSE;

	GLMSG::SNETPC_REQ_HOLD_TO_INVEN NetMsg;
	NetMsg.wPosX = wInvenPosX;
	NetMsg.wPosY = wInvenPosY;
#if defined(VN_PARAM) //vietnamtest%%%
	NetMsg.bUseVietnamInven = GET_HOLD_ITEM().bVietnamGainItem;
#else
	NetMsg.bUseVietnamInven = FALSE;
#endif
	NETSENDTOFIELD ( &NetMsg );

	return TRUE;
}

/*item color, Juver, 2018/01/10 */
void GLCharacter::ReqItemColorChange( EMSLOT emSlot )
{
	if ( emSlot >= SLOT_TSIZE )	return;
	
	SITEM* pitem_data_hold = GET_SLOT_ITEMDATA( SLOT_HOLD );
	if ( !pitem_data_hold )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_COLOR_CHANGE_NOT_CARD") );
		return;
	}

	if ( pitem_data_hold->sBasicOp.emItemType != ITEM_COSTUME_COLOR )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_COLOR_CHANGE_NOT_CARD") );
		return;
	}

	SITEM* pitem_data_slot = GET_SLOT_ITEMDATA( emSlot );
	if ( !pitem_data_slot )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_COLOR_CHANGE_SLOT_ERROR") );
		return;
	}

	if ( GET_SLOT_ITEM(emSlot).nidDISGUISE == NATIVEID_NULL() )
	{
		if ( !pitem_data_slot->sBasicOp.bItemColor )
		{
			CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_COLOR_CHANGE_SLOT_ERROR") );
			return;
		}
	}
	else
	{
		SITEM* pitem_data_disguise = GLItemMan::GetInstance().GetItem( GET_SLOT_ITEM(emSlot).nidDISGUISE );
		if ( !pitem_data_disguise )
		{
			CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_COLOR_CHANGE_SLOT_ERROR") );
			return;
		}

		if ( !pitem_data_disguise->sBasicOp.bItemColor )
		{
			CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_COLOR_CHANGE_SLOT_ERROR") );
			return;
		}
	}

	
	m_bEnableItemColor = TRUE;
	m_emSlotItemColor = emSlot;
	m_wItemColor1 = GET_SLOT_ITEM( m_emSlotItemColor ).wColor1;
	m_wItemColor2 = GET_SLOT_ITEM( m_emSlotItemColor ).wColor2;
	CInnerInterface::GetInstance().ShowGroupFocus( ITEM_COLOR_WINDOW );
}

/*item color, Juver, 2018/01/10 */
void GLCharacter::ItemColorEnd()
{
	m_bEnableItemColor = FALSE;
	m_wItemColor1 = ITEMCOLOR_WHITE;
	m_wItemColor2 = ITEMCOLOR_WHITE;
	m_emSlotItemColor = SLOT_TSIZE;

	if ( m_pd3dDevice )
		UpdateSuit();
}

/*item color, Juver, 2018/01/10 */
void GLCharacter::ItemColorUpdate()
{
	EMPIECECHAR empiece = SLOT_2_PIECE ( m_emSlotItemColor );

	if ( empiece >= PIECE_SIZE )	return;

	if ( m_pSkinChar )
	{
		m_pSkinChar->SetColor1( empiece, m_wItemColor1 );
		m_pSkinChar->SetColor2( empiece, m_wItemColor2 );
	}
}

/*item color, Juver, 2018/01/10 */
void GLCharacter::ItemColorChange()
{
	if ( m_emSlotItemColor >= SLOT_TSIZE )	return;

	SITEM* pitem_data_slot = GET_SLOT_ITEMDATA( m_emSlotItemColor );
	if ( !pitem_data_slot )	return;

	if ( GET_SLOT_ITEM(m_emSlotItemColor).nidDISGUISE == NATIVEID_NULL() )
	{
		if ( !pitem_data_slot->sBasicOp.bItemColor )	return;
	}
	else
	{
		SITEM* pitem_data_disguise = GLItemMan::GetInstance().GetItem( GET_SLOT_ITEM(m_emSlotItemColor).nidDISGUISE );
		if ( !pitem_data_disguise )	return;
		if ( !pitem_data_disguise->sBasicOp.bItemColor )	return;
	}

	SINVENITEM*	 pinvenitem = m_cInventory.FindItem( ITEM_COSTUME_COLOR );
	if ( !pinvenitem )	return;

	GLMSG::SNETPC_INVEN_ITEMCOLOR_CHANGE NetMsg;
	NetMsg.emSlot = m_emSlotItemColor;
	NetMsg.wColor1 = m_wItemColor1;
	NetMsg.wColor2 = m_wItemColor2;
	NETSENDTOFIELD ( &NetMsg );
}

/*item wrapper, Juver, 2018/01/11 */
void GLCharacter::ReqItemWrap( WORD wPosX, WORD wPosY )
{
	if ( !IsValidBody() )						return;
	if ( GLTradeClient::GetInstance().Valid() )	return;

	SINVENITEM* pinven_item_target = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !pinven_item_target )	return;

	wPosX = pinven_item_target->wPosX;
	wPosY = pinven_item_target->wPosY;

	SITEM* pitem_data_target = GLItemMan::GetInstance().GetItem ( pinven_item_target->sItemCustom.sNativeID );
	if ( !pitem_data_target )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_WRAP_TARGET_NOT_VALID") );
		return;
	}

	if ( !pinven_item_target->sItemCustom.IsGM_GENITEM() && pitem_data_target->sBasicOp.IsEXCHANGE() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_WRAP_TARGET_NO_NEED") );
		return;
	}

	if ( !pitem_data_target->isWrappable() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_WRAP_TARGET_NOT_VALID") );
		return;
	}

	if ( pinven_item_target->sItemCustom.nidDISGUISE != NATIVEID_NULL() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_WRAP_TARGET_HAVE_DISGUISE") );
		return;
	}

	SITEM* pitem_data_hold = GET_SLOT_ITEMDATA ( SLOT_HOLD );
	if ( !pitem_data_hold )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_WRAP_HOLD_INVALID") );
		return;
	}

	if ( pitem_data_hold->sBasicOp.emItemType != ITEM_WRAPPER )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_WRAP_HOLD_NOT_WRAPPER") );
		return;
	}

	SITEM* phold_item_box = GLItemMan::GetInstance().GetItem( pitem_data_hold->sBasicOp.sidWrapperBox );
	if ( !phold_item_box )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_WRAP_HOLD_NOT_BOX") );
		return;
	}

	DoModal( ID2GAMEINTEXT("MODAL_WRAP_ITEM"),  MODAL_INFOMATION, YESNO, MODAL_WRAP_ITEM );	

	m_wInvenPosX3 = wPosX;
	m_wInvenPosY3 = wPosY;
}

/*item wrapper, Juver, 2018/01/11 */
void GLCharacter::ReqItemUnwrap( WORD wPosX, WORD wPosY )
{
	if ( !IsValidBody() )						return;
	if ( GLTradeClient::GetInstance().Valid() )	return;


	SINVENITEM* pinven_item_target = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !pinven_item_target )	return;

	wPosX = pinven_item_target->wPosX;
	wPosY = pinven_item_target->wPosY;

	SITEM* pitem_data_target = GLItemMan::GetInstance().GetItem ( pinven_item_target->sItemCustom.sNativeID );
	if ( !pitem_data_target )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_UNWRAP_TARGET_NOT_VALID") );
		return;
	}

	if ( pinven_item_target->sItemCustom.nidDISGUISE == NATIVEID_NULL() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_UNWRAP_TARGET_NOT_VALID") );
		return;
	}

	if ( pitem_data_target->sBasicOp.emItemType != ITEM_WRAPPER_BOX )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_UNWRAP_TARGET_NOT_VALID") );
		return;
	}

	SITEM* pitem_data_disguise = GLItemMan::GetInstance().GetItem( pinven_item_target->sItemCustom.nidDISGUISE );
	if ( !pitem_data_disguise )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_UNWRAP_TARGET_NOT_VALID") );
		return;
	}

	DoModal( ID2GAMEINTEXT("MODAL_UNWRAP_ITEM"),  MODAL_INFOMATION, YESNO, MODAL_UNWRAP_ITEM );	

	m_wInvenPosX3 = wPosX;
	m_wInvenPosY3 = wPosY;
}

/*item wrapper, Juver, 2018/01/12 */
void GLCharacter::ReqItemWrapMsg()
{
	if ( !IsValidBody() )						return;
	if ( GLTradeClient::GetInstance().Valid() )	return;

	SINVENITEM* pinven_item_target = m_cInventory.FindPosItem ( m_wInvenPosX3, m_wInvenPosY3 );
	if ( !pinven_item_target )	return;

	m_wInvenPosX3 = pinven_item_target->wPosX;
	m_wInvenPosY3 = pinven_item_target->wPosY;

	SITEM* pitem_data_target = GLItemMan::GetInstance().GetItem ( pinven_item_target->sItemCustom.sNativeID );
	if ( !pitem_data_target )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_WRAP_TARGET_NOT_VALID") );
		return;
	}

	if ( !pinven_item_target->sItemCustom.IsGM_GENITEM() && pitem_data_target->sBasicOp.IsEXCHANGE() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_WRAP_TARGET_NO_NEED") );
		return;
	}

	if ( !pitem_data_target->isWrappable() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_WRAP_TARGET_NOT_VALID") );
		return;
	}

	if ( pinven_item_target->sItemCustom.nidDISGUISE != NATIVEID_NULL() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_WRAP_TARGET_HAVE_DISGUISE") );
		return;
	}

	SITEM* pitem_data_hold = GET_SLOT_ITEMDATA ( SLOT_HOLD );
	if ( !pitem_data_hold )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_WRAP_HOLD_INVALID") );
		return;
	}

	if ( pitem_data_hold->sBasicOp.emItemType != ITEM_WRAPPER )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_WRAP_HOLD_NOT_WRAPPER") );
		return;
	}

	SITEM* phold_item_box = GLItemMan::GetInstance().GetItem( pitem_data_hold->sBasicOp.sidWrapperBox );
	if ( !phold_item_box )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_WRAP_HOLD_NOT_BOX") );
		return;
	}
	
	GLMSG::SNETPC_INVEN_WRAP NetMsg;
	NetMsg.wPosX = m_wInvenPosX3;
	NetMsg.wPosY = m_wInvenPosY3;
	NETSENDTOFIELD ( &NetMsg );
}

/*item wrapper, Juver, 2018/01/11 */
void GLCharacter::ReqItemUnwrapMsg()
{
	if ( !IsValidBody() )						return;
	if ( GLTradeClient::GetInstance().Valid() )	return;


	SINVENITEM* pinven_item_target = m_cInventory.FindPosItem ( m_wInvenPosX3, m_wInvenPosY3 );
	if ( !pinven_item_target )	return;

	m_wInvenPosX3 = pinven_item_target->wPosX;
	m_wInvenPosY3 = pinven_item_target->wPosY;

	SITEM* pitem_data_target = GLItemMan::GetInstance().GetItem ( pinven_item_target->sItemCustom.sNativeID );
	if ( !pitem_data_target )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_UNWRAP_TARGET_NOT_VALID") );
		return;
	}

	if ( pinven_item_target->sItemCustom.nidDISGUISE == NATIVEID_NULL() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_UNWRAP_TARGET_NOT_VALID") );
		return;
	}

	if ( pitem_data_target->sBasicOp.emItemType != ITEM_WRAPPER_BOX )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_UNWRAP_TARGET_NOT_VALID") );
		return;
	}

	SITEM* pitem_data_disguise = GLItemMan::GetInstance().GetItem( pinven_item_target->sItemCustom.nidDISGUISE );
	if ( !pitem_data_disguise )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_UNWRAP_TARGET_NOT_VALID") );
		return;
	}

	GLMSG::SNETPC_INVEN_UNWRAP NetMsg;
	NetMsg.wPosX = m_wInvenPosX3;
	NetMsg.wPosY = m_wInvenPosY3;
	NETSENDTOFIELD ( &NetMsg );
}

/*change school card, Juver, 2018/01/12 */
void GLCharacter::ReqChangeSchool( WORD wPosX, WORD wPosY )
{
	SINVENITEM* pInvenItem = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !pInvenItem )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_CHANGE_SCHOOL_NOT_CARD") );
		return;
	}

	wPosX = pInvenItem->wPosX;
	wPosY = pInvenItem->wPosY;

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_CHANGE_SCHOOL_NOT_CARD") );
		return;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_CHANGE_SCHOOL )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_CHANGE_SCHOOL_NOT_CARD") );
		return;
	}

	if ( RANPARAM::bFeatureChangeSchoolReqCleanQuest && m_cQuestPlay.GetQuestProc().size() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_CHANGE_SCHOOL_REQ_CLEAN_QUEST") );
		return;
	}

	if ( RANPARAM::bFeatureChangeSchoolReqCleanGuild && m_dwGuild!=CLUB_NULL )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMFB_CHANGE_SCHOOL_REQ_CLEAN_GUILD") );
		return;
	}

	if ( m_emClass == GLCC_EXTREME_M || m_emClass == GLCC_EXTREME_W )
	{
		if ( RANPARAM::bFeatureChangeSchoolReqMaxLevel && m_wLevel < GLCONST_CHAR::wMAX_EXTREME_LEVEL )
		{
			CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMFB_CHANGE_SCHOOL_REQ_MAX_LEVEL") );
			return;
		}
	}
	else
	{
		if ( RANPARAM::bFeatureChangeSchoolReqMaxLevel && m_wLevel < GLCONST_CHAR::wMAX_LEVEL )
		{
			CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMFB_CHANGE_SCHOOL_REQ_MAX_LEVEL") );
			return;
		}
	}
	
	CInnerInterface::GetInstance().ShowGroupFocus( CHANGE_SCHOOL_WINDOW );
}

/*change school card, Juver, 2018/01/12 */
void GLCharacter::ReqChangeSchoolMsg( WORD wSelected )
{
	if ( wSelected >= GLSCHOOL_NUM )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_CHANGE_SCHOOL_NOT_SELECTED") );
		return;
	}

	if ( wSelected == m_wSchool )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_CHANGE_SCHOOL_SAME_SCHOOL") );
		return;
	}

	SINVENITEM* pInvenItem = m_cInventory.FindItem ( ITEM_CHANGE_SCHOOL );
	if ( !pInvenItem )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_CHANGE_SCHOOL_NOT_CARD") );
		return;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_CHANGE_SCHOOL_NOT_CARD") );
		return;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_CHANGE_SCHOOL )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_CHANGE_SCHOOL_NOT_CARD") );
		return;
	}

	if ( RANPARAM::bFeatureChangeSchoolReqCleanQuest && m_cQuestPlay.GetQuestProc().size() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_CHANGE_SCHOOL_REQ_CLEAN_QUEST") );
		return;
	}

	if ( RANPARAM::bFeatureChangeSchoolReqCleanGuild && m_dwGuild!=CLUB_NULL )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMFB_CHANGE_SCHOOL_REQ_CLEAN_GUILD") );
		return;
	}

	if ( m_emClass == GLCC_EXTREME_M || m_emClass == GLCC_EXTREME_W )
	{
		if ( RANPARAM::bFeatureChangeSchoolReqMaxLevel && m_wLevel < GLCONST_CHAR::wMAX_EXTREME_LEVEL )
		{
			CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMFB_CHANGE_SCHOOL_REQ_MAX_LEVEL") );
			return;
		}
	}
	else
	{
		if ( RANPARAM::bFeatureChangeSchoolReqMaxLevel && m_wLevel < GLCONST_CHAR::wMAX_LEVEL )
		{
			CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::NEGATIVE, ID2GAMEINTEXT("EMFB_CHANGE_SCHOOL_REQ_MAX_LEVEL") );
			return;
		}
	}


	GLMSG::SNETPC_INVEN_CHANGE_SCHOOL NetMsg;
	NetMsg.wSchool = wSelected;
	NETSENDTOFIELD ( &NetMsg );
}

/*equipment lock, Juver, 2018/01/13 */
void GLCharacter::ReqEnableEquipmentLock( WORD wPosX, WORD wPosY )
{
	SINVENITEM* pInvenItem = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !pInvenItem )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_ENABLE_NOT_CARD") );
		return;
	}

	wPosX = pInvenItem->wPosX;
	wPosY = pInvenItem->wPosY;

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_ENABLE_NOT_CARD") );
		return;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_EQUIPMENT_LOCK_ENABLE )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_ENABLE_NOT_CARD") );
		return;
	}

	if ( m_bEnableEquipmentLock )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_ENABLE_ALREADY_ENABLED") );
		return;
	}

	CInnerInterface::GetInstance().ShowGroupFocus( EQUIPMENT_LOCK_ENABLE_WINDOW );
}

/*equipment lock, Juver, 2018/01/13 */
void GLCharacter::ReqEnableEquipmentLockMsg( std::string strKey1, std::string strKey2, std::string strAgree )
{
	SINVENITEM* pInvenItem = m_cInventory.FindItem ( ITEM_EQUIPMENT_LOCK_ENABLE );
	if ( !pInvenItem )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_ENABLE_NOT_CARD") );
		return;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_ENABLE_NOT_CARD") );
		return;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_EQUIPMENT_LOCK_ENABLE )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_ENABLE_NOT_CARD") );
		return;
	}

	if ( m_bEnableEquipmentLock )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_ENABLE_ALREADY_ENABLED") );
		return;
	}

	if ( strKey1.empty() || strKey2.empty() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_ENABLE_KEY_EMPTY") );
		return;
	}

	if ( strKey1.size() < CEquipmentLockEnableWindow::nLIMIT_PIN || strKey1.size() > CEquipmentLockEnableWindow::nLIMIT_PIN || 
		strKey2.size() < CEquipmentLockEnableWindow::nLIMIT_PIN || strKey2.size() > CEquipmentLockEnableWindow::nLIMIT_PIN )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_ENABLE_KEY_SIZE") );
		return;
	}

	if ( strcmp( strKey1.c_str(), strKey2.c_str() ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_ENABLE_KEY_NOT_SAME") );
		return;
	}

	if ( strKey1.find_first_not_of( "0123456789" ) != string::npos || strKey2.find_first_not_of( "0123456789" ) != string::npos )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_ENABLE_KEY_NUMBER_ONLY") );
		return;
	}

	if ( strAgree.empty() || strAgree.size() < CEquipmentLockEnableWindow::nLIMIT_AGREE || strAgree.size() > CEquipmentLockEnableWindow::nLIMIT_AGREE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_ENABLE_AGREE_MISMATCH") );
		return;
	}

	if ( strcmp( strAgree.c_str(), "ACCEPT" ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_ENABLE_AGREE_MISMATCH") );
		return;
	}

	CString strTEMP1( strKey1.c_str() );
	CString strTEMP2( strKey2.c_str() );

	if ( STRUTIL::CheckString( strTEMP1 ) || STRUTIL::CheckString( strTEMP2 ) || 
		CRanFilter::GetInstance().NameFilter( strTEMP1 ) || CRanFilter::GetInstance().NameFilter( strTEMP2 ) )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_ENABLE_KEY_NUMBER_ONLY") );
		return;
	}

	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_ENABLE net_msg;
	StringCchCopy ( net_msg.szPin1, CHAR_EQUIPMENT_LOCK_PASS_SIZE+1, strKey1.c_str() );
	StringCchCopy ( net_msg.szPin2, CHAR_EQUIPMENT_LOCK_PASS_SIZE+1, strKey2.c_str() );
	NETSENDTOFIELD ( &net_msg );
}

/*equipment lock, Juver, 2018/01/16 */
void GLCharacter::ReqEnableEquipmentLockInputMsg( std::string strKey )
{
	if ( !m_bEnableEquipmentLock )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_INPUT_NOT_ENABLED") );
		return;
	}

	if ( strKey.empty() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_INPUT_KEY_EMPTY") );
		return;
	}

	if ( strKey.size() < CEquipmentLockInputWindow::nLIMIT_PIN || strKey.size() > CEquipmentLockInputWindow::nLIMIT_PIN  )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_INPUT_KEY_SIZE") );
		return;
	}

	if ( strKey.find_first_not_of( "0123456789" ) != string::npos )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_INPUT_KEY_NUMBER_ONLY") );
		return;
	}


	CString strTEMP( strKey.c_str() );
	if ( STRUTIL::CheckString( strTEMP ) || CRanFilter::GetInstance().NameFilter( strTEMP ) )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_INPUT_KEY_NUMBER_ONLY") );
		return;
	}

	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_INPUT net_msg;
	StringCchCopy ( net_msg.szPin, CHAR_EQUIPMENT_LOCK_PASS_SIZE+1, strKey.c_str() );
	NETSENDTOFIELD ( &net_msg );
}

/*equipment lock, Juver, 2018/01/13 */
void GLCharacter::ReqRecoverEquipmentLockPassword( WORD wPosX, WORD wPosY )
{
	if ( CInnerInterface::GetInstance().IsVisibleGroup( EQUIPMENT_LOCK_RECOVER_WINDOW ) )	return;
	if ( CInnerInterface::GetInstance().IsVisibleGroup( EQUIPMENT_LOCK_INPUT_WINDOW ) )		return;

	SINVENITEM* pInvenItem = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !pInvenItem )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_NOT_CARD") );
		return;
	}

	wPosX = pInvenItem->wPosX;
	wPosY = pInvenItem->wPosY;

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_NOT_CARD") );
		return;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_EQUIPMENT_LOCK_RECOVER )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_NOT_CARD") );
		return;
	}

	if ( !m_bEnableEquipmentLock )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_NOT_ENABLED") );
		return;
	}

	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER net_msg;
	NETSENDTOFIELD ( &net_msg );
}

/*equipment lock, Juver, 2018/01/17 */
void GLCharacter::ReqRecoverEquipmentLockChange( std::string strKey1, std::string strKey2, std::string strAgree )
{
	if ( !m_bEnableEquipmentLock )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_NOT_ENABLED") );
		return;
	}

	if ( strKey1.empty() || strKey2.empty() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_KEY_EMPTY") );
		return;
	}

	if ( strKey1.size() < CEquipmentLockRecoverWindow::nLIMIT_PIN || strKey1.size() > CEquipmentLockRecoverWindow::nLIMIT_PIN || 
		strKey2.size() < CEquipmentLockRecoverWindow::nLIMIT_PIN || strKey2.size() > CEquipmentLockRecoverWindow::nLIMIT_PIN )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_KEY_SIZE") );
		return;
	}

	if ( strcmp( strKey1.c_str(), strKey2.c_str() ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_KEY_NOT_SAME") );
		return;
	}

	if ( strKey1.find_first_not_of( "0123456789" ) != string::npos || strKey2.find_first_not_of( "0123456789" ) != string::npos )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_KEY_NUMBER_ONLY") );
		return;
	}

	if ( strAgree.empty() || strAgree.size() < CEquipmentLockRecoverWindow::nLIMIT_AGREE || strAgree.size() > CEquipmentLockRecoverWindow::nLIMIT_AGREE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_AGREE_MISMATCH") );
		return;
	}

	if ( strcmp( strAgree.c_str(), "CONFIRM" ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_AGREE_MISMATCH") );
		return;
	}

	CString strTEMP1( strKey1.c_str() );
	CString strTEMP2( strKey2.c_str() );

	if ( STRUTIL::CheckString( strTEMP1 ) || STRUTIL::CheckString( strTEMP2 ) || 
		CRanFilter::GetInstance().NameFilter( strTEMP1 ) || CRanFilter::GetInstance().NameFilter( strTEMP2 ) )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_KEY_NUMBER_ONLY") );
		return;
	}

	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE net_msg;
	StringCchCopy ( net_msg.szPin1, CHAR_EQUIPMENT_LOCK_PASS_SIZE+1, strKey1.c_str() );
	StringCchCopy ( net_msg.szPin2, CHAR_EQUIPMENT_LOCK_PASS_SIZE+1, strKey2.c_str() );
	NETSENDTOFIELD ( &net_msg );
}

/*equipment lock, Juver, 2018/01/17 */
void GLCharacter::ReqRecoverEquipmentLockDelete( std::string strAgree )
{
	if ( strAgree.empty() || strAgree.size() < CEquipmentLockRecoverWindow::nLIMIT_AGREE || strAgree.size() > CEquipmentLockRecoverWindow::nLIMIT_AGREE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_DELETE_AGREE_MISMATCH") );
		return;
	}

	if ( strcmp( strAgree.c_str(), "CONFIRM" ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_EQUIPMENT_LOCK_RECOVER_DELETE_AGREE_MISMATCH") );
		return;
	}

	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE net_msg;
	NETSENDTOFIELD ( &net_msg );
}

/*item transfer card, Juver, 2018/01/18 */
void GLCharacter::ReqItemTransferOpen( WORD wPosX, WORD wPosY )
{
	SINVENITEM* pInvenItem = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !pInvenItem )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_NOT_CARD") );
		return;
	}

	wPosX = pInvenItem->wPosX;
	wPosY = pInvenItem->wPosY;

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_NOT_CARD") );
		return;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_TRANSFER_CARD )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_NOT_CARD") );
		return;
	}

	CInnerInterface::GetInstance().OpenItemTransferWindow();
}

/*item transfer card, Juver, 2018/01/18 */
const SITEMCUSTOM GLCharacter::GetItemTransferItemOld()
{
	SITEMCUSTOM sItemCustom;
	sItemCustom.sNativeID = NATIVEID_NULL();
	if( !m_sItemTransferPosOld.VALID() )	return sItemCustom;

	SINVENITEM* pResistItem = m_cInventory.GetItem( m_sItemTransferPosOld.wPosX, m_sItemTransferPosOld.wPosY );
	if( !pResistItem )	return sItemCustom;

	sItemCustom = pResistItem->sItemCustom;
	return sItemCustom;
}

/*item transfer card, Juver, 2018/01/18 */
const SITEMCUSTOM GLCharacter::GetItemTransferItemNew()
{
	SITEMCUSTOM sItemCustom;
	sItemCustom.sNativeID = NATIVEID_NULL();
	if( !m_sItemTransferPosNew.VALID() )	return sItemCustom;

	SINVENITEM* pResistItem = m_cInventory.GetItem( m_sItemTransferPosNew.wPosX, m_sItemTransferPosNew.wPosY );
	if( !pResistItem )	return sItemCustom;

	sItemCustom = pResistItem->sItemCustom;
	return sItemCustom;
}

/*item transfer card, Juver, 2018/01/18 */
void GLCharacter::SetItemTransferMoveItemOld()
{
	if( m_sPreInventoryItem.VALID() )
	{
		if (m_sItemTransferPosOld.wPosX == m_sPreInventoryItem.wPosX &&
			m_sItemTransferPosOld.wPosY == m_sPreInventoryItem.wPosY )
		{
			m_sPreInventoryItem.RESET();
			return;
		}

		if ( m_sItemTransferPosNew.wPosX == m_sPreInventoryItem.wPosX &&
			m_sItemTransferPosNew.wPosY == m_sPreInventoryItem.wPosY )
		{
			m_sPreInventoryItem.RESET();
			return;
		}

		m_sItemTransferPosOld.SET( m_sPreInventoryItem.wPosX, m_sPreInventoryItem.wPosY );
		m_sPreInventoryItem.RESET();
	}
}

/*item transfer card, Juver, 2018/01/18 */
void GLCharacter::SetItemTransferMoveItemNew()
{
	if( m_sPreInventoryItem.VALID() )
	{
		if (m_sItemTransferPosOld.wPosX == m_sPreInventoryItem.wPosX &&
			m_sItemTransferPosOld.wPosY == m_sPreInventoryItem.wPosY )
		{
			m_sPreInventoryItem.RESET();
			return;
		}

		if ( m_sItemTransferPosNew.wPosX == m_sPreInventoryItem.wPosX &&
			m_sItemTransferPosNew.wPosY == m_sPreInventoryItem.wPosY )
		{
			m_sPreInventoryItem.RESET();
			return;
		}

		m_sItemTransferPosNew.SET( m_sPreInventoryItem.wPosX, m_sPreInventoryItem.wPosY );
		m_sPreInventoryItem.RESET();
	}
}

/*item transfer card, Juver, 2018/01/18 */
void GLCharacter::ReSetItemTransferItemOld()
{
	m_sItemTransferPosOld.RESET();
}

/*item transfer card, Juver, 2018/01/18 */
void GLCharacter::ReSetItemTransferItemNew()
{
	m_sItemTransferPosNew.RESET();
}

/*item transfer card, Juver, 2018/01/18 */
void GLCharacter::ResetItemTransferItem()
{
	m_sItemTransferPosOld.RESET();
	m_sItemTransferPosNew.RESET();
	m_sPreInventoryItem.RESET();
}

/*item transfer card, Juver, 2018/01/18 */
bool GLCharacter::ValidItemTransferOpen()
{
	if ( CInnerInterface::GetInstance().IsVisibleGroup( ITEM_TRANSFER_INVEN_WINDOW ) 
		|| CInnerInterface::GetInstance().IsVisibleGroup( ITEM_TRANSFER_WINDOW ) )
		return true;
	return false;
}

/*item transfer card, Juver, 2018/01/18 */
void GLCharacter::ItemTransferStart()
{
	SINVENITEM* pInvenItem = m_cInventory.FindItem ( ITEM_TRANSFER_CARD );
	if ( !pInvenItem )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_NOT_CARD") );
		return;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_NOT_CARD") );
		return;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_TRANSFER_CARD )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_NOT_CARD") );
		return;
	}

	const SITEMCUSTOM sitemcustom_old = GetItemTransferItemOld();
	const SITEMCUSTOM sitemcustom_new = GetItemTransferItemNew();

	SITEM* pitem_data_old = GLItemMan::GetInstance().GetItem( sitemcustom_old.sNativeID );
	SITEM* pitem_data_new = GLItemMan::GetInstance().GetItem( sitemcustom_new.sNativeID );
	if ( !pitem_data_old || !pitem_data_new )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_INVALID_ITEM") );
		return;
	}

	if ( pitem_data_old->sBasicOp.emItemType != ITEM_SUIT || pitem_data_new->sBasicOp.emItemType != ITEM_SUIT )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_INVALID_ITEM_TYPE") );
		return;
	}

	if ( pitem_data_old->IsTIMELMT() || pitem_data_new->IsTIMELMT() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_TIME_LIMIT") );
		return;
	}

	if ( pitem_data_old->sBasicOp.IsDISGUISE() || pitem_data_new->sBasicOp.IsDISGUISE() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_DISGUISE") );
		return;
	}

	if ( !pitem_data_old->sBasicOp.bItemTransfer || !pitem_data_new->sBasicOp.bItemTransfer )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_NOT_TRANSFER_ITEM") );
		return;
	}

	if ( pitem_data_old->sSuitOp.emSuit != pitem_data_new->sSuitOp.emSuit )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_SUIT_MISMATCH") );
		return;
	}

	if ( pitem_data_old->sSuitOp.emSuit == SUIT_HANDHELD && pitem_data_old->sSuitOp.emAttack != pitem_data_new->sSuitOp.emAttack )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_ATTACK_MISMATCH") );
		return;
	}

	if ( RANPARAM::dwFeatureItemTransferCost != 0 && m_lnMoney < RANPARAM::dwFeatureItemTransferCost ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_COST") );
		return;
	}

	if ( sitemcustom_old.nidDISGUISE != NATIVEID_NULL() || sitemcustom_new.nidDISGUISE != NATIVEID_NULL() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_ITEM_TRANSFER_HAVE_DISGUISE") );
		return;
	}

	GLMSG::SNETPC_INVEN_TRANSFER_STATS net_msg;
	net_msg.sInvenPosOLD = m_sItemTransferPosOld;
	net_msg.sInvenPosNEW = m_sItemTransferPosNew;
	NETSENDTOFIELD ( &net_msg );

	ResetItemTransferItem();
}

/*storage lock, Juver, 2018/01/13 */
void GLCharacter::ReqEnableStorageLock( WORD wPosX, WORD wPosY )
{
	SINVENITEM* pInvenItem = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !pInvenItem )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_NOT_CARD") );
		return;
	}

	wPosX = pInvenItem->wPosX;
	wPosY = pInvenItem->wPosY;

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_NOT_CARD") );
		return;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_STORAGE_LOCK_ENABLE )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_NOT_CARD") );
		return;
	}

	if ( m_bEnableStorageLock )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_ALREADY_ENABLED") );
		return;
	}

	CInnerInterface::GetInstance().ShowGroupFocus( STORAGE_LOCK_ENABLE_WINDOW );
}

/*storage lock, Juver, 2018/01/13 */
void GLCharacter::ReqEnableStorageLockMsg( std::string strKey1, std::string strKey2, std::string strAgree )
{
	SINVENITEM* pInvenItem = m_cInventory.FindItem ( ITEM_STORAGE_LOCK_ENABLE );
	if ( !pInvenItem )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_NOT_CARD") );
		return;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_NOT_CARD") );
		return;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_STORAGE_LOCK_ENABLE )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_NOT_CARD") );
		return;
	}

	if ( m_bEnableStorageLock )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_ALREADY_ENABLED") );
		return;
	}

	if ( strKey1.empty() || strKey2.empty() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_KEY_EMPTY") );
		return;
	}

	if ( strKey1.size() < CStorageLockEnableWindow::nLIMIT_PIN || strKey1.size() > CStorageLockEnableWindow::nLIMIT_PIN || 
		strKey2.size() < CStorageLockEnableWindow::nLIMIT_PIN || strKey2.size() > CStorageLockEnableWindow::nLIMIT_PIN )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_KEY_SIZE") );
		return;
	}

	if ( strcmp( strKey1.c_str(), strKey2.c_str() ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_KEY_NOT_SAME") );
		return;
	}

	if ( strKey1.find_first_not_of( "0123456789" ) != string::npos || strKey2.find_first_not_of( "0123456789" ) != string::npos )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_KEY_NUMBER_ONLY") );
		return;
	}

	if ( strAgree.empty() || strAgree.size() < CStorageLockEnableWindow::nLIMIT_AGREE || strAgree.size() > CStorageLockEnableWindow::nLIMIT_AGREE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_AGREE_MISMATCH") );
		return;
	}

	if ( strcmp( strAgree.c_str(), "ACCEPT" ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_AGREE_MISMATCH") );
		return;
	}

	CString strTEMP1( strKey1.c_str() );
	CString strTEMP2( strKey2.c_str() );

	if ( STRUTIL::CheckString( strTEMP1 ) || STRUTIL::CheckString( strTEMP2 ) || 
		CRanFilter::GetInstance().NameFilter( strTEMP1 ) || CRanFilter::GetInstance().NameFilter( strTEMP2 ) )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_ENABLE_KEY_NUMBER_ONLY") );
		return;
	}

	GLMSG::SNETPC_STORAGE_LOCK_ENABLE net_msg;
	StringCchCopy ( net_msg.szPin1, USER_STORAGE_LOCK_PASS_SIZE+1, strKey1.c_str() );
	StringCchCopy ( net_msg.szPin2, USER_STORAGE_LOCK_PASS_SIZE+1, strKey2.c_str() );
	NETSENDTOFIELD ( &net_msg );
}

/*storage lock, Juver, 2018/01/16 */
void GLCharacter::ReqEnableStorageLockInputMsg( std::string strKey )
{
	if ( !m_bEnableStorageLock )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_INPUT_NOT_ENABLED") );
		return;
	}

	if ( strKey.empty() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_INPUT_KEY_EMPTY") );
		return;
	}

	if ( strKey.size() < CStorageLockInputWindow::nLIMIT_PIN || strKey.size() > CStorageLockInputWindow::nLIMIT_PIN  )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_INPUT_KEY_SIZE") );
		return;
	}

	if ( strKey.find_first_not_of( "0123456789" ) != string::npos )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_INPUT_KEY_NUMBER_ONLY") );
		return;
	}


	CString strTEMP( strKey.c_str() );
	if ( STRUTIL::CheckString( strTEMP ) || CRanFilter::GetInstance().NameFilter( strTEMP ) )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_INPUT_KEY_NUMBER_ONLY") );
		return;
	}

	GLMSG::SNETPC_STORAGE_LOCK_INPUT net_msg;
	StringCchCopy ( net_msg.szPin, USER_STORAGE_LOCK_PASS_SIZE+1, strKey.c_str() );
	NETSENDTOFIELD ( &net_msg );
}

/*storage lock, Juver, 2018/01/13 */
void GLCharacter::ReqRecoverStorageLockPassword( WORD wPosX, WORD wPosY )
{
	if ( CInnerInterface::GetInstance().IsVisibleGroup( STORAGE_LOCK_RECOVER_WINDOW ) )	return;
	if ( CInnerInterface::GetInstance().IsVisibleGroup( STORAGE_LOCK_INPUT_WINDOW ) )		return;

	SINVENITEM* pInvenItem = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !pInvenItem )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_RECOVER_NOT_CARD") );
		return;
	}

	wPosX = pInvenItem->wPosX;
	wPosY = pInvenItem->wPosY;

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_RECOVER_NOT_CARD") );
		return;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_STORAGE_LOCK_RECOVER )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_RECOVER_NOT_CARD") );
		return;
	}

	if ( !m_bEnableStorageLock )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_RECOVER_NOT_ENABLED") );
		return;
	}

	GLMSG::SNETPC_STORAGE_LOCK_RECOVER net_msg;
	NETSENDTOFIELD ( &net_msg );
}

/*storage lock, Juver, 2018/01/17 */
void GLCharacter::ReqRecoverStorageLockChange( std::string strKey1, std::string strKey2, std::string strAgree )
{
	if ( !m_bEnableStorageLock )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_RECOVER_CHANGE_NOT_ENABLED") );
		return;
	}

	if ( strKey1.empty() || strKey2.empty() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_RECOVER_CHANGE_KEY_EMPTY") );
		return;
	}

	if ( strKey1.size() < CStorageLockRecoverWindow::nLIMIT_PIN || strKey1.size() > CStorageLockRecoverWindow::nLIMIT_PIN || 
		strKey2.size() < CStorageLockRecoverWindow::nLIMIT_PIN || strKey2.size() > CStorageLockRecoverWindow::nLIMIT_PIN )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_RECOVER_CHANGE_KEY_SIZE") );
		return;
	}

	if ( strcmp( strKey1.c_str(), strKey2.c_str() ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_RECOVER_CHANGE_KEY_NOT_SAME") );
		return;
	}

	if ( strKey1.find_first_not_of( "0123456789" ) != string::npos || strKey2.find_first_not_of( "0123456789" ) != string::npos )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_RECOVER_CHANGE_KEY_NUMBER_ONLY") );
		return;
	}

	if ( strAgree.empty() || strAgree.size() < CStorageLockRecoverWindow::nLIMIT_AGREE || strAgree.size() > CStorageLockRecoverWindow::nLIMIT_AGREE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_RECOVER_CHANGE_AGREE_MISMATCH") );
		return;
	}

	if ( strcmp( strAgree.c_str(), "CONFIRM" ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_RECOVER_CHANGE_AGREE_MISMATCH") );
		return;
	}

	CString strTEMP1( strKey1.c_str() );
	CString strTEMP2( strKey2.c_str() );

	if ( STRUTIL::CheckString( strTEMP1 ) || STRUTIL::CheckString( strTEMP2 ) || 
		CRanFilter::GetInstance().NameFilter( strTEMP1 ) || CRanFilter::GetInstance().NameFilter( strTEMP2 ) )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_RECOVER_CHANGE_KEY_NUMBER_ONLY") );
		return;
	}

	GLMSG::SNETPC_STORAGE_LOCK_RECOVER_CHANGE net_msg;
	StringCchCopy ( net_msg.szPin1, USER_STORAGE_LOCK_PASS_SIZE+1, strKey1.c_str() );
	StringCchCopy ( net_msg.szPin2, USER_STORAGE_LOCK_PASS_SIZE+1, strKey2.c_str() );
	NETSENDTOFIELD ( &net_msg );
}

/*storage lock, Juver, 2018/01/17 */
void GLCharacter::ReqRecoverStorageLockDelete( std::string strAgree )
{
	if ( strAgree.empty() || strAgree.size() < CStorageLockRecoverWindow::nLIMIT_AGREE || strAgree.size() > CStorageLockRecoverWindow::nLIMIT_AGREE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_RECOVER_DELETE_AGREE_MISMATCH") );
		return;
	}

	if ( strcmp( strAgree.c_str(), "CONFIRM" ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_STORAGE_LOCK_RECOVER_DELETE_AGREE_MISMATCH") );
		return;
	}

	GLMSG::SNETPC_STORAGE_LOCK_RECOVER_DELETE net_msg;
	NETSENDTOFIELD ( &net_msg );
}

/*inventory lock, Juver, 2018/01/13 */
void GLCharacter::ReqEnableInventoryLock( WORD wPosX, WORD wPosY )
{
	SINVENITEM* pInvenItem = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !pInvenItem )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_ENABLE_NOT_CARD") );
		return;
	}

	wPosX = pInvenItem->wPosX;
	wPosY = pInvenItem->wPosY;

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_ENABLE_NOT_CARD") );
		return;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_INVENTORY_LOCK_ENABLE )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_ENABLE_NOT_CARD") );
		return;
	}

	if ( m_bEnableInventoryLock )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_ENABLE_ALREADY_ENABLED") );
		return;
	}

	CInnerInterface::GetInstance().ShowGroupFocus( INVENTORY_LOCK_ENABLE_WINDOW );
}

/*inventory lock, Juver, 2018/01/13 */
void GLCharacter::ReqEnableInventoryLockMsg( std::string strKey1, std::string strKey2, std::string strAgree )
{
	SINVENITEM* pInvenItem = m_cInventory.FindItem ( ITEM_INVENTORY_LOCK_ENABLE );
	if ( !pInvenItem )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_ENABLE_NOT_CARD") );
		return;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_ENABLE_NOT_CARD") );
		return;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_INVENTORY_LOCK_ENABLE )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_ENABLE_NOT_CARD") );
		return;
	}

	if ( m_bEnableInventoryLock )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_ENABLE_ALREADY_ENABLED") );
		return;
	}

	if ( strKey1.empty() || strKey2.empty() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_ENABLE_KEY_EMPTY") );
		return;
	}

	if ( strKey1.size() < CInventoryLockEnableWindow::nLIMIT_PIN || strKey1.size() > CInventoryLockEnableWindow::nLIMIT_PIN || 
		strKey2.size() < CInventoryLockEnableWindow::nLIMIT_PIN || strKey2.size() > CInventoryLockEnableWindow::nLIMIT_PIN )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_ENABLE_KEY_SIZE") );
		return;
	}

	if ( strcmp( strKey1.c_str(), strKey2.c_str() ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_ENABLE_KEY_NOT_SAME") );
		return;
	}

	if ( strKey1.find_first_not_of( "0123456789" ) != string::npos || strKey2.find_first_not_of( "0123456789" ) != string::npos )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_ENABLE_KEY_NUMBER_ONLY") );
		return;
	}

	if ( strAgree.empty() || strAgree.size() < CInventoryLockEnableWindow::nLIMIT_AGREE || strAgree.size() > CInventoryLockEnableWindow::nLIMIT_AGREE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_ENABLE_AGREE_MISMATCH") );
		return;
	}

	if ( strcmp( strAgree.c_str(), "ACCEPT" ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_ENABLE_AGREE_MISMATCH") );
		return;
	}

	CString strTEMP1( strKey1.c_str() );
	CString strTEMP2( strKey2.c_str() );

	if ( STRUTIL::CheckString( strTEMP1 ) || STRUTIL::CheckString( strTEMP2 ) || 
		CRanFilter::GetInstance().NameFilter( strTEMP1 ) || CRanFilter::GetInstance().NameFilter( strTEMP2 ) )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_ENABLE_KEY_NUMBER_ONLY") );
		return;
	}

	GLMSG::SNETPC_INVENTORY_LOCK_ENABLE net_msg;
	StringCchCopy ( net_msg.szPin1, CHAR_INVENTORY_LOCK_PASS_SIZE+1, strKey1.c_str() );
	StringCchCopy ( net_msg.szPin2, CHAR_INVENTORY_LOCK_PASS_SIZE+1, strKey2.c_str() );
	NETSENDTOFIELD ( &net_msg );
}

/*inventory lock, Juver, 2018/01/16 */
void GLCharacter::ReqEnableInventoryLockInputMsg( std::string strKey )
{
	if ( !m_bEnableInventoryLock )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_INPUT_NOT_ENABLED") );
		return;
	}

	if ( strKey.empty() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_INPUT_KEY_EMPTY") );
		return;
	}

	if ( strKey.size() < CInventoryLockInputWindow::nLIMIT_PIN || strKey.size() > CInventoryLockInputWindow::nLIMIT_PIN  )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_INPUT_KEY_SIZE") );
		return;
	}

	if ( strKey.find_first_not_of( "0123456789" ) != string::npos )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_INPUT_KEY_NUMBER_ONLY") );
		return;
	}


	CString strTEMP( strKey.c_str() );
	if ( STRUTIL::CheckString( strTEMP ) || CRanFilter::GetInstance().NameFilter( strTEMP ) )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_INPUT_KEY_NUMBER_ONLY") );
		return;
	}

	GLMSG::SNETPC_INVENTORY_LOCK_INPUT net_msg;
	StringCchCopy ( net_msg.szPin, CHAR_INVENTORY_LOCK_PASS_SIZE+1, strKey.c_str() );
	NETSENDTOFIELD ( &net_msg );
}

/*inventory lock, Juver, 2018/01/13 */
void GLCharacter::ReqRecoverInventoryLockPassword( WORD wPosX, WORD wPosY )
{
	if ( CInnerInterface::GetInstance().IsVisibleGroup( INVENTORY_LOCK_RECOVER_WINDOW ) )	return;
	if ( CInnerInterface::GetInstance().IsVisibleGroup( INVENTORY_LOCK_INPUT_WINDOW ) )		return;

	SINVENITEM* pInvenItem = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !pInvenItem )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_RECOVER_NOT_CARD") );
		return;
	}

	wPosX = pInvenItem->wPosX;
	wPosY = pInvenItem->wPosY;

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_RECOVER_NOT_CARD") );
		return;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_INVENTORY_LOCK_RECOVER )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_RECOVER_NOT_CARD") );
		return;
	}

	if ( !m_bEnableInventoryLock )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_RECOVER_NOT_ENABLED") );
		return;
	}

	GLMSG::SNETPC_INVENTORY_LOCK_RECOVER net_msg;
	NETSENDTOFIELD ( &net_msg );
}

/*inventory lock, Juver, 2018/01/17 */
void GLCharacter::ReqRecoverInventoryLockChange( std::string strKey1, std::string strKey2, std::string strAgree )
{
	if ( !m_bEnableInventoryLock )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_RECOVER_CHANGE_NOT_ENABLED") );
		return;
	}

	if ( strKey1.empty() || strKey2.empty() )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_RECOVER_CHANGE_KEY_EMPTY") );
		return;
	}

	if ( strKey1.size() < CInventoryLockRecoverWindow::nLIMIT_PIN || strKey1.size() > CInventoryLockRecoverWindow::nLIMIT_PIN || 
		strKey2.size() < CInventoryLockRecoverWindow::nLIMIT_PIN || strKey2.size() > CInventoryLockRecoverWindow::nLIMIT_PIN )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_RECOVER_CHANGE_KEY_SIZE") );
		return;
	}

	if ( strcmp( strKey1.c_str(), strKey2.c_str() ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_RECOVER_CHANGE_KEY_NOT_SAME") );
		return;
	}

	if ( strKey1.find_first_not_of( "0123456789" ) != string::npos || strKey2.find_first_not_of( "0123456789" ) != string::npos )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_RECOVER_CHANGE_KEY_NUMBER_ONLY") );
		return;
	}

	if ( strAgree.empty() || strAgree.size() < CInventoryLockRecoverWindow::nLIMIT_AGREE || strAgree.size() > CInventoryLockRecoverWindow::nLIMIT_AGREE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_RECOVER_CHANGE_AGREE_MISMATCH") );
		return;
	}

	if ( strcmp( strAgree.c_str(), "CONFIRM" ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_RECOVER_CHANGE_AGREE_MISMATCH") );
		return;
	}

	CString strTEMP1( strKey1.c_str() );
	CString strTEMP2( strKey2.c_str() );

	if ( STRUTIL::CheckString( strTEMP1 ) || STRUTIL::CheckString( strTEMP2 ) || 
		CRanFilter::GetInstance().NameFilter( strTEMP1 ) || CRanFilter::GetInstance().NameFilter( strTEMP2 ) )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_RECOVER_CHANGE_KEY_NUMBER_ONLY") );
		return;
	}

	GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_CHANGE net_msg;
	StringCchCopy ( net_msg.szPin1, CHAR_INVENTORY_LOCK_PASS_SIZE+1, strKey1.c_str() );
	StringCchCopy ( net_msg.szPin2, CHAR_INVENTORY_LOCK_PASS_SIZE+1, strKey2.c_str() );
	NETSENDTOFIELD ( &net_msg );
}

/*inventory lock, Juver, 2018/01/17 */
void GLCharacter::ReqRecoverInventoryLockDelete( std::string strAgree )
{
	if ( strAgree.empty() || strAgree.size() < CInventoryLockRecoverWindow::nLIMIT_AGREE || strAgree.size() > CInventoryLockRecoverWindow::nLIMIT_AGREE )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_RECOVER_DELETE_AGREE_MISMATCH") );
		return;
	}

	if ( strcmp( strAgree.c_str(), "CONFIRM" ) != 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMFB_INVENTORY_LOCK_RECOVER_DELETE_AGREE_MISMATCH") );
		return;
	}

	GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_DELETE net_msg;
	NETSENDTOFIELD ( &net_msg );
}

/* car, cart color, Juver, 2018/02/14 */
void GLCharacter::ReqCarColor( WORD wPosX, WORD wPosY )
{
	SINVENITEM* pInvenItem = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !pInvenItem )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_CHANGE_CAR_COLOR_FB_INVALID_CARD") );
		return;
	}

	wPosX = pInvenItem->wPosX;
	wPosY = pInvenItem->wPosY;

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_CHANGE_CAR_COLOR_FB_INVALID_CARD") );
		return;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_CARD_CAR_COLOR )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_CHANGE_CAR_COLOR_FB_INVALID_CARD") );
		return;
	}

	if ( !m_bVehicle )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_CHANGE_CAR_COLOR_FB_VEHICLE_INACTIVE") );
		return;
	}

	SITEM* pVehicle = GET_SLOT_ITEMDATA( SLOT_VEHICLE );
	if ( !pVehicle )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_CHANGE_CAR_COLOR_FB_VEHICLE_INACTIVE") );
		return;
	}

	if ( pVehicle->sVehicle.emVehicleType != VEHICLE_TYPE_CAR && pVehicle->sVehicle.emVehicleType != VEHICLE_TYPE_CART )
	{
		CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_CHANGE_CAR_COLOR_FB_VEHICLE_NOT_CAR") );
		return;
	}

	m_wCarColorA = m_sVehicle.m_wColor[BIKE_COLOR_SLOT_PART_A1];
	m_wCarColorB = m_sVehicle.m_wColor[BIKE_COLOR_SLOT_PART_A2];
	
	m_bEnableCarColor = TRUE;

	CInnerInterface::GetInstance().ShowGroupFocus( CAR_COLOR_WINDOW );
}

/* car, cart color, Juver, 2018/02/14 */
void GLCharacter::CarColorEnd()
{
	m_bEnableCarColor = FALSE;
	m_wCarColorA = ITEMCOLOR_WHITE;
	m_wCarColorB = ITEMCOLOR_WHITE;

	if ( m_pd3dDevice )
		UpdateSuit();
}

/* car, cart color, Juver, 2018/02/14 */
void GLCharacter::CarColorUpdate()
{
	if ( m_bEnableCarColor && m_pSkinChar && m_pSkinChar->m_pVehicle && ( m_sVehicle.m_emTYPE == VEHICLE_TYPE_CAR || m_sVehicle.m_emTYPE == VEHICLE_TYPE_CART ) )
	{
		m_pSkinChar->m_pVehicle->SetColor1( PIECE_SIZE, m_wCarColorA );
		m_pSkinChar->m_pVehicle->SetColor2( PIECE_SIZE, m_wCarColorB );
	}
}

/* car, cart color, Juver, 2018/02/14 */
void GLCharacter::CarColorChange()
{
	SINVENITEM*	 pInvenItem = m_cInventory.FindItem( ITEM_CARD_CAR_COLOR );
	if ( !pInvenItem )	return;

	if ( !m_bVehicle )	return;
	if ( !m_bEnableCarColor )	return;

	GLMSG::SNET_VEHICLE_REQ_CHANGE_CAR_COLOR NetMsg;
	NetMsg.wColorA = m_wCarColorA;
	NetMsg.wColorB = m_wCarColorB;

	NETSENDTOFIELD ( &NetMsg );
}

/* booster all vehicle, Juver, 2018/02/14 */
void GLCharacter::ReqAllVehicleEnableBooster( WORD wPosX, WORD wPosY )
{
	SINVENITEM* pInvenItem = m_cInventory.FindPosItem ( wPosX, wPosY );
	if ( !pInvenItem ) return;

	wPosX = pInvenItem->wPosX;
	wPosY = pInvenItem->wPosY;

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) return;

	if ( pItem->sBasicOp.emItemType != ITEM_VEHICLE )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMALLVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_ITEM") );
		return;
	}

	SITEM* pHold = GET_SLOT_ITEMDATA ( SLOT_HOLD );
	if ( !pHold ) return;

	if ( pHold->sBasicOp.emItemType != ITEM_CARD_VEHICLE_BOOST )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMALLVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_CARD") );
		return;
	}

	if ( pInvenItem->sItemCustom.dwVehicleID == 0 )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMALLVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_ITEM") );
		return;
	}

	if ( pItem->sVehicle.emVehicleType == VEHICLE_TYPE_BOARD )	
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMALLVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_TYPE") );
		return;
	}

	SVEHICLEITEMINFO sVehicle;

	if ( !DxGlobalStage::GetInstance().IsEmulator() )
	{
		VEHICLEITEMINFO_MAP_ITER iter = m_mapVEHICLEItemInfo.find ( pInvenItem->sItemCustom.dwVehicleID );
		if ( iter==m_mapVEHICLEItemInfo.end() ) return;
		sVehicle = (*iter).second;
	}
	else
	{
		sVehicle.m_bBooster = m_sVehicle.m_bBooster;
	}

	if ( sVehicle.m_bBooster ) 
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMALLVEHICLE_REQ_ENABLE_BOOSTER_FB_ALREADY_ENABLED") );
		return;
	}

	GLMSG::SNET_ALLVEHICLE_REQ_ENABLE_BOOSTER NetMsg;
	NetMsg.wPosX = wPosX;
	NetMsg.wPosY = wPosY;

	NETSENDTOFIELD ( &NetMsg );
}
HRESULT GLCharacter::ReqGMItem ( SNATIVEID sItemID ,WORD wNum ,WORD wPass) //add itemcmd
{
	if ( m_dwUserLvl < USER_MASTER)	return E_FAIL;

	if ( wPass != GLCONST_CHAR::wItemPass )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_GMITEM_PASS") );
		return E_FAIL;
	}
	bool bDrop(false);
	//if ( wPass == GLCONST_CHAR::wItemPass ) bDrop = true;

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( sItemID );
	if ( !pItem )
	{
		CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_GMITEM_NOTITEM") );
		return E_FAIL;
	}

	CInnerInterface::GetInstance().PrintMsgText ( NS_UITEXTCOLOR::GREENYELLOW, "Attempting to insert %s quantity: %d", pItem->GetName(), wNum );

	GLInventory cInvenTemp;
	cInvenTemp.SetAddLine ( m_cInventory.GETAddLine(), true );
	cInvenTemp.Assign ( m_cInventory );

	//for ( int i=0; i< wNum; ++i )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.sNativeID = pItem->sBasicOp.sNativeID;

		if ( sCUSTOM.sNativeID==SNATIVEID(false) )				return E_FAIL;

		SITEM *pITEM = GLItemMan::GetInstance().GetItem ( sCUSTOM.sNativeID );
		if ( !pITEM )
		{
			CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_GMITEM_FAIL") );
			return E_FAIL;
		}

		BOOL bOK = cInvenTemp.InsertItem ( sCUSTOM );
		if ( !bOK )
		{
			CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("EMREQ_GMITEM_INFAIL") );
			return E_FAIL;
		}
	}

	GLMSG::SNET_INVEN_GMITEM NetMsg;
	NetMsg.sItemID = sItemID;
	NetMsg.wNum = wNum;
	NetMsg.wPass = wPass;
	NetMsg.bDrop = bDrop;
	NETSENDTOFIELD ( &NetMsg );

	return S_OK;
}

/*dmk14 ingame web*/
HRESULT GLCharacter::ReqItemMallBuy( ITEMMALLDATA sItemData )
{
	if ( ValidWindowOpen() )						return E_FAIL;	

	if ( sItemData.dwItemStocks <= 0 || 
		 sItemData.sNativeID == NATIVEID_NULL() )
	{
		return E_FAIL;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( sItemData.sNativeID );
	if( pItem )
	{
		LONGLONG dwPrice = sItemData.dwItemPrice;
		if( dwPrice <= 0 )							return E_FAIL;
		//item section - cndev
		if( sItemData.wItemSec >= 13 )						return E_FAIL;

		switch( sItemData.wItemCtg ) 
		{
		case 0:
			{
				if( m_dwPremiumPoints < (LONGLONG)dwPrice )
				{
					CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("BUYITEM_NOPREMIUM") );
					return E_FAIL;
				}
			}
			break;
		case 1:
			{
				if( m_dwCombatPoints < (LONGLONG)dwPrice )
				{
					CInnerInterface::GetInstance().PrintMsgTextDlg ( NS_UITEXTCOLOR::DISABLE, ID2GAMEINTEXT("BUYITEM_NOCOMBAT") );
					return E_FAIL;
				}
			}
			break;
		}
	}


	GLMSG::SNETPC_REQ_BUY_FROM_ITEMMALL NetMsg;
	NetMsg.dwProductNum = sItemData.dwProductNum;
	NetMsg.dwIndex = sItemData.dwIndex;
	NETSENDTOFIELD ( &NetMsg );

	return S_OK;
}

/*dmk14 remove active buff selection*/
void GLCharacter::ReqDisableSkill( DWORD dwSKILL, SNATIVEID sKILLID )
{
	if ( dwSKILL > SKILLFACT_SIZE ) return;

	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sKILLID.wMainID, sKILLID.wSubID );
	if ( !pSkill ) return;
	if ( pSkill->m_sBASIC.emIMPACT_SIDE == SIDE_ENEMY ) return;

	GLMSG::SNETPC_REQ_DISABLESKILLEFF NetMsg;
	NetMsg.dwSKILL = dwSKILL;
	NETSENDTOFIELD ( &NetMsg );
}
void GLCharacter::ReqDisableItemFact( DWORD dwSKILL, SNATIVEID sKILLID )
{
	if ( dwSKILL > FITEMFACT_SIZE ) return;

	PGLSKILL pSkill = GLSkillMan::GetInstance().GetData ( sKILLID.wMainID, sKILLID.wSubID );
	if ( !pSkill ) return;
	if ( pSkill->m_sBASIC.emIMPACT_SIDE == SIDE_ENEMY ) return;

	GLMSG::SNETPC_REQ_DISABLEITEMFACT NetMsg;
	NetMsg.dwSKILL = dwSKILL;
	NETSENDTOFIELD ( &NetMsg );
}
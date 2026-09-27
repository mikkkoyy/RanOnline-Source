#include "stdafx.h"
#include "./GLChar.h"
#include "./GLGaeaServer.h"
#include "./GLItemMan.h"
#include "./GLSkill.h"
#include "./GLItemMixMan.h"
#include "../Lib_Engine/Common/StringUtils.h"
#include "RanFilter.h"

#include "RANPARAM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*itemfood system, Juver, 2017/05/25 */
HRESULT GLChar::MsgReqConsumeFood( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_INVEN_CONSUME_FOOD* pNetMsg = (GLMSG::SNET_INVEN_CONSUME_FOOD*)nmg;
	GLMSG::SNET_INVEN_CONSUME_FOOD_FB	NetMsgFB;

	SINVENITEM *pINVENITEM = m_cInventory.FindPosItem ( pNetMsg->wPosX, pNetMsg->wPosY );
	if ( !pINVENITEM )
	{
		NetMsgFB.emFB = EMCONSUME_FOOD_FB_NOITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	SITEM* pITEM = GLItemMan::GetInstance().GetItem ( pINVENITEM->sItemCustom.sNativeID );
	if ( !pITEM )
	{
		NetMsgFB.emFB = EMCONSUME_FOOD_FB_NOITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( pITEM->sBasicOp.emItemType != ITEM_FOOD )
	{
		NetMsgFB.emFB = EMCONSUME_FOOD_FB_NOITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( m_pLandMan && m_pLandMan->IsLunchBoxForbid() )
	{
		NetMsgFB.emFB = EMCONSUME_FOOD_FB_NONAREA;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( CheckCoolTime( pINVENITEM->sItemCustom.sNativeID ) )
	{
		NetMsgFB.emFB = EMCONSUME_FOOD_FB_COOLTIME;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	PGLSKILL pSKILL = GLSkillMan::GetInstance().GetData( pITEM->sSkillBookOp.sSkill_ID );
	if ( !pSKILL )
	{
		NetMsgFB.emFB = EMCONSUME_FOOD_FB_NONDATA;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	SNATIVEID _sidSKILL = pSKILL->m_sBASIC.sNATIVEID;
	WORD _wSLOT = ItemFoodFindSlot( _sidSKILL );
	if ( _wSLOT == FITEMFACT_SIZE )
	{
		NetMsgFB.emFB = EMCONSUME_FOOD_FB_NOSLOT;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	WORD _wUSESLOT = ItemFoodAdd( _sidSKILL, 0, _wSLOT );
	if ( _wUSESLOT == FITEMFACT_SIZE )
	{
		NetMsgFB.emFB = EMCONSUME_FOOD_FB_NOSLOT;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	DoDrugInvenItem ( pNetMsg->wPosX, pNetMsg->wPosY );

	GLMSG::SNETPC_FITEMFACT_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID = m_dwGaeaID;
	NetMsgBrd.sidSkill = _sidSKILL;
	NetMsgBrd.wSLOT = _wUSESLOT;
	NetMsgBrd.sFACT = m_sFITEMFACT[_wUSESLOT];
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgBrd);
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

	NetMsgFB.emFB = EMCONSUME_FOOD_FB_OK;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);

	return S_OK;
}

/*itemfood system, Juver, 2017/05/26 */
HRESULT GLChar::MsgReqUnlockFood( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_INVEN_UNLOCK_FOOD* pNetMsg = (GLMSG::SNET_INVEN_UNLOCK_FOOD*)nmg;
	GLMSG::SNET_INVEN_UNLOCK_FOOD_FB	NetMsgFB;

	SINVENITEM *pINVENITEM = m_cInventory.FindPosItem ( pNetMsg->wPosX, pNetMsg->wPosY );
	if ( !pINVENITEM )
	{
		NetMsgFB.emFB = EMUNLOCK_FOOD_FB_NOITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	SITEM* pITEM = GLItemMan::GetInstance().GetItem ( pINVENITEM->sItemCustom.sNativeID );
	if ( !pITEM )
	{
		NetMsgFB.emFB = EMUNLOCK_FOOD_FB_NOITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( pITEM->sBasicOp.emItemType != ITEM_CARD_FOODUNLOCK )
	{
		NetMsgFB.emFB = EMUNLOCK_FOOD_FB_NOITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
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
		NetMsgFB.emFB = EMUNLOCK_FOOD_FB_UNLOCKED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	for( int i=0; i<FITEMFACT_SIZE; ++i ){
		m_sFITEMFACT[i].bENABLE = TRUE;
	}

	DoDrugInvenItem ( pNetMsg->wPosX, pNetMsg->wPosY );

	NetMsgFB.emFB = EMUNLOCK_FOOD_FB_OK;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);

	return S_OK;
}

/*rightclick wear/unwear, Juver, 2017/06/24 */
HRESULT GLChar::MsgReqInvenToWear ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_INVEN_TO_WEAR *pNetMsgClient = reinterpret_cast<GLMSG::SNETPC_REQ_INVEN_TO_WEAR*> ( nmg );

	/*equipment lock, Juver, 2018/01/18 */
	if ( m_bEnableEquipmentLock && m_bEquipmentLockStatus && !( pNetMsgClient->emSlot == SLOT_LHAND || pNetMsgClient->emSlot == SLOT_LHAND_S ) )
		return S_FALSE;

	GLMSG::SNETPC_REQ_INVEN_TO_HOLD NetMsgInvenToHold;
	NetMsgInvenToHold.wPosX = pNetMsgClient->wPosX;
	NetMsgInvenToHold.wPosY = pNetMsgClient->wPosY;
	HRESULT hrInvenToHold = MsgReqInvenToHold( (NET_MSG_GENERIC*)&NetMsgInvenToHold );
	if ( hrInvenToHold != S_OK )	return S_FALSE;
	
	bool bEMPTY_SLOT = ISEMPTY_SLOT( GET_HOLD_ITEM().sNativeID, pNetMsgClient->emSlot);
	if ( VALID_HOLD_ITEM() && !bEMPTY_SLOT )
	{
		GLMSG::SNETPC_REQ_SLOT_EX_HOLD NetMsgHoldToSlot;
		NetMsgHoldToSlot.emSlot = pNetMsgClient->emSlot;
		HRESULT hrWear = MsgReqSlotExHold( (NET_MSG_GENERIC*)&NetMsgHoldToSlot );
		if ( hrWear != S_OK ) return S_FALSE;
	}
	else if ( VALID_HOLD_ITEM() )
	{
		GLMSG::SNETPC_REQ_HOLD_TO_SLOT NetMsgHoldToSlot;
		NetMsgHoldToSlot.emSlot = pNetMsgClient->emSlot;
		HRESULT hrWear = MsgReqHoldToSlot( (NET_MSG_GENERIC*)&NetMsgHoldToSlot );
		if ( hrWear != S_OK ) return S_FALSE;
	}

	GLMSG::SNETPC_REQ_HOLD_TO_INVEN NetMsgHoldToInven;
	NetMsgHoldToInven.wPosX = pNetMsgClient->wPosX;
	NetMsgHoldToInven.wPosY = pNetMsgClient->wPosY;
	MsgReqHoldToInven( (NET_MSG_GENERIC*)&NetMsgHoldToInven );

	return S_OK;
}

/*rightclick wear/unwear, Juver, 2017/06/24 */
HRESULT GLChar::MsgReqWearToInven ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_WEAR_TO_INVEN *pNetMsgClient = reinterpret_cast<GLMSG::SNETPC_REQ_WEAR_TO_INVEN*> ( nmg );
	if ( pNetMsgClient->emSlot >= SLOT_TSIZE )		return S_FALSE;

	/*equipment lock, Juver, 2018/01/18 */
	if ( m_bEnableEquipmentLock && m_bEquipmentLockStatus && !( pNetMsgClient->emSlot == SLOT_LHAND || pNetMsgClient->emSlot == SLOT_LHAND_S ) )
		return S_FALSE;

	GLMSG::SNETPC_REQ_SLOT_TO_HOLD NetMsgHoldToSlot;
	NetMsgHoldToSlot.emSlot = pNetMsgClient->emSlot;
	HRESULT hrUnwear = MsgReqSlotToHold( (NET_MSG_GENERIC*)&NetMsgHoldToSlot );
	if ( hrUnwear != S_OK ) return S_FALSE;

	if( !VALID_HOLD_ITEM() )	return S_FALSE;

	SITEM* pITEMHOLD = GLItemMan::GetInstance().GetItem( GET_HOLD_ITEM().sNativeID );
	if ( !pITEMHOLD )	return S_FALSE;

	WORD wInvenPosX, wInvenPosY;
	BOOL bOk = m_cInventory.FindInsrtable ( pITEMHOLD->sBasicOp.wInvenSizeX, pITEMHOLD->sBasicOp.wInvenSizeY, wInvenPosX, wInvenPosY );
	if ( !bOk )	return S_FALSE;

	GLMSG::SNETPC_REQ_HOLD_TO_INVEN NetMsgHoldToInven;
	NetMsgHoldToInven.wPosX = wInvenPosX;
	NetMsgHoldToInven.wPosY = wInvenPosY;
	MsgReqHoldToInven( (NET_MSG_GENERIC*)&NetMsgHoldToInven );

	return S_OK;
}

/*npc shop, Juver, 2017/07/27 */
HRESULT GLChar::ReqNpcShopPurchaseMoney ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_NPCSHOP_PURCHASE_MONEY *pnetmsgclient = reinterpret_cast<GLMSG::SNETPC_REQ_NPCSHOP_PURCHASE_MONEY*> ( nmg );
	GLMSG::SNETPC_REQ_NPCSHOP_PURCHASE_MONEY_FB	netmsgfb;

	SITEM* pitem = GLItemMan::GetInstance().GetItem ( pnetmsgclient->sidItem );
	if ( !pitem )	
	{
		netmsgfb.emFB = EMNPCSHOP_PURCHASE_FB_INVALID_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		return E_FAIL;
	}

	SCROWDATA*	pcrow = GLCrowDataMan::GetInstance().GetCrowData( pnetmsgclient->sidCrow );
	if ( !pcrow )
	{
		netmsgfb.emFB = EMNPCSHOP_PURCHASE_FB_INVALID_CROW;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		return E_FAIL;
	}

	SNPC_SHOP& npc_shop = pcrow->m_sNPCShop;

	if ( npc_shop.wShopType != pnetmsgclient->wShopType )
	{
		netmsgfb.emFB = EMNPCSHOP_PURCHASE_FB_INVALID_SHOPTYPE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		return E_FAIL;
	}

	SNPC_SHOP_ITEM* pshopitem = npc_shop.findShopItem( pnetmsgclient->sidItem.dwID );
	if ( !pshopitem )
	{
		netmsgfb.emFB = EMNPCSHOP_PURCHASE_FB_INVALID_SHOPITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		return E_FAIL;
	}


	if ( npc_shop.wShopType == 0 )
	{
		//money check
		volatile LONGLONG llPRICE = (LONGLONG) pitem->sBasicOp.dwBuyPrice;
		if ( m_lnMoney < llPRICE )
		{
			netmsgfb.emFB = EMNPCSHOP_PURCHASE_FB_NOT_MONEY;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
			return E_FAIL;
		}

		/*contribution point, Juver, 2017/08/23 */
		if ( pitem->sBasicOp.dwReqContributionPoint != 0 &&
			m_llContributionPoint < (LONGLONG)pitem->sBasicOp.dwReqContributionPoint )	
		{
			return E_FAIL;
		}

		/*activity point, Juver, 2017/08/23 */
		if ( pitem->sBasicOp.dwReqActivityPoint != 0 &&
			m_dwActivityPoint < pitem->sBasicOp.dwReqActivityPoint )
		{
			return E_FAIL;
		}

		WORD winvensizex = pitem->sBasicOp.wInvenSizeX;
		WORD winvensizey = pitem->sBasicOp.wInvenSizeY;

		BOOL bITEM_SPACE(FALSE);
		if ( pitem->ISPILE() )
		{
			WORD wPILENUM = pitem->sDrugOp.wPileNum;
			SNATIVEID sNID = pitem->sBasicOp.sNativeID;
			WORD wREQINSRTNUM = ( 1 * pitem->GETAPPLYNUM() );

			bITEM_SPACE = m_cInventory.ValidPileInsrt ( wREQINSRTNUM, sNID, wPILENUM, winvensizex, winvensizey );

			if ( !bITEM_SPACE )
			{
				netmsgfb.emFB = EMNPCSHOP_PURCHASE_FB_NOT_INVEN_SPACE;
				GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
				return E_FAIL;
			}

			SITEMCUSTOM snew_item(sNID);
			snew_item.cGenType = EMGEN_SHOP;
			snew_item.cChnID = (BYTE)GLGaeaServer::GetInstance().GetServerChannel();
			snew_item.cFieldID = (BYTE)GLGaeaServer::GetInstance().GetFieldSvrID();
			snew_item.wTurnNum = wPILENUM;
			snew_item.tBORNTIME = CTime::GetCurrentTime().GetTime();

			/*item color, Juver, 2018/01/08 */
			SITEM* pitem_data_check = GLItemMan::GetInstance().GetItem( snew_item.sNativeID );
			if ( pitem_data_check && pitem_data_check->sBasicOp.bItemColor )
			{
				snew_item.wColor1 = pitem_data_check->sBasicOp.wItemColor1;
				snew_item.wColor2 = pitem_data_check->sBasicOp.wItemColor2;
			}

			GLITEMLMT::GetInstance().ReqItemRoute ( snew_item, ID_CHAR, 0, ID_CHAR, m_dwCharID, EMITEM_ROUTE_CHAR, snew_item.wTurnNum );

			CheckMoneyUpdate( m_lnMoney, llPRICE, FALSE, "Buy From SHOP." );
			m_bMoneyUpdate = TRUE;

			m_lnMoney -= llPRICE;

			if ( llPRICE>EMMONEY_LOG )
			{
				GLITEMLMT::GetInstance().ReqMoneyExc ( ID_CHAR, m_dwCharID, ID_CHAR, 0, -LONGLONG(llPRICE), EMITEM_ROUTE_DELETE );
				GLITEMLMT::GetInstance().ReqMoneyExc ( ID_CHAR, m_dwCharID, ID_CHAR, 0, m_lnMoney, EMITEM_ROUTE_CHAR );
			}

			InsertPileItem ( snew_item, wREQINSRTNUM );

			/*activity system, Juver, 2017/11/03 */
			DoActivityTakeItem( snew_item.sNativeID, wREQINSRTNUM );
		}
		else
		{
			WORD wInsertPosX(0), wInsertPosY(0);
			bITEM_SPACE = m_cInventory.FindInsrtable ( winvensizex, winvensizey, wInsertPosX, wInsertPosY );

			if ( !bITEM_SPACE )	
			{
				netmsgfb.emFB = EMNPCSHOP_PURCHASE_FB_NOT_INVEN_SPACE;
				GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
				return E_FAIL;
			}

			SITEMCUSTOM snew_item( pitem->sBasicOp.sNativeID );
			snew_item.cDAMAGE = pshopitem->cDAMAGE;
			snew_item.cDEFENSE = pshopitem->cDEFENSE;
			snew_item.cRESIST_FIRE = pshopitem->cRESIST_FIRE;
			snew_item.cRESIST_ICE = pshopitem->cRESIST_ICE;
			snew_item.cRESIST_ELEC = pshopitem->cRESIST_ELEC;
			snew_item.cRESIST_POISON = pshopitem->cRESIST_POISON;
			snew_item.cRESIST_SPIRIT = pshopitem->cRESIST_SPIRIT;

			CTime cTIME = CTime::GetCurrentTime();
			snew_item.tBORNTIME = cTIME.GetTime();
			snew_item.wTurnNum = 1;
			snew_item.cGenType = (BYTE) EMGEN_SHOP;
			snew_item.cChnID = (BYTE)GLGaeaServer::GetInstance().GetServerChannel();
			snew_item.cFieldID = (BYTE) GLGaeaServer::GetInstance().GetFieldSvrID();
			snew_item.lnGenNum = GLITEMLMT::GetInstance().RegItemGen ( pitem->sBasicOp.sNativeID, EMGEN_SHOP );

			/*item color, Juver, 2018/01/08 */
			SITEM* pitem_data_check = GLItemMan::GetInstance().GetItem( snew_item.sNativeID );
			if ( pitem_data_check && pitem_data_check->sBasicOp.bItemColor )
			{
				snew_item.wColor1 = pitem_data_check->sBasicOp.wItemColor1;
				snew_item.wColor2 = pitem_data_check->sBasicOp.wItemColor2;
			}

			if ( pshopitem->bGenerateRandomValue )
			{
				if( snew_item.GENERATE_RANDOM_OPT () )
				{
					GLITEMLMT::GetInstance().ReqRandomItem( snew_item );
				}
			}

			CheckMoneyUpdate( m_lnMoney, llPRICE, FALSE, "Buy From SHOP." );
			m_bMoneyUpdate = TRUE;

			m_lnMoney -= llPRICE;

			if ( llPRICE>EMMONEY_LOG )
			{
				GLITEMLMT::GetInstance().ReqMoneyExc ( ID_CHAR, m_dwCharID, ID_CHAR, 0, -LONGLONG(llPRICE), EMITEM_ROUTE_DELETE );
				GLITEMLMT::GetInstance().ReqMoneyExc ( ID_CHAR, m_dwCharID, ID_CHAR, 0, m_lnMoney, EMITEM_ROUTE_CHAR );
			}

			BOOL bOK = m_cInventory.InsertItem ( snew_item, wInsertPosX, wInsertPosY );
			if ( !bOK )		return S_OK;

			GLITEMLMT::GetInstance().ReqItemRoute ( snew_item, ID_CHAR, 0, ID_CHAR, m_dwCharID, EMITEM_ROUTE_CHAR, snew_item.wTurnNum );

			GLMSG::SNETPC_INVEN_INSERT NetMsgInven;
			NetMsgInven.Data = *m_cInventory.GetItem ( wInsertPosX, wInsertPosY );
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgInven);

			/*activity system, Juver, 2017/11/03 */
			DoActivityTakeItem( snew_item.sNativeID, snew_item.wTurnNum );
		}

		GLMSG::SNETPC_UPDATE_MONEY NetMsgMoney;
		NetMsgMoney.lnMoney = m_lnMoney;
		/*dmk14 ingame web*/
		NetMsgMoney.dwPremium = m_dwPremiumPoints;
		NetMsgMoney.dwCombat = m_dwCombatPoints;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgMoney);
	}

	netmsgfb.emFB = EMNPCSHOP_PURCHASE_FB_PURCHASED;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);

	return S_OK;
}

/*item exchange, Juver, 2017/10/13 */
HRESULT GLChar::ReqNpcItemExchangeTrade ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_NPC_ITEM_EXCHANGE_TRADE *pnetmsgclient = reinterpret_cast<GLMSG::SNETPC_REQ_NPC_ITEM_EXCHANGE_TRADE*> ( nmg );
	GLMSG::SNETPC_REQ_NPC_ITEM_EXCHANGE_TRADE_FB	netmsgfb;

	PCROWDATA pcrow_data = GLCrowDataMan::GetInstance().GetCrowData( pnetmsgclient->sid_NPC );
	if ( !pcrow_data )
	{
		netmsgfb.emFB = EMNPC_ITEM_EXCHANGE_FB_INVALID_NPC;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		CDebugSet::ToFileWithTime( "_npcexchange.txt", "[%d][%s] EMNPC_ITEM_EXCHANGE_FB_INVALID_NPC %d~%d", m_dwCharID, m_szName,
			pnetmsgclient->sid_NPC.wMainID, pnetmsgclient->sid_NPC.wSubID );
		return E_FAIL;
	}

	SNPC_ITEM_EXCHANGE& npc_item_exchange = pcrow_data->m_sNPCItemExchange;
	SNPC_ITEM_EXCHANGE_DATA* pnpc_item_exchange_data = npc_item_exchange.findData( pnetmsgclient->wTradeID );
	if ( !pnpc_item_exchange_data )
	{
		netmsgfb.emFB = EMNPC_ITEM_EXCHANGE_FB_INVALID_ID;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		CDebugSet::ToFileWithTime( "_npcexchange.txt", "[%d][%s] EMNPC_ITEM_EXCHANGE_FB_INVALID_ID %d", m_dwCharID, m_szName,
			pnetmsgclient->wTradeID );
		return E_FAIL;
	}

	SITEM* pitem_data_result = GLItemMan::GetInstance().GetItem( pnpc_item_exchange_data->sidItemResult.sidItem );
	if ( !pitem_data_result )
	{
		netmsgfb.emFB = EMNPC_ITEM_EXCHANGE_FB_INVALID_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		CDebugSet::ToFileWithTime( "_npcexchange.txt", "[%d][%s] EMNPC_ITEM_EXCHANGE_FB_INVALID_ITEM %d~%d", m_dwCharID, m_szName,
			pnpc_item_exchange_data->sidItemResult.sidItem.wMainID, pnpc_item_exchange_data->sidItemResult.sidItem.wSubID );
		return E_FAIL;
	}

	/* disable
	contribution point, Juver, 2017/08/23 */
	/*if ( pitem_data_result->sBasicOp.dwReqContributionPoint != 0 &&
		m_llContributionPoint < (LONGLONG)pitem_data_result->sBasicOp.dwReqContributionPoint )
	{
		netmsgfb.emFB = EMNPC_ITEM_EXCHANGE_FB_NOCONTRIBUTIONPOINT;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		CDebugSet::ToFileWithTime( "_npcexchange.txt", "[%d][%s] EMNPC_ITEM_EXCHANGE_FB_NOCONTRIBUTIONPOINT %I64d~%d", m_dwCharID, m_szName,
			m_llContributionPoint, pitem_data_result->sBasicOp.dwReqContributionPoint );
		return E_FAIL;
	}
	*/

	/*activity point, Juver, 2017/08/23 */
	if ( pitem_data_result->sBasicOp.dwReqActivityPoint != 0 &&
		m_dwActivityPoint < pitem_data_result->sBasicOp.dwReqActivityPoint )
	{
		netmsgfb.emFB = EMNPC_ITEM_EXCHANGE_FB_NOPOINT;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		CDebugSet::ToFileWithTime( "_npcexchange.txt", "[%d][%s] EMNPC_ITEM_EXCHANGE_FB_NOPOINT %d~%d", m_dwCharID, m_szName,
			m_dwActivityPoint, pitem_data_result->sBasicOp.dwReqActivityPoint );
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
					netmsgfb.emFB = EMNPC_ITEM_EXCHANGE_FB_REQUIRE_MISSING;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
					CDebugSet::ToFileWithTime( "_npcexchange.txt", "[%d][%s] EMNPC_ITEM_EXCHANGE_FB_REQUIRE_MISSING %d~%d %d", m_dwCharID, m_szName,
						pitem_data_require->sBasicOp.sNativeID.wMainID, pitem_data_require->sBasicOp.sNativeID.wSubID, wNum );
					return E_FAIL;
				}

				BOOL bDelete = DeletePileItem( pitem_data_require->sBasicOp.sNativeID, wNum );
				if ( !bDelete )
				{
					netmsgfb.emFB = EMNPC_ITEM_EXCHANGE_FB_DELETE_FAIL;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
					CDebugSet::ToFileWithTime( "_npcexchange.txt", "[%d][%s] EMNPC_ITEM_EXCHANGE_FB_DELETE_FAIL %d~%d %d", m_dwCharID, m_szName,
						pitem_data_require->sBasicOp.sNativeID.wMainID, pitem_data_require->sBasicOp.sNativeID.wSubID, wNum );
					return E_FAIL;
				}
			}
			else
			{
				//not pile just check if exist
				SINVENITEM* pinvenitem = m_cInventory.FindItem( pitem_data_require->sBasicOp.sNativeID );
				if ( !pinvenitem )
				{
					netmsgfb.emFB = EMNPC_ITEM_EXCHANGE_FB_REQUIRE_MISSING;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
					CDebugSet::ToFileWithTime( "_npcexchange.txt", "[%d][%s] EMNPC_ITEM_EXCHANGE_FB_REQUIRE_MISSING %d~%d", m_dwCharID, m_szName,
						pitem_data_require->sBasicOp.sNativeID.wMainID, pitem_data_require->sBasicOp.sNativeID.wSubID );
					return E_FAIL;
				}

				WORD wposX = pinvenitem->wPosX;
				WORD wposY = pinvenitem->wPosY;

				BOOL bDelete = m_cInventory.DeleteItem ( wposX, wposY );
				if ( !bDelete )
				{
					netmsgfb.emFB = EMNPC_ITEM_EXCHANGE_FB_DELETE_FAIL;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
					CDebugSet::ToFileWithTime( "_npcexchange.txt", "[%d][%s] EMNPC_ITEM_EXCHANGE_FB_DELETE_FAIL %d~%d %d~%d", m_dwCharID, m_szName,
						pitem_data_require->sBasicOp.sNativeID.wMainID, pitem_data_require->sBasicOp.sNativeID.wSubID, wposX, wposY );
					return E_FAIL;
				}

				GLMSG::SNETPC_INVEN_DELETE NetMsg_Inven_Delete;
				NetMsg_Inven_Delete.wPosX = wposX;
				NetMsg_Inven_Delete.wPosY = wposY;
				GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg_Inven_Delete);
			}
		}
	}

	WORD winvensizex = pitem_data_result->sBasicOp.wInvenSizeX;
	WORD winvensizey = pitem_data_result->sBasicOp.wInvenSizeY;

	BOOL bITEM_SPACE(FALSE);
	if ( pitem_data_result->ISPILE() )
	{
		WORD wPILENUM = pitem_data_result->sDrugOp.wPileNum;
		SNATIVEID sNID = pitem_data_result->sBasicOp.sNativeID;
		WORD wREQINSRTNUM = ( pnpc_item_exchange_data->sidItemResult.wItemNum * pitem_data_result->GETAPPLYNUM() );

		bITEM_SPACE = m_cInventory.ValidPileInsrt ( wREQINSRTNUM, sNID, wPILENUM, winvensizex, winvensizey );

		if ( !bITEM_SPACE )
		{
			netmsgfb.emFB = EMNPC_ITEM_EXCHANGE_FB_NOT_INVEN_SPACE;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
			CDebugSet::ToFileWithTime( "_npcexchange.txt", "[%d][%s] EMNPC_ITEM_EXCHANGE_FB_NOT_INVEN_SPACE", m_dwCharID, m_szName );
			return E_FAIL;
		}

		SITEMCUSTOM snew_item(sNID);
		snew_item.cGenType = EMGEN_SHOP;
		snew_item.cChnID = (BYTE)GLGaeaServer::GetInstance().GetServerChannel();
		snew_item.cFieldID = (BYTE)GLGaeaServer::GetInstance().GetFieldSvrID();
		snew_item.lnGenNum = GLITEMLMT::GetInstance().RegItemGen ( pitem_data_result->sBasicOp.sNativeID, EMGEN_SHOP );
		snew_item.wTurnNum = wPILENUM;
		snew_item.tBORNTIME = CTime::GetCurrentTime().GetTime();

		/*item color, Juver, 2018/01/08 */
		SITEM* pitem_data_check = GLItemMan::GetInstance().GetItem( snew_item.sNativeID );
		if ( pitem_data_check && pitem_data_check->sBasicOp.bItemColor )
		{
			snew_item.wColor1 = pitem_data_check->sBasicOp.wItemColor1;
			snew_item.wColor2 = pitem_data_check->sBasicOp.wItemColor2;
		}

		GLITEMLMT::GetInstance().ReqItemRoute ( snew_item, ID_CHAR, 0, ID_CHAR, m_dwCharID, EMITEM_ROUTE_CHAR, snew_item.wTurnNum );

		BOOL bOK = InsertPileItem ( snew_item, wREQINSRTNUM );
		if ( !bOK )	
		{
			netmsgfb.emFB = EMNPC_ITEM_EXCHANGE_FB_INSERT_FAIL;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
			CDebugSet::ToFileWithTime( "_npcexchange.txt", "[%d][%s] EMNPC_ITEM_EXCHANGE_FB_INSERT_FAIL", m_dwCharID, m_szName );
			return E_FAIL;
		}

		/*activity system, Juver, 2017/11/03 */
		DoActivityTakeItem( snew_item.sNativeID, snew_item.wTurnNum );
	}
	else
	{
		WORD wInsertPosX(0), wInsertPosY(0);
		bITEM_SPACE = m_cInventory.FindInsrtable ( winvensizex, winvensizey, wInsertPosX, wInsertPosY );
		if ( !bITEM_SPACE )	
		{
			netmsgfb.emFB = EMNPC_ITEM_EXCHANGE_FB_NOT_INVEN_SPACE;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
			CDebugSet::ToFileWithTime( "_npcexchange.txt", "[%d][%s] EMNPC_ITEM_EXCHANGE_FB_NOT_INVEN_SPACE", m_dwCharID, m_szName );
			return E_FAIL;
		}

		SITEMCUSTOM snew_item( pitem_data_result->sBasicOp.sNativeID );
		snew_item.cDAMAGE = pnpc_item_exchange_data->cDAMAGE;
		snew_item.cDEFENSE = pnpc_item_exchange_data->cDEFENSE;
		snew_item.cRESIST_FIRE = pnpc_item_exchange_data->cRESIST_FIRE;
		snew_item.cRESIST_ICE = pnpc_item_exchange_data->cRESIST_ICE;
		snew_item.cRESIST_ELEC = pnpc_item_exchange_data->cRESIST_ELEC;
		snew_item.cRESIST_POISON = pnpc_item_exchange_data->cRESIST_POISON;
		snew_item.cRESIST_SPIRIT = pnpc_item_exchange_data->cRESIST_SPIRIT;
		snew_item.tBORNTIME = CTime::GetCurrentTime().GetTime();
		snew_item.wTurnNum = 1;
		snew_item.cGenType = (BYTE) EMGEN_SHOP;
		snew_item.cChnID = (BYTE)GLGaeaServer::GetInstance().GetServerChannel();
		snew_item.cFieldID = (BYTE) GLGaeaServer::GetInstance().GetFieldSvrID();
		snew_item.lnGenNum = GLITEMLMT::GetInstance().RegItemGen ( pitem_data_result->sBasicOp.sNativeID, EMGEN_SHOP );

		/*item color, Juver, 2018/01/08 */
		SITEM* pitem_data_check = GLItemMan::GetInstance().GetItem( snew_item.sNativeID );
		if ( pitem_data_check && pitem_data_check->sBasicOp.bItemColor )
		{
			snew_item.wColor1 = pitem_data_check->sBasicOp.wItemColor1;
			snew_item.wColor2 = pitem_data_check->sBasicOp.wItemColor2;
		}

		if ( pnpc_item_exchange_data->bGenerateRandomValue )
		{
			if( snew_item.GENERATE_RANDOM_OPT () )
			{
				GLITEMLMT::GetInstance().ReqRandomItem( snew_item );
			}
		}
		
		BOOL bOK = m_cInventory.InsertItem ( snew_item, wInsertPosX, wInsertPosY );
		if ( !bOK )	
		{
			netmsgfb.emFB = EMNPC_ITEM_EXCHANGE_FB_INSERT_FAIL;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
			CDebugSet::ToFileWithTime( "_npcexchange.txt", "[%d][%s] EMNPC_ITEM_EXCHANGE_FB_INSERT_FAIL", m_dwCharID, m_szName );
			return E_FAIL;
		}

		GLITEMLMT::GetInstance().ReqItemRoute ( snew_item, ID_CHAR, 0, ID_CHAR, m_dwCharID, EMITEM_ROUTE_CHAR, snew_item.wTurnNum );

		GLMSG::SNETPC_INVEN_INSERT NetMsgInven;
		NetMsgInven.Data = *m_cInventory.GetItem ( wInsertPosX, wInsertPosY );
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgInven);

		/*activity system, Juver, 2017/11/03 */
		DoActivityTakeItem( snew_item.sNativeID, snew_item.wTurnNum );
	}

	netmsgfb.emFB = EMNPC_ITEM_EXCHANGE_FB_OK;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);

	return S_OK;
}

HRESULT GLChar::ReqItemCompoundStart( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_ITEM_COMPOUND_START* pnetmsgclient = (GLMSG::SNETPC_REQ_ITEM_COMPOUND_START*)nmg;
	GLMSG::SNETPC_REQ_ITEM_COMPOUND_START_FB	netmsgfb;

	if ( !RANPARAM::bFeatureProduct )
	{
		netmsgfb.emFB = RAN_PRODUCT_FB_FEATURE_OFF;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_FB_FEATURE_OFF %d task:%d", m_dwCharID, m_szName,
		pnetmsgclient->dwProductID, m_dwItemCompoundTaskID );
		return E_FAIL;
	}

	if ( m_bItemCompoundTask )
	{
		netmsgfb.emFB = RAN_PRODUCT_FB_TASK_RUNNING;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_FB_TASK_RUNNING %d task:%d", m_dwCharID, m_szName,
		pnetmsgclient->dwProductID, m_dwItemCompoundTaskID );
		return E_FAIL;
	}

	const ITEM_MIX* pitem_mix = GLItemMixMan::GetInstance().GetProduct( pnetmsgclient->dwProductID );
	if ( !pitem_mix )
	{
		netmsgfb.emFB = RAN_PRODUCT_FB_INVALID_ID;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_FB_INVALID_ID %d", m_dwCharID, m_szName,
		pnetmsgclient->dwProductID );
		return E_FAIL;
	}

	//money check
	if( m_lnMoney < ( pitem_mix->dwPrice * pnetmsgclient->nProductCount ) )
	{
		netmsgfb.emFB = RAN_PRODUCT_FB_INVALID_COST;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_FB_INVALID_COST %d char money:%I64d", m_dwCharID, m_szName,
		pitem_mix->dwPrice, m_lnMoney );
		return E_FAIL;
	}

	//level check
	if ( m_wLevel < pitem_mix->wLevelReq )
	{
		netmsgfb.emFB = RAN_PRODUCT_FB_INVALID_CHAR_LEVEL;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_FB_INVALID_CHAR_LEVEL %d char level:%d", m_dwCharID, m_szName,
		pitem_mix->wLevelReq, m_wLevel );
		return E_FAIL;
	}

	SITEM* pitem_data_result = GLItemMan::GetInstance().GetItem( pitem_mix->sResultItem.sNID );
	if ( !pitem_data_result )
	{
		netmsgfb.emFB = RAN_PRODUCT_FB_RESULT_ITEM_INVALID;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_FB_RESULT_ITEM_INVALID %d~%d", m_dwCharID, m_szName,
			pitem_mix->sResultItem.sNID.wMainID, pitem_mix->sResultItem.sNID.wSubID );
		return E_FAIL;
	}

	WORD winvensizex = pitem_data_result->sBasicOp.wInvenSizeX;
	WORD winvensizey = pitem_data_result->sBasicOp.wInvenSizeY;

	BOOL bITEM_SPACE(FALSE);
	if ( pitem_data_result->ISPILE() )
	{
		WORD wPILENUM = pitem_data_result->sDrugOp.wPileNum;
		SNATIVEID sNID = pitem_data_result->sBasicOp.sNativeID;
		int nNum = pitem_mix->sResultItem.nNum * pnetmsgclient->nProductCount;
		WORD wREQINSRTNUM = ( nNum * pitem_data_result->GETAPPLYNUM() );

		bITEM_SPACE = m_cInventory.ValidPileInsrt ( wREQINSRTNUM, sNID, wPILENUM, winvensizex, winvensizey );

		if ( !bITEM_SPACE )
		{
			netmsgfb.emFB = RAN_PRODUCT_FB_RESULT_ITEM_NO_SPACE;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
			CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_FB_RESULT_ITEM_NO_SPACE", m_dwCharID, m_szName );
			return E_FAIL;
		}
	}
	else
	{
		WORD wInsertPosX(0), wInsertPosY(0);
		bITEM_SPACE = m_cInventory.FindInsrtable ( winvensizex, winvensizey, wInsertPosX, wInsertPosY );
		if ( !bITEM_SPACE )	
		{
			netmsgfb.emFB = RAN_PRODUCT_FB_RESULT_ITEM_NO_SPACE;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
			CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_FB_RESULT_ITEM_NO_SPACE", m_dwCharID, m_szName );
			return E_FAIL;
		}
	}

	//requirements check
	/*
	for ( int i=0; i<ITEMMIX_ITEMNUM; ++i )
	{	
		SITEM* pitem_data_require = GLItemMan::GetInstance().GetItem( pitem_mix->sMeterialItem[i].sNID );
		if ( pitem_data_require )
		{
			if ( pitem_data_require->ISPILE() )
			{
				WORD wNum = ( pitem_mix->sMeterialItem[i].nNum * pnetmsgclient->nProductCount );
				WORD witem_num = m_cInventory.CountTurnItem( pitem_data_require->sBasicOp.sNativeID );
				if ( witem_num < wNum )
				{
					netmsgfb.emFB = RAN_PRODUCT_FB_MATERIAL;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
					CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_FB_MATERIAL %d~%d %d", m_dwCharID, m_szName,
					pitem_data_require->sBasicOp.sNativeID.wMainID, pitem_data_require->sBasicOp.sNativeID.wSubID, wNum );
					return E_FAIL;
				}

				BOOL bDelete = DeletePileItem( pitem_data_require->sBasicOp.sNativeID, wNum );
				if ( !bDelete )
				{
					netmsgfb.emFB = RAN_PRODUCT_FB_FAIL_DELETE;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
					CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_FB_FAIL_DELETE %d~%d %d", m_dwCharID, m_szName,
					pitem_data_require->sBasicOp.sNativeID.wMainID, pitem_data_require->sBasicOp.sNativeID.wSubID, wNum );
					return E_FAIL;
				}
			}
			else
			{
				//not pile just check if exist
				SINVENITEM* pinvenitem = m_cInventory.FindItem( pitem_data_require->sBasicOp.sNativeID );
				if ( !pinvenitem )
				{
					netmsgfb.emFB = RAN_PRODUCT_FB_MATERIAL;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
					CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_FB_MATERIAL %d~%d", m_dwCharID, m_szName,
					pitem_data_require->sBasicOp.sNativeID.wMainID, pitem_data_require->sBasicOp.sNativeID.wSubID );
					return E_FAIL;
				}

				if ( pinvenitem->sItemCustom.nidDISGUISE !=SNATIVEID(false) )
				{
					netmsgfb.emFB = RAN_PRODUCT_FB_FAIL_COSTUME;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
					CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_FB_MATERIAL %d~%d  %d~%d", m_dwCharID, m_szName,
					pitem_data_require->sBasicOp.sNativeID.wMainID, pitem_data_require->sBasicOp.sNativeID.wSubID,
					pinvenitem->sItemCustom.nidDISGUISE.wMainID, pinvenitem->sItemCustom.nidDISGUISE.wSubID );
					return E_FAIL;
				}

				WORD wposX = pinvenitem->wPosX;
				WORD wposY = pinvenitem->wPosY;

				BOOL bDelete = m_cInventory.DeleteItem ( wposX, wposY );
				if ( !bDelete )
				{
					netmsgfb.emFB = RAN_PRODUCT_FB_FAIL_DELETE;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
					CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] EMNPC_ITEM_EXCHANGE_FB_DELETE_FAIL %d~%d %d~%d", m_dwCharID, m_szName,
					pitem_data_require->sBasicOp.sNativeID.wMainID, pitem_data_require->sBasicOp.sNativeID.wSubID, wposX, wposY );
					return E_FAIL;
				}

				GLMSG::SNETPC_INVEN_DELETE NetMsg_Inven_Delete;
				NetMsg_Inven_Delete.wPosX = wposX;
				NetMsg_Inven_Delete.wPosY = wposY;
				GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg_Inven_Delete);
			}
		}
	}
	*/

	//consume gold
	/*
	if ( pitem_mix->dwPrice > 0 )
	{
		CheckMoneyUpdate( m_lnMoney, pitem_mix->dwPrice, FALSE, "ITEM_COMPOUND" );
		m_bMoneyUpdate = TRUE;

		m_lnMoney -= ( pitem_mix->dwPrice * pnetmsgclient->nProductCount );

		GLMSG::SNETPC_UPDATE_MONEY NetMsgMoney;
		NetMsgMoney.lnMoney = m_lnMoney;
		NetMsgMoney.dwPremium = m_dwPremiumPoints;
		NetMsgMoney.dwCombat = m_dwCombatPoints;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &NetMsgMoney );
	}
	*/

	m_bItemCompoundTask = TRUE;
	m_dwItemCompoundTaskID = pitem_mix->dwKey;
	m_fItemCompoundTaskTime = pitem_mix->fTime;
	m_fItemCompoundTaskTimer = 0.0f;
	m_nProductCounter = 1;
	m_nProductMake = pnetmsgclient->nProductCount;

	netmsgfb.emFB = RAN_PRODUCT_FB_STARTED;
	netmsgfb.bItemCompoundTask = m_bItemCompoundTask;
	netmsgfb.dwItemCompoundTaskID = m_dwItemCompoundTaskID;
	netmsgfb.fItemCompoundTaskTime = m_fItemCompoundTaskTime;
	netmsgfb.fItemCompoundTaskTimer = m_fItemCompoundTaskTimer;

	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);

	return S_OK;
}

HRESULT GLChar::ReqItemCompoundCancel( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_ITEM_COMPOUND_CANCEL* pnetmsgclient = (GLMSG::SNETPC_REQ_ITEM_COMPOUND_CANCEL*)nmg;

	if( m_bItemCompoundTask )
	{
		m_bItemCompoundTask = FALSE;
		
		GLMSG::SNETPC_REQ_ITEM_COMPOUND_START_FB	netmsgfb;
		netmsgfb.emFB = RAN_PRODUCT_FB_CANCEL;
		netmsgfb.bItemCompoundTask = m_bItemCompoundTask;
		netmsgfb.fItemCompoundTaskTime = m_fItemCompoundTaskTime;
		netmsgfb.fItemCompoundTaskTimer = m_fItemCompoundTaskTimer;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);	

		m_fItemCompoundTaskTimer = 0.0f;
		m_fItemCompoundTaskTime = 0.0f;
	}

	return S_OK;
}

HRESULT GLChar::DoItemCompoundTask()
{
	GLMSG::SNETPC_REQ_ITEM_COMPOUND_PROCESS_FB	netmsgfb;
	GLMSG::SNETPC_REQ_ITEM_COMPOUND_START_FB	netmsgfb2;

	const ITEM_MIX* pitem_mix = GLItemMixMan::GetInstance().GetProduct( m_dwItemCompoundTaskID );
	if ( !pitem_mix )
	{
		netmsgfb.emFB = RAN_PRODUCT_PROCESS_FB_FAILED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_PROCESS_FB_FAILED %d", m_dwCharID, m_szName,
		m_dwItemCompoundTaskID );
		return E_FAIL;
	}

	//money check
	if( m_lnMoney < pitem_mix->dwPrice )
	{
		netmsgfb2.emFB = RAN_PRODUCT_FB_INVALID_COST;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_FB_INVALID_COST %d char money:%I64d", m_dwCharID, m_szName,
		pitem_mix->dwPrice, m_lnMoney );
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
				WORD wNum = pitem_mix->sMeterialItem[i].nNum;
				WORD witem_num = m_cInventory.CountTurnItem( pitem_data_require->sBasicOp.sNativeID );
				if ( witem_num < wNum )
				{
					netmsgfb2.emFB = RAN_PRODUCT_FB_MATERIAL;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb2);
					CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_FB_MATERIAL %d~%d %d", m_dwCharID, m_szName,
					pitem_data_require->sBasicOp.sNativeID.wMainID, pitem_data_require->sBasicOp.sNativeID.wSubID, wNum );
					return E_FAIL;
				}

				BOOL bDelete = DeletePileItem( pitem_data_require->sBasicOp.sNativeID, wNum );
				if ( !bDelete )
				{
					netmsgfb2.emFB = RAN_PRODUCT_FB_FAIL_DELETE;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
					CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_FB_FAIL_DELETE %d~%d %d", m_dwCharID, m_szName,
					pitem_data_require->sBasicOp.sNativeID.wMainID, pitem_data_require->sBasicOp.sNativeID.wSubID, wNum );
					return E_FAIL;
				}
			}
			else
			{
				//not pile just check if exist
				SINVENITEM* pinvenitem = m_cInventory.FindItem( pitem_data_require->sBasicOp.sNativeID );
				if ( !pinvenitem )
				{
					netmsgfb2.emFB = RAN_PRODUCT_FB_MATERIAL;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
					CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_FB_MATERIAL %d~%d", m_dwCharID, m_szName,
					pitem_data_require->sBasicOp.sNativeID.wMainID, pitem_data_require->sBasicOp.sNativeID.wSubID );
					return E_FAIL;
				}

				if ( pinvenitem->sItemCustom.nidDISGUISE !=SNATIVEID(false) )
				{
					netmsgfb2.emFB = RAN_PRODUCT_FB_FAIL_COSTUME;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
					CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_FB_MATERIAL %d~%d  %d~%d", m_dwCharID, m_szName,
					pitem_data_require->sBasicOp.sNativeID.wMainID, pitem_data_require->sBasicOp.sNativeID.wSubID,
					pinvenitem->sItemCustom.nidDISGUISE.wMainID, pinvenitem->sItemCustom.nidDISGUISE.wSubID );
					return E_FAIL;
				}

				WORD wposX = pinvenitem->wPosX;
				WORD wposY = pinvenitem->wPosY;

				BOOL bDelete = m_cInventory.DeleteItem ( wposX, wposY );
				if ( !bDelete )
				{
					netmsgfb2.emFB = RAN_PRODUCT_FB_FAIL_DELETE;
					GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
					CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] EMNPC_ITEM_EXCHANGE_FB_DELETE_FAIL %d~%d %d~%d", m_dwCharID, m_szName,
					pitem_data_require->sBasicOp.sNativeID.wMainID, pitem_data_require->sBasicOp.sNativeID.wSubID, wposX, wposY );
					return E_FAIL;
				}

				GLMSG::SNETPC_INVEN_DELETE NetMsg_Inven_Delete;
				NetMsg_Inven_Delete.wPosX = wposX;
				NetMsg_Inven_Delete.wPosY = wposY;
				GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg_Inven_Delete);
			}
		}
	}

	//consume gold
	if ( pitem_mix->dwPrice > 0 )
	{
		CheckMoneyUpdate( m_lnMoney, pitem_mix->dwPrice, FALSE, "ITEM_COMPOUND" );
		m_bMoneyUpdate = TRUE;

		m_lnMoney -= pitem_mix->dwPrice;

		GLMSG::SNETPC_UPDATE_MONEY NetMsgMoney;
		NetMsgMoney.lnMoney = m_lnMoney;
		/*dmk14 ingame web*/
		NetMsgMoney.dwPremium = m_dwPremiumPoints;
		NetMsgMoney.dwCombat = m_dwCombatPoints;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &NetMsgMoney );
	}


	float fRate = (float)pitem_mix->dwRate;
	if ( !RANDOM_GEN( fRate ) )
	{
		netmsgfb.emFB = RAN_PRODUCT_PROCESS_FB_FAILED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_PROCESS_FB_FAILED %d %g", m_dwCharID, m_szName,
		m_dwItemCompoundTaskID, fRate );
		return E_FAIL;
	}

	SITEM* pitem_data_result = GLItemMan::GetInstance().GetItem( pitem_mix->sResultItem.sNID );
	if ( !pitem_data_result )
	{
		netmsgfb.emFB = RAN_PRODUCT_PROCESS_FB_FAILED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
		CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_PROCESS_FB_FAILED %d~%d", m_dwCharID, m_szName,
		pitem_mix->sResultItem.sNID.wMainID, pitem_mix->sResultItem.sNID.wSubID );
		return E_FAIL;
	}

	WORD winvensizex = pitem_data_result->sBasicOp.wInvenSizeX;
	WORD winvensizey = pitem_data_result->sBasicOp.wInvenSizeY;

	BOOL bITEM_SPACE(FALSE);
	if ( pitem_data_result->ISPILE() )
	{
		WORD wPILENUM = pitem_data_result->sDrugOp.wPileNum;
		SNATIVEID sNID = pitem_data_result->sBasicOp.sNativeID;
		WORD wREQINSRTNUM = ( pitem_mix->sResultItem.nNum * pitem_data_result->GETAPPLYNUM() );

		bITEM_SPACE = m_cInventory.ValidPileInsrt ( wREQINSRTNUM, sNID, wPILENUM, winvensizex, winvensizey );

		if ( !bITEM_SPACE )
		{
			netmsgfb.emFB = RAN_PRODUCT_PROCESS_FB_FULL_INVEN;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
			CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_PROCESS_FB_FULL_INVEN", m_dwCharID, m_szName );
			return E_FAIL;
		}

		SITEMCUSTOM snew_item(sNID);
		snew_item.cGenType = EMGEN_SHOP;
		snew_item.cChnID = (BYTE)GLGaeaServer::GetInstance().GetServerChannel();
		snew_item.cFieldID = (BYTE)GLGaeaServer::GetInstance().GetFieldSvrID();
		snew_item.lnGenNum = GLITEMLMT::GetInstance().RegItemGen ( pitem_data_result->sBasicOp.sNativeID, EMGEN_SHOP );
		snew_item.wTurnNum = wPILENUM;
		snew_item.tBORNTIME = CTime::GetCurrentTime().GetTime();

		/*item color, Juver, 2018/01/08 */
		SITEM* pitem_data_check = GLItemMan::GetInstance().GetItem( snew_item.sNativeID );
		if ( pitem_data_check && pitem_data_check->sBasicOp.bItemColor )
		{
			snew_item.wColor1 = pitem_data_check->sBasicOp.wItemColor1;
			snew_item.wColor2 = pitem_data_check->sBasicOp.wItemColor2;
		}

		GLITEMLMT::GetInstance().ReqItemRoute ( snew_item, ID_CHAR, 0, ID_CHAR, m_dwCharID, EMITEM_ROUTE_CHAR, snew_item.wTurnNum );

		BOOL bOK = InsertPileItem ( snew_item, wREQINSRTNUM );
		if ( !bOK )	
		{
			netmsgfb.emFB = RAN_PRODUCT_PROCESS_FB_FAILED_INSERT;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
			CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_PROCESS_FB_FAILED_INSERT", m_dwCharID, m_szName );
			return E_FAIL;
		}

		/*activity system, Juver, 2017/11/03 */
		DoActivityTakeItem( snew_item.sNativeID, snew_item.wTurnNum );
	}
	else
	{
		WORD wInsertPosX(0), wInsertPosY(0);
		bITEM_SPACE = m_cInventory.FindInsrtable ( winvensizex, winvensizey, wInsertPosX, wInsertPosY );
		if ( !bITEM_SPACE )	
		{
			netmsgfb.emFB = RAN_PRODUCT_PROCESS_FB_FULL_INVEN;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
			CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_PROCESS_FB_FULL_INVEN", m_dwCharID, m_szName );
			return E_FAIL;
		}

		SITEMCUSTOM snew_item( pitem_data_result->sBasicOp.sNativeID );
		snew_item.cDAMAGE = pitem_mix->cDAMAGE;
		snew_item.cDEFENSE = pitem_mix->cDEFENSE;
		snew_item.cRESIST_FIRE = pitem_mix->cRESIST_FIRE;
		snew_item.cRESIST_ICE = pitem_mix->cRESIST_ICE;
		snew_item.cRESIST_ELEC = pitem_mix->cRESIST_ELEC;
		snew_item.cRESIST_POISON = pitem_mix->cRESIST_POISON;
		snew_item.cRESIST_SPIRIT = pitem_mix->cRESIST_SPIRIT;

		snew_item.tBORNTIME = CTime::GetCurrentTime().GetTime();
		snew_item.wTurnNum = 1;
		snew_item.cGenType = (BYTE) EMGEN_SHOP;
		snew_item.cChnID = (BYTE)GLGaeaServer::GetInstance().GetServerChannel();
		snew_item.cFieldID = (BYTE) GLGaeaServer::GetInstance().GetFieldSvrID(); 
		snew_item.lnGenNum = GLITEMLMT::GetInstance().RegItemGen ( pitem_data_result->sBasicOp.sNativeID, EMGEN_SHOP );

		/*item color, Juver, 2018/01/08 */
		SITEM* pitem_data_check = GLItemMan::GetInstance().GetItem( snew_item.sNativeID );
		if ( pitem_data_check && pitem_data_check->sBasicOp.bItemColor )
		{
			snew_item.wColor1 = pitem_data_check->sBasicOp.wItemColor1;
			snew_item.wColor2 = pitem_data_check->sBasicOp.wItemColor2;
		}

		if ( pitem_mix->bGenerateRandomValue )
		{
			if( snew_item.GENERATE_RANDOM_OPT () )
			{
				GLITEMLMT::GetInstance().ReqRandomItem( snew_item );
			}
		}
		
		BOOL bOK = m_cInventory.InsertItem ( snew_item, wInsertPosX, wInsertPosY );
		if ( !bOK )	
		{
			netmsgfb.emFB = RAN_PRODUCT_PROCESS_FB_FAILED_INSERT;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);
			CDebugSet::ToFileWithTime( "_itemcompound.txt", "[%d][%s] RAN_PRODUCT_PROCESS_FB_FAILED_INSERT", m_dwCharID, m_szName );
			return E_FAIL;
		}

		GLITEMLMT::GetInstance().ReqItemRoute ( snew_item, ID_CHAR, 0, ID_CHAR, m_dwCharID, EMITEM_ROUTE_CHAR, snew_item.wTurnNum );

		GLMSG::SNETPC_INVEN_INSERT NetMsgInven;
		NetMsgInven.Data = *m_cInventory.GetItem ( wInsertPosX, wInsertPosY );
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgInven);

		/*activity system, Juver, 2017/11/03 */
		DoActivityTakeItem( snew_item.sNativeID, snew_item.wTurnNum );
	}

	netmsgfb.emFB = RAN_PRODUCT_PROCESS_FB_OK;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);

	if( m_nProductMake > m_nProductCounter )
	{
		m_bItemCompoundTask = TRUE;
		m_fItemCompoundTaskTime = pitem_mix->fTime;
		m_fItemCompoundTaskTimer = 0.0f;

		GLMSG::SNETPC_REQ_ITEM_COMPOUND_START_FB	netmsgfb;
		netmsgfb.emFB = RAN_PRODUCT_FB_STARTED;
		netmsgfb.bItemCompoundTask = m_bItemCompoundTask;
		netmsgfb.dwItemCompoundTaskID = m_dwItemCompoundTaskID;
		netmsgfb.fItemCompoundTaskTime = m_fItemCompoundTaskTime;
		netmsgfb.fItemCompoundTaskTimer = m_fItemCompoundTaskTimer;

		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&netmsgfb);

		m_nProductCounter++;
	}

	return S_OK;
}

HRESULT GLChar::MsgVehicleChangeColor( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_VEHICLE_REQ_CHANGE_COLOR* pNetMsg = ( GLMSG::SNET_VEHICLE_REQ_CHANGE_COLOR* ) nmg;

	GLMSG::SNET_VEHICLE_REQ_CHANGE_COLOR_FB NetMsgFB;
	SNATIVEID sNativeID;

	SINVENITEM* pinvencard = m_cInventory.FindItem( ITEM_CARD_BIKECOLOR );
	if ( !pinvencard ) 
	{
		NetMsgFB.emFB = EMREQ_CHANGE_BIKE_COLOR_FB_INVALID_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	SITEM* pitemcard = GLItemMan::GetInstance().GetItem( pinvencard->sItemCustom.sNativeID );
	if ( pitemcard->sBasicOp.emItemType != ITEM_CARD_BIKECOLOR )	
	{
		NetMsgFB.emFB = EMREQ_CHANGE_BIKE_COLOR_FB_INVALID_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( !m_bVehicle )
	{
		NetMsgFB.emFB = EMREQ_CHANGE_BIKE_COLOR_FB_VEHICLE_INACTIVE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	SITEM* pVehicle = GET_SLOT_ITEMDATA( SLOT_VEHICLE );
	if ( !pVehicle )
	{
		NetMsgFB.emFB = EMREQ_CHANGE_BIKE_COLOR_FB_VEHICLE_INACTIVE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( pVehicle->sVehicle.emVehicleType != VEHICLE_TYPE_BIKE )
	{
		NetMsgFB.emFB = EMREQ_CHANGE_BIKE_COLOR_FB_VEHICLE_NOT_BIKE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	BOOL bDelete = DoDrugInvenItem( pinvencard->wPosX, pinvencard->wPosY );
	if ( !bDelete )
	{
		NetMsgFB.emFB = EMREQ_CHANGE_BIKE_COLOR_FB_CHANGE_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	for( WORD i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
	{
		m_sVehicle.m_wColor[i] = pNetMsg->wColor[i];
	}

	NetMsgFB.emFB = EMREQ_CHANGE_BIKE_COLOR_FB_CHANGE_DONE;
	NetMsgFB.dwVehicleID = m_sVehicle.m_dwGUID;

	GLMSG::SNET_VEHICLE_REQ_CHANGE_COLOR_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID = m_dwGaeaID;
	NetMsgBrd.dwVehicleID = m_sVehicle.m_dwGUID;

	for( WORD i=0; i<BIKE_COLOR_SLOT_PART_SIZE; ++i )
	{
		NetMsgFB.wColor[i] = m_sVehicle.m_wColor[i];
		NetMsgBrd.wColor[i] = m_sVehicle.m_wColor[i];
	}
	
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
		
	return S_OK;

}

/*rv card, Juver, 2017/11/25 */
HRESULT GLChar::MsgRandomOptionChange ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_INVEN_RANDOM_OPTION_CHANGE *pnet_msg_client = (GLMSG::SNET_INVEN_RANDOM_OPTION_CHANGE *)nmg;
	GLMSG::SNET_INVEN_RANDOM_OPTION_CHANGE_FB net_msg_fb;

	if ( !VALID_HOLD_ITEM() )	return S_FALSE;

	SITEM* pitem_data_hold = GLItemMan::GetInstance().GetItem ( m_PutOnItems[SLOT_HOLD].sNativeID );
	if ( !pitem_data_hold )		return E_FAIL;

	if ( CheckCoolTime( pitem_data_hold->sBasicOp.sNativeID ) ) return E_FAIL;

	if ( pitem_data_hold->sBasicOp.emItemType != ITEM_RANDOM_OPTION_CARD )
	{
		net_msg_fb.emFB = EM_RANDOM_OPTION_CHANGE_INVALID_CARD_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
		return E_FAIL;
	}

	SINVENITEM* pinven_item_target = m_cInventory.GetItem ( pnet_msg_client->wPosX, pnet_msg_client->wPosY );
	if ( !pinven_item_target )
	{
		net_msg_fb.emFB = EM_RANDOM_OPTION_CHANGE_INVALID_TARGET_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
		return E_FAIL;
	}

	SITEM* pitem_data_target = GLItemMan::GetInstance().GetItem ( pinven_item_target->sItemCustom.sNativeID );
	if ( !pitem_data_target )
	{
		net_msg_fb.emFB = EM_RANDOM_OPTION_CHANGE_INVALID_TARGET_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
		return E_FAIL;
	}

	if ( pitem_data_target->sBasicOp.emItemType != ITEM_SUIT )
	{	
		net_msg_fb.emFB = EM_RANDOM_OPTION_CHANGE_TARGET_NOT_SUIT;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
		return E_FAIL;
	}

	//pvp item
	if ( !pitem_data_target->sRandomOpt.bPVPItem )
	{
		net_msg_fb.emFB = EM_RANDOM_OPTION_CHANGE_SUIT_MISMATCH;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
		return E_FAIL;
	}

	if ( !pitem_data_hold->sRvCard.bUseSuit[pitem_data_target->sSuitOp.emSuit] )
	{	
		net_msg_fb.emFB = EM_RANDOM_OPTION_CHANGE_SUIT_MISMATCH;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
		return E_FAIL;
	}


	if ( pitem_data_hold->sRvCard.bReplaceOpt )
	{
		if ( pinven_item_target->sItemCustom.cOptTYPE1 == (BYTE) pitem_data_hold->sRvCard.emOption 
			&& pinven_item_target->sItemCustom.nOptVALUE1 >= (short) pitem_data_hold->sRvCard.wValue )
		{
			net_msg_fb.emFB = EM_RANDOM_OPTION_CHANGE_TARGET_OPTION_ALREADY_SET;
			GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
			return E_FAIL;
		}

		if ( pinven_item_target->sItemCustom.cOptTYPE2 == (BYTE) pitem_data_hold->sRvCard.emOption 
			&& pinven_item_target->sItemCustom.nOptVALUE2 >= (short) pitem_data_hold->sRvCard.wValue )
		{
			net_msg_fb.emFB = EM_RANDOM_OPTION_CHANGE_TARGET_OPTION_ALREADY_SET;
			GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
			return E_FAIL;
		}

		if ( pinven_item_target->sItemCustom.cOptTYPE3 == (BYTE) pitem_data_hold->sRvCard.emOption 
			&& pinven_item_target->sItemCustom.nOptVALUE3 >= (short) pitem_data_hold->sRvCard.wValue )
		{
			net_msg_fb.emFB = EM_RANDOM_OPTION_CHANGE_TARGET_OPTION_ALREADY_SET;
			GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
			return E_FAIL;
		}

		if ( pinven_item_target->sItemCustom.cOptTYPE4 == (BYTE) pitem_data_hold->sRvCard.emOption 
			&& pinven_item_target->sItemCustom.nOptVALUE4 >= (short) pitem_data_hold->sRvCard.wValue )
		{
			net_msg_fb.emFB = EM_RANDOM_OPTION_CHANGE_TARGET_OPTION_ALREADY_SET;
			GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
			return E_FAIL;
		}
	}

	if ( pitem_data_hold->sRvCard.bCheckExist )
	{
		BOOL bfound_option = FALSE;

		if ( !bfound_option && pinven_item_target->sItemCustom.cOptTYPE1 == (BYTE) pitem_data_hold->sRvCard.emOption )
		{
			pinven_item_target->sItemCustom.cOptTYPE1 = (BYTE) pitem_data_hold->sRvCard.emOption;
			if ( pitem_data_hold->sRvCard.bReplaceOpt )
				pinven_item_target->sItemCustom.nOptVALUE1 = (short) pitem_data_hold->sRvCard.wValue;
			else
				pinven_item_target->sItemCustom.nOptVALUE1 += (short) pitem_data_hold->sRvCard.wValue;
			bfound_option = TRUE;
		}

		if ( !bfound_option && pinven_item_target->sItemCustom.cOptTYPE2 == (BYTE) pitem_data_hold->sRvCard.emOption )
		{
			pinven_item_target->sItemCustom.cOptTYPE2 = (BYTE) pitem_data_hold->sRvCard.emOption;
			if ( pitem_data_hold->sRvCard.bReplaceOpt )
				pinven_item_target->sItemCustom.nOptVALUE2 = (short) pitem_data_hold->sRvCard.wValue;
			else
				pinven_item_target->sItemCustom.nOptVALUE2 += (short) pitem_data_hold->sRvCard.wValue;
			bfound_option = TRUE;
		}

		if ( !bfound_option && pinven_item_target->sItemCustom.cOptTYPE3 == (BYTE) pitem_data_hold->sRvCard.emOption )
		{
			pinven_item_target->sItemCustom.cOptTYPE3 = (BYTE) pitem_data_hold->sRvCard.emOption;
			if ( pitem_data_hold->sRvCard.bReplaceOpt )
				pinven_item_target->sItemCustom.nOptVALUE3 = (short) pitem_data_hold->sRvCard.wValue;
			else
				pinven_item_target->sItemCustom.nOptVALUE3 += (short) pitem_data_hold->sRvCard.wValue;
			bfound_option = TRUE;
		}

		if ( !bfound_option && pinven_item_target->sItemCustom.cOptTYPE4 == (BYTE) pitem_data_hold->sRvCard.emOption )
		{
			pinven_item_target->sItemCustom.cOptTYPE4 = (BYTE) pitem_data_hold->sRvCard.emOption;
			if ( pitem_data_hold->sRvCard.bReplaceOpt )
				pinven_item_target->sItemCustom.nOptVALUE4 = (short) pitem_data_hold->sRvCard.wValue;
			else
				pinven_item_target->sItemCustom.nOptVALUE4 += (short) pitem_data_hold->sRvCard.wValue;
			bfound_option = TRUE;
		}

		if ( !bfound_option )
		{
			net_msg_fb.emFB = EM_RANDOM_OPTION_CHANGE_TARGET_OPTION_NOT_EXIST;
			GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
			return E_FAIL;
		}
		else
		{
			BOOL bErase = DoDrugSlotItem(SLOT_HOLD);
			if ( !bErase )
			{
				SINVENITEM* pinven_item_card = m_cInventory.FindItem( ITEM_RANDOM_OPTION_CARD );
				if ( pinven_item_card )
					bErase = DoDrugInvenItem( pinven_item_card->wPosX, pinven_item_card->wPosY );
			}

			if ( !bErase )
				CDebugSet::ToFileWithTime( "_random_option_change.txt", "[%d] %s cant find option card", m_dwCharID, m_szName );

			GLMSG::SNET_INVEN_ITEM_UPDATE net_msg_client_item_update;
			net_msg_client_item_update.wPosX = pnet_msg_client->wPosX;
			net_msg_client_item_update.wPosY = pnet_msg_client->wPosY;
			net_msg_client_item_update.sItemCustom = pinven_item_target->sItemCustom;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &net_msg_client_item_update );

			GLITEMLMT::GetInstance().ReqItemConversion ( pinven_item_target->sItemCustom, ID_CHAR, m_dwCharID );

			net_msg_fb.emFB = EM_RANDOM_OPTION_CHANGE_DONE_CHANGE;
			GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
		}
	}
	else
	{
		BOOL bfound_free = FALSE;

		if ( !bfound_free && pinven_item_target->sItemCustom.cOptTYPE1 == (BYTE) pitem_data_hold->sRvCard.emOption )
		{
			pinven_item_target->sItemCustom.cOptTYPE1 = (BYTE) pitem_data_hold->sRvCard.emOption;
			if ( pitem_data_hold->sRvCard.bReplaceOpt )
				pinven_item_target->sItemCustom.nOptVALUE1 = (short) pitem_data_hold->sRvCard.wValue;
			else
				pinven_item_target->sItemCustom.nOptVALUE1 += (short) pitem_data_hold->sRvCard.wValue;
			bfound_free = TRUE;
		}

		if ( !bfound_free && pinven_item_target->sItemCustom.cOptTYPE2 == (BYTE) pitem_data_hold->sRvCard.emOption )
		{
			pinven_item_target->sItemCustom.cOptTYPE2 = (BYTE) pitem_data_hold->sRvCard.emOption;
			if ( pitem_data_hold->sRvCard.bReplaceOpt )
				pinven_item_target->sItemCustom.nOptVALUE2 = (short) pitem_data_hold->sRvCard.wValue;
			else
				pinven_item_target->sItemCustom.nOptVALUE2 += (short) pitem_data_hold->sRvCard.wValue;
			bfound_free = TRUE;
		}

		if ( !bfound_free && pinven_item_target->sItemCustom.cOptTYPE3 == (BYTE) pitem_data_hold->sRvCard.emOption )
		{
			pinven_item_target->sItemCustom.cOptTYPE3 = (BYTE) pitem_data_hold->sRvCard.emOption;
			if ( pitem_data_hold->sRvCard.bReplaceOpt )
				pinven_item_target->sItemCustom.nOptVALUE3 = (short) pitem_data_hold->sRvCard.wValue;
			else
				pinven_item_target->sItemCustom.nOptVALUE3 += (short) pitem_data_hold->sRvCard.wValue;
			bfound_free = TRUE;
		}

		if ( !bfound_free && pinven_item_target->sItemCustom.cOptTYPE4 == (BYTE) pitem_data_hold->sRvCard.emOption )
		{
			pinven_item_target->sItemCustom.cOptTYPE4 = (BYTE) pitem_data_hold->sRvCard.emOption;
			if ( pitem_data_hold->sRvCard.bReplaceOpt )
				pinven_item_target->sItemCustom.nOptVALUE4 = (short) pitem_data_hold->sRvCard.wValue;
			else
				pinven_item_target->sItemCustom.nOptVALUE4 += (short) pitem_data_hold->sRvCard.wValue;
			bfound_free = TRUE;
		}


		if ( !bfound_free && pinven_item_target->sItemCustom.cOptTYPE1 == EMR_OPT_NULL )
		{
			pinven_item_target->sItemCustom.cOptTYPE1 = (BYTE) pitem_data_hold->sRvCard.emOption;
			pinven_item_target->sItemCustom.nOptVALUE1 = (short) pitem_data_hold->sRvCard.wValue;
			bfound_free = TRUE;
		}

		if ( !bfound_free && pinven_item_target->sItemCustom.cOptTYPE2 == EMR_OPT_NULL )
		{
			pinven_item_target->sItemCustom.cOptTYPE2 = (BYTE) pitem_data_hold->sRvCard.emOption;
			pinven_item_target->sItemCustom.nOptVALUE2 = (short) pitem_data_hold->sRvCard.wValue;
			bfound_free = TRUE;
		}

		if ( !bfound_free && pinven_item_target->sItemCustom.cOptTYPE3 == EMR_OPT_NULL )
		{
			pinven_item_target->sItemCustom.cOptTYPE3 = (BYTE) pitem_data_hold->sRvCard.emOption;
			pinven_item_target->sItemCustom.nOptVALUE3 = (short) pitem_data_hold->sRvCard.wValue;
			bfound_free = TRUE;
		}

		if ( !bfound_free && pinven_item_target->sItemCustom.cOptTYPE4 == EMR_OPT_NULL )
		{
			pinven_item_target->sItemCustom.cOptTYPE4 = (BYTE) pitem_data_hold->sRvCard.emOption;
			pinven_item_target->sItemCustom.nOptVALUE4 = (short) pitem_data_hold->sRvCard.wValue;
			bfound_free = TRUE;
		}

		if ( !bfound_free )
		{
			net_msg_fb.emFB = EM_RANDOM_OPTION_CHANGE_TARGET_OPTION_NOT_FREE;
			GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
			return E_FAIL;
		}
		else
		{
			BOOL bErase = DoDrugSlotItem(SLOT_HOLD);
			if ( !bErase )
			{
				SINVENITEM* pinven_item_card = m_cInventory.FindItem( ITEM_RANDOM_OPTION_CARD );
				if ( pinven_item_card )
					bErase = DoDrugInvenItem( pinven_item_card->wPosX, pinven_item_card->wPosY );
			}

			if ( !bErase )
				CDebugSet::ToFileWithTime( "_random_option_change.txt", "[%d] %s cant find option card", m_dwCharID, m_szName );

			GLMSG::SNET_INVEN_ITEM_UPDATE net_msg_client_item_update;
			net_msg_client_item_update.wPosX = pnet_msg_client->wPosX;
			net_msg_client_item_update.wPosY = pnet_msg_client->wPosY;
			net_msg_client_item_update.sItemCustom = pinven_item_target->sItemCustom;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &net_msg_client_item_update );

			GLITEMLMT::GetInstance().ReqItemConversion ( pinven_item_target->sItemCustom, ID_CHAR, m_dwCharID );

			net_msg_fb.emFB = EM_RANDOM_OPTION_CHANGE_DONE_CHANGE;
			GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
		}
	}

	return S_OK;
}

/*nondrop card, Juver, 2017/11/26 */
HRESULT GLChar::MsgNonDropCard ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_INVEN_NONDROP_CARD *pnet_msg_client = (GLMSG::SNET_INVEN_NONDROP_CARD *)nmg;
	GLMSG::SNET_INVEN_NONDROP_CARD_FB net_msg_fb;

	if ( !VALID_HOLD_ITEM() )	return S_FALSE;

	SITEM* pitem_data_hold = GLItemMan::GetInstance().GetItem ( m_PutOnItems[SLOT_HOLD].sNativeID );
	if ( !pitem_data_hold )		return E_FAIL;

	if ( CheckCoolTime( pitem_data_hold->sBasicOp.sNativeID ) ) return E_FAIL;

	if ( pitem_data_hold->sBasicOp.emItemType != ITEM_NONDROP_CARD )
	{
		net_msg_fb.emFB = EM_NONDROP_CARD_INVALID_CARD_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
		return E_FAIL;
	}

	SINVENITEM* pinven_item_target = m_cInventory.GetItem ( pnet_msg_client->wPosX, pnet_msg_client->wPosY );
	if ( !pinven_item_target )
	{
		net_msg_fb.emFB = EM_NONDROP_CARD_INVALID_TARGET_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
		return E_FAIL;
	}

	SITEM* pitem_data_target = GLItemMan::GetInstance().GetItem ( pinven_item_target->sItemCustom.sNativeID );
	if ( !pitem_data_target )
	{
		net_msg_fb.emFB = EM_NONDROP_CARD_INVALID_TARGET_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
		return E_FAIL;
	}

	if ( pitem_data_target->sBasicOp.emItemType != ITEM_SUIT )
	{	
		net_msg_fb.emFB = EM_NONDROP_CARD_TARGET_NOT_SUIT;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
		return E_FAIL;
	}

	if ( pinven_item_target->sItemCustom.IsGM_GENITEM() )
	{
		net_msg_fb.emFB = EM_NONDROP_CARD_TARGET_ALREADY;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );
		return E_FAIL;
	}

	pinven_item_target->sItemCustom.SetGM_GEN( GetUserID() );


	BOOL bErase = DoDrugSlotItem(SLOT_HOLD);
	if ( !bErase )
	{
		SINVENITEM* pinven_item_card = m_cInventory.FindItem( ITEM_NONDROP_CARD );
		if ( pinven_item_card )
			bErase = DoDrugInvenItem( pinven_item_card->wPosX, pinven_item_card->wPosY );
	}

	if ( !bErase )
		CDebugSet::ToFileWithTime( "_nondrop_card.txt", "[%d] %s cant find nondrop card", m_dwCharID, m_szName );

	GLMSG::SNET_INVEN_ITEM_UPDATE net_msg_client_item_update;
	net_msg_client_item_update.wPosX = pnet_msg_client->wPosX;
	net_msg_client_item_update.wPosY = pnet_msg_client->wPosY;
	net_msg_client_item_update.sItemCustom = pinven_item_target->sItemCustom;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID, &net_msg_client_item_update );

	GLITEMLMT::GetInstance().ReqItemConversion ( pinven_item_target->sItemCustom, ID_CHAR, m_dwCharID );

	net_msg_fb.emFB = EM_NONDROP_CARD_DONE_CHANGE;
	GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &net_msg_fb );


	return S_OK;
}

/*regional chat, Juver, 2017/12/06 */
HRESULT GLChar::MsgRegionalChatPay ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_REGIONAL_CHAT_PAY *pnet_msg_client = (GLMSG::SNET_REGIONAL_CHAT_PAY *)nmg;
	
	if( m_lnMoney < pnet_msg_client->dwCost )
	{
		CDebugSet::ToFileWithTime( "_regionalpay.txt", "[%d][%s] not enough money %d char money:%I64d", m_dwCharID, m_szName, pnet_msg_client->dwCost, m_lnMoney );
		return E_FAIL;
	}

	if ( pnet_msg_client->dwCost <= 0 )
	{
		CDebugSet::ToFileWithTime( "_regionalpay.txt", "[%d][%s] invalid cost %d char money:%I64d", m_dwCharID, m_szName, pnet_msg_client->dwCost, m_lnMoney );
		return E_FAIL;
	}

	CheckMoneyUpdate( m_lnMoney, pnet_msg_client->dwCost, FALSE, "Regional Chat Pay." );
	m_bMoneyUpdate = TRUE;

	m_lnMoney -= pnet_msg_client->dwCost;

	if ( pnet_msg_client->dwCost>EMMONEY_LOG )
	{
		GLITEMLMT::GetInstance().ReqMoneyExc ( ID_CHAR, m_dwCharID, ID_CHAR, 0, -LONGLONG(pnet_msg_client->dwCost), EMITEM_ROUTE_DELETE );
		GLITEMLMT::GetInstance().ReqMoneyExc ( ID_CHAR, m_dwCharID, ID_CHAR, 0, m_lnMoney, EMITEM_ROUTE_CHAR );
	}

	GLMSG::SNETPC_UPDATE_MONEY NetMsgMoney;
	NetMsgMoney.lnMoney = m_lnMoney;
	/*dmk14 ingame web*/
	NetMsgMoney.dwPremium = m_dwPremiumPoints;
	NetMsgMoney.dwCombat = m_dwCombatPoints;
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgMoney );

	return S_OK;
}

/*dual pet skill, Juver, 2017/12/29 */
HRESULT	GLChar::MsgEnableDualSkill ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPET_REQ_ENABLE_DUAL_SKILL* pclient_msg = ( GLMSG::SNETPET_REQ_ENABLE_DUAL_SKILL* ) nmg;
	GLMSG::SNETPET_REQ_ENABLE_DUAL_SKILL_FB net_msg_fb_client;

	SINVENITEM* pinven_item = m_cInventory.FindPosItem ( pclient_msg->wPosX, pclient_msg->wPosY );
	if ( !pinven_item ) 
	{
		net_msg_fb_client.emFB = EMPET_REQ_ENABLE_DUAL_SKILL_INVALID_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &net_msg_fb_client );
		return E_FAIL;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pinven_item->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		net_msg_fb_client.emFB = EMPET_REQ_ENABLE_DUAL_SKILL_INVALID_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &net_msg_fb_client );
		return E_FAIL;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_PET_CARD )	
	{
		net_msg_fb_client.emFB = EMPET_REQ_ENABLE_DUAL_SKILL_INVALID_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &net_msg_fb_client );
		return E_FAIL;
	}

	SITEM* pitem_data_hold = GET_SLOT_ITEMDATA ( SLOT_HOLD );
	if ( !pitem_data_hold ) 
	{
		net_msg_fb_client.emFB = EMPET_REQ_ENABLE_DUAL_SKILL_INVALID_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &net_msg_fb_client );
		return E_FAIL;
	}

	if ( CheckCoolTime( pitem_data_hold->sBasicOp.sNativeID ) ) return E_FAIL;

	if ( pitem_data_hold->sBasicOp.emItemType != ITEM_CARD_DUALPETSKILL )	
	{
		net_msg_fb_client.emFB = EMPET_REQ_ENABLE_DUAL_SKILL_INVALID_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &net_msg_fb_client );
		return E_FAIL;
	}

	SITEM* pPutOnItem = GLItemMan::GetInstance().GetItem ( m_PutOnItems[SLOT_HOLD].sNativeID );
	if( pitem_data_hold != pPutOnItem )
	{
		net_msg_fb_client.emFB = EMPET_REQ_ENABLE_DUAL_SKILL_INVALID_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &net_msg_fb_client );
		return E_FAIL;
	}

	SNATIVEID sNativeID = pitem_data_hold->sBasicOp.sNativeID;

	PGLPETFIELD pMyPet = GLGaeaServer::GetInstance().GetPET ( m_dwPetGUID );
	if ( pMyPet && pMyPet->m_dwPetID == pinven_item->sItemCustom.dwPetID )
	{
		if ( pMyPet->m_bDualSkill )
		{
			net_msg_fb_client.emFB = EMPET_REQ_ENABLE_DUAL_SKILL_ALREADY_ENABLED;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &net_msg_fb_client );
			return E_FAIL;
		}

		pMyPet->m_bDualSkill = TRUE;

		if ( pMyPet->IsValid () )	pMyPet->m_fTIMER = 0.0f;

		BOOL bdelete_item = DoDrugSlotItem ( SLOT_HOLD );
		if ( !bdelete_item )
		{
			SINVENITEM* pinven_item_card = m_cInventory.FindItem ( ITEM_CARD_DUALPETSKILL );
			if ( pinven_item_card )	
				bdelete_item = DoDrugInvenItem ( pinven_item_card->wPosX, pinven_item_card->wPosY );

			if ( !bdelete_item )	
				CDebugSet::ToFileWithTime( "_pet_dual_skill.txt", "[%d]%s pet dual skill delete failed", m_dwCharID, m_szName );
		}

		GLITEMLMT::GetInstance().ReqPetAction( pMyPet->m_dwPetID, 
												pinven_item->sItemCustom.sNativeID, 
												EMPET_ACTION_PET_DUAL_SKILL, 
												pMyPet->m_bDualSkill );

		net_msg_fb_client.emFB	     = EMPET_REQ_ENABLE_DUAL_SKILL_FB_OK;
		net_msg_fb_client.dwPetID    = pMyPet->m_dwPetID;
		net_msg_fb_client.sNativeID  = sNativeID;
		net_msg_fb_client.bDualSkill = pMyPet->m_bDualSkill;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &net_msg_fb_client );

		CSetPetDualSkill* pDBAction = new CSetPetDualSkill( m_dwCharID, pinven_item->sItemCustom.dwPetID, true );
		GLDBMan* pDBManager = GLGaeaServer::GetInstance().GetDBMan ();
		if ( pDBManager ) pDBManager->AddJob ( pDBAction );
	}
	else
	{
		net_msg_fb_client.emFB = EMPET_REQ_ENABLE_DUAL_SKILL_NOT_ACTIVE;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &net_msg_fb_client );
		return E_FAIL;
	}

	return S_OK;
}

/*change scale card, Juver, 2018/01/04 */
HRESULT GLChar::MsgChangeScale( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVEN_SCALE_CHANGE* pNetMsg = ( GLMSG::SNETPC_INVEN_SCALE_CHANGE* ) nmg;

	GLMSG::SNETPC_INVEN_SCALE_CHANGE_FB NetMsgFB;
	SNATIVEID sNativeID;

	SINVENITEM* pinvencard = m_cInventory.FindItem( ITEM_CHANGE_SCALE_CARD );
	if ( !pinvencard ) 
	{
		NetMsgFB.emFB = EMFB_CHANGE_SCALE_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	SITEM* pitemcard = GLItemMan::GetInstance().GetItem( pinvencard->sItemCustom.sNativeID );
	if ( pitemcard->sBasicOp.emItemType != ITEM_CHANGE_SCALE_CARD )	
	{
		NetMsgFB.emFB = EMFB_CHANGE_SCALE_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	BOOL bDelete = DoDrugInvenItem( pinvencard->wPosX, pinvencard->wPosY );
	if ( !bDelete )
	{
		NetMsgFB.emFB = EMFB_CHANGE_SCALE_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	m_fScaleRange = pNetMsg->fScale;

	NetMsgFB.emFB	= EMFB_CHANGE_SCALE_OK;
	NetMsgFB.fScale = m_fScaleRange;

	GLMSG::SNETPC_INVENSCALE_CHANGE_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID = m_dwGaeaID;
	NetMsgBrd.fScale = m_fScaleRange;

	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

	return S_OK;
}

/*item color, Juver, 2018/01/10 */
HRESULT GLChar::MsgReqInvenItemColorChange( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVEN_ITEMCOLOR_CHANGE* pNetMsg = ( GLMSG::SNETPC_INVEN_ITEMCOLOR_CHANGE* ) nmg;

	GLMSG::SNETPC_INVEN_ITEMCOLOR_CHANGE_FB NetMsgFB;
	SNATIVEID sNativeID;

	SINVENITEM* pinvencard = m_cInventory.FindItem( ITEM_COSTUME_COLOR );
	if ( !pinvencard ) 
	{
		NetMsgFB.emFB = EMFB_ITEM_COLOR_CHANGE_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	SITEM* pitemcard = GLItemMan::GetInstance().GetItem( pinvencard->sItemCustom.sNativeID );
	if ( pitemcard->sBasicOp.emItemType != ITEM_COSTUME_COLOR )	
	{
		NetMsgFB.emFB = EMFB_ITEM_COLOR_CHANGE_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( pNetMsg->emSlot >= SLOT_TSIZE )
	{
		NetMsgFB.emFB = EMFB_ITEM_COLOR_CHANGE_SLOT_ERROR;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	SITEM* pitem_data_slot = GET_SLOT_ITEMDATA( pNetMsg->emSlot );
	if ( !pitem_data_slot )
	{
		NetMsgFB.emFB = EMFB_ITEM_COLOR_CHANGE_SLOT_ERROR;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( GET_SLOT_ITEM(pNetMsg->emSlot).nidDISGUISE == NATIVEID_NULL() )
	{
		if ( !pitem_data_slot->sBasicOp.bItemColor )
		{
			NetMsgFB.emFB = EMFB_ITEM_COLOR_CHANGE_SLOT_ERROR;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
			return E_FAIL;
		}
	}
	else
	{
		SITEM* pitem_data_disguise = GLItemMan::GetInstance().GetItem( GET_SLOT_ITEM(pNetMsg->emSlot).nidDISGUISE );
		if ( !pitem_data_disguise )	
		{
			NetMsgFB.emFB = EMFB_ITEM_COLOR_CHANGE_SLOT_ERROR;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
			return E_FAIL;
		}

		if ( !pitem_data_disguise->sBasicOp.bItemColor )
		{
			NetMsgFB.emFB = EMFB_ITEM_COLOR_CHANGE_SLOT_ERROR;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
			return E_FAIL;
		}
	}

	BOOL bDelete = DoDrugInvenItem( pinvencard->wPosX, pinvencard->wPosY );
	if ( !bDelete )
	{
		NetMsgFB.emFB = EMFB_ITEM_COLOR_CHANGE_FAILED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	m_PutOnItems[pNetMsg->emSlot].wColor1 = pNetMsg->wColor1;
	m_PutOnItems[pNetMsg->emSlot].wColor2 = pNetMsg->wColor2;

	NetMsgFB.emFB = EMFB_ITEM_COLOR_CHANGE_DONE;
	NetMsgFB.emSlot = pNetMsg->emSlot;
	NetMsgFB.wColor1 = pNetMsg->wColor1;
	NetMsgFB.wColor2 = pNetMsg->wColor2;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);

	GLMSG::SNETPC_INVEN_ITEMCOLOR_CHANGE_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID = m_dwGaeaID;
	NetMsgBrd.emSlot = pNetMsg->emSlot;
	NetMsgBrd.wColor1 = pNetMsg->wColor1;
	NetMsgBrd.wColor2 = pNetMsg->wColor2;
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

	return S_OK;
}

/*item wrapper, Juver, 2018/01/11 */
HRESULT GLChar::MsgReqInvenItemWrap( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVEN_WRAP* pnet_msg_client = ( GLMSG::SNETPC_INVEN_WRAP* ) nmg;

	GLMSG::SNETPC_INVEN_WRAP_FB net_msg_fb;

	if ( m_sTrade.Valid() )
	{
		net_msg_fb.emFB = EMFB_ITEM_WRAP_FAILED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	SINVENITEM* pinven_item_target = m_cInventory.FindPosItem ( pnet_msg_client->wPosX, pnet_msg_client->wPosY );
	if ( !pinven_item_target )	
	{
		net_msg_fb.emFB = EMFB_ITEM_WRAP_FAILED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	SITEM* pitem_data_target = GLItemMan::GetInstance().GetItem ( pinven_item_target->sItemCustom.sNativeID );
	if ( !pitem_data_target )
	{
		net_msg_fb.emFB = EMFB_ITEM_WRAP_TARGET_NOT_VALID;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( !pinven_item_target->sItemCustom.IsGM_GENITEM() && pitem_data_target->sBasicOp.IsEXCHANGE() )
	{
		net_msg_fb.emFB = EMFB_ITEM_WRAP_TARGET_NO_NEED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( !pitem_data_target->isWrappable() )
	{
		net_msg_fb.emFB = EMFB_ITEM_WRAP_TARGET_NOT_VALID;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( pinven_item_target->sItemCustom.nidDISGUISE != NATIVEID_NULL() )
	{
		net_msg_fb.emFB = EMFB_ITEM_WRAP_TARGET_HAVE_DISGUISE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	SITEM* pitem_data_hold = GET_SLOT_ITEMDATA ( SLOT_HOLD );
	if ( !pitem_data_hold )
	{
		net_msg_fb.emFB = EMFB_ITEM_WRAP_HOLD_INVALID;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( pitem_data_hold->sBasicOp.emItemType != ITEM_WRAPPER )	
	{
		net_msg_fb.emFB = EMFB_ITEM_WRAP_HOLD_NOT_WRAPPER;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	SITEM* phold_item_box = GLItemMan::GetInstance().GetItem( pitem_data_hold->sBasicOp.sidWrapperBox );
	if ( !phold_item_box )
	{
		net_msg_fb.emFB = EMFB_ITEM_WRAP_HOLD_NOT_BOX;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	BOOL bdelete_item = DoDrugSlotItem ( SLOT_HOLD );
	if ( !bdelete_item )
	{
		SINVENITEM* pinven_item_card = m_cInventory.FindItem ( ITEM_WRAPPER );
		if ( pinven_item_card )	
			bdelete_item = DoDrugInvenItem ( pinven_item_card->wPosX, pinven_item_card->wPosY );

		if ( !bdelete_item )	
			CDebugSet::ToFileWithTime( "_wrapper.txt", "[%d]%s unable to delete card", m_dwCharID, m_szName );
	}

	SITEMCUSTOM sitem_new = pinven_item_target->sItemCustom;
	sitem_new.nidDISGUISE = sitem_new.sNativeID;
	sitem_new.sNativeID = pitem_data_hold->sBasicOp.sidWrapperBox;
	
	WORD wposX = pinven_item_target->wPosX;
	WORD wposY = pinven_item_target->wPosY;
	BOOL bDelete = m_cInventory.DeleteItem ( wposX, wposY );
	if ( !bDelete )
	{
		net_msg_fb.emFB = EMFB_ITEM_WRAP_FAILED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_wrapper.txt", "[%d] %s unable to delete target item %d~%d %s", m_dwCharID, m_szName,
			pitem_data_target->sBasicOp.sNativeID.wMainID, pitem_data_target->sBasicOp.sNativeID.wSubID, pitem_data_target->GetName() );
		return E_FAIL;
	}

	GLMSG::SNETPC_INVEN_DELETE net_msg_inven_delete;
	net_msg_inven_delete.wPosX = wposX;
	net_msg_inven_delete.wPosY = wposY;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_inven_delete);

	BOOL bOK = m_cInventory.InsertItem ( sitem_new, wposX, wposY );
	if ( !bOK )	
	{
		net_msg_fb.emFB = EMFB_ITEM_WRAP_FAILED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_wrapper.txt", "[%d] %s unable to insert new item %d~%d", m_dwCharID, m_szName,
			sitem_new.sNativeID.wMainID, sitem_new.sNativeID.wSubID );
		return E_FAIL;
	}

	GLITEMLMT::GetInstance().ReqItemRoute ( sitem_new, ID_CHAR, 0, ID_CHAR, m_dwCharID, EMITEM_ROUTE_CHAR, sitem_new.wTurnNum );

	GLMSG::SNETPC_INVEN_INSERT NetMsgInven;
	NetMsgInven.Data = *m_cInventory.GetItem ( wposX, wposY );
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgInven);

	net_msg_fb.emFB = EMFB_ITEM_WRAP_DONE;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);

	return S_OK;
}

/*item wrapper, Juver, 2018/01/12 */
HRESULT GLChar::MsgReqInvenItemUnWrap( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVEN_UNWRAP* pnet_msg_client = ( GLMSG::SNETPC_INVEN_UNWRAP* ) nmg;

	GLMSG::SNETPC_INVEN_UNWRAP_FB net_msg_fb;

	if ( m_sTrade.Valid() )
	{
		net_msg_fb.emFB = EMFB_ITEM_UNWRAP_FAILED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	SINVENITEM* pinven_item_target = m_cInventory.FindPosItem ( pnet_msg_client->wPosX, pnet_msg_client->wPosY );
	if ( !pinven_item_target )	
	{
		net_msg_fb.emFB = EMFB_ITEM_UNWRAP_TARGET_NOT_VALID;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	SITEM* pitem_data_target = GLItemMan::GetInstance().GetItem ( pinven_item_target->sItemCustom.sNativeID );
	if ( !pitem_data_target )
	{
		net_msg_fb.emFB = EMFB_ITEM_UNWRAP_TARGET_NOT_VALID;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( pinven_item_target->sItemCustom.nidDISGUISE == NATIVEID_NULL() )
	{
		net_msg_fb.emFB = EMFB_ITEM_UNWRAP_TARGET_NOT_VALID;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( pitem_data_target->sBasicOp.emItemType != ITEM_WRAPPER_BOX )
	{
		net_msg_fb.emFB = EMFB_ITEM_UNWRAP_TARGET_NOT_VALID;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	SITEM* pitem_data_disguise = GLItemMan::GetInstance().GetItem( pinven_item_target->sItemCustom.nidDISGUISE );
	if ( !pitem_data_disguise )
	{
		net_msg_fb.emFB = EMFB_ITEM_UNWRAP_TARGET_NOT_VALID;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	SITEMCUSTOM sitem_new = pinven_item_target->sItemCustom;
	sitem_new.sNativeID = sitem_new.nidDISGUISE;
	sitem_new.nidDISGUISE = NATIVEID_NULL();

	WORD wposX = pinven_item_target->wPosX;
	WORD wposY = pinven_item_target->wPosY;
	BOOL bDelete = m_cInventory.DeleteItem ( wposX, wposY );
	if ( !bDelete )
	{
		net_msg_fb.emFB = EMFB_ITEM_UNWRAP_FAILED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_wrapperbox.txt", "[%d] %s unable to delete target item %d~%d %s", m_dwCharID, m_szName,
			pitem_data_target->sBasicOp.sNativeID.wMainID, pitem_data_target->sBasicOp.sNativeID.wSubID, pitem_data_target->GetName() );
		return E_FAIL;
	}

	GLMSG::SNETPC_INVEN_DELETE net_msg_inven_delete;
	net_msg_inven_delete.wPosX = wposX;
	net_msg_inven_delete.wPosY = wposY;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_inven_delete);

	BOOL bOK = m_cInventory.InsertItem ( sitem_new, wposX, wposY );
	if ( !bOK )	
	{
		net_msg_fb.emFB = EMFB_ITEM_UNWRAP_FAILED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_wrapperbox.txt", "[%d] %s unable to insert new item %d~%d", m_dwCharID, m_szName,
			sitem_new.sNativeID.wMainID, sitem_new.sNativeID.wSubID );
		return E_FAIL;
	}

	GLITEMLMT::GetInstance().ReqItemRoute ( sitem_new, ID_CHAR, 0, ID_CHAR, m_dwCharID, EMITEM_ROUTE_CHAR, sitem_new.wTurnNum );

	GLMSG::SNETPC_INVEN_INSERT NetMsgInven;
	NetMsgInven.Data = *m_cInventory.GetItem ( wposX, wposY );
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgInven);

	net_msg_fb.emFB = EMFB_ITEM_UNWRAP_DONE;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);

	return S_OK;
}

/*change school card, Juver, 2018/01/12 */
HRESULT GLChar::MsgReqInvenChangeSchool( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVEN_CHANGE_SCHOOL* pnet_msg_client = ( GLMSG::SNETPC_INVEN_CHANGE_SCHOOL* ) nmg;
	GLMSG::SNETPC_INVEN_CHANGE_SCHOOL_FB net_msg_fb;

	if ( pnet_msg_client->wSchool >= GLSCHOOL_NUM )
	{
		net_msg_fb.emFB = EMFB_CHANGE_SCHOOL_NOT_SELECTED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( pnet_msg_client->wSchool == m_wSchool )
	{
		net_msg_fb.emFB = EMFB_CHANGE_SCHOOL_SAME_SCHOOL;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	SINVENITEM* pInvenItem = m_cInventory.FindItem ( ITEM_CHANGE_SCHOOL );
	if ( !pInvenItem )
	{
		net_msg_fb.emFB = EMFB_CHANGE_SCHOOL_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		net_msg_fb.emFB = EMFB_CHANGE_SCHOOL_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_CHANGE_SCHOOL )	
	{
		net_msg_fb.emFB = EMFB_CHANGE_SCHOOL_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( RANPARAM::bFeatureChangeSchoolReqCleanQuest && m_cQuestPlay.GetQuestProc().size() )
	{
		net_msg_fb.emFB = EMFB_CHANGE_SCHOOL_REQ_CLEAN_QUEST;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( RANPARAM::bFeatureChangeSchoolReqCleanGuild && m_dwGuild!=CLUB_NULL )
	{
		net_msg_fb.emFB = EMFB_CHANGE_SCHOOL_REQ_CLEAN_GUILD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( m_emClass == GLCC_EXTREME_M || m_emClass == GLCC_EXTREME_W )
	{
		if ( RANPARAM::bFeatureChangeSchoolReqMaxLevel && m_wLevel < GLCONST_CHAR::wMAX_EXTREME_LEVEL )
		{
			net_msg_fb.emFB = EMFB_CHANGE_SCHOOL_REQ_MAX_LEVEL;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
			return E_FAIL;
		}
	}
	else
	{
		if ( RANPARAM::bFeatureChangeSchoolReqMaxLevel && m_wLevel < GLCONST_CHAR::wMAX_LEVEL )
		{
			net_msg_fb.emFB = EMFB_CHANGE_SCHOOL_REQ_MAX_LEVEL;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
			return E_FAIL;
		}
	}

	BOOL bDelete = DoDrugInvenItem( pInvenItem->wPosX, pInvenItem->wPosY );
	if ( !bDelete )
	{
		net_msg_fb.emFB = EMFB_CHANGE_SCHOOL_FAILED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	m_wSchool = pnet_msg_client->wSchool;

	if ( RANPARAM::bFeatureChangeSchoolWipeQuest )
	{
		m_cQuestPlay.DeleteAll();
	}

	net_msg_fb.emFB = EMFB_CHANGE_SCHOOL_DONE;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
	return E_FAIL;

	return S_OK;
}

/*equipment lock, Juver, 2018/01/13 */
HRESULT GLChar::MsgReqInvenEnableEquipmentLock( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_ENABLE* pnet_msg_client = ( GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_ENABLE* ) nmg;
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_ENABLE_FB net_msg_fb;

	SINVENITEM* pInvenItem = m_cInventory.FindItem ( ITEM_EQUIPMENT_LOCK_ENABLE );
	if ( !pInvenItem )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_ENABLE_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_ENABLE_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_EQUIPMENT_LOCK_ENABLE )	
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_ENABLE_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( m_bEnableEquipmentLock )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_ENABLE_ALREADY_ENABLED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	std::string strKey1(pnet_msg_client->szPin1);
	std::string strKey2(pnet_msg_client->szPin2);

	if ( strKey1.size() <= 0  || strKey2.size() <= 0  )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_ENABLE_KEY_EMPTY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strKey1.size() < CHAR_EQUIPMENT_LOCK_PASS_SIZE || strKey1.size() > CHAR_EQUIPMENT_LOCK_PASS_SIZE || 
		strKey2.size() < CHAR_EQUIPMENT_LOCK_PASS_SIZE || strKey2.size() > CHAR_EQUIPMENT_LOCK_PASS_SIZE )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_ENABLE_KEY_SIZE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strcmp( strKey1.c_str(), strKey2.c_str() ) != 0 )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_ENABLE_KEY_NOT_SAME;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strKey1.find_first_not_of( "0123456789" ) != string::npos || strKey2.find_first_not_of( "0123456789" ) != string::npos )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_ENABLE_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CString strTEMP1( strKey1.c_str() );
	CString strTEMP2( strKey2.c_str() );

	if ( STRUTIL::CheckString( strTEMP1 ) || STRUTIL::CheckString( strTEMP2 ) || 
		CRanFilter::GetInstance().NameFilter( strTEMP1 ) || CRanFilter::GetInstance().NameFilter( strTEMP2 ) )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_ENABLE_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ((strTEMP1.FindOneOf(" ") != -1) || (strTEMP2.FindOneOf(" ") != -1) ) 
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_ENABLE_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CEquipmentLockEnable* pDBAction = new CEquipmentLockEnable(m_dwClientID, m_dwCharID, m_szName, pnet_msg_client->szPin1 );
	if ( GLGaeaServer::GetInstance().GetDBMan() )
		GLGaeaServer::GetInstance().GetDBMan()->AddJob ( pDBAction );
	
	return S_OK;
}

/*equipment lock, Juver, 2018/01/14 */
HRESULT GLChar::MsgReqInvenEnableEquipmentLockDB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_ENABLE_FROM_DB *pnet_msg_database = (GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_ENABLE_FROM_DB *)nmg;

	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_ENABLE_FB net_msg_fb;

	if( pnet_msg_database->emFB == EMFB_EQUIPMENT_LOCK_ENABLE_ALREADY_ENABLED_DB )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_ENABLE_ALREADY_ENABLED_DB;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_equipmentlock_enable.txt", "[%d]%s enable equipment lock EMFB_EQUIPMENT_LOCK_ENABLE_ALREADY_ENABLED_DB %s %s", 
			m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );
		return S_FALSE;
	}
	else if( pnet_msg_database->emFB == EMFB_EQUIPMENT_LOCK_ENABLE_CHAR_NOT_EXIST )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_ENABLE_CHAR_NOT_EXIST;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_equipmentlock_enable.txt", "[%d]%s enable equipment lock EMFB_EQUIPMENT_LOCK_ENABLE_CHAR_NOT_EXIST %s %s", 
			m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );
		return S_FALSE;
	}
	else if ( pnet_msg_database->emFB == EMFB_EQUIPMENT_LOCK_ENABLE_FAILED )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_ENABLE_FAILED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_equipmentlock_enable.txt", "[%d]%s enable equipment lock EMFB_EQUIPMENT_LOCK_ENABLE_FAILED %s %s", 
			m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );
		return S_FALSE;
	}

	BOOL bdelete_item = FALSE;
	SINVENITEM* pinven_item_card = m_cInventory.FindItem ( ITEM_EQUIPMENT_LOCK_ENABLE );
	if ( pinven_item_card )	
		bdelete_item = DoDrugInvenItem ( pinven_item_card->wPosX, pinven_item_card->wPosY );

	if ( !bdelete_item )
	{
		SITEMCUSTOM shold_item = GET_SLOT_ITEM( SLOT_HOLD );
		SITEM* pitem_data_hold = GLItemMan::GetInstance().GetItem( shold_item.sNativeID );
		if ( pitem_data_hold && pitem_data_hold->sBasicOp.emItemType == ITEM_EQUIPMENT_LOCK_ENABLE ) 
		{
			bdelete_item = DoDrugSlotItem ( SLOT_HOLD );
		}
	}

	if ( !bdelete_item )	
		CDebugSet::ToFileWithTime( "_equipmentlock_enable.txt", "[%d]%s item delete failed %s %s", 
		m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );


	m_bEnableEquipmentLock = TRUE;

	net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_ENABLE_DONE;
	net_msg_fb.bEquipmentLockEnable = m_bEnableEquipmentLock;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);

	CDebugSet::ToFileWithTime( "_equipmentlock_enable.txt", "[%d]%s enable equipment lock done %s %s", 
		m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );

	return S_OK;
}

/*equipment lock, Juver, 2018/01/16 */
HRESULT GLChar::MsgReqInvenInputEquipmentLock( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_INPUT* pnet_msg_client = ( GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_INPUT* ) nmg;
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_INPUT_FB net_msg_fb;

	if  ( m_fEquipmentLockTimer < EQUIPMENT_LOCK_TIMER )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_INPUT_TIMER;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( !m_bEnableEquipmentLock )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_INPUT_NOT_ENABLED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	std::string strKey(pnet_msg_client->szPin);

	if ( strKey.size() <= 0 )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_INPUT_KEY_EMPTY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strKey.size() < CHAR_EQUIPMENT_LOCK_PASS_SIZE || strKey.size() > CHAR_EQUIPMENT_LOCK_PASS_SIZE )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_INPUT_KEY_SIZE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strKey.find_first_not_of( "0123456789" ) != string::npos )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_INPUT_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CString strTEMP( strKey.c_str() );

	if ( STRUTIL::CheckString( strTEMP ) || CRanFilter::GetInstance().NameFilter( strTEMP ) )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_INPUT_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strTEMP.FindOneOf(" ") != -1 ) 
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_INPUT_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CEquipmentLockInput* pDBAction = new CEquipmentLockInput(m_dwClientID, m_dwCharID, m_szName, pnet_msg_client->szPin );
	if ( GLGaeaServer::GetInstance().GetDBMan() )
		GLGaeaServer::GetInstance().GetDBMan()->AddJob ( pDBAction );

	m_fEquipmentLockTimer = 0.0f;

	return S_OK;
}

/*equipment lock, Juver, 2018/01/16 */
HRESULT GLChar::MsgReqInvenInputEquipmentLockDB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_INPUT_FROM_DB *pnet_msg_database = (GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_INPUT_FROM_DB *)nmg;
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_INPUT_FB net_msg_fb;

	if ( pnet_msg_database->emFB == EMFB_EQUIPMENT_LOCK_INPUT_FAILED )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_INPUT_FAILED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_equipmentlock_input.txt", "[%d]%s input equipment lock EMFB_EQUIPMENT_LOCK_INPUT_FAILED %s %s", 
			m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );
		return S_FALSE;
	}
	else if( pnet_msg_database->emFB == EMFB_EQUIPMENT_LOCK_INPUT_DATA_NOT_EXIST )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_INPUT_DATA_NOT_EXIST;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return S_FALSE;
	}
 
	m_bEquipmentLockStatus = !m_bEquipmentLockStatus;

	net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_INPUT_DONE;
	net_msg_fb.bEquipmentLockStatus = m_bEquipmentLockStatus;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);

	return S_OK;
}

/*equipment lock, Juver, 2018/01/16 */
HRESULT GLChar::MsgReqInvenRecoverEquipmentLock ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER* pnet_msg_client = ( GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER* ) nmg;
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_FB net_msg_fb;

	SINVENITEM* pInvenItem = m_cInventory.FindItem ( ITEM_EQUIPMENT_LOCK_RECOVER );
	if ( !pInvenItem )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_EQUIPMENT_LOCK_RECOVER )	
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( !m_bEnableEquipmentLock )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_NOT_ENABLED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CEquipmentLockRecover* pDBAction = new CEquipmentLockRecover(m_dwClientID, m_dwCharID, m_szName );
	if ( GLGaeaServer::GetInstance().GetDBMan() )
		GLGaeaServer::GetInstance().GetDBMan()->AddJob ( pDBAction );

	return S_OK;
}

/*equipment lock, Juver, 2018/01/16 */
HRESULT GLChar::MsgReqInvenRecoverEquipmentLockDB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_FROM_DB *pnet_msg_database = (GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_FROM_DB *)nmg;
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_FB net_msg_fb;

	StringCchCopy ( net_msg_fb.szName, CHAR_SZNAME+1, pnet_msg_database->szName );
	StringCchCopy ( net_msg_fb.szPin, CHAR_EQUIPMENT_LOCK_PASS_SIZE+1, pnet_msg_database->szPin );

	if ( pnet_msg_database->emFB == EMFB_EQUIPMENT_LOCK_RECOVER_NOT_EXIST )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_NOT_EXIST;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_equipmentlock_recover.txt", "[%d]%s input equipment lock EMFB_EQUIPMENT_LOCK_RECOVER_NOT_EXIST", m_dwCharID, m_szName );
		return S_FALSE;
	}
	
	BOOL bdelete_item = FALSE;
	SINVENITEM* pinven_item_card = m_cInventory.FindItem ( ITEM_EQUIPMENT_LOCK_RECOVER );
	if ( pinven_item_card )	
		bdelete_item = DoDrugInvenItem ( pinven_item_card->wPosX, pinven_item_card->wPosY );

	if ( !bdelete_item )
	{
		SITEMCUSTOM shold_item = GET_SLOT_ITEM( SLOT_HOLD );
		SITEM* pitem_data_hold = GLItemMan::GetInstance().GetItem( shold_item.sNativeID );
		if ( pitem_data_hold && pitem_data_hold->sBasicOp.emItemType == ITEM_EQUIPMENT_LOCK_RECOVER ) 
		{
			bdelete_item = DoDrugSlotItem ( SLOT_HOLD );
		}
	}

	if ( !bdelete_item )	
		CDebugSet::ToFileWithTime( "_equipmentlock_recover.txt", "[%d]%s item delete failed", m_dwCharID, m_szName );

	net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_DONE;

	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);

	CDebugSet::ToFileWithTime( "_equipmentlock_recover.txt", "[%d]%s recover equipment lock done %s %s", 
		m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );

	return S_OK;
}

/*equipment lock, Juver, 2018/01/17 */
HRESULT GLChar::MsgReqInvenRecoverEquipmentLockChange ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE* pnet_msg_client = ( GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE* ) nmg;
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE_FB net_msg_fb;

	if ( !m_bEnableEquipmentLock )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_NOT_ENABLED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	std::string strKey1(pnet_msg_client->szPin1);
	std::string strKey2(pnet_msg_client->szPin2);

	if ( strKey1.size() <= 0  || strKey2.size() <= 0  )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_KEY_EMPTY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strKey1.size() < CHAR_EQUIPMENT_LOCK_PASS_SIZE || strKey1.size() > CHAR_EQUIPMENT_LOCK_PASS_SIZE || 
		strKey2.size() < CHAR_EQUIPMENT_LOCK_PASS_SIZE || strKey2.size() > CHAR_EQUIPMENT_LOCK_PASS_SIZE )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_KEY_SIZE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strcmp( strKey1.c_str(), strKey2.c_str() ) != 0 )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_KEY_NOT_SAME;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strKey1.find_first_not_of( "0123456789" ) != string::npos || strKey2.find_first_not_of( "0123456789" ) != string::npos )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CString strTEMP1( strKey1.c_str() );
	CString strTEMP2( strKey2.c_str() );

	if ( STRUTIL::CheckString( strTEMP1 ) || STRUTIL::CheckString( strTEMP2 ) || 
		CRanFilter::GetInstance().NameFilter( strTEMP1 ) || CRanFilter::GetInstance().NameFilter( strTEMP2 ) )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ((strTEMP1.FindOneOf(" ") != -1) || (strTEMP2.FindOneOf(" ") != -1) ) 
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CEquipmentLockRecoverChange* pDBAction = new CEquipmentLockRecoverChange(m_dwClientID, m_dwCharID, m_szName, pnet_msg_client->szPin1 );
	if ( GLGaeaServer::GetInstance().GetDBMan() )
		GLGaeaServer::GetInstance().GetDBMan()->AddJob ( pDBAction );

	return S_OK;
}

/*equipment lock, Juver, 2018/01/18 */
HRESULT GLChar::MsgReqInvenRecoverEquipmentLockChangeDB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_FROM_DB *pnet_msg_database = (GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_FROM_DB *)nmg;
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE_FB net_msg_fb;

	if ( pnet_msg_database->emFB == EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_NOT_FOUND )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_NOT_FOUND;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_equipmentlock_recover_change.txt", "[%d]%s input equipment lock EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_NOT_FOUND", m_dwCharID, m_szName );
		return S_FALSE;
	}

	net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_DONE;

	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);

	CDebugSet::ToFileWithTime( "_equipmentlock_recover_change.txt", "[%d]%s recover equipment change done %s %s", 
		m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );

	return S_OK;
}

/*equipment lock, Juver, 2018/01/18 */
HRESULT GLChar::MsgReqInvenRecoverEquipmentLockDelete ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE* pnet_msg_client = ( GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE* ) nmg;
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE_FB net_msg_fb;

	if ( !m_bEnableEquipmentLock )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_DELETE_NOT_ENABLED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CEquipmentLockRecoverDelete* pDBAction = new CEquipmentLockRecoverDelete(m_dwClientID, m_dwCharID, m_szName );
	if ( GLGaeaServer::GetInstance().GetDBMan() )
		GLGaeaServer::GetInstance().GetDBMan()->AddJob ( pDBAction );

	return S_OK;
}

/*equipment lock, Juver, 2018/01/18 */
HRESULT GLChar::MsgReqInvenRecoverEquipmentLockDeleteDB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE_FROM_DB *pnet_msg_database = (GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE_FROM_DB *)nmg;
	GLMSG::SNETPC_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE_FB net_msg_fb;

	if ( pnet_msg_database->emFB == EMFB_EQUIPMENT_LOCK_RECOVER_DELETE_NOT_FOUND )
	{
		net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_DELETE_NOT_FOUND;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_equipmentlock_recover_delete.txt", "[%d]%s input equipment lock EMFB_EQUIPMENT_LOCK_RECOVER_CHANGE_NOT_FOUND", m_dwCharID, m_szName );
		return S_FALSE;
	}

	net_msg_fb.emFB = EMFB_EQUIPMENT_LOCK_RECOVER_DELETE_DONE;

	m_bEnableEquipmentLock = FALSE;
	m_bEquipmentLockStatus = FALSE;

	net_msg_fb.bEquipmentLockEnable = m_bEnableEquipmentLock;
	net_msg_fb.bEquipmentLockStatus = m_bEquipmentLockStatus;

	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);

	CDebugSet::ToFileWithTime( "_equipmentlock_recover_delete.txt", "[%d]%s recover equipment delete done %s", 
		m_dwCharID, m_szName, pnet_msg_database->szName );

	return S_OK;
}

/*item transfer card, Juver, 2018/01/18 */
HRESULT GLChar::MsgReqInvenItemTransfer( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVEN_TRANSFER_STATS* pnet_msg_client = ( GLMSG::SNETPC_INVEN_TRANSFER_STATS* ) nmg;
	GLMSG::SNETPC_INVEN_TRANSFER_STATS_FB net_msg_fb;

	SINVENITEM* pInvenItem = m_cInventory.FindItem ( ITEM_TRANSFER_CARD );
	if ( !pInvenItem )
	{
		net_msg_fb.emFB = EMFB_ITEM_TRANSFER_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return S_FALSE;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		net_msg_fb.emFB = EMFB_ITEM_TRANSFER_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return S_FALSE;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_TRANSFER_CARD )	
	{
		net_msg_fb.emFB = EMFB_ITEM_TRANSFER_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return S_FALSE;
	}

	SINVENITEM* pinven_item_old =  m_cInventory.GetItem ( pnet_msg_client->sInvenPosOLD.wPosX,pnet_msg_client->sInvenPosOLD.wPosY );
	SINVENITEM* pinven_item_new =  m_cInventory.GetItem ( pnet_msg_client->sInvenPosNEW.wPosX,pnet_msg_client->sInvenPosNEW.wPosY );
	if ( !pinven_item_old || !pinven_item_new )
	{
		net_msg_fb.emFB = EMFB_ITEM_TRANSFER_INVALID_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return S_FALSE;
	}

	SITEM* pitem_data_old = GLItemMan::GetInstance().GetItem( pinven_item_old->sItemCustom.sNativeID );
	SITEM* pitem_data_new = GLItemMan::GetInstance().GetItem( pinven_item_new->sItemCustom.sNativeID );
	if ( !pitem_data_old || !pitem_data_new )
	{
		net_msg_fb.emFB = EMFB_ITEM_TRANSFER_INVALID_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return S_FALSE;
	}

	if ( pitem_data_old->sBasicOp.emItemType != ITEM_SUIT || pitem_data_new->sBasicOp.emItemType != ITEM_SUIT )
	{
		net_msg_fb.emFB = EMFB_ITEM_TRANSFER_INVALID_ITEM_TYPE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return S_FALSE;
	}

	if ( pitem_data_old->IsTIMELMT() || pitem_data_new->IsTIMELMT() )
	{
		net_msg_fb.emFB = EMFB_ITEM_TRANSFER_TIME_LIMIT;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return S_FALSE;
	}

	if ( pitem_data_old->sBasicOp.IsDISGUISE() || pitem_data_new->sBasicOp.IsDISGUISE() )
	{
		net_msg_fb.emFB = EMFB_ITEM_TRANSFER_DISGUISE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return S_FALSE;
	}

	if ( !pitem_data_old->sBasicOp.bItemTransfer || !pitem_data_new->sBasicOp.bItemTransfer )
	{
		net_msg_fb.emFB = EMFB_ITEM_TRANSFER_NOT_TRANSFER_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return S_FALSE;
	}

	if ( pitem_data_old->sSuitOp.emSuit != pitem_data_new->sSuitOp.emSuit )
	{
		net_msg_fb.emFB = EMFB_ITEM_TRANSFER_SUIT_MISMATCH;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return S_FALSE;
	}

	if ( pitem_data_old->sSuitOp.emSuit == SUIT_HANDHELD && pitem_data_old->sSuitOp.emAttack != pitem_data_new->sSuitOp.emAttack )
	{
		net_msg_fb.emFB = EMFB_ITEM_TRANSFER_ATTACK_MISMATCH;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return S_FALSE;
	}

	if ( RANPARAM::dwFeatureItemTransferCost != 0 && m_lnMoney < RANPARAM::dwFeatureItemTransferCost ) 
	{
		net_msg_fb.emFB = EMFB_ITEM_TRANSFER_COST;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return S_FALSE;
	}

	if ( pinven_item_old->sItemCustom.nidDISGUISE != NATIVEID_NULL() || pinven_item_new->sItemCustom.nidDISGUISE != NATIVEID_NULL() )
	{
		net_msg_fb.emFB = EMFB_ITEM_TRANSFER_HAVE_DISGUISE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return S_FALSE;
	}

	BOOL bdelete_item = FALSE;
	SINVENITEM* pinven_item_card = m_cInventory.FindItem ( ITEM_TRANSFER_CARD );
	if ( pinven_item_card )	
		bdelete_item = DoDrugInvenItem ( pinven_item_card->wPosX, pinven_item_card->wPosY );

	if ( !bdelete_item )
	{
		SITEMCUSTOM shold_item = GET_SLOT_ITEM( SLOT_HOLD );
		SITEM* pitem_data_hold = GLItemMan::GetInstance().GetItem( shold_item.sNativeID );
		if ( pitem_data_hold && pitem_data_hold->sBasicOp.emItemType == ITEM_TRANSFER_CARD ) 
		{
			bdelete_item = DoDrugSlotItem ( SLOT_HOLD );
		}
	}

	if ( !bdelete_item )	
		CDebugSet::ToFileWithTime( "_item_transfer_card.txt", "[%d]%s card item delete failed", m_dwCharID, m_szName );

	if ( RANPARAM::dwFeatureItemTransferCost != 0 )
	{
		CheckMoneyUpdate( m_lnMoney, RANPARAM::dwFeatureItemTransferCost , FALSE, "Item Transfer Cost." );
		m_bMoneyUpdate = TRUE;

		m_lnMoney -= RANPARAM::dwFeatureItemTransferCost;

		if ( RANPARAM::dwFeatureItemTransferCost>EMMONEY_LOG )
		{
			GLITEMLMT::GetInstance().ReqMoneyExc ( ID_CHAR, m_dwCharID, ID_CHAR, 0, -LONGLONG(RANPARAM::dwFeatureItemTransferCost), EMITEM_ROUTE_DELETE );
			GLITEMLMT::GetInstance().ReqMoneyExc ( ID_CHAR, m_dwCharID, ID_CHAR, 0, m_lnMoney, EMITEM_ROUTE_CHAR );
		}

		GLMSG::SNETPC_UPDATE_MONEY net_msg_money;
		net_msg_money.lnMoney = m_lnMoney;
		/*dmk14 ingame web*/
		net_msg_money.dwPremium = m_dwPremiumPoints;
		net_msg_money.dwCombat = m_dwCombatPoints;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_money);
	}
	

	SITEMCUSTOM sitem_new = pinven_item_old->sItemCustom;
	sitem_new.sNativeID = pinven_item_new->sItemCustom.sNativeID;


	{
		WORD wposX = pinven_item_old->wPosX;
		WORD wposY = pinven_item_old->wPosY;
		GLITEMLMT::GetInstance().ReqItemRoute ( pinven_item_old->sItemCustom, ID_CHAR, m_dwCharID, ID_CHAR, 0, EMITEM_ROUTE_DELETE, pinven_item_old->sItemCustom.wTurnNum );
		BOOL bDelete = m_cInventory.DeleteItem ( wposX, wposY );
		if ( !bDelete )
		{
			net_msg_fb.emFB = EMFB_ITEM_TRANSFER_FAILED;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
			CDebugSet::ToFileWithTime( "_item_transfer_card.txt", "[%d] %s unable to delete old item %d~%d %s", m_dwCharID, m_szName,
				pitem_data_old->sBasicOp.sNativeID.wMainID, pitem_data_old->sBasicOp.sNativeID.wSubID, pitem_data_old->GetName() );
			return E_FAIL;
		}

		GLMSG::SNETPC_INVEN_DELETE net_msg_inven_delete;
		net_msg_inven_delete.wPosX = wposX;
		net_msg_inven_delete.wPosY = wposY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_inven_delete);
	}
	{
		WORD wposX = pinven_item_new->wPosX;
		WORD wposY = pinven_item_new->wPosY;
		GLITEMLMT::GetInstance().ReqItemRoute ( pinven_item_new->sItemCustom, ID_CHAR, m_dwCharID, ID_CHAR, 0, EMITEM_ROUTE_DELETE, pinven_item_new->sItemCustom.wTurnNum );
		BOOL bDelete = m_cInventory.DeleteItem ( wposX, wposY );
		if ( !bDelete )
		{
			net_msg_fb.emFB = EMFB_ITEM_TRANSFER_FAILED;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
			CDebugSet::ToFileWithTime( "_item_transfer_card.txt", "[%d] %s unable to delete new item %d~%d %s", m_dwCharID, m_szName,
				pitem_data_new->sBasicOp.sNativeID.wMainID, pitem_data_new->sBasicOp.sNativeID.wSubID, pitem_data_new->GetName() );
			return E_FAIL;
		}

		GLMSG::SNETPC_INVEN_DELETE net_msg_inven_delete;
		net_msg_inven_delete.wPosX = wposX;
		net_msg_inven_delete.wPosY = wposY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_inven_delete);
	}

	{
		WORD wposX(0);
		WORD wposY(0);
		BOOL bFindSpace = m_cInventory.FindInsrtable ( pitem_data_new->sBasicOp.wInvenSizeX, pitem_data_new->sBasicOp.wInvenSizeY, wposX, wposY );
		if ( !bFindSpace )
		{
			net_msg_fb.emFB = EMFB_ITEM_TRANSFER_FAILED;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
			CDebugSet::ToFileWithTime( "_item_transfer_card.txt", "[%d] %s unable to find space for new item %d~%d", m_dwCharID, m_szName,
				sitem_new.sNativeID.wMainID, sitem_new.sNativeID.wSubID );
			return E_FAIL;
		}
		
		BOOL bInsertNewItem = m_cInventory.InsertItem ( sitem_new, wposX, wposY );
		if ( !bInsertNewItem )	
		{
			net_msg_fb.emFB = EMFB_ITEM_TRANSFER_FAILED;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
			CDebugSet::ToFileWithTime( "_item_transfer_card.txt", "[%d] %s unable to insert new item %d~%d", m_dwCharID, m_szName,
				sitem_new.sNativeID.wMainID, sitem_new.sNativeID.wSubID );
			return E_FAIL;
		}

		GLITEMLMT::GetInstance().ReqItemRoute ( sitem_new, ID_CHAR, 0, ID_CHAR, m_dwCharID, EMITEM_ROUTE_CHAR, sitem_new.wTurnNum );

		GLMSG::SNETPC_INVEN_INSERT NetMsgInven;
		NetMsgInven.Data = *m_cInventory.GetItem ( wposX, wposY );
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgInven);
	}

	CDebugSet::ToFileWithTime( "_item_transfer_card.txt", "[%d] %s transfer old item [%d~%d %s] new item [%d~%d %s]", m_dwCharID, m_szName,
		pitem_data_old->sBasicOp.sNativeID.wMainID, pitem_data_old->sBasicOp.sNativeID.wSubID, pitem_data_old->GetName(), 
		pitem_data_new->sBasicOp.sNativeID.wMainID, pitem_data_new->sBasicOp.sNativeID.wSubID, pitem_data_new->GetName() );

	net_msg_fb.emFB = EMFB_ITEM_TRANSFER_DONE;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);

	return S_OK;
}

/* car, cart color, Juver, 2018/02/14 */
HRESULT GLChar::MsgCarChangeColor( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_VEHICLE_REQ_CHANGE_CAR_COLOR* pNetMsg = ( GLMSG::SNET_VEHICLE_REQ_CHANGE_CAR_COLOR* ) nmg;

	GLMSG::SNET_VEHICLE_REQ_CHANGE_CAR_COLOR_FB NetMsgFB;
	SNATIVEID sNativeID;

	SINVENITEM* pinvencard = m_cInventory.FindItem( ITEM_CARD_CAR_COLOR );
	if ( !pinvencard ) 
	{
		NetMsgFB.emFB = EMREQ_CHANGE_CAR_COLOR_FB_INVALID_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	SITEM* pitemcard = GLItemMan::GetInstance().GetItem( pinvencard->sItemCustom.sNativeID );
	if ( pitemcard->sBasicOp.emItemType != ITEM_CARD_CAR_COLOR )	
	{
		NetMsgFB.emFB = EMREQ_CHANGE_CAR_COLOR_FB_INVALID_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( !m_bVehicle )
	{
		NetMsgFB.emFB = EMREQ_CHANGE_CAR_COLOR_FB_VEHICLE_INACTIVE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	SITEM* pVehicle = GET_SLOT_ITEMDATA( SLOT_VEHICLE );
	if ( !pVehicle )
	{
		NetMsgFB.emFB = EMREQ_CHANGE_CAR_COLOR_FB_VEHICLE_INACTIVE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( pVehicle->sVehicle.emVehicleType != VEHICLE_TYPE_CAR && pVehicle->sVehicle.emVehicleType != VEHICLE_TYPE_CART )
	{
		NetMsgFB.emFB = EMREQ_CHANGE_CAR_COLOR_FB_VEHICLE_NOT_CAR;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	BOOL bDelete = DoDrugInvenItem( pinvencard->wPosX, pinvencard->wPosY );
	if ( !bDelete )
	{
		NetMsgFB.emFB = EMREQ_CHANGE_CAR_COLOR_FB_CHANGE_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	m_sVehicle.m_wColor[BIKE_COLOR_SLOT_PART_A1] = pNetMsg->wColorA;
	m_sVehicle.m_wColor[BIKE_COLOR_SLOT_PART_A2] = pNetMsg->wColorB;

	NetMsgFB.emFB = EMREQ_CHANGE_CAR_COLOR_FB_CHANGE_DONE;
	NetMsgFB.dwVehicleID = m_sVehicle.m_dwGUID;
	NetMsgFB.wColorA = m_sVehicle.m_wColor[BIKE_COLOR_SLOT_PART_A1];
	NetMsgFB.wColorB = m_sVehicle.m_wColor[BIKE_COLOR_SLOT_PART_A2];
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );

	GLMSG::SNET_VEHICLE_REQ_CHANGE_CAR_COLOR_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID = m_dwGaeaID;
	NetMsgBrd.dwVehicleID = m_sVehicle.m_dwGUID;
	NetMsgBrd.wColorA = m_sVehicle.m_wColor[BIKE_COLOR_SLOT_PART_A1];
	NetMsgBrd.wColorB = m_sVehicle.m_wColor[BIKE_COLOR_SLOT_PART_A2];
	SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

	return S_OK;
}

/*storage lock, Juver, 2018/01/13 */
HRESULT GLChar::MsgReqInvenEnableStorageLock( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_STORAGE_LOCK_ENABLE* pnet_msg_client = ( GLMSG::SNETPC_STORAGE_LOCK_ENABLE* ) nmg;
	GLMSG::SNETPC_STORAGE_LOCK_ENABLE_FB net_msg_fb;

	SINVENITEM* pInvenItem = m_cInventory.FindItem ( ITEM_STORAGE_LOCK_ENABLE );
	if ( !pInvenItem )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_ENABLE_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_ENABLE_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_STORAGE_LOCK_ENABLE )	
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_ENABLE_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( m_bEnableStorageLock )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_ENABLE_ALREADY_ENABLED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	std::string strKey1(pnet_msg_client->szPin1);
	std::string strKey2(pnet_msg_client->szPin2);

	if ( strKey1.size() <= 0  || strKey2.size() <= 0  )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_ENABLE_KEY_EMPTY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strKey1.size() < USER_STORAGE_LOCK_PASS_SIZE || strKey1.size() > USER_STORAGE_LOCK_PASS_SIZE || 
		strKey2.size() < USER_STORAGE_LOCK_PASS_SIZE || strKey2.size() > USER_STORAGE_LOCK_PASS_SIZE )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_ENABLE_KEY_SIZE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strcmp( strKey1.c_str(), strKey2.c_str() ) != 0 )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_ENABLE_KEY_NOT_SAME;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strKey1.find_first_not_of( "0123456789" ) != string::npos || strKey2.find_first_not_of( "0123456789" ) != string::npos )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_ENABLE_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CString strTEMP1( strKey1.c_str() );
	CString strTEMP2( strKey2.c_str() );

	if ( STRUTIL::CheckString( strTEMP1 ) || STRUTIL::CheckString( strTEMP2 ) || 
		CRanFilter::GetInstance().NameFilter( strTEMP1 ) || CRanFilter::GetInstance().NameFilter( strTEMP2 ) )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_ENABLE_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ((strTEMP1.FindOneOf(" ") != -1) || (strTEMP2.FindOneOf(" ") != -1) ) 
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_ENABLE_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CStorageLockEnable* pDBAction = new CStorageLockEnable(m_dwClientID, m_dwCharID, m_szName, pnet_msg_client->szPin1 );
	if ( GLGaeaServer::GetInstance().GetDBMan() )
		GLGaeaServer::GetInstance().GetDBMan()->AddJob ( pDBAction );
	
	return S_OK;
}

/*storage lock, Juver, 2018/01/14 */
HRESULT GLChar::MsgReqInvenEnableStorageLockDB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_STORAGE_LOCK_ENABLE_FROM_DB *pnet_msg_database = (GLMSG::SNETPC_STORAGE_LOCK_ENABLE_FROM_DB *)nmg;

	GLMSG::SNETPC_STORAGE_LOCK_ENABLE_FB net_msg_fb;

	if( pnet_msg_database->emFB == EMFB_STORAGE_LOCK_ENABLE_ALREADY_ENABLED_DB )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_ENABLE_ALREADY_ENABLED_DB;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_storagelock_enable.txt", "[%d]%s enable storage lock EMFB_STORAGE_LOCK_ENABLE_ALREADY_ENABLED_DB %s %s", 
			m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );
		return S_FALSE;
	}
	else if( pnet_msg_database->emFB == EMFB_STORAGE_LOCK_ENABLE_CHAR_NOT_EXIST )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_ENABLE_CHAR_NOT_EXIST;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_storagelock_enable.txt", "[%d]%s enable storage lock EMFB_STORAGE_LOCK_ENABLE_CHAR_NOT_EXIST %s %s", 
			m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );
		return S_FALSE;
	}
	else if ( pnet_msg_database->emFB == EMFB_STORAGE_LOCK_ENABLE_FAILED )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_ENABLE_FAILED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_storagelock_enable.txt", "[%d]%s enable storage lock EMFB_STORAGE_LOCK_ENABLE_FAILED %s %s", 
			m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );
		return S_FALSE;
	}

	BOOL bdelete_item = FALSE;
	SINVENITEM* pinven_item_card = m_cInventory.FindItem ( ITEM_STORAGE_LOCK_ENABLE );
	if ( pinven_item_card )	
		bdelete_item = DoDrugInvenItem ( pinven_item_card->wPosX, pinven_item_card->wPosY );

	if ( !bdelete_item )
	{
		SITEMCUSTOM shold_item = GET_SLOT_ITEM( SLOT_HOLD );
		SITEM* pitem_data_hold = GLItemMan::GetInstance().GetItem( shold_item.sNativeID );
		if ( pitem_data_hold && pitem_data_hold->sBasicOp.emItemType == ITEM_STORAGE_LOCK_ENABLE ) 
		{
			bdelete_item = DoDrugSlotItem ( SLOT_HOLD );
		}
	}

	if ( !bdelete_item )	
		CDebugSet::ToFileWithTime( "_storagelock_enable.txt", "[%d]%s item delete failed %s %s", 
		m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );


	m_bEnableStorageLock = TRUE;

	net_msg_fb.emFB = EMFB_STORAGE_LOCK_ENABLE_DONE;
	net_msg_fb.bStorageLockEnable = m_bEnableStorageLock;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);

	CDebugSet::ToFileWithTime( "_storagelock_enable.txt", "[%d]%s enable storage lock done %s %s", 
		m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );

	return S_OK;
}

/*storage lock, Juver, 2018/01/16 */
HRESULT GLChar::MsgReqInvenInputStorageLock( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_STORAGE_LOCK_INPUT* pnet_msg_client = ( GLMSG::SNETPC_STORAGE_LOCK_INPUT* ) nmg;
	GLMSG::SNETPC_STORAGE_LOCK_INPUT_FB net_msg_fb;

	if  ( m_fStorageLockTimer < STORAGE_LOCK_TIMER )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_INPUT_TIMER;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( !m_bEnableStorageLock )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_INPUT_NOT_ENABLED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	std::string strKey(pnet_msg_client->szPin);

	if ( strKey.size() <= 0 )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_INPUT_KEY_EMPTY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strKey.size() < USER_STORAGE_LOCK_PASS_SIZE || strKey.size() > USER_STORAGE_LOCK_PASS_SIZE )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_INPUT_KEY_SIZE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strKey.find_first_not_of( "0123456789" ) != string::npos )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_INPUT_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CString strTEMP( strKey.c_str() );

	if ( STRUTIL::CheckString( strTEMP ) || CRanFilter::GetInstance().NameFilter( strTEMP ) )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_INPUT_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strTEMP.FindOneOf(" ") != -1 ) 
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_INPUT_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CStorageLockInput* pDBAction = new CStorageLockInput(m_dwClientID, m_dwCharID, m_szName, pnet_msg_client->szPin );
	if ( GLGaeaServer::GetInstance().GetDBMan() )
		GLGaeaServer::GetInstance().GetDBMan()->AddJob ( pDBAction );

	m_fStorageLockTimer = 0.0f;

	return S_OK;
}

/*storage lock, Juver, 2018/01/16 */
HRESULT GLChar::MsgReqInvenInputStorageLockDB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_STORAGE_LOCK_INPUT_FROM_DB *pnet_msg_database = (GLMSG::SNETPC_STORAGE_LOCK_INPUT_FROM_DB *)nmg;
	GLMSG::SNETPC_STORAGE_LOCK_INPUT_FB net_msg_fb;

	if ( pnet_msg_database->emFB == EMFB_STORAGE_LOCK_INPUT_FAILED )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_INPUT_FAILED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_storagelock_input.txt", "[%d]%s input storage lock EMFB_STORAGE_LOCK_INPUT_FAILED %s %s", 
			m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );
		return S_FALSE;
	}
	else if( pnet_msg_database->emFB == EMFB_STORAGE_LOCK_INPUT_DATA_NOT_EXIST )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_INPUT_DATA_NOT_EXIST;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return S_FALSE;
	}
 
	m_bStorageLockStatus = !m_bStorageLockStatus;

	net_msg_fb.emFB = EMFB_STORAGE_LOCK_INPUT_DONE;
	net_msg_fb.bStorageLockStatus = m_bStorageLockStatus;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);

	return S_OK;
}

/*storage lock, Juver, 2018/01/16 */
HRESULT GLChar::MsgReqInvenRecoverStorageLock ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_STORAGE_LOCK_RECOVER* pnet_msg_client = ( GLMSG::SNETPC_STORAGE_LOCK_RECOVER* ) nmg;
	GLMSG::SNETPC_STORAGE_LOCK_RECOVER_FB net_msg_fb;

	SINVENITEM* pInvenItem = m_cInventory.FindItem ( ITEM_STORAGE_LOCK_RECOVER );
	if ( !pInvenItem )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_RECOVER_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_RECOVER_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_STORAGE_LOCK_RECOVER )	
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_RECOVER_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( !m_bEnableStorageLock )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_RECOVER_NOT_ENABLED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CStorageLockRecover* pDBAction = new CStorageLockRecover(m_dwClientID, m_dwCharID, m_szName );
	if ( GLGaeaServer::GetInstance().GetDBMan() )
		GLGaeaServer::GetInstance().GetDBMan()->AddJob ( pDBAction );

	return S_OK;
}

/*storage lock, Juver, 2018/01/16 */
HRESULT GLChar::MsgReqInvenRecoverStorageLockDB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_STORAGE_LOCK_RECOVER_FROM_DB *pnet_msg_database = (GLMSG::SNETPC_STORAGE_LOCK_RECOVER_FROM_DB *)nmg;
	GLMSG::SNETPC_STORAGE_LOCK_RECOVER_FB net_msg_fb;

	StringCchCopy ( net_msg_fb.szName, CHAR_SZNAME+1, pnet_msg_database->szName );
	StringCchCopy ( net_msg_fb.szPin, USER_STORAGE_LOCK_PASS_SIZE+1, pnet_msg_database->szPin );

	if ( pnet_msg_database->emFB == EMFB_STORAGE_LOCK_RECOVER_NOT_EXIST )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_RECOVER_NOT_EXIST;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_storagelock_recover.txt", "[%d]%s input storage lock EMFB_STORAGE_LOCK_RECOVER_NOT_EXIST", m_dwCharID, m_szName );
		return S_FALSE;
	}
	
	BOOL bdelete_item = FALSE;
	SINVENITEM* pinven_item_card = m_cInventory.FindItem ( ITEM_STORAGE_LOCK_RECOVER );
	if ( pinven_item_card )	
		bdelete_item = DoDrugInvenItem ( pinven_item_card->wPosX, pinven_item_card->wPosY );

	if ( !bdelete_item )
	{
		SITEMCUSTOM shold_item = GET_SLOT_ITEM( SLOT_HOLD );
		SITEM* pitem_data_hold = GLItemMan::GetInstance().GetItem( shold_item.sNativeID );
		if ( pitem_data_hold && pitem_data_hold->sBasicOp.emItemType == ITEM_STORAGE_LOCK_RECOVER ) 
		{
			bdelete_item = DoDrugSlotItem ( SLOT_HOLD );
		}
	}

	if ( !bdelete_item )	
		CDebugSet::ToFileWithTime( "_storagelock_recover.txt", "[%d]%s item delete failed", m_dwCharID, m_szName );

	net_msg_fb.emFB = EMFB_STORAGE_LOCK_RECOVER_DONE;

	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);

	CDebugSet::ToFileWithTime( "_storagelock_recover.txt", "[%d]%s recover storage lock done %s %s", 
		m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );

	return S_OK;
}

/*storage lock, Juver, 2018/01/17 */
HRESULT GLChar::MsgReqInvenRecoverStorageLockChange ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_STORAGE_LOCK_RECOVER_CHANGE* pnet_msg_client = ( GLMSG::SNETPC_STORAGE_LOCK_RECOVER_CHANGE* ) nmg;
	GLMSG::SNETPC_STORAGE_LOCK_RECOVER_CHANGE_FB net_msg_fb;

	if ( !m_bEnableStorageLock )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_RECOVER_CHANGE_NOT_ENABLED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	std::string strKey1(pnet_msg_client->szPin1);
	std::string strKey2(pnet_msg_client->szPin2);

	if ( strKey1.size() <= 0  || strKey2.size() <= 0  )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_RECOVER_CHANGE_KEY_EMPTY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strKey1.size() < USER_STORAGE_LOCK_PASS_SIZE || strKey1.size() > USER_STORAGE_LOCK_PASS_SIZE || 
		strKey2.size() < USER_STORAGE_LOCK_PASS_SIZE || strKey2.size() > USER_STORAGE_LOCK_PASS_SIZE )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_RECOVER_CHANGE_KEY_SIZE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strcmp( strKey1.c_str(), strKey2.c_str() ) != 0 )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_RECOVER_CHANGE_KEY_NOT_SAME;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strKey1.find_first_not_of( "0123456789" ) != string::npos || strKey2.find_first_not_of( "0123456789" ) != string::npos )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_RECOVER_CHANGE_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CString strTEMP1( strKey1.c_str() );
	CString strTEMP2( strKey2.c_str() );

	if ( STRUTIL::CheckString( strTEMP1 ) || STRUTIL::CheckString( strTEMP2 ) || 
		CRanFilter::GetInstance().NameFilter( strTEMP1 ) || CRanFilter::GetInstance().NameFilter( strTEMP2 ) )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_RECOVER_CHANGE_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ((strTEMP1.FindOneOf(" ") != -1) || (strTEMP2.FindOneOf(" ") != -1) ) 
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_RECOVER_CHANGE_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CStorageLockRecoverChange* pDBAction = new CStorageLockRecoverChange(m_dwClientID, m_dwCharID, m_szName, pnet_msg_client->szPin1 );
	if ( GLGaeaServer::GetInstance().GetDBMan() )
		GLGaeaServer::GetInstance().GetDBMan()->AddJob ( pDBAction );

	return S_OK;
}

/*storage lock, Juver, 2018/01/18 */
HRESULT GLChar::MsgReqInvenRecoverStorageLockChangeDB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_STORAGE_LOCK_RECOVER_FROM_DB *pnet_msg_database = (GLMSG::SNETPC_STORAGE_LOCK_RECOVER_FROM_DB *)nmg;
	GLMSG::SNETPC_STORAGE_LOCK_RECOVER_CHANGE_FB net_msg_fb;

	if ( pnet_msg_database->emFB == EMFB_STORAGE_LOCK_RECOVER_CHANGE_NOT_FOUND )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_RECOVER_CHANGE_NOT_FOUND;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_storagelock_recover_change.txt", "[%d]%s input storage lock EMFB_STORAGE_LOCK_RECOVER_CHANGE_NOT_FOUND", m_dwCharID, m_szName );
		return S_FALSE;
	}

	net_msg_fb.emFB = EMFB_STORAGE_LOCK_RECOVER_CHANGE_DONE;

	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);

	CDebugSet::ToFileWithTime( "_storagelock_recover_change.txt", "[%d]%s recover storage change done %s %s", 
		m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );

	return S_OK;
}

/*storage lock, Juver, 2018/01/18 */
HRESULT GLChar::MsgReqInvenRecoverStorageLockDelete ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_STORAGE_LOCK_RECOVER_DELETE* pnet_msg_client = ( GLMSG::SNETPC_STORAGE_LOCK_RECOVER_DELETE* ) nmg;
	GLMSG::SNETPC_STORAGE_LOCK_RECOVER_DELETE_FB net_msg_fb;

	if ( !m_bEnableStorageLock )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_RECOVER_DELETE_NOT_ENABLED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CStorageLockRecoverDelete* pDBAction = new CStorageLockRecoverDelete(m_dwClientID, m_dwCharID, m_szName );
	if ( GLGaeaServer::GetInstance().GetDBMan() )
		GLGaeaServer::GetInstance().GetDBMan()->AddJob ( pDBAction );

	return S_OK;
}

/*storage lock, Juver, 2018/01/18 */
HRESULT GLChar::MsgReqInvenRecoverStorageLockDeleteDB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_STORAGE_LOCK_RECOVER_DELETE_FROM_DB *pnet_msg_database = (GLMSG::SNETPC_STORAGE_LOCK_RECOVER_DELETE_FROM_DB *)nmg;
	GLMSG::SNETPC_STORAGE_LOCK_RECOVER_DELETE_FB net_msg_fb;

	if ( pnet_msg_database->emFB == EMFB_STORAGE_LOCK_RECOVER_DELETE_NOT_FOUND )
	{
		net_msg_fb.emFB = EMFB_STORAGE_LOCK_RECOVER_DELETE_NOT_FOUND;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_storagelock_recover_delete.txt", "[%d]%s input storage lock EMFB_STORAGE_LOCK_RECOVER_CHANGE_NOT_FOUND", m_dwCharID, m_szName );
		return S_FALSE;
	}

	net_msg_fb.emFB = EMFB_STORAGE_LOCK_RECOVER_DELETE_DONE;

	m_bEnableStorageLock = FALSE;
	m_bStorageLockStatus = FALSE;

	net_msg_fb.bStorageLockEnable = m_bEnableStorageLock;
	net_msg_fb.bStorageLockStatus = m_bStorageLockStatus;

	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);

	CDebugSet::ToFileWithTime( "_storagelock_recover_delete.txt", "[%d]%s recover storage delete done %s", 
		m_dwCharID, m_szName, pnet_msg_database->szName );

	return S_OK;
}

/*inventory lock, Juver, 2018/01/13 */
HRESULT GLChar::MsgReqInvenEnableInventoryLock( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVENTORY_LOCK_ENABLE* pnet_msg_client = ( GLMSG::SNETPC_INVENTORY_LOCK_ENABLE* ) nmg;
	GLMSG::SNETPC_INVENTORY_LOCK_ENABLE_FB net_msg_fb;

	SINVENITEM* pInvenItem = m_cInventory.FindItem ( ITEM_INVENTORY_LOCK_ENABLE );
	if ( !pInvenItem )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_ENABLE_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_ENABLE_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_INVENTORY_LOCK_ENABLE )	
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_ENABLE_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( m_bEnableInventoryLock )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_ENABLE_ALREADY_ENABLED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	std::string strKey1(pnet_msg_client->szPin1);
	std::string strKey2(pnet_msg_client->szPin2);

	if ( strKey1.size() <= 0  || strKey2.size() <= 0  )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_ENABLE_KEY_EMPTY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strKey1.size() < CHAR_INVENTORY_LOCK_PASS_SIZE || strKey1.size() > CHAR_INVENTORY_LOCK_PASS_SIZE || 
		strKey2.size() < CHAR_INVENTORY_LOCK_PASS_SIZE || strKey2.size() > CHAR_INVENTORY_LOCK_PASS_SIZE )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_ENABLE_KEY_SIZE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strcmp( strKey1.c_str(), strKey2.c_str() ) != 0 )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_ENABLE_KEY_NOT_SAME;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strKey1.find_first_not_of( "0123456789" ) != string::npos || strKey2.find_first_not_of( "0123456789" ) != string::npos )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_ENABLE_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CString strTEMP1( strKey1.c_str() );
	CString strTEMP2( strKey2.c_str() );

	if ( STRUTIL::CheckString( strTEMP1 ) || STRUTIL::CheckString( strTEMP2 ) || 
		CRanFilter::GetInstance().NameFilter( strTEMP1 ) || CRanFilter::GetInstance().NameFilter( strTEMP2 ) )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_ENABLE_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ((strTEMP1.FindOneOf(" ") != -1) || (strTEMP2.FindOneOf(" ") != -1) ) 
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_ENABLE_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CInventoryLockEnable* pDBAction = new CInventoryLockEnable(m_dwClientID, m_dwCharID, m_szName, pnet_msg_client->szPin1 );
	if ( GLGaeaServer::GetInstance().GetDBMan() )
		GLGaeaServer::GetInstance().GetDBMan()->AddJob ( pDBAction );
	
	return S_OK;
}

/*inventory lock, Juver, 2018/01/14 */
HRESULT GLChar::MsgReqInvenEnableInventoryLockDB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVENTORY_LOCK_ENABLE_FROM_DB *pnet_msg_database = (GLMSG::SNETPC_INVENTORY_LOCK_ENABLE_FROM_DB *)nmg;

	GLMSG::SNETPC_INVENTORY_LOCK_ENABLE_FB net_msg_fb;

	if( pnet_msg_database->emFB == EMFB_INVENTORY_LOCK_ENABLE_ALREADY_ENABLED_DB )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_ENABLE_ALREADY_ENABLED_DB;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_inventorylock_enable.txt", "[%d]%s enable inventory lock EMFB_INVENTORY_LOCK_ENABLE_ALREADY_ENABLED_DB %s %s", 
			m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );
		return S_FALSE;
	}
	else if( pnet_msg_database->emFB == EMFB_INVENTORY_LOCK_ENABLE_CHAR_NOT_EXIST )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_ENABLE_CHAR_NOT_EXIST;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_inventorylock_enable.txt", "[%d]%s enable inventory lock EMFB_INVENTORY_LOCK_ENABLE_CHAR_NOT_EXIST %s %s", 
			m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );
		return S_FALSE;
	}
	else if ( pnet_msg_database->emFB == EMFB_INVENTORY_LOCK_ENABLE_FAILED )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_ENABLE_FAILED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_inventorylock_enable.txt", "[%d]%s enable inventory lock EMFB_INVENTORY_LOCK_ENABLE_FAILED %s %s", 
			m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );
		return S_FALSE;
	}

	BOOL bdelete_item = FALSE;
	SINVENITEM* pinven_item_card = m_cInventory.FindItem ( ITEM_INVENTORY_LOCK_ENABLE );
	if ( pinven_item_card )	
		bdelete_item = DoDrugInvenItem ( pinven_item_card->wPosX, pinven_item_card->wPosY );

	if ( !bdelete_item )
	{
		SITEMCUSTOM shold_item = GET_SLOT_ITEM( SLOT_HOLD );
		SITEM* pitem_data_hold = GLItemMan::GetInstance().GetItem( shold_item.sNativeID );
		if ( pitem_data_hold && pitem_data_hold->sBasicOp.emItemType == ITEM_INVENTORY_LOCK_ENABLE ) 
		{
			bdelete_item = DoDrugSlotItem ( SLOT_HOLD );
		}
	}

	if ( !bdelete_item )	
		CDebugSet::ToFileWithTime( "_inventorylock_enable.txt", "[%d]%s item delete failed %s %s", 
		m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );


	m_bEnableInventoryLock = TRUE;

	net_msg_fb.emFB = EMFB_INVENTORY_LOCK_ENABLE_DONE;
	net_msg_fb.bInventoryLockEnable = m_bEnableInventoryLock;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);

	CDebugSet::ToFileWithTime( "_inventorylock_enable.txt", "[%d]%s enable inventory lock done %s %s", 
		m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );

	return S_OK;
}

/*inventory lock, Juver, 2018/01/16 */
HRESULT GLChar::MsgReqInvenInputInventoryLock( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVENTORY_LOCK_INPUT* pnet_msg_client = ( GLMSG::SNETPC_INVENTORY_LOCK_INPUT* ) nmg;
	GLMSG::SNETPC_INVENTORY_LOCK_INPUT_FB net_msg_fb;

	if  ( m_fInventoryLockTimer < INVENTORY_LOCK_TIMER )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_INPUT_TIMER;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( !m_bEnableInventoryLock )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_INPUT_NOT_ENABLED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	std::string strKey(pnet_msg_client->szPin);

	if ( strKey.size() <= 0 )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_INPUT_KEY_EMPTY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strKey.size() < CHAR_INVENTORY_LOCK_PASS_SIZE || strKey.size() > CHAR_INVENTORY_LOCK_PASS_SIZE )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_INPUT_KEY_SIZE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strKey.find_first_not_of( "0123456789" ) != string::npos )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_INPUT_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CString strTEMP( strKey.c_str() );

	if ( STRUTIL::CheckString( strTEMP ) || CRanFilter::GetInstance().NameFilter( strTEMP ) )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_INPUT_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strTEMP.FindOneOf(" ") != -1 ) 
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_INPUT_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CInventoryLockInput* pDBAction = new CInventoryLockInput(m_dwClientID, m_dwCharID, m_szName, pnet_msg_client->szPin );
	if ( GLGaeaServer::GetInstance().GetDBMan() )
		GLGaeaServer::GetInstance().GetDBMan()->AddJob ( pDBAction );

	m_fInventoryLockTimer = 0.0f;

	return S_OK;
}

/*inventory lock, Juver, 2018/01/16 */
HRESULT GLChar::MsgReqInvenInputInventoryLockDB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVENTORY_LOCK_INPUT_FROM_DB *pnet_msg_database = (GLMSG::SNETPC_INVENTORY_LOCK_INPUT_FROM_DB *)nmg;
	GLMSG::SNETPC_INVENTORY_LOCK_INPUT_FB net_msg_fb;

	if ( pnet_msg_database->emFB == EMFB_INVENTORY_LOCK_INPUT_FAILED )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_INPUT_FAILED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_inventorylock_input.txt", "[%d]%s input inventory lock EMFB_INVENTORY_LOCK_INPUT_FAILED %s %s", 
			m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );
		return S_FALSE;
	}
	else if( pnet_msg_database->emFB == EMFB_INVENTORY_LOCK_INPUT_DATA_NOT_EXIST )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_INPUT_DATA_NOT_EXIST;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return S_FALSE;
	}
 
	m_bInventoryLockStatus = !m_bInventoryLockStatus;

	net_msg_fb.emFB = EMFB_INVENTORY_LOCK_INPUT_DONE;
	net_msg_fb.bInventoryLockStatus = m_bInventoryLockStatus;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);

	return S_OK;
}

/*inventory lock, Juver, 2018/01/16 */
HRESULT GLChar::MsgReqInvenRecoverInventoryLock ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVENTORY_LOCK_RECOVER* pnet_msg_client = ( GLMSG::SNETPC_INVENTORY_LOCK_RECOVER* ) nmg;
	GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_FB net_msg_fb;

	SINVENITEM* pInvenItem = m_cInventory.FindItem ( ITEM_INVENTORY_LOCK_RECOVER );
	if ( !pInvenItem )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_RECOVER_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_RECOVER_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_INVENTORY_LOCK_RECOVER )	
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_RECOVER_NOT_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( !m_bEnableInventoryLock )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_RECOVER_NOT_ENABLED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CInventoryLockRecover* pDBAction = new CInventoryLockRecover(m_dwClientID, m_dwCharID, m_szName );
	if ( GLGaeaServer::GetInstance().GetDBMan() )
		GLGaeaServer::GetInstance().GetDBMan()->AddJob ( pDBAction );

	return S_OK;
}

/*inventory lock, Juver, 2018/01/16 */
HRESULT GLChar::MsgReqInvenRecoverInventoryLockDB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_FROM_DB *pnet_msg_database = (GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_FROM_DB *)nmg;
	GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_FB net_msg_fb;

	StringCchCopy ( net_msg_fb.szName, CHAR_SZNAME+1, pnet_msg_database->szName );
	StringCchCopy ( net_msg_fb.szPin, CHAR_INVENTORY_LOCK_PASS_SIZE+1, pnet_msg_database->szPin );

	if ( pnet_msg_database->emFB == EMFB_INVENTORY_LOCK_RECOVER_NOT_EXIST )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_RECOVER_NOT_EXIST;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_inventorylock_recover.txt", "[%d]%s input inventory lock EMFB_INVENTORY_LOCK_RECOVER_NOT_EXIST", m_dwCharID, m_szName );
		return S_FALSE;
	}
	
	BOOL bdelete_item = FALSE;
	SINVENITEM* pinven_item_card = m_cInventory.FindItem ( ITEM_INVENTORY_LOCK_RECOVER );
	if ( pinven_item_card )	
		bdelete_item = DoDrugInvenItem ( pinven_item_card->wPosX, pinven_item_card->wPosY );

	if ( !bdelete_item )
	{
		SITEMCUSTOM shold_item = GET_SLOT_ITEM( SLOT_HOLD );
		SITEM* pitem_data_hold = GLItemMan::GetInstance().GetItem( shold_item.sNativeID );
		if ( pitem_data_hold && pitem_data_hold->sBasicOp.emItemType == ITEM_INVENTORY_LOCK_RECOVER ) 
		{
			bdelete_item = DoDrugSlotItem ( SLOT_HOLD );
		}
	}

	if ( !bdelete_item )	
		CDebugSet::ToFileWithTime( "_inventorylock_recover.txt", "[%d]%s item delete failed", m_dwCharID, m_szName );

	net_msg_fb.emFB = EMFB_INVENTORY_LOCK_RECOVER_DONE;

	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);

	CDebugSet::ToFileWithTime( "_inventorylock_recover.txt", "[%d]%s recover inventory lock done %s %s", 
		m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );

	return S_OK;
}

/*inventory lock, Juver, 2018/01/17 */
HRESULT GLChar::MsgReqInvenRecoverInventoryLockChange ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_CHANGE* pnet_msg_client = ( GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_CHANGE* ) nmg;
	GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_CHANGE_FB net_msg_fb;

	if ( !m_bEnableInventoryLock )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_RECOVER_CHANGE_NOT_ENABLED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	std::string strKey1(pnet_msg_client->szPin1);
	std::string strKey2(pnet_msg_client->szPin2);

	if ( strKey1.size() <= 0  || strKey2.size() <= 0  )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_RECOVER_CHANGE_KEY_EMPTY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strKey1.size() < CHAR_INVENTORY_LOCK_PASS_SIZE || strKey1.size() > CHAR_INVENTORY_LOCK_PASS_SIZE || 
		strKey2.size() < CHAR_INVENTORY_LOCK_PASS_SIZE || strKey2.size() > CHAR_INVENTORY_LOCK_PASS_SIZE )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_RECOVER_CHANGE_KEY_SIZE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strcmp( strKey1.c_str(), strKey2.c_str() ) != 0 )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_RECOVER_CHANGE_KEY_NOT_SAME;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ( strKey1.find_first_not_of( "0123456789" ) != string::npos || strKey2.find_first_not_of( "0123456789" ) != string::npos )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_RECOVER_CHANGE_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CString strTEMP1( strKey1.c_str() );
	CString strTEMP2( strKey2.c_str() );

	if ( STRUTIL::CheckString( strTEMP1 ) || STRUTIL::CheckString( strTEMP2 ) || 
		CRanFilter::GetInstance().NameFilter( strTEMP1 ) || CRanFilter::GetInstance().NameFilter( strTEMP2 ) )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_RECOVER_CHANGE_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	if ((strTEMP1.FindOneOf(" ") != -1) || (strTEMP2.FindOneOf(" ") != -1) ) 
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_RECOVER_CHANGE_KEY_NUMBER_ONLY;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CInventoryLockRecoverChange* pDBAction = new CInventoryLockRecoverChange(m_dwClientID, m_dwCharID, m_szName, pnet_msg_client->szPin1 );
	if ( GLGaeaServer::GetInstance().GetDBMan() )
		GLGaeaServer::GetInstance().GetDBMan()->AddJob ( pDBAction );

	return S_OK;
}

/*inventory lock, Juver, 2018/01/18 */
HRESULT GLChar::MsgReqInvenRecoverInventoryLockChangeDB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_FROM_DB *pnet_msg_database = (GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_FROM_DB *)nmg;
	GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_CHANGE_FB net_msg_fb;

	if ( pnet_msg_database->emFB == EMFB_INVENTORY_LOCK_RECOVER_CHANGE_NOT_FOUND )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_RECOVER_CHANGE_NOT_FOUND;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_inventorylock_recover_change.txt", "[%d]%s input inventory lock EMFB_INVENTORY_LOCK_RECOVER_CHANGE_NOT_FOUND", m_dwCharID, m_szName );
		return S_FALSE;
	}

	net_msg_fb.emFB = EMFB_INVENTORY_LOCK_RECOVER_CHANGE_DONE;

	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);

	CDebugSet::ToFileWithTime( "_inventorylock_recover_change.txt", "[%d]%s recover inventory change done %s %s", 
		m_dwCharID, m_szName, pnet_msg_database->szName, pnet_msg_database->szPin );

	return S_OK;
}

/*inventory lock, Juver, 2018/01/18 */
HRESULT GLChar::MsgReqInvenRecoverInventoryLockDelete ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_DELETE* pnet_msg_client = ( GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_DELETE* ) nmg;
	GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_DELETE_FB net_msg_fb;

	if ( !m_bEnableInventoryLock )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_RECOVER_DELETE_NOT_ENABLED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		return E_FAIL;
	}

	CInventoryLockRecoverDelete* pDBAction = new CInventoryLockRecoverDelete(m_dwClientID, m_dwCharID, m_szName );
	if ( GLGaeaServer::GetInstance().GetDBMan() )
		GLGaeaServer::GetInstance().GetDBMan()->AddJob ( pDBAction );

	return S_OK;
}

/*inventory lock, Juver, 2018/01/18 */
HRESULT GLChar::MsgReqInvenRecoverInventoryLockDeleteDB ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_DELETE_FROM_DB *pnet_msg_database = (GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_DELETE_FROM_DB *)nmg;
	GLMSG::SNETPC_INVENTORY_LOCK_RECOVER_DELETE_FB net_msg_fb;

	if ( pnet_msg_database->emFB == EMFB_INVENTORY_LOCK_RECOVER_DELETE_NOT_FOUND )
	{
		net_msg_fb.emFB = EMFB_INVENTORY_LOCK_RECOVER_DELETE_NOT_FOUND;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);
		CDebugSet::ToFileWithTime( "_inventorylock_recover_delete.txt", "[%d]%s input inventory lock EMFB_INVENTORY_LOCK_RECOVER_CHANGE_NOT_FOUND", m_dwCharID, m_szName );
		return S_FALSE;
	}

	net_msg_fb.emFB = EMFB_INVENTORY_LOCK_RECOVER_DELETE_DONE;

	m_bEnableInventoryLock = FALSE;
	m_bInventoryLockStatus = FALSE;

	net_msg_fb.bInventoryLockEnable = m_bEnableInventoryLock;
	net_msg_fb.bInventoryLockStatus = m_bInventoryLockStatus;

	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&net_msg_fb);

	CDebugSet::ToFileWithTime( "_inventorylock_recover_delete.txt", "[%d]%s recover inventory delete done %s", 
		m_dwCharID, m_szName, pnet_msg_database->szName );

	return S_OK;
}

/* booster all vehicle, Juver, 2018/02/14 */
HRESULT GLChar::MsgAllVehicleEnableBooster( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNET_ALLVEHICLE_REQ_ENABLE_BOOSTER* pNetMsg = ( GLMSG::SNET_ALLVEHICLE_REQ_ENABLE_BOOSTER* ) nmg;

	GLMSG::SNET_ALLVEHICLE_REQ_ENABLE_BOOSTER_FB NetMsgFB;
	SNATIVEID sNativeID;

	SINVENITEM* pInvenItem = m_cInventory.FindPosItem ( pNetMsg->wPosX, pNetMsg->wPosY );
	if ( !pInvenItem ) 
	{
		NetMsgFB.emFB = EMALLVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pItem ) 
	{
		NetMsgFB.emFB = EMALLVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( pItem->sBasicOp.emItemType != ITEM_VEHICLE )	
	{
		NetMsgFB.emFB = EMALLVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	SITEM* pHold = GET_SLOT_ITEMDATA ( SLOT_HOLD );
	if ( !pHold ) 
	{
		NetMsgFB.emFB = EMALLVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( pHold->sBasicOp.emItemType != ITEM_CARD_VEHICLE_BOOST )	
	{
		NetMsgFB.emFB = EMALLVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	SITEM* pPutOnItem = GLItemMan::GetInstance().GetItem ( m_PutOnItems[SLOT_HOLD].sNativeID );
	if( pHold != pPutOnItem )
	{
		NetMsgFB.emFB = EMALLVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_CARD;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( pItem->sVehicle.emVehicleType == VEHICLE_TYPE_BOARD )	
	{
		NetMsgFB.emFB = EMALLVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_TYPE;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	DWORD dwVehicleID = pInvenItem->sItemCustom.dwVehicleID;
	if (  dwVehicleID == 0 )
	{
		NetMsgFB.emFB = EMALLVEHICLE_REQ_ENABLE_BOOSTER_FB_INVALID_ITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	if ( m_sVehicle.m_dwGUID == dwVehicleID && m_sVehicle.IsBooster() )	
	{
		NetMsgFB.emFB = EMALLVEHICLE_REQ_ENABLE_BOOSTER_FB_ALREADY_ENABLED;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	CSetVehicleBooster* pDbAction = new CSetVehicleBooster( m_dwClientID, m_dwCharID, dwVehicleID, TRUE );
	GLDBMan* pDbMan = GLGaeaServer::GetInstance().GetDBMan ();
	if ( pDbMan ) pDbMan->AddJob ( pDbAction );

	DoDrugSlotItem ( SLOT_HOLD );

	if ( m_sVehicle.m_dwGUID == dwVehicleID )	
	{
		m_sVehicle.SetBooster( TRUE );
	}

	GLITEMLMT::GetInstance().ReqVehicleAction( dwVehicleID, pHold->sBasicOp.sNativeID, EMVEHICLE_ACTION_BOOSTER_ENABLE, m_sVehicle.m_bBooster );


	NetMsgFB.emFB = EMALLVEHICLE_REQ_ENABLE_BOOSTER_FB_OK;
	NetMsgFB.dwVehicleID = dwVehicleID;
	NetMsgFB.sCardID = pHold->sBasicOp.sNativeID;
	NetMsgFB.sItemID = pItem->sBasicOp.sNativeID;
	GLGaeaServer::GetInstance().SENDTOCLIENT ( m_dwClientID, &NetMsgFB );

	return S_OK;

}

HRESULT GLChar::MsgReqInvenGMItem ( NET_MSG_GENERIC* nmg ) //add itemcmd
{
	if ( !IsValidBody() )	return E_FAIL;
	if ( m_dwUserLvl < USER_GM3 ) return E_FAIL;

	GLMSG::SNET_INVEN_GMITEM *pNetMsg = (GLMSG::SNET_INVEN_GMITEM *) nmg;

	GLMSG::SNET_INVEN_GMITEM_FB NetMsgFB;

	if ( pNetMsg->wPass != GLCONST_CHAR::wItemPass )
	{
		NetMsgFB.emFB = EMREQ_GMITEM_FB_PASS;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pNetMsg->sItemID );
	if ( !pItem  )
	{
		NetMsgFB.emFB = EMREQ_GMITEM_FB_NOTITEM;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
		return E_FAIL;
	}

	GLInventory cInvenTemp;
	cInvenTemp.Assign ( m_cInventory );

	CString strTemp;
	strTemp.Format( ",[%s][%s],Generate Item, [%02d_%02d]",m_szUID, m_szName, pItem->sBasicOp.sNativeID.wMainID,pItem->sBasicOp.sNativeID.wSubID);
	CDebugSet::ToFileWithTime ( "GMItemLogs.csv",strTemp );
	/*CString strName;
	strName.Format ( "%s.csv",m_szName );
	CDebugSet::ToFileWithTime ( strName.GetString(),strTemp );
	strName.Format ( "%d.csv",GetUserID() );
	CDebugSet::ToFileWithTime ( strName.GetString(),strTemp );*/

	WORD	wNum =  pNetMsg->wNum;
	//for ( int i=0; i<wNum; ++i )
	{
		SITEMCUSTOM sCUSTOM;
		sCUSTOM.sNativeID = pItem->sBasicOp.sNativeID;

		if ( sCUSTOM.sNativeID==SNATIVEID(false) )				return E_FAIL;

		SITEM *pITEM = GLItemMan::GetInstance().GetItem ( sCUSTOM.sNativeID );
		if ( !pITEM )
		{
			NetMsgFB.emFB = EMREQ_GMITEM_FB_FAIL;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
			return E_FAIL;
		}

		BOOL bOK = cInvenTemp.InsertItem ( sCUSTOM );
		if ( !bOK )
		{
			NetMsgFB.emFB = EMREQ_GMITEM_FB_INFAIL;
			GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);
			return E_FAIL;
		}
	}

	//for ( int i=0; i<wNum; ++i )
	{
		SITEMCUSTOM sITEM_NEW;
		sITEM_NEW.sNativeID = pItem->sBasicOp.sNativeID;
		//DWORD dwAMOUNT = pItem->sBox.sITEMS[i].dwAMOUNT;
		if ( sITEM_NEW.sNativeID==SNATIVEID(false) )				return E_FAIL;

		SITEM *pITEM = GLItemMan::GetInstance().GetItem ( sITEM_NEW.sNativeID );
		if ( !pITEM )											return E_FAIL;

		CTime cTIME = CTime::GetCurrentTime();
		sITEM_NEW.tBORNTIME = cTIME.GetTime();

		sITEM_NEW.wTurnNum = wNum;
		if( pNetMsg->bDrop ) sITEM_NEW.cGenType = EMGEN_GMEDIT;
		else sITEM_NEW.cGenType = EMGEN_SYSTEM;
		sITEM_NEW.cChnID = (BYTE)GLGaeaServer::GetInstance().GetServerChannel();
		sITEM_NEW.cFieldID = (BYTE)GLGaeaServer::GetInstance().GetFieldSvrID();
		sITEM_NEW.lnGenNum = GLITEMLMT::GetInstance().RegItemGen ( sITEM_NEW.sNativeID, (EMITEMGEN)sITEM_NEW.cGenType );

		sITEM_NEW.cDAMAGE = (BYTE)0; //item is +
		sITEM_NEW.cDEFENSE = (BYTE)0;//item is +



		if( sITEM_NEW.GENERATE_RANDOM_OPT() )
		{
			GLITEMLMT::GetInstance().ReqRandomItem( sITEM_NEW );
		}

		WORD wInsertPosX, wInsertPosY;
		BOOL bOk = m_cInventory.FindInsrtable ( pITEM->sBasicOp.wInvenSizeX, pITEM->sBasicOp.wInvenSizeY, wInsertPosX, wInsertPosY );
		if ( !bOk )		return S_OK;	

		m_cInventory.InsertItem ( sITEM_NEW, wInsertPosX, wInsertPosY );
		SINVENITEM *pInsertItem = m_cInventory.GetItem ( wInsertPosX, wInsertPosY );

		GLITEMLMT::GetInstance().ReqItemRoute ( pInsertItem->sItemCustom, ID_CHAR, 0, ID_CHAR, m_dwCharID, EMITEM_ROUTE_CHAR, pInsertItem->sItemCustom.wTurnNum );

		GLMSG::SNETPC_INVEN_INSERT NetMsg_Inven;
		NetMsg_Inven.Data = *pInsertItem;
		GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsg_Inven);
	}

	NetMsgFB.emFB = EMREQ_GMITEM_FB_OK;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgFB);

	return S_OK;
}

/*dmk14 ingame web*/
HRESULT GLChar::MsgReqBuyFromItemMall ( NET_MSG_GENERIC* nmg )
{
	GLMSG::SNETPC_REQ_BUY_FROM_ITEMMALL *pNetMsg = (GLMSG::SNETPC_REQ_BUY_FROM_ITEMMALL *) nmg;
	if ( !m_pLandMan )		return E_FAIL;

	GLDBMan* pDBMan = GLGaeaServer::GetInstance().GetDBMan ();
	if( !pDBMan )			return E_FAIL;


	MAPITEMMALL_ITER iter = GLGaeaServer::GetInstance().m_mapItemMall.find( pNetMsg->dwProductNum );
	if( iter == GLGaeaServer::GetInstance().m_mapItemMall.end() )	return E_FAIL;
	ITEMMALLDATA sData = iter->second;

	SITEM* pItem = GLItemMan::GetInstance().GetItem ( sData.sNativeID );
	if ( !pItem )		
	{
		TEXTCONSOLEMSG_WRITE("GLChar GetItemMall data not valid item.");	
		return E_FAIL;
	}

	if( sData.dwItemStocks <= 0 )
	{
		GLMSG::SNET_GET_ITEMMALL_FROMDB_FB NetMsgFB;
		NetMsgFB.emFB = EMREQ_ITEMMALL_FROMDB_FB_UPDATE;
		NetMsgFB.sItemMall = sData;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &NetMsgFB );

		NetMsgFB.emFB = EMREQ_ITEMMALL_FROMDB_FB_BUY_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &NetMsgFB );
		return E_FAIL;
	}

	DWORD dwPrice = sData.dwItemPrice;
	if( dwPrice == 0 )
	{
		return E_FAIL;
	}
	
	switch ( sData.wItemCtg )
	{
	case 0:		//premium
		{
			if( m_dwPremiumPoints < dwPrice /*|| m_dwPremiumPoints != dwMoney*/ )		
			{
				//TEXTCONSOLEMSG_WRITE("Premium points not synch %d", m_dwUserID );
				return E_FAIL;
			}
		}
		break;
	case 1:		//combat
		{
			if( m_dwCombatPoints < dwPrice /*|| m_dwCombatPoints != dwMoney*/ )
			{
				//TEXTCONSOLEMSG_WRITE("Combat points not synch %d", m_dwUserID );
				return E_FAIL;
			}
		}
		break;
	}

	int nReturn = pDBMan->InsertBuyItemMall( sData.dwProductNum, sData.dwItemPrice, sData.wItemCtg, m_szUID );
	if( nReturn != 1 )
	{
		return E_FAIL;
	}

	switch ( sData.wItemCtg )
	{
	case 0:		//premium
		{
			m_dwPremiumPoints -= dwPrice;
		}
		break;
	case 1:		//combat
		{
			m_dwCombatPoints -= dwPrice;
		}
		break;
	}
	sData.dwItemStocks -= 1;
	GLGaeaServer::GetInstance().m_mapItemMall[sData.dwProductNum] = sData;

	GLMSG::SNETPC_UPDATE_MONEY NetMsgMoney;
	NetMsgMoney.lnMoney = m_lnMoney;
	NetMsgMoney.dwPremium = m_dwPremiumPoints;
	NetMsgMoney.dwCombat = m_dwCombatPoints;
	GLGaeaServer::GetInstance().SENDTOCLIENT(m_dwClientID,&NetMsgMoney);
	
	GLMSG::SNET_GET_ITEMMALL_FROMDB_FB NetMsgFB;
	NetMsgFB.emFB = EMREQ_ITEMMALL_FROMDB_FB_UPDATE;
	NetMsgFB.sItemMall = sData;
	GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &NetMsgFB );

	NetMsgFB.emFB = EMREQ_ITEMMALL_FROMDB_FB_BUY_SUCCESS;
	GLGaeaServer::GetInstance().SENDTOCLIENT( m_dwClientID, &NetMsgFB );


	return S_OK;
}
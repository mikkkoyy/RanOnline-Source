#include "stdafx.h"

#include "./GLGaeaServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*event map move, Juver, 2017/08/25 */
BOOL GLGaeaServer::EventMoveMap ( DWORD dwCharID, SNATIVEID sMapID, WORD wPosX, WORD wPosY, BOOL bCheckCurMap /*= TRUE*/, BOOL bCalculatePos /*= TRUE*/ )
{
	GLChar* pchar = GetCharID(dwCharID);
	if ( !pchar )	return FALSE;

	//player already inside the target map
	if ( bCheckCurMap && pchar->m_sMapID == sMapID )	return S_OK;

	GLMSG::SNETPC_REQ_EVENT_MOVEMAP_FB	netmsgfb;

	if ( !pchar->IsValidBody() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_CONDITION;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	if ( pchar->m_sCONFTING.IsCONFRONTING() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_CONDITION;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	if ( pchar->m_sTrade.Valid() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_CONDITION;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	if ( sMapID==NATIVEID_NULL() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_FAIL;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	GLMapList::FIELDMAP_ITER iter = m_MapList.find ( sMapID.dwID );
	if ( iter==m_MapList.end() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_FAIL;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	const SMAPNODE *pMapNode = &(*iter).second;

	if ( pchar->m_dwUserLvl < USER_GM3 )
	{
		EMREQFAIL emReqFail(EMREQUIRE_COMPLETE);
		const SLEVEL_REQUIRE &sRequire = pMapNode->sLEVEL_REQUIRE;
		emReqFail = sRequire.ISCOMPLETE ( pchar ); 
		if ( emReqFail != EMREQUIRE_COMPLETE )
		{
			netmsgfb.emFB = EMEVENT_MOVEMAP_FB_CONDITION;
			SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
			return FALSE;
		}
	}

	GLMapAxisInfo sMapAxisInfo;
	D3DXVECTOR3 vPOS(wPosX,0,wPosY); 

	if ( bCalculatePos )
	{
		sMapAxisInfo.LoadFile ( pMapNode->strFile.c_str() );
		sMapAxisInfo.MapPos2MiniPos ( wPosX, wPosY, vPOS.x, vPOS.z );
	}
	
	netmsgfb.emFB = EMEVENT_MOVEMAP_FB_OK;
	SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );

	DropOutPET ( pchar->m_dwPetGUID,false,true );
	//DropOutSummon ( pchar->m_dwSummonGUID, false );
	SetActiveVehicle( pchar->m_dwClientID, pchar->m_dwGaeaID, false );

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		DropOutSummon ( pchar->m_dwSummonGUID_FLD[i] );

	if ( pMapNode->dwFieldSID!=m_dwFieldSvrID )
	{
		GLMSG::SNETPC_REQ_RECALL_AG netmsgag;
		netmsgag.sMAPID = sMapID;
		netmsgag.dwGATEID = UINT_MAX;
		netmsgag.vPOS = vPOS;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pchar->m_dwClientID, &netmsgag );
	}
	else
	{
		GLMSG::SNETPC_REQ_RECALL_FB	netmsgfb2;
		SNATIVEID sCurMapID = pchar->m_sMapID;

		BOOL bOK = RequestInvenRecallThisSvr ( pchar, sMapID, UINT_MAX, vPOS );
		if ( !bOK )
		{
			netmsgfb2.emFB = EMREQ_RECALL_FB_FAIL;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb2 );
			return FALSE;
		}

		if ( sCurMapID != pchar->m_sMapID )
		{
			for ( int i=0; i<EMBLOW_MULTI; ++i )		pchar->DISABLEBLOW ( i );
			for ( int i=0; i<SKILLFACT_SIZE; ++i )		pchar->DISABLESKEFF ( i );
			pchar->DISABLEALLLANDEFF();
		}

		pchar->ResetAction();

		netmsgfb2.emFB = EMREQ_RECALL_FB_OK;
		netmsgfb2.sMAPID = sMapID;
		netmsgfb2.vPOS = pchar->m_vPos;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pchar->m_dwClientID, &netmsgfb2 );
	}

	return TRUE;
}

/*pvp tyranny, Juver, 2017/08/25 */
BOOL GLGaeaServer::TyrannyMoveMap ( DWORD dwCharID, SNATIVEID sMapID, D3DXVECTOR3 vPos, BOOL bCheckCurMap /*= TRUE*/ )
{
	GLChar* pchar = GetCharID(dwCharID);
	if ( !pchar )	return FALSE;

	//player already inside the target map
	if ( bCheckCurMap && pchar->m_sMapID == sMapID )	return S_OK;

	GLMSG::SNETPC_REQ_EVENT_MOVEMAP_FB	netmsgfb;

	if ( !pchar->IsValidBody() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_CONDITION;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	if ( pchar->m_sCONFTING.IsCONFRONTING() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_CONDITION;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	if ( pchar->m_sTrade.Valid() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_CONDITION;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	if ( sMapID==NATIVEID_NULL() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_FAIL;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	GLMapList::FIELDMAP_ITER iter = m_MapList.find ( sMapID.dwID );
	if ( iter==m_MapList.end() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_FAIL;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	const SMAPNODE *pMapNode = &(*iter).second;

	if ( pchar->m_dwUserLvl < USER_GM3 )
	{
		EMREQFAIL emReqFail(EMREQUIRE_COMPLETE);
		const SLEVEL_REQUIRE &sRequire = pMapNode->sLEVEL_REQUIRE;
		emReqFail = sRequire.ISCOMPLETE ( pchar ); 
		if ( emReqFail != EMREQUIRE_COMPLETE )
		{
			netmsgfb.emFB = EMEVENT_MOVEMAP_FB_CONDITION;
			SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
			return FALSE;
		}
	}

	netmsgfb.emFB = EMEVENT_MOVEMAP_FB_OK;
	SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );

	DropOutPET ( pchar->m_dwPetGUID,false,true );
	//DropOutSummon ( pchar->m_dwSummonGUID, false );
	SetActiveVehicle( pchar->m_dwClientID, pchar->m_dwGaeaID, false );

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		DropOutSummon ( pchar->m_dwSummonGUID_FLD[i] );

	if ( pMapNode->dwFieldSID!=m_dwFieldSvrID )
	{
		GLMSG::SNETPC_REQ_RECALL_AG netmsgag;
		netmsgag.sMAPID = sMapID;
		netmsgag.dwGATEID = UINT_MAX;
		netmsgag.vPOS = vPos;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pchar->m_dwClientID, &netmsgag );
	}
	else
	{
		GLMSG::SNETPC_REQ_RECALL_FB	netmsgfb2;
		SNATIVEID sCurMapID = pchar->m_sMapID;

		BOOL bOK = RequestInvenRecallThisSvr ( pchar, sMapID, UINT_MAX, vPos );
		if ( !bOK )
		{
			netmsgfb2.emFB = EMREQ_RECALL_FB_FAIL;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb2 );
			return FALSE;
		}

		if ( sCurMapID != pchar->m_sMapID )
		{
			for ( int i=0; i<EMBLOW_MULTI; ++i )		pchar->DISABLEBLOW ( i );
			for ( int i=0; i<SKILLFACT_SIZE; ++i )		pchar->DISABLESKEFF ( i );
			pchar->DISABLEALLLANDEFF();
		}

		pchar->ResetAction();

		netmsgfb2.emFB = EMREQ_RECALL_FB_OK;
		netmsgfb2.sMAPID = sMapID;
		netmsgfb2.vPOS = pchar->m_vPos;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pchar->m_dwClientID, &netmsgfb2 );
	}

	return TRUE;
}

/*pvp tyranny, Juver, 2017/08/25 */
void GLGaeaServer::TyrannyResetBuff( GLMSG::SNETPC_TYRANNY_A2F_BUFF_RESET *pNetMsg )
{
	PGLCHAR pchar  = GetCharID( pNetMsg->dwCharID );
	if ( !pchar )	return;

	pchar->SystemBuffReset( SYSTEM_BUFF_SIZE );

	GLMSG::SNETPC_SYSTEMBUFF_BRD netmsg_systembuff_brd;
	netmsg_systembuff_brd.dwGaeaID = pchar->m_dwGaeaID;

	for( int i=0; i<SYSTEM_BUFF_SIZE; ++i )
		netmsg_systembuff_brd.sSYSTEM_BUFF[i].Assign( pchar->m_sSYSTEM_BUFF[i], i );

	GLGaeaServer::GetInstance().SENDTOCLIENT( pchar->m_dwClientID, &netmsg_systembuff_brd);
	pchar->SendMsgViewAround ( (NET_MSG_GENERIC*) &netmsg_systembuff_brd );
}

/*pvp tyranny, Juver, 2017/08/25 */
void GLGaeaServer::TyrannyRewardBuff( GLMSG::SNETPC_TYRANNY_A2F_BUFF_REWARD *pNetMsg )
{
	PGLCHAR pchar  = GetCharID( pNetMsg->dwCharID );
	if ( !pchar )	return;

	pchar->SystemBuffReset( SYSTEM_BUFF_SIZE );

	//main reward buff
	pchar->SystemBuffAdd( pNetMsg->sRewardBuffMain.sidSkill, pNetMsg->sRewardBuffMain.wLevel, 0 );

	//reward buff
	for( int i=0; i<TYRANNY_REWARD_BUFF_NUM; ++i )
	{
		WORD _wslot = pchar->SystemBuffFindSlot( pNetMsg->sRewardBuff[i].sidSkill );
		pchar->SystemBuffAdd( pNetMsg->sRewardBuff[i].sidSkill, pNetMsg->sRewardBuff[i].wLevel, _wslot );
	}

	GLMSG::SNETPC_SYSTEMBUFF_BRD netmsg_systembuff_brd;
	netmsg_systembuff_brd.dwGaeaID = pchar->m_dwGaeaID;

	for( int i=0; i<SYSTEM_BUFF_SIZE; ++i )
		netmsg_systembuff_brd.sSYSTEM_BUFF[i].Assign( pchar->m_sSYSTEM_BUFF[i], i );

	GLGaeaServer::GetInstance().SENDTOCLIENT( pchar->m_dwClientID, &netmsg_systembuff_brd);
	pchar->SendMsgViewAround ( (NET_MSG_GENERIC*) &netmsg_systembuff_brd );
}

/*pvp tyranny, Juver, 2017/08/25 */
void GLGaeaServer::TyrannyBattleBuff( GLMSG::SNETPC_TYRANNY_A2F_BUFF_BATTLE *pNetMsg )
{
	PGLCHAR pchar  = GetCharID( pNetMsg->dwCharID );
	if ( !pchar )	return;

	pchar->SystemBuffReset( SYSTEM_BUFF_SIZE );

	//battle buff
	for( int i=0; i<TYRANNY_BATTLE_BUFF_NUM; ++i )
	{
		WORD _wslot = pchar->SystemBuffFindSlot( pNetMsg->sBattleBuff[i].sidSkill );
		pchar->SystemBuffAdd( pNetMsg->sBattleBuff[i].sidSkill, pNetMsg->sBattleBuff[i].wLevel, _wslot );

		//CDebugSet::ToFileWithTime( "_pvptyranny.txt", "buff player:[%u]%s buff[%d~%d] %d", pchar->m_dwCharID, pchar->m_szName, 
		//	pNetMsg->sBattleBuff[i].sidSkill.wMainID, pNetMsg->sBattleBuff[i].sidSkill.wSubID, pNetMsg->sBattleBuff[i].wLevel );
	}

	GLMSG::SNETPC_SYSTEMBUFF_BRD netmsg_systembuff_brd;
	netmsg_systembuff_brd.dwGaeaID = pchar->m_dwGaeaID;

	for( int i=0; i<SYSTEM_BUFF_SIZE; ++i )
		netmsg_systembuff_brd.sSYSTEM_BUFF[i].Assign( pchar->m_sSYSTEM_BUFF[i], i );

	GLGaeaServer::GetInstance().SENDTOCLIENT( pchar->m_dwClientID, &netmsg_systembuff_brd);
	pchar->SendMsgViewAround ( (NET_MSG_GENERIC*) &netmsg_systembuff_brd );
}

/*school wars, Juver, 2018/01/19 */
BOOL GLGaeaServer::SchoolWarsMoveMap ( DWORD dwCharID, SNATIVEID sMapID, D3DXVECTOR3 vPos, BOOL bCheckCurMap /*= TRUE*/ )
{
	GLChar* pchar = GetCharID(dwCharID);
	if ( !pchar )	return FALSE;

	//player already inside the target map
	if ( bCheckCurMap && pchar->m_sMapID == sMapID )	return S_OK;

	GLMSG::SNETPC_REQ_EVENT_MOVEMAP_FB	netmsgfb;

	if ( !pchar->IsValidBody() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_CONDITION;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	if ( pchar->m_sCONFTING.IsCONFRONTING() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_CONDITION;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	if ( pchar->m_sTrade.Valid() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_CONDITION;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	if ( sMapID==NATIVEID_NULL() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_FAIL;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	GLMapList::FIELDMAP_ITER iter = m_MapList.find ( sMapID.dwID );
	if ( iter==m_MapList.end() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_FAIL;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	const SMAPNODE *pMapNode = &(*iter).second;

	if ( pchar->m_dwUserLvl < USER_GM3 )
	{
		EMREQFAIL emReqFail(EMREQUIRE_COMPLETE);
		const SLEVEL_REQUIRE &sRequire = pMapNode->sLEVEL_REQUIRE;
		emReqFail = sRequire.ISCOMPLETE ( pchar ); 
		if ( emReqFail != EMREQUIRE_COMPLETE )
		{
			netmsgfb.emFB = EMEVENT_MOVEMAP_FB_CONDITION;
			SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
			return FALSE;
		}
	}

	netmsgfb.emFB = EMEVENT_MOVEMAP_FB_OK;
	SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );

	DropOutPET ( pchar->m_dwPetGUID,false,true );
	//DropOutSummon ( pchar->m_dwSummonGUID, false );
	SetActiveVehicle( pchar->m_dwClientID, pchar->m_dwGaeaID, false );

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		DropOutSummon ( pchar->m_dwSummonGUID_FLD[i] );

	if ( pMapNode->dwFieldSID!=m_dwFieldSvrID )
	{
		GLMSG::SNETPC_REQ_RECALL_AG netmsgag;
		netmsgag.sMAPID = sMapID;
		netmsgag.dwGATEID = UINT_MAX;
		netmsgag.vPOS = vPos;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pchar->m_dwClientID, &netmsgag );
	}
	else
	{
		GLMSG::SNETPC_REQ_RECALL_FB	netmsgfb2;
		SNATIVEID sCurMapID = pchar->m_sMapID;

		BOOL bOK = RequestInvenRecallThisSvr ( pchar, sMapID, UINT_MAX, vPos );
		if ( !bOK )
		{
			netmsgfb2.emFB = EMREQ_RECALL_FB_FAIL;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb2 );
			return FALSE;
		}

		if ( sCurMapID != pchar->m_sMapID )
		{
			for ( int i=0; i<EMBLOW_MULTI; ++i )		pchar->DISABLEBLOW ( i );
			for ( int i=0; i<SKILLFACT_SIZE; ++i )		pchar->DISABLESKEFF ( i );
			pchar->DISABLEALLLANDEFF();
		}

		pchar->ResetAction();

		netmsgfb2.emFB = EMREQ_RECALL_FB_OK;
		netmsgfb2.sMAPID = sMapID;
		netmsgfb2.vPOS = pchar->m_vPos;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pchar->m_dwClientID, &netmsgfb2 );
	}

	return TRUE;
}

/*pvp capture the flag, Juver, 2018/01/29 */
void GLGaeaServer::CaptureTheFlagPlayerTeam( GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_PLAYER_TEAM *pNetMsg )
{
	PGLCHAR pchar  = GetCharID( pNetMsg->dwCharID );
	if ( !pchar )	return;

	pchar->m_wCaptureTheFlagTeam = pNetMsg->wTeam;

	GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_PLAYER_TEAM_BRD netmsg_player_team_brd;
	netmsg_player_team_brd.dwGaeaID = pchar->m_dwGaeaID;
	netmsg_player_team_brd.wTeam = pchar->m_wCaptureTheFlagTeam;
	GLGaeaServer::GetInstance().SENDTOCLIENT( pchar->m_dwClientID, &netmsg_player_team_brd);
	pchar->SendMsgViewAround ( (NET_MSG_GENERIC*) &netmsg_player_team_brd );
}

/*pvp capture the flag, Juver, 2018/01/30 */
BOOL GLGaeaServer::CaptureTheFlagMoveMap ( DWORD dwCharID, SNATIVEID sMapID, D3DXVECTOR3 vPos, BOOL bCheckCurMap /*= TRUE*/ )
{
	GLChar* pchar = GetCharID(dwCharID);
	if ( !pchar )	return FALSE;

	//player already inside the target map
	if ( bCheckCurMap && pchar->m_sMapID == sMapID )	return S_OK;

	GLMSG::SNETPC_REQ_EVENT_MOVEMAP_FB	netmsgfb;

	if ( !pchar->IsValidBody() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_CONDITION;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	if ( pchar->m_sCONFTING.IsCONFRONTING() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_CONDITION;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	if ( pchar->m_sTrade.Valid() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_CONDITION;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	if ( sMapID==NATIVEID_NULL() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_FAIL;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	GLMapList::FIELDMAP_ITER iter = m_MapList.find ( sMapID.dwID );
	if ( iter==m_MapList.end() )
	{
		netmsgfb.emFB = EMEVENT_MOVEMAP_FB_FAIL;
		SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return FALSE;
	}

	const SMAPNODE *pMapNode = &(*iter).second;

	if ( pchar->m_dwUserLvl < USER_GM3 )
	{
		EMREQFAIL emReqFail(EMREQUIRE_COMPLETE);
		const SLEVEL_REQUIRE &sRequire = pMapNode->sLEVEL_REQUIRE;
		emReqFail = sRequire.ISCOMPLETE ( pchar ); 
		if ( emReqFail != EMREQUIRE_COMPLETE )
		{
			netmsgfb.emFB = EMEVENT_MOVEMAP_FB_CONDITION;
			SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
			return FALSE;
		}
	}

	netmsgfb.emFB = EMEVENT_MOVEMAP_FB_OK;
	SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );

	DropOutPET ( pchar->m_dwPetGUID,false,true );
	//DropOutSummon ( pchar->m_dwSummonGUID, false );
	SetActiveVehicle( pchar->m_dwClientID, pchar->m_dwGaeaID, false );

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		DropOutSummon ( pchar->m_dwSummonGUID_FLD[i] );

	if ( pMapNode->dwFieldSID!=m_dwFieldSvrID )
	{
		GLMSG::SNETPC_REQ_RECALL_AG netmsgag;
		netmsgag.sMAPID = sMapID;
		netmsgag.dwGATEID = UINT_MAX;
		netmsgag.vPOS = vPos;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pchar->m_dwClientID, &netmsgag );
	}
	else
	{
		GLMSG::SNETPC_REQ_RECALL_FB	netmsgfb2;
		SNATIVEID sCurMapID = pchar->m_sMapID;

		BOOL bOK = RequestInvenRecallThisSvr ( pchar, sMapID, UINT_MAX, vPos );
		if ( !bOK )
		{
			netmsgfb2.emFB = EMREQ_RECALL_FB_FAIL;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb2 );
			return FALSE;
		}

		if ( sCurMapID != pchar->m_sMapID )
		{
			for ( int i=0; i<EMBLOW_MULTI; ++i )		pchar->DISABLEBLOW ( i );
			for ( int i=0; i<SKILLFACT_SIZE; ++i )		pchar->DISABLESKEFF ( i );
			pchar->DISABLEALLLANDEFF();
		}

		pchar->ResetAction();

		netmsgfb2.emFB = EMREQ_RECALL_FB_OK;
		netmsgfb2.sMAPID = sMapID;
		netmsgfb2.vPOS = pchar->m_vPos;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pchar->m_dwClientID, &netmsgfb2 );
	}

	return TRUE;
}

/*dmk14 ingame web*/
BOOL GLGaeaServer::ReqItemMallList( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GET_ITEMMALL_FROMDB* pNetMsg )
{
	if ( m_bEmulator )	return FALSE;

	PGLCHAR pChar = GetChar ( dwGaeaID );
	if ( !pChar ) return FALSE;	
	//if( IsClubDMStarted() || IsClubBattleStarted () )		return FALSE;

	{
		VECITEMMALL vecITEMMALL;
		if( m_pDBMan ) m_pDBMan->GetItemMall( vecITEMMALL );

		m_mapItemMall.clear();
		DWORD dwSIZE = static_cast<DWORD>(vecITEMMALL.size());
		for ( DWORD i=0; i<dwSIZE; ++i )
		{
			const ITEMMALLDATA &sITEMMALL = vecITEMMALL[i];
			m_mapItemMall[sITEMMALL.dwProductNum] = sITEMMALL;
		}

		MAPITEMMALL_ITER pos = m_mapItemMall.begin();
		MAPITEMMALL_ITER end = m_mapItemMall.end();

		GLMSG::SNET_GET_ITEMMALL_FROMDB_FB NetMsgFB;
		NetMsgFB.emFB = EMREQ_ITEMMALL_FROMDB_FB_CLEAR;
		SENDTOCLIENT( dwClientID, &NetMsgFB );

		NetMsgFB.emFB = EMREQ_ITEMMALL_FROMDB_FB_ADD;
		for ( ; pos != end; ++pos )
		{
			ITEMMALLDATA& sItemMall = pos->second;
			if( sItemMall.dwIndex >= MAX_ITEM_MALL )	break;

			NetMsgFB.sItemMall = sItemMall;
			SENDTOCLIENT( dwClientID, &NetMsgFB );
		}

		if ( m_pConsoleMsg )	m_pConsoleMsg->Write("Sending ItemMall Data!");
		NetMsgFB.emFB = EMREQ_ITEMMALL_FROMDB_FB_END;
		SENDTOCLIENT( dwClientID, &NetMsgFB );
	}

	return TRUE;
}
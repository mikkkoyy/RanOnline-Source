#include "stdafx.h"
#include "Psapi.h"
#pragma comment( lib, "Psapi.lib" )
#include "./GLGaeaServer.h"
#include "./GLItemMan.h"
#include "./GLBusStation.h"
#include "./GLBusData.h"
#include "./GLTaxiStation.h"
#include "./GLGuidance.h"
#include "./GLClubDeathMatch.h"
#include "./GLSchoolFreePK.h"

/*pvp tyranny, Juver, 2017/08/25 */
#include "./GLPVPTyrannyField.h" 

/*school wars, Juver, 2018/01/19 */
#include "./GLPVPSchoolWarsField.h" 

/*pvp capture the flag, Juver, 2018/01/24 */
#include "./GLPVPCaptureTheFlagField.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void GLGaeaServer::SENDTOALLCLIENT ( LPVOID nmg )
{
	// GASSERT(m_pMsgServer);

	if ( m_pMsgServer )
		m_pMsgServer->SendAllClient ( nmg );
	return;
}

void GLGaeaServer::SENDTOCLIENT ( DWORD dwClientID, LPVOID nmg )
{
	if ( dwClientID>=(m_dwMaxClient*2) )	return;

	// GASSERT(m_pMsgServer);

	if ( m_pMsgServer )
		m_pMsgServer->SendClient ( dwClientID, nmg );

	/////////////////////////////////////////////////////////////////////////////
	// LG-7 GlobalRanking
	GLMSG::SNETPC_UPDATE_MONEY* pNetMsg = (GLMSG::SNETPC_UPDATE_MONEY*)nmg;
	if (pNetMsg->nmg.nType == NET_MSG_GCTRL_UPDATE_MONEY)
	{
		PGLCHAR pChar = GetCharClientID(dwClientID);
		if (pChar)
		{
			GLMSG::SNETPC_REQ_GLOBAL_RANKING_RICH_UPDATE NetMsg;
			NetMsg.sTopRich.dwChaNum	= pChar->m_dwCharID;
			StringCchCopy(NetMsg.sTopRich.szChaName, CHAR_SZNAME, pChar->m_szName);
			NetMsg.sTopRich.wChaLevel	= pChar->m_wLevel;
			NetMsg.sTopRich.nChaClass	= (int)pChar->m_emClass;
			NetMsg.sTopRich.wChaSchool	= pChar->m_wSchool;
			{
				BOOL bGuild = pChar->m_dwGuild ? TRUE : FALSE;
				if (bGuild)
				{
					NetMsg.sTopRich.wGuNum	= (WORD)pChar->m_dwGuild;
					GLClubMan& cClubMan		= GLGaeaServer::GetInstance().GetClubMan();
					GLCLUB* pCLUB			= cClubMan.GetClub(pChar->m_dwGuild);
					if (pCLUB)
					{
						NetMsg.sTopRich.wGuMarkVer = (WORD)pCLUB->m_dwMarkVER;
						StringCchCopy(NetMsg.sTopRich.szGuName, CHAR_SZNAME, pCLUB->m_szName);
					}
				}
			}
			NetMsg.sTopRich.llnMoney = pChar->m_lnMoney;
			SENDTOAGENT(&NetMsg);
		}
	}
	/////////////////////////////////////////////////////////////////////////////
	// Note If you have UPDATE ANYMONEY TYPE, add here too

	return;
}

void GLGaeaServer::SENDTOCLUBCLIENT ( DWORD dwClubID, LPVOID nmg )
{
	GLCLUB *pCLUB = m_cClubMan.GetClub ( dwClubID );
	if ( !pCLUB )	return;

	CLUBMEMBERS_ITER pos = pCLUB->m_mapMembers.begin();
	CLUBMEMBERS_ITER end = pCLUB->m_mapMembers.end();
	
	PGLCHAR pCHAR = NULL;
	for ( ; pos!=end; ++pos )
	{
		pCHAR = GetCharID ( (*pos).first );
		if ( !pCHAR )	
		{
			continue;
		}
		else
		{
			SENDTOCLIENT ( pCHAR->m_dwClientID, nmg );
		}
	}
	return;
}

void GLGaeaServer::SENDTOGM_ONMAP ( DWORD dwMapID, LPVOID nmg )
{
	GLLandMan* pLandMan = GetByMapID( dwMapID );
	if ( !pLandMan )	return;
	if ( !m_pMsgServer )	return;

	PGLCHAR pCHAR = NULL;
	GLCHARNODE* pCharNode = pLandMan->m_GlobPCList.m_pHead;
	for ( ; pCharNode; pCharNode = pCharNode->pNext )
	{
		pCHAR = pCharNode->Data;
		if ( pCHAR && pCHAR->m_dwUserLvl >= USER_GM3 && pCHAR->IsGMCamera() )
		m_pMsgServer->SendClient ( pCharNode->Data->m_dwClientID, nmg );
	}
}

void GLGaeaServer::SENDTOCLIENT_ONMAP ( DWORD dwMapID, LPVOID nmg )
{
	GLLandMan* pLandMan = GetByMapID( dwMapID );
	if ( !pLandMan )	return;
	if ( !m_pMsgServer )	return;

	GLCHARNODE* pCharNode = pLandMan->m_GlobPCList.m_pHead;
	for ( ; pCharNode; pCharNode = pCharNode->pNext )
	{		
		m_pMsgServer->SendClient ( pCharNode->Data->m_dwClientID, nmg );
	}
}

void GLGaeaServer::SENDTOCLUBCLIENT_ONMAP ( DWORD dwMapID, DWORD dwClubID, LPVOID nmg )
{
	GLLandMan* pLandMan = GetByMapID( dwMapID );
	if ( !pLandMan )	return;
	if ( !m_pMsgServer )	return;

	GLCHARNODE* pCharNode = pLandMan->m_GlobPCList.m_pHead;
	for ( ; pCharNode; pCharNode = pCharNode->pNext )
	{		
		if ( pCharNode->Data->m_dwGuild == dwClubID )
			m_pMsgServer->SendClient ( pCharNode->Data->m_dwClientID, nmg );
	}

	return;
}

BOOL GLGaeaServer::ChatMsgProc ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID )
{
	NET_CHAT* pNetMsg = (NET_CHAT*) nmg;

	switch ( pNetMsg->emType )
	{
	case CHAT_TYPE_NORMAL:	// �Ϲݸ޽���
		{
			PGLCHAR pChar = GetChar ( dwGaeaID );
			if ( !pChar )			return FALSE;
			if ( pChar->IsCHATBLOCK() )		return FALSE;

			/*dmk14 chat limit*/
			if( pChar->IsPapering() )		return FALSE;
			pChar->m_bChatProc = true;
			pChar->m_wChatCount++;

			NET_CHAT_FB NetChatFB;
			NetChatFB.emType = pNetMsg->emType;
			NetChatFB.sItemLink = pNetMsg->sItemLink; /*item link, Juver, 2017/07/31 */
			StringCchCopy ( NetChatFB.szName, CHR_ID_LENGTH+1, pChar->GetCharData2().m_szName );
			StringCchCopy ( NetChatFB.szChatMsg, CHAT_MSG_SIZE+1, pNetMsg->szChatMsg );
			
			pChar->SendMsgViewAround ( (NET_MSG_GENERIC*) &NetChatFB );
		}
		break;

	case CHAT_TYPE_PARTY : // ��Ƽ�޽���
		break;

	case CHAT_TYPE_PRIVATE : // ���θ޽���
		{
			PGLCHAR pChar = GetChar ( dwGaeaID );
			if ( !pChar )					return FALSE;
			if ( pChar->IsCHATBLOCK() )		return FALSE;

			GLCHAR_MAP_ITER iterChar = m_PCNameMap.find ( std::string(pNetMsg->szName) );
			if ( iterChar==m_PCNameMap.end() )
			{
				//	�ӼӸ��� ����� ������ FB �޽���.

				return FALSE;
			}

			NET_CHAT_FB NetChatFB;
			NetChatFB.emType = pNetMsg->emType;
			NetChatFB.sItemLink = pNetMsg->sItemLink; /*item link, Juver, 2017/07/31 */
			StringCchCopy ( NetChatFB.szName, CHR_ID_LENGTH+1, pChar->GetCharData2().m_szName );
			StringCchCopy ( NetChatFB.szChatMsg, CHAT_MSG_SIZE+1, pNetMsg->szChatMsg );

			PGLCHAR pGLChar = (*iterChar).second;
			SENDTOCLIENT ( pGLChar->m_dwClientID, &NetChatFB );
		}
		break;

	case CHAT_TYPE_GUILD : // ���޽���
		break;

	default:
		break;
	}

	return TRUE;
}

// *****************************************************
// Desc: �� ���� ó�� ( Normal Map, Instant Map ���� ��� )
// *****************************************************
BOOL GLGaeaServer::EntryLand( DWORD dwGaeaID, DWORD dwGateID, BOOL bInstantMap, SNATIVEID sMapID /*= NATIVEID_NULL()*/ )
{
	if ( dwGaeaID == GAEAID_NULL )			return FALSE;
	PGLCHAR pPC = GetChar ( dwGaeaID );
	if ( !pPC )								return FALSE;

	GLMSG::SNETREQ_GATEOUT_FB			 NetMsgGateOutFB1;
	GLMSG::SNETREQ_CREATE_INSTANT_MAP_FB NetMsgGateOutFB2;

	//	Note : ���̵��� ������.
	//
	const SCONFTING& sCONFTING = pPC->GETCONFRONTING();
	if ( sCONFTING.IsCONFRONTING() )
	{
		switch ( sCONFTING.emTYPE )
		{
		case EMCONFT_ONE:
			{
				PGLCHAR pTAR = GetChar ( sCONFTING.dwTAR_ID );
				if ( pTAR )		pTAR->ResetConfront ( EMCONFRONT_END_MOVEMAP );

				pPC->ResetConfront ( EMCONFRONT_END_MOVEMAP );
				pPC->ReceiveLivingPoint ( GLCONST_CHAR::nCONFRONT_LOSS_LP );
			}
			break;

		case EMCONFT_PARTY:
			{
				pPC->ResetConfront ( EMCONFRONT_END_NOTWIN );
				pPC->ReceiveLivingPoint ( GLCONST_CHAR::nCONFRONT_LOSS_LP );
			}
			break;

		case EMCONFT_GUILD:
			{
				pPC->ResetConfront ( EMCONFRONT_END_NOTWIN );
				pPC->ReceiveLivingPoint ( GLCONST_CHAR::nCONFRONT_LOSS_LP );
			}
			break;
		};
	}


	GLLandMan* pInLandMan = NULL;
	DxLandGateMan* pInGateMan = NULL;
	PDXLANDGATE pInGate = NULL;
	D3DXVECTOR3 vStartPos(0,0,0);

	DxLandGateMan* pOutGateMan = NULL;
	PDXLANDGATE pOutGate = NULL;

	SNATIVEID sCurMapID;
	SNATIVEID sToMapID;

	pOutGateMan = &pPC->m_pLandMan->GetLandGateMan();
	pOutGate = pOutGateMan->FindLandGate ( dwGateID );
	if ( !pOutGate )		goto ENTRY_FAIL;

	//	Memo :	���� ���� ID
	sCurMapID = pPC->m_pLandMan->GetMapID();
	

	//	Memo :	�̵��� ���� ID
	//
	if( bInstantMap )
	{
		if( sToMapID == NATIVEID_NULL() ) goto ENTRY_FAIL;
		sToMapID = sMapID;
	}else{
		sToMapID = pOutGate->GetToMapID();		
	}

	//	Note : Gate�� ����� �� ã��.
	pInLandMan = GetByMapID ( sToMapID );
	if ( !pInLandMan )		goto ENTRY_FAIL;

	if( bInstantMap && pInLandMan )
	{
		if ( pInLandMan->IsDeleteInstantMap( 0.0f ) )
		{
			CDebugSet::ToFileWithTime( "instancemap.txt", "GLGaeaServer::EntryLand instance map deleted [%d~%d]", sToMapID.wMainID, sToMapID.wSubID );
			goto ENTRY_FAIL;
		}
	}

	//	Note : ��ǥ ����Ʈ ��������.
	//
	pInGateMan = &pInLandMan->GetLandGateMan ();
	if ( !pInGateMan )		goto ENTRY_FAIL;

	pInGate = pInGateMan->FindLandGate ( pOutGate->GetToGateID() );
	if ( !pInGate )			goto ENTRY_FAIL;

	vStartPos = pInGate->GetGenPos ( DxLandGate::GEN_RENDUM );

	if ( !pPC->m_pLandMan )
	{
		GASSERT ( 0&&"!pPC->m_pLandMan" );
		goto ENTRY_FAIL;
	}

	if ( !pPC->m_pQuadNode )
	{
		//GASSERT ( 0&&"!pPC->m_pQuadNode" );
		GetConsoleMsg()->Write( "!pPC->m_pQuadNode NULL, %d %d %d", 
			pPC->m_dwCharID, sCurMapID, sToMapID );

		//		goto ENTRY_FAIL;
	}

	//	Note : �ڽ��� �� �ֺ� ���� ����.
	pPC->ResetViewAround ();

	//	Note : ���� �ʿ� �ִ� ���� ��� ����.
	//
	if ( pPC->m_pLandNode )
		pPC->m_pLandMan->m_GlobPCList.DELNODE ( pPC->m_pLandNode );

	//	Note : ���� �ʿ� �ִ� ���� ��� ����.
	//
	if ( pPC->m_pCellNode && pPC->m_pQuadNode )
		pPC->m_pQuadNode->pData->m_PCList.DELNODE ( pPC->m_pCellNode );

	//	Note : ���ο� �� ��ġ�� ������̼� �ʱ�ȭ.
	//
	pPC->SetNavi ( pInLandMan->GetNavi(), vStartPos );

	//	Note : ���ο� ���� ID �ο�.
	//
	pPC->m_sMapID = sToMapID;

	pPC->m_pLandMan = pInLandMan;
	pPC->m_pLandNode = pInLandMan->m_GlobPCList.ADDHEAD ( pPC );

	// �߱��� ��� GainType�� �ʱ�ȭ �Ͽ� �޽����� �� �� �� ���� �Ѵ�.
#ifdef CH_PARAM_USEGAIN //chinatest%%%
	pPC->m_ChinaGainType = 255;
#endif


	//	Note : GLLandMan�� ���� ����ϴ� �۾�.
	//			
	//	RegistChar(pPC); --> GLGaeaServer::RequestLandIn ()�� ȣ��� ������ ������.

	pPC->m_dwCeID = 0;
	pPC->m_pQuadNode = NULL;
	pPC->m_pCellNode = NULL;

	// PET
	// ���̵��� Pet ����
	DropOutPET ( pPC->m_dwPetGUID, false, true );
	//DropOutSummon ( pPC->m_dwSummonGUID, false );
	GLGaeaServer::GetInstance().SetActiveVehicle( pPC->m_dwClientID, pPC->m_dwGaeaID, false );

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		DropOutSummon ( pPC->m_dwSummonGUID_FLD[i] );

	// �ڽ��� ������ �����̻� ȿ���� ��� ������� ���ش�.
	if( sCurMapID != sToMapID )
	{
		for ( int i=0; i<EMBLOW_MULTI; ++i )		pPC->DISABLEBLOW ( i );
		for ( int i=0; i<SKILLFACT_SIZE; ++i )		pPC->DISABLESKEFF ( i );
		pPC->DISABLEALLLANDEFF();
	}

	pPC->ResetAction();


		//	Note : �ڽſ��� ���̵��� �������� �˸�.
	//
	if( !bInstantMap )
	{
		NetMsgGateOutFB1.emFB = EMCHAR_GATEOUT_OK;
		NetMsgGateOutFB1.sMapID = sToMapID;
		NetMsgGateOutFB1.vPos = vStartPos;
		SENDTOAGENT ( pPC->m_dwClientID, (LPVOID) &NetMsgGateOutFB1 );
	}else{
		NetMsgGateOutFB2.emFB		 = EMCHAR_CREATE_INSTANT_MAP_OK;
		NetMsgGateOutFB2.sInstantMapID = sToMapID;
		SNATIVEID sBaseMapID;
		sBaseMapID.wMainID = sMapID.wMainID;
		sBaseMapID.wSubID  = 0;
		NetMsgGateOutFB2.sBaseMapID  = sBaseMapID;
		NetMsgGateOutFB2.vPos		 = vStartPos;
		SENDTOAGENT ( pPC->m_dwClientID, (LPVOID) &NetMsgGateOutFB2 );

	}

	return TRUE;


ENTRY_FAIL:
	//	Note : �ڽſ��� ���̵��� �������� �˸�.
	//
	if( !bInstantMap )
	{
		NetMsgGateOutFB1.emFB = EMCHAR_GATEOUT_FAIL;
		SENDTOAGENT ( pPC->m_dwClientID, (LPVOID) &NetMsgGateOutFB1 );
	}else{
		NetMsgGateOutFB2.emFB = EMCHAR_CREATE_INSTANT_MAP_FAIL;
		SENDTOAGENT ( pPC->m_dwClientID, (LPVOID) &NetMsgGateOutFB2 );
	}

	return FALSE;

}


// *****************************************************
// Desc: ���̵� ��û ó��
// *****************************************************
BOOL GLGaeaServer::RequestMoveMapPC ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNETREQ_GATEOUT *pNetMsg )
{	

	if ( dwGaeaID!=GAEAID_NULL )			return FALSE;
    EntryLand( pNetMsg->dwGaeaID, pNetMsg->dwGateID, FALSE );	


	return FALSE;
}

BOOL GLGaeaServer::RequestCreateInstantMap( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNETREQ_CREATE_INSTANT_MAP_FLD *pNetMsg )
{
	

	if ( dwGaeaID!=GAEAID_NULL )			return FALSE;
	PGLCHAR pPC = GetChar ( pNetMsg->dwGaeaID );
	if ( !pPC )								return FALSE;
	
	// �δ� ���� ���� �׽�Ʈ�� ��� �ڵ�
	/*{

		SNATIVEID sToMapID = pNetMsg->sInstantMapID;

		DWORD StartTime = ::GetTickCount();


		if( CreateInstantMap( pNetMsg->sBaseMapID, sToMapID, dwGaeaID, pChar->GetPartyID() ) == E_FAIL )
		{
			return FALSE;
		}

		GLLandMan* pInLandMan = NULL;
		pInLandMan = GetByMapID ( sToMapID );
		if ( !pInLandMan )	return FALSE;

		DWORD ElspTime = ::GetTickCount() - StartTime;
		

		HANDLE hProcess			= GetCurrentProcess(); 
		PROCESS_MEMORY_COUNTERS pmc;
		if( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) )
		{
			TEXTCONSOLEMSG_WRITE( "[INFO]Create Instant Map! Mem %d Time %d msec Size %dbytes. Map ID [%d][%d]", 
				pmc.WorkingSetSize, ElspTime, sizeof(*pInLandMan), pNetMsg->sInstantMapID.wMainID, pNetMsg->sInstantMapID.wSubID );
		}



		return TRUE;
	}*/

	

	GLMSG::SNETREQ_CREATE_INSTANT_MAP_FB NetMsgFB;

	
	//	Memo :	�̵��� ���� ID
	SNATIVEID sToMapID = pNetMsg->sInstantMapID;

	DWORD StartTime = ::GetTickCount();


	if( pNetMsg->bCreate )
	{
		if( CreateInstantMap( pNetMsg->sBaseMapID, sToMapID, pNetMsg->dwGaeaID, pPC->GetPartyID() ) == E_FAIL )
		{
			NetMsgFB.emFB = EMCHAR_CREATE_INSTANT_MAP_FAIL;
			SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
			return FALSE;
		}
	}

	if( !EntryLand( pNetMsg->dwGaeaID, pNetMsg->dwGateID, TRUE, sToMapID ) ) 
	{
		CDebugSet::ToFileWithTime( "instancemap.txt", "[INFO FIELD]Create Instant Map Failed. MapInfo: ID [%d][%d]", 
							  pNetMsg->sInstantMapID.wMainID, pNetMsg->sInstantMapID.wSubID );
		return FALSE;
	}

	GLLandMan* pInLandMan = GetByMapID ( sToMapID );
	if ( pInLandMan )
	{
		if( pNetMsg->bCreate )
		{
			CDebugSet::ToFileWithTime( "instancemap.txt", "[INFO FIELD]Create Instant Map MapInfo: ID [%d][%d] PartyMap %d HostID %d", 
				pNetMsg->sInstantMapID.wMainID, pNetMsg->sInstantMapID.wSubID, pInLandMan->IsPartyInstantMap(), pInLandMan->GetInstantMapHostID() );
		}else{
			CDebugSet::ToFileWithTime( "instancemap.txt", "[INFO FIELD]Enter Instant Map MapInfo: ID [%d][%d] PartyMap %d HostID %d", 
				pNetMsg->sInstantMapID.wMainID, pNetMsg->sInstantMapID.wSubID, pInLandMan->IsPartyInstantMap(), pInLandMan->GetInstantMapHostID() );
		}
	}


	return FALSE;
}

// *****************************************************
// Desc: �ʵ弭������ �������� Agent ���� �˸�
// *****************************************************
BOOL GLGaeaServer::RequestFieldSvrOut ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNETPC_FIELDSVR_OUT *pNetMsg )
{
	PGLCHAR pPC = GetChar ( dwGaeaID );

	if ( !pPC )								return FALSE;
	if ( pPC->m_dwClientID!=dwClientID )	return FALSE;

	//	Note : ������ �̵��� ������.
	//
	const SCONFTING& sCONFTING = pPC->GETCONFRONTING();
	if ( sCONFTING.IsCONFRONTING() )
	{
		switch ( sCONFTING.emTYPE )
		{
		case EMCONFT_ONE:
			{
				PGLCHAR pTAR = GetChar ( sCONFTING.dwTAR_ID );
				if ( pTAR )		pTAR->ResetConfront ( EMCONFRONT_END_MOVEMAP );

				pPC->ResetConfront ( EMCONFRONT_END_MOVEMAP );
				pPC->ReceiveLivingPoint ( GLCONST_CHAR::nCONFRONT_LOSS_LP );
			}
			break;
		
		case EMCONFT_PARTY:
			{
				pPC->ResetConfront ( EMCONFRONT_END_NOTWIN );
				pPC->ReceiveLivingPoint ( GLCONST_CHAR::nCONFRONT_LOSS_LP );
			}
			break;
		
		case EMCONFT_GUILD:
			{
				pPC->ResetConfront ( EMCONFRONT_END_NOTWIN );
				pPC->ReceiveLivingPoint ( GLCONST_CHAR::nCONFRONT_LOSS_LP );
			}
			break;
		};
	}

	//	Note : CNetUser::m_dwGaeaID �� ��ȿȭ ó�� �ؾ���.
	//
	m_pMsgServer->ResetGaeaID ( dwClientID );

	//	Note : Agent�� ó������ �˸��� actor�� ������.	( new �� ��ü�� db actor ���� �����ȴ�. )
	//
	GLMSG::SNETPC_FIELDSVR_OUT_FB NetMsg;
	CDbActToAgentMsg *pDbActToAgentMsg = new CDbActToAgentMsg;
	pDbActToAgentMsg->SetMsg ( dwClientID, (NET_MSG_GENERIC *)&NetMsg );

	//	Note : �ɸ��� ���� ��û.
	//
	ReserveDropOutPC ( pPC->m_dwGaeaID, pDbActToAgentMsg );

#if defined(_RELEASED) || defined(TW_PARAM) || defined(TH_PARAM) || defined(HK_PARAM)// ***Tracing Log print
	if( pPC->m_bTracingUser && pPC->m_pLandMan)
	{
		NET_LOG_UPDATE_TRACINGCHAR TracingMsg;
		TracingMsg.nUserNum  = pPC->GetUserID();
		StringCchCopy( TracingMsg.szAccount, USR_ID_LENGTH+1, pPC->m_szUID );

		int nPosX(0);
		int nPosY(0);
		pPC->m_pLandMan->GetMapAxisInfo().Convert2MapPos ( pPC->m_vPos.x, pPC->m_vPos.z, nPosX, nPosY );

		CString strTemp;
		strTemp.Format( "FieldServer Out!!, [%s][%s], MAP:mid[%d]sid[%d], StartPos:[%d][%d], Money:[%I64d]",
			pPC->m_szUID, pPC->m_szName, pPC->m_sMapID.wMainID, pPC->m_sMapID.wSubID, nPosX, nPosY, pPC->m_lnMoney );

		StringCchCopy( TracingMsg.szLogMsg, TRACING_LOG_SIZE, strTemp.GetString() );

		SENDTOAGENT( pPC->m_dwClientID, &TracingMsg );
	}
#endif

	return TRUE;
}

// *****************************************************
// Desc: ���� ���� ���� �ʿ� �ӹ��� ���� �ʾƾ� �ϴ� ���
// *****************************************************
BOOL GLGaeaServer::RequestMustLeaveMap( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNETPC_REQ_MUST_LEAVE_MAP* pNetMsg )
{
	PGLCHAR pPC = GetChar ( dwGaeaID );

	if ( !pPC )								return FALSE;
	if ( pPC->m_dwClientID!=dwClientID )	return FALSE;


	SNATIVEID sMAPID(false);
	DWORD dwGATEID(UINT_MAX);
	D3DXVECTOR3 vPOS(0.0f,0.0f,0.0f);

	sMAPID	 = pNetMsg->sMAPID;
	dwGATEID = pNetMsg->dwGATEID;

	GLMSG::SNETPC_REQ_MUST_LEAVE_MAP_FB NetMsgFB;

	if ( dwGATEID==UINT_MAX )				
	{
		SENDTOCLIENT ( dwClientID, &NetMsgFB ); // ���� �޽���
		return FALSE;
	}
	if ( sMAPID==SNATIVEID(false) )			
	{
		SENDTOCLIENT ( dwClientID, &NetMsgFB ); // ���� �޽���
		return FALSE;
	}

	const SLEVEL_REQUIRE* pLEVEL_REQUIRE = GetLevelRequire(pPC->m_sMapID.dwID);
	if ( pLEVEL_REQUIRE != NULL )
	{
		EMREQFAIL emReqFail(EMREQUIRE_COMPLETE);
		emReqFail = pLEVEL_REQUIRE->ISCOMPLETE ( pPC );
		if( emReqFail == EMREQUIRE_COMPLETE )
		{
			SENDTOCLIENT ( dwClientID, &NetMsgFB );  // ���� �޽���
			return FALSE;
		}
	}else{
		SENDTOCLIENT ( dwClientID, &NetMsgFB );  // ���� �޽���
		return FALSE;
	}

	
	GLMapList::FIELDMAP_ITER iter = m_MapList.find ( sMAPID.dwID );
	if ( iter==m_MapList.end() )
	{
		SENDTOCLIENT( dwClientID, &NetMsgFB );
		return FALSE;
	}


	SMAPNODE *pMapNode = FindMapNode ( sMAPID );
	if ( !pMapNode )					return FALSE;

	// PET
	// ���̵��� Pet ����
	DropOutPET ( pPC->m_dwPetGUID, true, true );
	//DropOutSummon ( pPC->m_dwSummonGUID, true );
	SaveVehicle( pPC->m_dwClientID, pPC->m_dwGaeaID, true );

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		DropOutSummon ( pPC->m_dwSummonGUID_FLD[i] );

	//	Note : �ٸ� �ʵ� ������ ���.
	if ( pMapNode->dwFieldSID!=m_dwFieldSvrID )
	{
		GLMSG::SNETPC_REQ_MUST_LEAVE_MAP_AG NetMsgAg;
		NetMsgAg.sMAPID = sMAPID;
		NetMsgAg.dwGATEID = dwGATEID;
		NetMsgAg.vPOS = vPOS;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pPC->m_dwClientID, &NetMsgAg );
	}else{

		SNATIVEID sCurMapID = pPC->m_sMapID;

		BOOL bOK = RequestInvenRecallThisSvr ( pPC, sMAPID, dwGATEID, vPOS );
		if ( !bOK )
		{
			SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
			return FALSE;
		}
		
		// ������ �����̻� ����
		if ( sCurMapID != pPC->m_sMapID )
		{
			for ( int i=0; i<EMBLOW_MULTI; ++i )		pPC->DISABLEBLOW ( i );
			for ( int i=0; i<SKILLFACT_SIZE; ++i )		pPC->DISABLESKEFF ( i );
			pPC->DISABLEALLLANDEFF();
		}

		pPC->ResetAction();

		//	Note : �� �̵� ������ �˸�.
		//
		GLMSG::SNETPC_REQ_RECALL_FB NetRecallFB;
		NetRecallFB.emFB = EMREQ_RECALL_FB_OK;
		NetRecallFB.sMAPID = sMAPID;
		NetRecallFB.vPOS = pPC->m_vPos;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pPC->m_dwClientID, &NetRecallFB );

	}

	return TRUE;
}

// *****************************************************
// Desc: Ŭ���̾�Ʈ �δ� �� ���� ��û ó��
// *****************************************************
BOOL GLGaeaServer::RequestCreateInstantMapReq( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNETREQ_CREATE_INSTANT_MAP_REQ *pNetMsg )
{
	GLMSG::SNETREQ_CREATE_INSTANT_MAP_FB  NetMsgFB;
	GLMSG::SNETREQ_CREATE_INSTANT_MAP_REQ NetMsg;

	SNATIVEID sTargetMapID;

	

	PGLCHAR pPC = GetChar ( dwGaeaID );	

	if ( !pPC )								return FALSE;
	if ( pPC->m_dwClientID!=dwClientID )	goto _CreteInstantMapReqError;

	DWORD dwGateID = pNetMsg->dwGateID;
	if ( dwGateID==UINT_MAX )				goto _CreteInstantMapReqError;

	if ( !pPC->m_pLandMan )					goto _CreteInstantMapReqError;

	DxLandGateMan *pLandGateMan = &pPC->m_pLandMan->GetLandGateMan();
	if ( !pLandGateMan )					goto _CreteInstantMapReqError;

	PDXLANDGATE pLandGate = pLandGateMan->FindLandGate ( dwGateID );
	if ( !pLandGate )
	{
		NetMsgFB.emFB = EMCHAR_CREATE_INSTANT_MAP_FAIL_GATEID;
		SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
		return FALSE;
	}

	sTargetMapID = pLandGate->GetToMapID();

	const SMAPNODE  *pMapNode = FindMapNode ( sTargetMapID );
	const GLLandMan *pDestLandMan = GetInstantMapByMapID ( sTargetMapID );
	if ( !pMapNode || !pDestLandMan )
	{
		NetMsgFB.emFB = EMCHAR_CREATE_INSTANT_MAP_FAIL_TARMAPID;
		SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
		return FALSE;
	}
	if ( !pMapNode->bInstantMap )
	{
		NetMsgFB.emFB = EMCHAR_CREATE_INSTANT_MAP_FAIL_NOT;
		SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
		return FALSE;
	}

	// �������� �˻��� ����Ʈ
	PDXLANDGATE pLandGateInServer = pLandGateMan->DetectGate ( pPC->GetPosition() );

	// �������� �˻�
	if ( pPC->m_dwUserLvl < USER_GM3 )
	{
		EMREQFAIL emReqFail(EMREQUIRE_COMPLETE);
		const SLEVEL_REQUIRE &sRequire = pMapNode->sLEVEL_REQUIRE;
		emReqFail = sRequire.ISCOMPLETE ( pPC ); 
		if ( emReqFail != EMREQUIRE_COMPLETE )
		{
			//	��� ������ ����.
			NetMsgFB.emFB = EMCHAR_CREATE_INSTANT_MAP_FAIL_CONDITION;
			SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
			return FALSE;
		}

		// �������� �˻��� ����Ʈ�� ���� ��� ( �븸 GS ���� ��ó )
		if ( !pLandGateInServer || !(pLandGateInServer->GetFlags()&DxLandGate::GATE_OUT ) ) 
			goto _CreteInstantMapReqError;
	}


	// �̹� ������ �δ��� ������ �̰ų� ��Ƽ �߿� ���� �δ��� ������ ����� ��������� ó��
	{
		VEC_INSTANT_MAPID_ITER iter = m_vecInstantMapId.begin();
		bool  bEnterInstantMap = FALSE;
		for( ; iter != m_vecInstantMapId.end(); ++iter )
		{
			bEnterInstantMap = FALSE;

			PGLLANDMAN pLandMan = GetByMapID( *iter );
			if( !pLandMan ) continue;
			if( pLandMan->GetMapID().wMainID != sTargetMapID.wMainID ) continue;
			if( !pLandMan->IsInstantMap() )	continue;

			if( !pLandMan->IsExcessiveLimitTime() ) continue;
			if( pLandMan->IsPartyInstantMap() )
			{
				if( pLandMan->GetInstantMapHostID() == pPC->GetPartyID() )
					bEnterInstantMap = TRUE;
			}else if( pPC->GetPartyID() == PARTY_NULL ){
				if( pLandMan->GetInstantMapHostID() == pPC->m_dwGaeaID )
					bEnterInstantMap = TRUE;
			}			

			if( bEnterInstantMap )
			{
				NetMsg.sInstantMapID = *iter;
				NetMsg.dwGaeaID	  = dwGaeaID;
				NetMsg.dwGateID	  = dwGateID;
				NetMsg.bCreate	  = FALSE;
				SENDTOAGENT ( (LPVOID) &NetMsg );
				return TRUE;
			}			
		}
	}

	if ( pPC->GetPartyID() != PARTY_NULL )
	{
		//prevent entering map if another party member is inside another instance map
		BOOL bExist = FALSE;
		for( VEC_INSTANT_MAPID_ITER iter = m_vecInstantMapId.begin() ; iter != m_vecInstantMapId.end(); ++iter )
		{
			PGLLANDMAN pLandMan = GetByMapID( *iter );
			if( !pLandMan ) continue;
			if( !pLandMan->IsInstantMap() )			continue;
			if( !pLandMan->IsPartyInstantMap() )	continue;
			if( pLandMan->GetInstantMapHostID() == pPC->GetPartyID() )
			{
				bExist = TRUE;
				break;
			}
		}

		if( bExist )
			goto _CreteInstantMapReqError;
	}
	else
	{
		//prevent creating another instance map when previous instance map is active
		BOOL bExist = FALSE;
		for( VEC_INSTANT_MAPID_ITER iter = m_vecInstantMapId.begin() ; iter != m_vecInstantMapId.end(); ++iter )
		{
			PGLLANDMAN pLandMan = GetByMapID( *iter );
			if( !pLandMan ) continue;
			if( !pLandMan->IsInstantMap() )			continue;
			if( pLandMan->IsPartyInstantMap() )		continue;
			if( pLandMan->GetInstantMapHostID() == pPC->m_dwGaeaID )
			{
				bExist = TRUE;
				break;
			}
		}

		if( bExist )
			goto _CreteInstantMapReqError;
	}

	MEMORYSTATUS memStatus;
	GlobalMemoryStatus( &memStatus );

	// ������ �����Ҽ� �ִ� �޸𸮸� �ʰ����� ��� ������ ���� �ʴ´�.
	// ���� �������� 10kb�� ���� �д�.
	if( memStatus.dwAvailPhys - ( 1024 * 10 ) <= sizeof(*pDestLandMan) )
	{
		NetMsgFB.emFB = EMCHAR_CREATE_INSTANT_MAP_FAIL_FIELDSVR;
		SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
		return FALSE;
	}


	NetMsg.dwGateID = dwGateID;
	NetMsg.dwGaeaID = dwGaeaID;
	SENDTOAGENT ( (LPVOID) &NetMsg );

	return TRUE;

_CreteInstantMapReqError:
	NetMsgFB.emFB = EMCHAR_CREATE_INSTANT_MAP_FAIL;
	SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
	return FALSE;
}

// *****************************************************
// Desc: Ŭ���̾�Ʈ�� ����Ʈ �̵� ��û ó��
// *****************************************************
BOOL GLGaeaServer::RequestGateOutReq ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNETREQ_GATEOUT_REQ* pNetMsg )
{
	GLMSG::SNETREQ_GATEOUT_FB NetMsgFB;
	GLMSG::SNETREQ_GATEOUT NetMsg;

	SNATIVEID sMapID;

	PGLCHAR pPC = GetChar ( dwGaeaID );	

	if ( !pPC )								return FALSE;
	if ( pPC->m_dwClientID!=dwClientID )	goto _GateOutError;

	DWORD dwGateID = pNetMsg->dwGateID;
	if ( dwGateID==UINT_MAX )				goto _GateOutError;

	if ( !pPC->m_pLandMan )					goto _GateOutError;

	DxLandGateMan *pLandGateMan = &pPC->m_pLandMan->GetLandGateMan();
	if ( !pLandGateMan )					goto _GateOutError;

	PDXLANDGATE pLandGate = pLandGateMan->FindLandGate ( dwGateID );
	if ( !pLandGate )						goto _GateOutError;

	sMapID = pLandGate->GetToMapID();

	const SMAPNODE *pMapNode = FindMapNode ( sMapID );
	if ( !pMapNode )						goto _GateOutError;

	// �������� �˻��� ����Ʈ
	PDXLANDGATE pLandGateInServer = pLandGateMan->DetectGate ( pPC->GetPosition() );

	if( sMapID.wMainID == 251 ){
		GLPARTY_FIELD *pParty = GLGaeaServer::GetInstance().GetParty( pPC->m_dwPartyID );
		if( pParty && pParty->ISVAILD() )
		{
			GLPARTY_FIELD::MEMBER_ITER iter = pParty->m_cMEMBER.begin();
			GLPARTY_FIELD::MEMBER_ITER iter_end = pParty->m_cMEMBER.end();
			PGLCHAR pMemChar = NULL;
			SNATIVEID sNativeID;
			sNativeID.wMainID = 186;
			sNativeID.wSubID = 1;
			for ( ; iter!=iter_end; ++iter )
			{
				pMemChar = GLGaeaServer::GetInstance().GetChar( (*iter).first );
				if ( pMemChar )
				{
					if( pPC->m_dwGuild != pMemChar->m_dwGuild ){
						NetMsgFB.emFB = EMCHAR_GATEOUT_OTHERCLUB_NOTALLOWED_INPARTY;
						SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
						return FALSE;
					}
				}
			}
		}
	}
	

	// �������� �˻�
	if ( pPC->m_dwUserLvl < USER_GM3 )
	{
		EMREQFAIL emReqFail(EMREQUIRE_COMPLETE);
		const SLEVEL_REQUIRE &sRequire = pMapNode->sLEVEL_REQUIRE;
		emReqFail = sRequire.ISCOMPLETE ( pPC ); 
		if ( emReqFail != EMREQUIRE_COMPLETE )
		{
			//	��� ������ ����.
			NetMsgFB.emFB = EMCHAR_GATEOUT_CONDITION;
			SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
			return FALSE;
		}

		// �������� �˻��� ����Ʈ�� ���� ��� ( �븸 GS ���� ��ó )
		if ( !pLandGateInServer || !(pLandGateInServer->GetFlags()&DxLandGate::GATE_OUT ) ) 
			goto _GateOutError;
	}

	// ���������̸�
	GLLandMan* pLandMan = GetByMapID ( sMapID );
	if ( pLandMan )
	{
		if ( pMapNode->bBattleZone )
		{
			if ( GLCONST_CHAR::bPKLESS ) 
			{
				// ������ ���� ���� �� ����...
				NetMsgFB.emFB = EMCHAR_GATEOUT_HOLD;
				SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
				return FALSE;
			}			

			GLGuidance *pGuid = GLGuidanceFieldMan::GetInstance().Find ( pLandMan->m_dwClubMapID );
			if ( pGuid )
			{

				// ������ �϶� ��Ʈ��Ʋ ���̰ų� ����ð� �̴��̸� ���Ա���
				GLCLUB* pClub = m_cClubMan.GetClub ( pPC->m_dwGuild );
				if ( pClub && pGuid->m_bBattle && 
					( pClub->GetAllBattleNum() > 0 ||
					 !pClub->IsGuidBattle() ) )
				{
					NetMsgFB.emFB = EMCHAR_GATEOUT_CLUBBATTLE;
					SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
					return FALSE;
				}


				if ( !pGuid->m_bBattle && !pGuid->IsGuidClub(pPC->m_dwGuild) )
				{
					GLCLUB* pGuidanceClub = m_cClubMan.GetClub ( pGuid->m_dwGuidanceClub );
					if ( pGuidanceClub )
					{
						// ����Ŭ���� ������ �ƴϸ�
						if ( !pGuidanceClub->IsAllianceGuild ( pPC->m_dwGuild ) )
						{
							NetMsgFB.emFB = EMCHAR_GATEOUT_HOLD;
							SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
							return FALSE;
						}
					}
				}
			}
		}

		if ( pMapNode->bCDMZone )
		{
			if ( GLCONST_CHAR::bPKLESS ) 
			{
				// ������ ���� ���� �� ����...
				NetMsgFB.emFB = EMCHAR_GATEOUT_HOLD;
				SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
				return FALSE;
			}			

			GLClubDeathMatch *pCDM = GLClubDeathMatchFieldMan::GetInstance().Find ( pLandMan->m_dwClubMapID );
			if ( pCDM )
			{
				GLCLUB* pClub = m_cClubMan.GetClub ( pPC->m_dwGuild );
				if ( !pCDM->IsEnterMap( pPC->m_dwGuild ) || !pClub )
				{
					NetMsgFB.emFB = EMCHAR_GATEOUT_HOLD;
					SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
					return FALSE;
				}	
								
				if ( pClub )
				{
					// Ŭ����Ʋ �������϶��� ������.
                    if ( pClub->GetAllBattleNum() > 0 )
					{
						NetMsgFB.emFB = EMCHAR_GATEOUT_CLUBBATTLE2;
						SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
						return FALSE;
					}

					if ( !pClub->IsMemberFlgCDM(pPC->m_dwCharID) )
					{
						NetMsgFB.emFB = EMCHAR_GATEOUT_HOLD;
						SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
						return FALSE;
					}
				}
			}
		}

		/*clubwar map lock, Juver, 2017/06/27 */
		if ( pLandMan->m_bClubWarLock )
		{
			NetMsgFB.emFB = EMCHAR_GATEOUT_CLUBBATTLE_LOCK;
			SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
			return FALSE;
		}

		/*pvp tyranny, Juver, 2017/08/25 */
		if ( pMapNode->bPVPTyrannyZone )
		{
			TYRANNY_PLAYER_DATA* pplayerdata = GLPVPTyrannyField::GetInstance().PlayerDataGet( pPC->m_dwCharID );
			if ( !pplayerdata )
			{
				NetMsgFB.emFB = EMCHAR_GATEOUT_HOLD;
				SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
				return FALSE;
			}
		}

		/*school wars, Juver, 2018/01/19 */
		if ( pMapNode->bPVPSchoolWarsZone )
		{
			SCHOOLWARS_PLAYER_DATA* pplayerdata = GLPVPSchoolWarsField::GetInstance().PlayerDataGet( pPC->m_dwCharID );
			if ( !pplayerdata )
			{
				NetMsgFB.emFB = EMCHAR_GATEOUT_HOLD;
				SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
				return FALSE;
			}
		}

		/*pvp capture the flag, Juver, 2018/02/01 */
		if ( pMapNode->bPVPCaptureTheFlagZone )
		{
			CAPTURE_THE_FLAG_PLAYER_DATA* pplayerdata = GLPVPCaptureTheFlagField::GetInstance().PlayerDataGet( pPC->m_dwCharID );
			if ( !pplayerdata )
			{
				NetMsgFB.emFB = EMCHAR_GATEOUT_HOLD;
				SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
				return FALSE;
			}
		}
	}
	
	NetMsg.dwGateID = dwGateID;
	NetMsg.dwGaeaID = dwGaeaID;
	SENDTOAGENT ( (LPVOID) &NetMsg );

	return TRUE;

_GateOutError:
	NetMsgFB.emFB = EMCHAR_GATEOUT_FAIL;
	SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );
	return FALSE;

}

BOOL GLGaeaServer::RequestReBirthOut ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNETPC_REBIRTH_OUT *pNetMsg )
{
	PGLCHAR pPC = GetChar ( dwGaeaID );

	if ( !pPC )								return FALSE;
	if ( pPC->m_dwClientID!=dwClientID )	return FALSE;

	if ( !pPC->IsSTATE(EM_ACT_DIE) )
	{
		GetConsoleMsg()->Write ( LOG_TEXT_CONSOLE, 
			"State : %d , RequestReBirthOut() : ��Ȱ�޽��� ó���� ��� �÷��װ� ��� ��û�� �����Ͽ����ϴ�.", pPC->GetSTATE() );
	}

	//	��Ȱ�� ä�� ȸ��.
	//
	pPC->m_fGenAge = 0.0f;
	pPC->INIT_RECOVER();

	//	 Note : ���� ��ġ�� ������ ��ȯ ��ġ�� ����.
	//
	//
	pPC->SaveLastCall ();

	//	Note : ## ����� ���·� ��Ȱ�� ����ġ ���Ҵ� GLGaeaServer::DropOutPC() ���� ó��. ##

	//	Note : CNetUser::m_dwGaeaID �� ��ȿȭ ó�� �ؾ���.
	//
	m_pMsgServer->ResetGaeaID ( dwClientID );

	//	Note : Agent���� ó�� �Ϸ� �˸�, actor�� ������.	( new �� ��ü�� db actor ���� �����ȴ�. )
	//
	GLMSG::SNETPC_REBIRTH_OUT_FB NetMsg;
	CDbActToAgentMsg *pDbActToAgentMsg = new CDbActToAgentMsg;
	pDbActToAgentMsg->SetMsg ( dwClientID, (NET_MSG_GENERIC *)&NetMsg );
	
	//	�ɸ��� ���� ��û.
	ReserveDropOutPC ( pPC->m_dwGaeaID, pDbActToAgentMsg );

	return TRUE;
}

BOOL GLGaeaServer::RequestLandIn ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNETREQ_LANDIN *pNetMsg )
{
	PGLCHAR pPC = GetChar ( dwGaeaID );
	if ( !pPC )								return FALSE;

	if ( !pPC->IsSTATE(EM_GETVA_AFTER) )
	{
		//	Note : ĳ���� �ֺ��� ������ ����.
		//
		pPC->GetViewAround ();
	}
	else
	{
		CONSOLEMSG_WRITE ( "GLGaeaServer::RequestLandIn() Call this function after receive view around message already [%s][%d]", pPC->m_szName, pPC->m_dwGaeaID );
		//DEBUGMSG_WRITE( _T("GLGaeaServer::RequestLandIn() else [%s][%u]"), pPC->m_szName, pPC->m_dwGaeaID );
	}

	return TRUE;
}

BOOL GLGaeaServer::RequestTrade ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_TRADE *pNetMsg )
{
	PGLCHAR pMyChar = GetChar ( dwGaeaID );
	if ( !pMyChar )								return FALSE;

	PGLCHAR pTarChar = GetChar ( pNetMsg->dwTargetID );
	if ( !pTarChar )							return FALSE;

	//	Note : �̹� Ʈ���̵� ���̶�� ���.
	if ( pMyChar->m_sTrade.Valid() )			return FALSE;
	if ( pTarChar->m_sTrade.Valid() )			return FALSE;

	if( pTarChar->m_bDisTrade )					return FALSE;
	/*dmk14 test fix vend close*/
	if( pTarChar->m_sPMarket.IsOpen() )			return FALSE;

	//	Note : ��������� Ʈ���̵� �õ� �³� ����.
	//
	GLMSG::SNET_TRADE_TAR NetMsg;
	NetMsg.dwMasterID = dwGaeaID;
	SENDTOAGENT ( pTarChar->m_dwClientID, &NetMsg );

#if defined(_RELEASED) || defined(TW_PARAM) || defined(TH_PARAM) || defined(HK_PARAM) // ***Tracing Log print
	if( pMyChar->m_bTracingUser )
	{
		NET_LOG_UPDATE_TRACINGCHAR TracingMsg;
		TracingMsg.nUserNum  = pMyChar->GetUserID();
		StringCchCopy( TracingMsg.szAccount, USR_ID_LENGTH+1, pMyChar->m_szUID );

		CString strTemp;
		strTemp.Format( "Trade Request, [%s][%s], Target Char:[%s][%s]", 
				 pMyChar->m_szUID, pMyChar->m_szName, pMyChar->m_szUID, pMyChar->m_szName );
		StringCchCopy( TracingMsg.szLogMsg, TRACING_LOG_SIZE, strTemp.GetString() );

		SENDTOAGENT( pMyChar->m_dwClientID, &TracingMsg );
	}
#endif

	return TRUE;
}

BOOL GLGaeaServer::RequestTradeTarAns ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_TRADE_TAR_ANS *pNetMsg )
{
	PGLCHAR pMyChar = GetChar ( dwGaeaID );
	if ( !pMyChar )								return FALSE;

	PGLCHAR pMasterChar = GetChar ( pNetMsg->dwMasterID );
	if ( !pMasterChar )							return FALSE;

	//	Note : �̹� Ʈ���̵� ���̶�� ���.
	if ( pMyChar->m_sTrade.Valid() )			return FALSE;
	if ( pMasterChar->m_sTrade.Valid() )		return FALSE;

	if ( pNetMsg->emAns == EMTRADE_CANCEL )
	{
		//	Note : Ʈ���̵� �õ� �ź� MSG
		//
		GLMSG::SNET_TRADE_FB NetMsgFB;
		NetMsgFB.dwTargetID = dwGaeaID;
		NetMsgFB.emAns = EMTRADE_CANCEL;

		SENDTOAGENT ( pMasterChar->m_dwClientID, &NetMsgFB );

		return FALSE;
	}

	if( pMasterChar->m_bDisTrade ){
		GLMSG::SNET_TRADE_FB NetMsgFB;
		NetMsgFB.dwTargetID = dwGaeaID;
		NetMsgFB.emAns = EMTRADE_CANCEL;

		SENDTOAGENT ( pMasterChar->m_dwClientID, &NetMsgFB );
		return FALSE;
	}

	//	Note : Ʈ���̵� ����.
	//
	pMyChar->m_sTrade.Reset();
	pMyChar->m_sTrade.SetTarget ( pMasterChar->m_dwGaeaID );

	pMasterChar->m_sTrade.Reset();
	pMasterChar->m_sTrade.SetTarget ( pMyChar->m_dwGaeaID );

	//	Note : Ʈ���̵� ���� MSG
	//
	{
		GLMSG::SNET_TRADE_FB NetMsgFB;
		NetMsgFB.dwTargetID = dwGaeaID;
		NetMsgFB.emAns = EMTRADE_OK;

		SENDTOAGENT ( pMasterChar->m_dwClientID, &NetMsgFB );
	}

	{
		GLMSG::SNET_TRADE_FB NetMsgFB;
		NetMsgFB.dwTargetID = pNetMsg->dwMasterID;
		NetMsgFB.emAns = EMTRADE_OK;

		SENDTOAGENT ( pMyChar->m_dwClientID, &NetMsgFB );
	}

#if defined(_RELEASED) || defined(TW_PARAM) || defined(TH_PARAM) || defined(HK_PARAM)// ***Tracing Log print
	if( pMyChar->m_bTracingUser )
	{
		NET_LOG_UPDATE_TRACINGCHAR TracingMsg;
		TracingMsg.nUserNum  = pMyChar->GetUserID();
		StringCchCopy( TracingMsg.szAccount, USR_ID_LENGTH+1, pMyChar->m_szUID );

		CString strTemp;
		strTemp.Format( "Trade Answer OK, [%s][%s], Master Char: [%s][%s]", 
				 pMyChar->m_szUID, pMyChar->m_szName, pMasterChar->m_szUID, pMasterChar->m_szName );

		StringCchCopy( TracingMsg.szLogMsg, TRACING_LOG_SIZE, strTemp.GetString() );

		SENDTOAGENT( pMyChar->m_dwClientID, &TracingMsg );
	}
#endif

	return TRUE;
}

BOOL GLGaeaServer::RequestTradeMoney ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_TRADE_MONEY *pNetMsg )
{
	PGLCHAR pMyChar = GetChar ( dwGaeaID );
	if ( !pMyChar )									return FALSE;
	if ( !pMyChar->m_sTrade.Valid() )				return FALSE;

	DWORD dwTargetID = pMyChar->m_sTrade.GetTarget();
	PGLCHAR pTarChar = GetChar ( dwTargetID );

	if ( !pTarChar || pTarChar->m_sTrade.GetTarget()!=dwGaeaID )
	{
		//	Note : Ʈ���̵� ���� ����.
		pMyChar->m_sTrade.Reset ();

		//	Note : Ʈ���̵� ��� Ŭ���̾�Ʈ�� �˸�.
		GLMSG::SNET_TRADE_CANCEL_TAR NetMsg;
		SENDTOAGENT ( pMyChar->m_dwClientID, &NetMsg );

		return FALSE;
	}

	if( pTarChar->m_bDisTrade )						return FALSE;

	/*trade lock, Juver, 2018/01/02 */
	if ( pMyChar->m_sTrade.GetLock() )
		return FALSE;

	//	�ݾ׺�ȭ.
	if ( pNetMsg->lnMoney > pMyChar->m_lnMoney || pNetMsg->lnMoney < 0 )	return FALSE;

#if defined ( TW_PARAM ) || defined ( HK_PARAM ) 
	if ( pTarChar->m_wLevel == 1 )
	{
		if( (pTarChar->m_lnMoney + pNetMsg->lnMoney) > 50000 )
		{
			GetConsoleMsg()->Write ( LOG_TEXT_CONSOLE, 
									"Target(Level1) too many money!! TarMoney %I64d TarID %s TarUserNum %d MyUserNum %I64d MyID %s MyUserNum %d TradeMoney %I64d",
									pTarChar->m_lnMoney, pTarChar->m_szName, pTarChar->GetUserID(), 
									pMyChar->m_lnMoney, pMyChar->m_szName, pMyChar->GetUserID(), pNetMsg->lnMoney );
			return FALSE;
		}
	}
#endif


	pMyChar->m_sTrade.SetMoney ( pNetMsg->lnMoney );
	
	//	���� ����.
	pMyChar->m_sTrade.BlockAgree ();
	pTarChar->m_sTrade.BlockAgree ();

	//	Note : �ݾ� ��ȭ �˸�. MSG
	//
	GLMSG::SNET_TRADE_MONEY_TAR NetMsgMoneyTar;
	NetMsgMoneyTar.dwGaeaID = dwGaeaID;
	NetMsgMoneyTar.lnMoney = pNetMsg->lnMoney;

	SENDTOCLIENT ( pMyChar->m_dwClientID, &NetMsgMoneyTar );
	SENDTOCLIENT ( pTarChar->m_dwClientID, &NetMsgMoneyTar );

	return TRUE;
}

BOOL GLGaeaServer::RequestTradeItemResist ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_TRADE_ITEM_REGIST *pNetMsg )
{
	PGLCHAR pMyChar = GetChar ( dwGaeaID );
	if ( !pMyChar )									return FALSE;
	if ( !pMyChar->m_sTrade.Valid() )				return FALSE;

	DWORD dwTargetID = pMyChar->m_sTrade.GetTarget();
	PGLCHAR pTarChar = GetChar ( dwTargetID );

	if ( !pTarChar || pTarChar->m_sTrade.GetTarget()!=dwGaeaID )
	{
		//	Note : Ʈ���̵� ���� ����.
		pMyChar->m_sTrade.Reset ();

		//	Note : Ʈ���̵� ��� Ŭ���̾�Ʈ�� �˸�.
		GLMSG::SNET_TRADE_CANCEL_TAR NetMsg;
		SENDTOAGENT ( pMyChar->m_dwClientID, &NetMsg );

		return FALSE;
	}

	if( pTarChar->m_bDisTrade )						return FALSE;

	/*trade lock, Juver, 2018/01/02 */
	if ( pMyChar->m_sTrade.GetLock() )
		return FALSE;

	SINVENITEM *pInvenItem = pMyChar->m_cInventory.GetItem ( pNetMsg->wInvenX, pNetMsg->wInvenY );
	if ( !pInvenItem )								return FALSE;

	SITEM *pITEM = GLItemMan::GetInstance().GetItem ( pInvenItem->sItemCustom.sNativeID );
	if ( !pITEM )		return false;
	
	const SITEMCUSTOM& sITEMCUSTOM = pMyChar->GET_HOLD_ITEM();

	// ��ī���̸鼭 ���� Ȱ��ȭ �Ǿ� ������ �ŷ��� �� ����.
	if ( pITEM->sBasicOp.emItemType == ITEM_PET_CARD )
	{
		PGLPETFIELD pMyPet = GLGaeaServer::GetInstance().GetPET ( pMyChar->m_dwPetGUID );
		if ( pMyPet && pMyPet->IsValid () && sITEMCUSTOM.dwPetID == pMyPet->m_dwPetID ) return S_FALSE;
	}

	// Ż���̸� �ŷ��Ҽ� ����.
	if ( pITEM->sBasicOp.emItemType == ITEM_VEHICLE && pInvenItem->sItemCustom.dwVehicleID != 0 )
	{
		return S_FALSE;
	}

	//	�ŷ��ɼ�
	/*item wrapper, Juver, 2018/01/12 */
	if ( !pITEM->sBasicOp.IsEXCHANGE() && pITEM->sBasicOp.emItemType != ITEM_WRAPPER_BOX )					return FALSE;
	
	// GMCharEdit �� ���� �������� ������ �Ұ����ϴ�.
	/*item wrapper, Juver, 2018/01/12 */
	if ( pInvenItem->sItemCustom.IsGM_GENITEM() && pITEM->sBasicOp.emItemType != ITEM_WRAPPER_BOX )			return FALSE;

	BOOL bOk = pMyChar->m_sTrade.InsertItem ( pNetMsg->wPosX, pNetMsg->wPosY, pInvenItem );
	if ( !bOk )										return FALSE;

	SINVENITEM* pInven = pMyChar->m_sTrade.GetItem ( pNetMsg->wPosX, pNetMsg->wPosY );
	if ( !pInven )									return FALSE;

	// �� �������� Ȯ��
	if ( pMyChar->m_dwCharID <= 0 )
	{
		GetConsoleMsg()->Write ( LOG_TEXT_CONSOLE, "Invalid Trade CharacterID PetID %d CharID %d dwClientID %d", 
								pInvenItem->sItemCustom.dwPetID, pMyChar->m_dwCharID, pTarChar->m_dwClientID );
		return FALSE;
	}


	// ��ī���̸� ��ī�� ������ ���濡�� �˷��ش�.
	if ( pITEM->sBasicOp.emItemType == ITEM_PET_CARD && pInvenItem->sItemCustom.dwPetID != 0 )
	{
		PGLPET pPetInfo = new GLPET ();

		// DB�۾� ���н� Ȥ�� ������ �޸� ���������
		CGetPet* pDbAction = new CGetPet ( pPetInfo, pInvenItem->sItemCustom.dwPetID, pTarChar->m_dwClientID, pMyChar->m_dwCharID, 
										   pNetMsg->wInvenX, pNetMsg->wInvenY, false, false, true );
		m_pDBMan->AddJob ( pDbAction );		
	}

	//	���� ����.
	pMyChar->m_sTrade.BlockAgree ();
	pTarChar->m_sTrade.BlockAgree ();

	//	Note : ������ ��ȭ �˸�. MSG.
	//
	GLMSG::SNET_TRADE_ITEM_REGIST_TAR NetRegistTar;
	NetRegistTar.dwGaeaID = dwGaeaID;
	NetRegistTar.sInsert = *pInven;

	SENDTOAGENT ( pMyChar->m_dwClientID, &NetRegistTar );
	SENDTOAGENT ( pTarChar->m_dwClientID, &NetRegistTar );

	return TRUE;
}

BOOL GLGaeaServer::RequestTradeItemReMove ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_TRADE_ITEM_REMOVE *pNetMsg )
{
	PGLCHAR pMyChar = GetChar ( dwGaeaID );
	if ( !pMyChar )									return FALSE;
	if ( !pMyChar->m_sTrade.Valid() )				return FALSE;

	DWORD dwTargetID = pMyChar->m_sTrade.GetTarget();
	PGLCHAR pTarChar = GetChar ( dwTargetID );

	if ( !pTarChar || pTarChar->m_sTrade.GetTarget()!=dwGaeaID )
	{
		//	Note : Ʈ���̵� ���� ����.
		pMyChar->m_sTrade.Reset ();

		//	Note : Ʈ���̵� ��� Ŭ���̾�Ʈ�� �˸�.
		GLMSG::SNET_TRADE_CANCEL_TAR NetMsg;
		SENDTOAGENT ( pMyChar->m_dwClientID, &NetMsg );

		return FALSE;
	}

	if( pTarChar->m_bDisTrade )						return FALSE;
	/*trade lock, Juver, 2018/01/02 */
	if ( pMyChar->m_sTrade.GetLock() )
		return FALSE;

	SINVENITEM* pInven = pMyChar->m_sTrade.GetItem ( pNetMsg->wPosX, pNetMsg->wPosY );
	if ( !pInven )									return FALSE;

	//	Note : �ŷ� ��Ͽ��� ������ ����.
	pMyChar->m_sTrade.DeleteItem ( pNetMsg->wPosX, pNetMsg->wPosY );

	//	���� ����.
	pMyChar->m_sTrade.BlockAgree ();
	pTarChar->m_sTrade.BlockAgree ();

	//	Note : ������ ��ȭ �˸�. MSG.
	//
	GLMSG::SNET_TRADE_ITEM_REMOVE_TAR NetReMoveTar;
	NetReMoveTar.dwGaeaID = dwGaeaID;
	NetReMoveTar.wPosX = pNetMsg->wPosX;
	NetReMoveTar.wPosY = pNetMsg->wPosY;

	SENDTOAGENT ( pMyChar->m_dwClientID, &NetReMoveTar );
	SENDTOAGENT ( pTarChar->m_dwClientID, &NetReMoveTar );

	return TRUE;
}

BOOL GLGaeaServer::RequestTradeAgree ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_TRADE_AGREE *pNetMsg )
{
	PGLCHAR pMyChar = GetChar ( dwGaeaID );
	if ( !pMyChar )									return FALSE;
	if ( !pMyChar->m_sTrade.Valid() )				return FALSE;

	DWORD dwTargetID = pMyChar->m_sTrade.GetTarget();
	PGLCHAR pTarChar = GetChar ( dwTargetID );

	if ( !pTarChar || pTarChar->m_sTrade.GetTarget()!=dwGaeaID )
	{
		//	Note : Ʈ���̵� ���� ����.
		pMyChar->m_sTrade.Reset ();

		//	Note : Ʈ���̵� ��� Ŭ���̾�Ʈ�� �˸�.
		GLMSG::SNET_TRADE_CANCEL_TAR NetMsg;
		SENDTOAGENT ( pMyChar->m_dwClientID, &NetMsg );

		return FALSE;
	}

	if( pTarChar->m_bDisTrade )						return FALSE;

	//	���� �õ�.
	BOOL bOk = pMyChar->m_sTrade.SetAgree ();

	if ( !bOk )
	{
		//	���� ���� ��� MSG.
		GLMSG::SNET_TRADE_AGREE_TAR NetMsgAgreeTar;
		NetMsgAgreeTar.dwGaeaID = dwGaeaID;
		NetMsgAgreeTar.emFB = EMTRADE_AGREE_TIME;

		SENDTOAGENT ( pMyChar->m_dwClientID, &NetMsgAgreeTar );

		return FALSE;
	}

	if ( pMyChar->m_sTrade.GetAgree() && pTarChar->m_sTrade.GetAgree() )
	{
		
		EMTRADE_CANCEL_TAR emCancelFB = EMTRADE_CANCEL_NORMAL;

		do
		{
			//	Note : �ŷ� ����� ��ȿ���� �˻�.
			//
			if ( !pMyChar->IsVaildTradeData() || !pTarChar->IsVaildTradeData() )
			{
				break;
			}

			//	Note : ��ǰ ��ȯ ������ �ι꿡 ������� �˻�.
			//
			BOOL bOk;
			bOk = pMyChar->IsVaildTradeInvenSpace ( pTarChar->m_sTrade.GetTradeBox() );
			if ( !bOk )
			{
				emCancelFB = EMTRADE_CANCEL_MYINVEN;
				break;
			}

			bOk = pTarChar->IsVaildTradeInvenSpace ( pMyChar->m_sTrade.GetTradeBox() );
			if ( !bOk )
			{
				emCancelFB = EMTRADE_CANCEL_TARINVEN;
				break;
			}

			//	Note : �ŷ� ����.
			//
			bOk = pMyChar->DoTrade ( pTarChar->m_sTrade, pTarChar->m_dwCharID );
			if ( !bOk ) break; // �� �ݾ��� ���̳ʽ��� �Ǵ� ��� (�븸 ��ŷ)
			//GASSERT ( bOk && "DoTrade()������ FALSE�� �Ͼ���� �ȵȴ�." );

			bOk = pTarChar->DoTrade ( pMyChar->m_sTrade, pMyChar->m_dwCharID );
			if ( !bOk ) break; // �� �ݾ��� ���̳ʽ��� �Ǵ� ��� (�븸 ��ŷ)
			//GASSERT ( bOk && "DoTrade()������ FALSE�� �Ͼ���� �ȵȴ�." );

			//	Note : �ŷ� ���� ����.
			//
			pMyChar->m_sTrade.Reset();
			pTarChar->m_sTrade.Reset();

			//	Note : �ŷ� ����.
			//
			GLMSG::SNET_TRADE_COMPLETE_TAR NetMsgComplete;
			SENDTOAGENT ( pMyChar->m_dwClientID, &NetMsgComplete );
			SENDTOAGENT ( pTarChar->m_dwClientID, &NetMsgComplete );

			return TRUE;
		}
		while(0);

		//	Note : �ŷ� ���� ����.
		//
		pMyChar->m_sTrade.Reset();
		pTarChar->m_sTrade.Reset();

		//	Note : �ŷ� ��ҵ��� �˸�. ( dwGaeaID ����. )
		//
		{
			GLMSG::SNET_TRADE_CANCEL_TAR NetMsgCancelTar;
			NetMsgCancelTar.emCancel = emCancelFB;
			SENDTOAGENT ( pMyChar->m_dwClientID, &NetMsgCancelTar );
		}

		//	Note : �ŷ� ��ҵ��� �˸�. ( dwTargetID ����. )
		//
		{
			if ( emCancelFB==EMTRADE_CANCEL_MYINVEN || emCancelFB==EMTRADE_CANCEL_TARINVEN )
			{
				emCancelFB = ( emCancelFB==EMTRADE_CANCEL_MYINVEN ? EMTRADE_CANCEL_TARINVEN : EMTRADE_CANCEL_MYINVEN );
			}

			GLMSG::SNET_TRADE_CANCEL_TAR NetMsgCancelTar;
			NetMsgCancelTar.emCancel = emCancelFB;
			SENDTOAGENT ( pTarChar->m_dwClientID, &NetMsgCancelTar );
		}
	}
	else
	{
		//	���� ���� ��� MSG.
		GLMSG::SNET_TRADE_AGREE_TAR NetMsgAgreeTar;
		NetMsgAgreeTar.dwGaeaID = dwGaeaID;
		NetMsgAgreeTar.emFB = EMTRADE_AGREE_OK;

		SENDTOAGENT ( pMyChar->m_dwClientID, &NetMsgAgreeTar );
		SENDTOAGENT ( pTarChar->m_dwClientID, &NetMsgAgreeTar );
	}

	return TRUE;
}

BOOL GLGaeaServer::RequestTradeCancel ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_TRADE_CANCEL *pNetMsg )
{
	PGLCHAR pMyChar = GetChar ( dwGaeaID );
	if ( !pMyChar )									return FALSE;
	if ( !pMyChar->m_sTrade.Valid() )				return FALSE;

	DWORD dwTargetID = pMyChar->m_sTrade.GetTarget();
	PGLCHAR pTarChar = GetChar ( dwTargetID );

	if ( !pTarChar || pTarChar->m_sTrade.GetTarget()!=dwGaeaID )
	{
		//	Note : Ʈ���̵� ���� ����.
		pMyChar->m_sTrade.Reset ();

		//	Note : Ʈ���̵� ��� Ŭ���̾�Ʈ�� �˸�.
		GLMSG::SNET_TRADE_CANCEL_TAR NetMsg;
		SENDTOAGENT ( pMyChar->m_dwClientID, &NetMsg );

		return FALSE;
	}

	//	�ŷ� ���.
	pMyChar->m_sTrade.Reset();
	pTarChar->m_sTrade.Reset();

	{
		GLMSG::SNET_TRADE_CANCEL_TAR NetMsg;
		NetMsg.emCancel = EMTRADE_CANCEL_NORMAL;
		SENDTOAGENT ( pMyChar->m_dwClientID, &NetMsg );
	}

	{
		GLMSG::SNET_TRADE_CANCEL_TAR NetMsg;
		NetMsg.emCancel = EMTRADE_CANCEL_OTHER;
		SENDTOAGENT ( pTarChar->m_dwClientID, &NetMsg );
	}

	return TRUE;
}

BOOL GLGaeaServer::RequestFieldSvrCharChk ( DWORD dwClientID, GLMSG::SNETPC_FIELDSVR_CHARCHK *pNetMsg  )
{
	bool bExist(false);

	//	Note : ���� �̸��� ĳ���Ͱ� �̹� ���ӵǾ� �ִ��� �˻��մϴ�.
	{
		GLCHAR_MAP_ITER name_iter = m_PCNameMap.find(pNetMsg->szName);
		if ( name_iter != m_PCNameMap.end() )
		{
			PGLCHAR pChar = (*name_iter).second;

			bExist = true;

			//	���� �����ڿ��� DropOut �޽����� �����ϴ�.
			GLMSG::SNET_DROP_OUT_FORCED NetMsgForced;
			NetMsgForced.emForced = EMDROPOUT_ERROR;
			SENDTOAGENT ( pChar->m_dwClientID, &NetMsgForced );

			//	Note : ���� ��ġ ����.
			//
			pChar->SavePosition ();

			//	Note : ����� ���·� ���Ž� ����ġ ����.
			//
			if ( pChar->IsSTATE(EM_ACT_DIE) )
			{
				//	Note : ��Ȱ�� ����ġ ����.
				pChar->ReBirthDecExp ();
			}

			if ( m_pDBMan )
			{
				//	Note : �ɸ��� ���� ��û.
				SetSaveDBUserID(pChar->GetUserID());

				//	Note : Ŭ�� â�� ����.
				if ( pChar->m_dwGuild!=CLUB_NULL )
				{
					GLCLUB *pCLUB = GLGaeaServer::GetInstance().GetClubMan().GetClub ( pChar->m_dwGuild );
					if ( pCLUB && pChar->m_dwCharID==pCLUB->m_dwMasterID )
					{
						pCLUB->SAVESTORAGE2DB ();
						pCLUB->RESET_STORAGE ();
					}
				}

				//	Note : �ɸ��� ���� ��û.
				CDbActSaveChar *pSaveDB = new CDbActSaveChar;
				pSaveDB->SetInfo ( pChar->m_dwClientID, pChar->m_dwGaeaID, pChar );
				m_pDBMan->AddJob ( pSaveDB );
			}

			//	Note : �ɸ��Ͱ� ��������.
			//
			DropOutPC(pChar->m_dwGaeaID);
		}
	}

	//	Note : ���� Gaea id �� ���������� �˻�.
	//
	if ( GetChar(pNetMsg->dwGaeaID) )
	{
		PGLCHAR pChar = GetChar(pNetMsg->dwGaeaID);

		if ( pChar )
		{
			bExist = true;

			//	���� �����ڿ��� DropOut �޽����� �����ϴ�.
			GLMSG::SNET_DROP_OUT_FORCED NetMsgForced;
			NetMsgForced.emForced = EMDROPOUT_ERROR;
			SENDTOAGENT ( pChar->m_dwClientID, &NetMsgForced );

			//	Note : ���� ��ġ ����.
			//
			pChar->SavePosition ();

			//	Note : ����� ���·� ���Ž� ����ġ ����.
			//
			if ( pChar->IsSTATE(EM_ACT_DIE) )
			{
				//	Note : ��Ȱ�� ����ġ ����.
				pChar->ReBirthDecExp ();
			}

			if ( m_pDBMan )
			{
				//	Note : �ɸ��� ���� ��û.
				//
				SetSaveDBUserID(pChar->GetUserID());

				//	Note : Ŭ�� â�� ����.
				if ( pChar->m_dwGuild!=CLUB_NULL )
				{
					GLCLUB *pCLUB = GLGaeaServer::GetInstance().GetClubMan().GetClub ( pChar->m_dwGuild );
					if ( pCLUB && pChar->m_dwCharID==pCLUB->m_dwMasterID )
					{
						pCLUB->SAVESTORAGE2DB ();
						pCLUB->RESET_STORAGE ();
					}
				}

				//	Note : �ɸ��� ���� ����.
				CDbActSaveChar *pSaveDB = new CDbActSaveChar;
				pSaveDB->SetInfo ( pChar->m_dwClientID, pChar->m_dwGaeaID, pChar );
				m_pDBMan->AddJob ( pSaveDB );
			}

			//	Note : �ɸ��Ͱ� ��������.
			//
			DropOutPC(pChar->m_dwGaeaID);
		}
	}

	//	Note : Ȥ�� user id �� db �۾������� �˻�.
	if ( FindSaveDBUserID ( pNetMsg->dwUserID ) )
	{
		bExist = true;
	}

	//	Note : ������Ʈ�� ���� FB.
	//
	GLMSG::SNETPC_FIELDSVR_CHARCHK_FB NetMsgFb;
	NetMsgFb.bExist = bExist;
	NetMsgFb.nChannel = m_nServerChannel;
	NetMsgFb.dwFIELDID = m_dwFieldSvrID;
	NetMsgFb.dwGaeaID = pNetMsg->dwGaeaID;
	StringCchCopy ( NetMsgFb.szName, CHR_ID_LENGTH+1, pNetMsg->szName );

	if ( bExist )
	{
		if ( m_pDBMan )
		{
			//	Note : DB �����Ŀ� �޽��� ó���ϱ� ���ؼ� db action�� ����Ͽ� �޽��� ����.
			CDbActToAgentMsg *pToAgent = new CDbActToAgentMsg;
			pToAgent->SetMsg ( dwClientID, (NET_MSG_GENERIC*)&NetMsgFb );
			m_pDBMan->AddJob ( pToAgent );
		}
	}
	else
	{
		//	Note : DB �۾��� ���� ��쿡�� ��� ����.
		SENDTOAGENT ( dwClientID, &NetMsgFb );
	}

	return TRUE;
}

BOOL GLGaeaServer::ServerFieldInfoReset ( GLMSG::SNET_FIELDINFO_RESET* pNetMsg )
{
	m_cPartyFieldMan.DelPartyAll();

	return TRUE;
}

BOOL GLGaeaServer::ServerSchoolFreePk ( GLMSG::SNETPC_SCHOOLFREEPK_FLD *pNetMsg )
{
	if( GLCONST_CHAR::bPKLESS ) return FALSE;

	GLSchoolFreePK::GetInstance().SET ( pNetMsg->bSCHOOL_FREEPK );
	
	GLMSG::SNETPC_SERVER_INFO NetMsgServerInfo;
	NetMsgServerInfo.bSCHOOL_FREEPK = GLSchoolFreePK::GetInstance().IsON();

	PGLCHAR pCHAR = NULL;
	for ( DWORD i=0; i<m_dwMaxClient; ++i )
	{
		pCHAR = GetChar(i);
		if ( !pCHAR )	continue;

		SENDTOCLIENT ( pCHAR->m_dwClientID, &NetMsgServerInfo );
	}

	return TRUE;
}

BOOL GLGaeaServer::ServerCtrlPeriod ( GLMSG::SNET_PERIOD *pNetMsg )
{
	GLPeriod::GetInstance().SetPeriod ( pNetMsg->sPRERIODTIME );

	return TRUE;
}

BOOL GLGaeaServer::ServerCtrlWeather ( GLMSG::SNETSERVER_CTRL_WEATHER *pNetMsg )
{
	GLPeriod::GetInstance().SetActiveWeather ( pNetMsg->bActive );
	GLPeriod::GetInstance().SetWeather ( pNetMsg->dwWeather );

	return TRUE;
}

BOOL GLGaeaServer::ServerCtrlWeather2 ( GLMSG::SNETSERVER_CTRL_WEATHER2 *pNetMsg )
{
	SONEMAPWEATHER oneMapWeather;
	oneMapWeather.dwWeather   = pNetMsg->dwWeather;
	oneMapWeather.dwApplyTime = pNetMsg->dwApplyTime;
	oneMapWeather.map_mID     = pNetMsg->map_mID;
	oneMapWeather.map_sID     = pNetMsg->map_sID;

	if( pNetMsg->dwWeather == NULL )
	{
		GLPeriod::GetInstance().SetActiveWeather ( FALSE );
	}else{
		GLPeriod::GetInstance().SetActiveWeather ( TRUE );
	}
	GLPeriod::GetInstance().SetOneMapActiveWeather( oneMapWeather );

	return TRUE;
}

BOOL GLGaeaServer::ServerCtrlTime ( GLMSG::SNETSERVER_CTRL_TIME *pNetMsg )
{
	GLPeriod::GetInstance().SetHour ( pNetMsg->dwTime );
	
	for ( DWORD i=0; i<m_dwMaxClient; ++i )
	{
		PGLCHAR pChar = GetChar(i);
		if ( !pChar )	continue;

		pChar->MsgSendPeriod ();
	}

	return TRUE;
}

BOOL GLGaeaServer::ServerCtrlMonth ( GLMSG::SNETSERVER_CTRL_MONTH *pNetMsg )
{

	return TRUE;
}

BOOL GLGaeaServer::ServerCtrlGenItemHold ( GLMSG::SNETSERVER_CTRL_GENITEMHOLD *pNetMsg )
{
	m_bGenItemHold = pNetMsg->bHold;

	return TRUE;
}

BOOL GLGaeaServer::ServerCtrlPlayerKillingMode ( GLMSG::SNET_SERVER_PLAYERKILLING_MODE *pNetMsg )
{
	GLCONST_CHAR::bPK_MODE = pNetMsg->bMODE;

	return TRUE;
}

BOOL GLGaeaServer::VietnamAllInitTime ( GLMSG::SNETPC_VIETNAM_ALLINITTIME *pNetMsg )
{

	GLMSG::SNETPC_VIETNAM_ALLINITTIME netMsg;

	GetConsoleMsg()->Write ( LOG_TEXT_CONSOLE, "-=-=-=-= Init Vietnam Time =-=-=-=-" );

	PGLCHAR pCHAR = NULL;
	CLIENTMAP_ITER iter = m_mapCHARID.begin();

	for( ; iter != m_mapCHARID.end(); ++iter )
	{
		pCHAR = GetChar((*iter).second);
		if ( !pCHAR )	continue;

		pCHAR->ReceiveVietnam( pNetMsg->initTime );
	}
	return TRUE;
}

BOOL GLGaeaServer::MsgVietTimeReset( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNETPC_VIETNAM_TIME_REQ_FB *pNetMsg ) 
{
#if defined(VN_PARAM) //vietnamtest%%% && vietnamtest2
	
	PGLCHAR pChar = GetChar(dwGaeaID);
	if( !pChar )	return FALSE;

	pChar->m_dwVietnamGainType = GAINTYPE_MAX;
	pChar->m_sVietnamSystem.Init();
	pChar->m_sVietnamSystem.loginTime = pNetMsg->loginTime;
	pChar->m_sVietnamSystem.gameTime  = 0;

	GLMSG::SNETPC_VIETNAM_ALLINITTIME NetMsg;
	NetMsg.initTime = pNetMsg->loginTime;
	SENDTOCLIENT(dwClientID,&NetMsg);

#endif

	return TRUE;
}

BOOL GLGaeaServer::ServerCtrlCharDropOutForced ( GLMSG::SNET_DROP_OUT_FORCED *pNetMsg )
{
	PGLCHAR pPC = GetCharID ( pNetMsg->dwCharID );
	if ( !pPC )							return FALSE;
	
	ReserveDropOutPC ( pPC->m_dwGaeaID );

	return TRUE;
}

BOOL GLGaeaServer::CyberCafeClassUpdate( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_CYBERCAFECLASS_UPDATE *pNetMsg )
{
	PGLCHAR pCHAR = GetChar ( dwGaeaID );
	if( !pCHAR ) return FALSE;
	pCHAR->m_dwThaiCCafeClass = pNetMsg->dwCyberCafeClass;
	pCHAR->m_nMyCCafeClass	  = pNetMsg->dwCyberCafeClass;

	return TRUE;
}

BOOL GLGaeaServer::GMCtrolMove2Gate ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_MOVE2GATE_FLD *pNetMsg )
{
	PGLCHAR pPC = GetChar ( pNetMsg->dwGaeaID );
	if ( !pPC )							return FALSE;

	if ( m_dwAgentSlot != dwClientID )	return FALSE;
	

	GLLandMan* pInLandMan = NULL;
	DxLandGateMan* pInGateMan = NULL;
	PDXLANDGATE pInGate = NULL;
	D3DXVECTOR3 vStartPos(0,0,0);

	DxLandGateMan* pGateMan = NULL;
	PDXLANDGATE pGate = NULL;
	
	pGateMan = &pPC->m_pLandMan->GetLandGateMan();
	pGate = pGateMan->FindLandGate ( pNetMsg->dwGATE );
	if ( !pGate )						return FALSE;

	vStartPos = pGate->GetGenPos ( DxLandGate::GEN_RENDUM );

	//	Note : ���ο� �� ��ġ�� ������̼� �ʱ�ȭ.
	//
	pPC->SetPosition ( vStartPos );

	//	Note : ����� ��� ��ȯ.
	//
	GLMSG::SNET_GM_MOVE2GATE_FB NetMsgFB;
	NetMsgFB.vPOS = pPC->m_vPos;
	SENDTOCLIENT ( pPC->m_dwClientID, &NetMsgFB );

	GLMSG::SNETPC_JUMP_POS_BRD NetMsgBrd;
	NetMsgBrd.dwGaeaID = pPC->m_dwGaeaID;
	NetMsgBrd.vPOS = pPC->m_vPos;

	pPC->SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );

	return TRUE;
}

BOOL GLGaeaServer::GMCtrolMove2MapPos ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_MOVE2MAPPOS_FLD *pNetMsg )
{
	PGLCHAR pChar = GetChar ( pNetMsg->dwGaeaID );
	if ( !pChar )							return FALSE;

	if ( m_dwAgentSlot != dwClientID )	return FALSE;

	SNATIVEID nidMAP = pNetMsg->nidMAP;

	if ( nidMAP==SNATIVEID(false) )		return FALSE;

	GLMapList::FIELDMAP_ITER iter = m_MapList.find ( nidMAP.dwID );
	if ( iter==m_MapList.end() )		return FALSE;

	const SMAPNODE *pMapNode = &(*iter).second;

	GLMapAxisInfo sMapAxisInfo;
	sMapAxisInfo.LoadFile ( pMapNode->strFile.c_str() );

	D3DXVECTOR3 vPOS(0,0,0); 
	sMapAxisInfo.MapPos2MiniPos ( pNetMsg->dwPOSX, pNetMsg->dwPOSY, vPOS.x, vPOS.z );

	//	Note : �ٸ� �ʵ� ������ ���.
	if ( pMapNode->dwFieldSID!=m_dwFieldSvrID )
	{
		GLMSG::SNETPC_REQ_RECALL_AG NetMsgAg;
		NetMsgAg.sMAPID = nidMAP;
		NetMsgAg.dwGATEID = UINT_MAX;
		NetMsgAg.vPOS = vPOS;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pChar->m_dwClientID, &NetMsgAg );
	}
	//	Note : ���� �ʵ� ������ ���.
	else
	{
		GLMSG::SNETPC_REQ_RECALL_FB NetMsgFB;

		SNATIVEID sCurMapID = pChar->m_sMapID;

		BOOL bOK = RequestInvenRecallThisSvr ( pChar, nidMAP, UINT_MAX, vPOS );
		if ( !bOK )
		{
			NetMsgFB.emFB = EMREQ_RECALL_FB_FAIL;
			SENDTOCLIENT ( pChar->m_dwClientID, &NetMsgFB );
			return FALSE;
		}

		// ������ �����̻� ����
		if ( sCurMapID != nidMAP )
		{
			for ( int i=0; i<EMBLOW_MULTI; ++i )		pChar->DISABLEBLOW ( i );
			for ( int i=0; i<SKILLFACT_SIZE; ++i )		pChar->DISABLESKEFF ( i );
			pChar->DISABLEALLLANDEFF();
		}

		pChar->ResetAction();

		//	Note : �� �̵� ������ �˸�.
		//
		NetMsgFB.emFB = EMREQ_RECALL_FB_OK;
		NetMsgFB.sMAPID = nidMAP;
		NetMsgFB.vPOS = pChar->m_vPos;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pChar->m_dwClientID, &NetMsgFB );
	}

	return TRUE;
}

BOOL GLGaeaServer::GMCtrlWhereNpc ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_WHERE_NPC_FLD *pNetMsg )
{
	if ( !m_bEmulator )
	{
		if ( dwClientID != m_dwAgentSlot ) return FALSE;
	
	}

	PGLCHAR pCHAR = GetChar ( pNetMsg->dwGaeaID );
	if ( !pCHAR )							return FALSE;

	if ( pCHAR->m_pLandMan )
	{
		GLLandMan *pLandMan = pCHAR->m_pLandMan;
		
		GLMSG::SNET_GM_WHERE_NPC_FB NetMsgFB;
		PGLCROW pCrow = NULL;
		int nPosX(INT_MAX), nPosY(INT_MAX);
		for ( DWORD i=0; i<MAXCROW; ++i )
		{
			pCrow = pLandMan->GetCrow(i);
			if ( pCrow && pCrow->m_pCrowData->sNativeID==pNetMsg->nidNPC )
			{				
				pLandMan->GetMapAxisInfo().Convert2MapPos ( pCrow->GetPosition().x, pCrow->GetPosition().z, nPosX, nPosY );
				
				NetMsgFB.nPosX = nPosX;
				NetMsgFB.nPosY = nPosY;
				GLGaeaServer::GetInstance().SENDTOCLIENT ( pCHAR->m_dwClientID, &NetMsgFB );
			}
		}
	}
	
	return TRUE;
}

BOOL GLGaeaServer::GMCtrolWherePcPos ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_WHERE_PC_POS *pNetMsg )
{
	PGLCHAR pCHAR = GetChar ( dwGaeaID );
	if ( !pCHAR )							return FALSE;
	if ( pCHAR->m_dwUserLvl < USER_GM3 )	return FALSE;

	bool bFOUD(false);
	int nPosX(INT_MAX), nPosY(INT_MAX);

	pCHAR = GetCharID(pNetMsg->dwCHARID);
	if ( pCHAR && pCHAR->m_pLandMan )
	{
		bFOUD = true;
		pCHAR->m_pLandMan->GetMapAxisInfo().Convert2MapPos ( pCHAR->m_vPos.x, pCHAR->m_vPos.z, nPosX, nPosY );
	}

	GLMSG::SNET_GM_WHERE_PC_POS_FB NetMsgFB;
	NetMsgFB.bFOUND = bFOUD;
	NetMsgFB.nPosX = nPosX;
	NetMsgFB.nPosY = nPosY;
	GLGaeaServer::GetInstance().SENDTOCLIENT ( dwClientID, &NetMsgFB );

	return TRUE;
}

BOOL GLGaeaServer::GMCtrlWarningMSG ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_WARNING_MSG_FLD* pNetMsg )
{
	GLMSG::SNET_GM_WARNING_MSG_BRD NetMsgBrd;
	NetMsgBrd.bOn = pNetMsg->bOn;

	if ( dwClientID != m_dwAgentSlot )		return FALSE;

	PGLCHAR pCHAR = GetChar ( pNetMsg->dwGaeaID );
	if ( !pCHAR )							return FALSE;

	LANDQUADTREE* pLandTree = pCHAR->m_pLandMan->GetLandTree();
	if ( !pLandTree )						return FALSE;

	// ���� ũ�⸦ ���´�.
	D3DXVECTOR3 vMax, vMin;
	pCHAR->m_pLandMan->GetNaviMeshAABB ( vMax, vMin );

	LANDQUADNODE* pQuadNode = NULL;
	BOUDRECT bRect( (int)vMax.x,(int)vMax.z,(int)vMin.x,(int)vMin.z );
	pLandTree->FindNodes ( bRect, pLandTree->GetRootNode(), &pQuadNode );

	GLLandNode* pLandNode = NULL;
	GLCHARNODE *pCharCur = NULL;
	PGLCHAR pGLChar = NULL;
	for ( ; pQuadNode; pQuadNode = pQuadNode->pFindNext )
	{
		pLandNode = pQuadNode->pData;

		GASSERT(pLandTree->GetCellNum()>pQuadNode->dwCID&&"Index over cell size");

		pCharCur = pLandNode->m_PCList.m_pHead;
		for ( ; pCharCur; pCharCur = pCharCur->pNext )
		{
			pGLChar = pCharCur->Data;
			if( !pGLChar ) continue;

			SENDTOCLIENT ( pGLChar->m_dwClientID, &NetMsgBrd );
		}
	}

	return TRUE;
}

BOOL GLGaeaServer::GMCtrolBigHead ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_BIGHEAD* pNetMsg )
{
	if ( dwClientID != m_dwAgentSlot ) return FALSE;

	m_bBigHead = pNetMsg->bBIGHEAD;

	return TRUE;
}

BOOL GLGaeaServer::GMCtrolBigHand ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_BIGHAND *pNetMsg )
{
	if ( dwClientID != m_dwAgentSlot ) return FALSE;

	m_bBigHand = pNetMsg->bBIGHAND;

	return TRUE;
}

BOOL GLGaeaServer::GMCtrolFreePK ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_FREEPK *pNetMsg )
{
	if ( dwClientID != m_dwAgentSlot ) return FALSE;

	if ( pNetMsg->dwPKTIME != 0 )
        m_bBrightEvent = true;
	else 
		m_bBrightEvent = false;

	return TRUE;
}

/*private market set, Juver, 2018/01/02 */
BOOL GLGaeaServer::GMCtrolSetPrivateMarket ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_SET_PRIVATE_MARKET *pNetMsg )
{
	if ( dwClientID != m_dwAgentSlot ) return FALSE;

	m_bAllowPrivateMarket = pNetMsg->bEnable;

	GetConsoleMsg()->Write ( LOG_TEXT_CONSOLE, "Set Private Market :%s", m_bAllowPrivateMarket? "Enable":"Disable" );

	return TRUE;
}

/*megaphone set, Juver, 2018/01/02 */
BOOL GLGaeaServer::GMCtrolSetMegaPhone ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_SET_MEGAPHONE *pNetMsg )
{
	if ( dwClientID != m_dwAgentSlot ) return FALSE;

	m_bAllowMegaPhone = pNetMsg->bEnable;

	GetConsoleMsg()->Write ( LOG_TEXT_CONSOLE, "Set Megaphone :%s", m_bAllowMegaPhone? "Enable":"Disable" );

	return TRUE;
}

BOOL GLGaeaServer::GMCtrolEventEx ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_EVENT_EX *pNetMsg )
{
	if( dwClientID != m_dwAgentSlot )			return FALSE;
	if( !pNetMsg )								return FALSE;
	if( pNetMsg->emType == EMGM_EVENT_NONE )	return FALSE;

	m_sEVENTFACT.SetEVENT( pNetMsg->emType, pNetMsg->wValue );

	PGLCHAR pCHAR = NULL;
	CLIENTMAP_ITER iter = m_mapCHARID.begin();

	for( ; iter != m_mapCHARID.end(); ++iter )
	{
		pCHAR = GetChar((*iter).second);
		if ( !pCHAR )	continue;

		pCHAR->ReceiveEventEx( pNetMsg->emType, pNetMsg->wValue );
	}

	return TRUE;
}

BOOL GLGaeaServer::GMCtrolEventExEnd ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_EVENT_EX_END *pNetMsg )
{
	if( dwClientID != m_dwAgentSlot )			return FALSE;
	if( !pNetMsg )								return FALSE;
	if( pNetMsg->emType == EMGM_EVENT_NONE )	return FALSE;

	m_sEVENTFACT.ResetEVENT( pNetMsg->emType );

	PGLCHAR pCHAR = NULL;
	CLIENTMAP_ITER iter = m_mapCHARID.begin();

	for( ; iter != m_mapCHARID.end(); ++iter )
	{
		pCHAR = GetChar((*iter).second);
		if ( !pCHAR )	continue;

		pCHAR->ReceiveEventEx( pNetMsg->emType, 0 );
	}

	return TRUE;
}

BOOL GLGaeaServer::GMCtrolLimitEventBegin ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_LIMIT_EVENT_BEGIN *pNetMsg )
{
	if( dwClientID != m_dwAgentSlot )			return FALSE;
	if( !pNetMsg )								return FALSE;

	SEventState sEventState;

	sEventState.bEventStart = TRUE;
	sEventState.fItemGainRate = pNetMsg->itemGain_Rate;		
	sEventState.fExpGainRate  = pNetMsg->expGain_Rate;			
	sEventState.MinEventLevel = pNetMsg->start_Lv;		
	sEventState.MaxEventLevel = pNetMsg->end_Lv;	


	sEventState.EventPlayTime   = pNetMsg->play_Time * 60;
	sEventState.EventBusterTime = pNetMsg->buster_Time * 60; 		

	m_sEventState = sEventState;

	PGLCHAR pCHAR = NULL;
	CLIENTMAP_ITER iter = m_mapCHARID.begin();

	for( ; iter != m_mapCHARID.end(); ++iter )
	{
		pCHAR = GetChar((*iter).second);
		if ( !pCHAR )	continue;

		pCHAR->ReceiveBeginEvent( sEventState );
	}


	return TRUE;
}

BOOL GLGaeaServer::GMCtrolLimitEventTimeReset ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_LIMIT_EVENT_TIME_RESET *pNetMsg )
{
	if( dwClientID != m_dwAgentSlot )			return FALSE;
	if( !pNetMsg )								return FALSE;

	PGLCHAR pCHAR = NULL;
	pCHAR = GetChar( pNetMsg->dwGaeaID );
	pCHAR->m_sEventTime.loginTime = pNetMsg->loginTime;

	return TRUE;
}

BOOL GLGaeaServer::GMCtrolLimitEventEnd ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_LIMIT_EVENT_END *pNetMsg )
{
	if( dwClientID != m_dwAgentSlot )			return FALSE;
	if( !pNetMsg )								return FALSE;

	SEventState sEventState;
	sEventState.bEventStart = FALSE;
	m_sEventState = sEventState;


	PGLCHAR pCHAR = NULL;
	CLIENTMAP_ITER iter = m_mapCHARID.begin();

	GLMSG::SNET_GM_LIMIT_EVENT_END_FB NetMsgEventEndFb;
	for( ; iter != m_mapCHARID.end(); ++iter )
	{
		pCHAR = GetChar((*iter).second);
		if ( !pCHAR )	continue;

		SENDTOCLIENT ( pCHAR->m_dwClientID, &NetMsgEventEndFb );
			
	}

	return TRUE;
}


BOOL GLGaeaServer::GMKicUser( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_KICK_USER_PROC_FLD* pNetMsgFld )
{
	if ( dwClientID != m_dwAgentSlot ) return FALSE;

	PGLCHAR pTARCHAR = GetCharID ( pNetMsgFld->dwID );
	if ( !pTARCHAR ) return FALSE;

	PGLCHAR pMyChar = GetChar( dwGaeaID );
	if( !pMyChar ) return FALSE;

	CDebugSet::ToFileWithTime( "_gm_cmd.txt", "[%d]%s Kickusercmd [%d]%s", pMyChar->m_dwCharID, pMyChar->m_szName, pTARCHAR->m_dwCharID, pTARCHAR->m_szName );

	ReserveDropOutPC ( pTARCHAR->GetCtrlID() );

	return TRUE;
}

BOOL GLGaeaServer::GMCtrolShowMeTheMoney( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_SHOWMETHEMONEY_FLD* pNetMsgFld )
{
#if defined(_RELEASED) || defined(KRT_PARAM)
	if ( dwClientID != m_dwAgentSlot ) return FALSE;

	PGLCHAR pCHAR = GetChar ( pNetMsgFld->dwGaeaID );
	if ( !pCHAR )						return FALSE;

	pCHAR->CheckMoneyUpdate( pCHAR->m_lnMoney, pNetMsgFld->llMoney, TRUE, "Show Me The Money." );
	pCHAR->m_lnMoney += pNetMsgFld->llMoney;

	GLMSG::SNET_GM_SHOWMETHEMONEY NetMsgFB;
	NetMsgFB.llMoney = pCHAR->m_lnMoney;
	SENDTOCLIENT ( pCHAR->m_dwClientID, &NetMsgFB );
#endif

	return TRUE;
}


BOOL GLGaeaServer::GMCtrolMoPrintCrowList ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_PRINT_CROWLIST_FLD* pNetMsg )
{
	if ( !m_bEmulator && dwClientID != m_dwAgentSlot ) return FALSE;

	PGLCHAR pCHAR = GetChar ( pNetMsg->dwGaeaID );
	if ( !pCHAR )				return FALSE;
	if ( !pCHAR->m_pLandMan )	return FALSE;

	//	Note : ���� ������ �ִ� ��� ���� ������Ʈ���� Ž���Ѵ�.
	//
	int i;
	PGLCROW		pCrow;
	PCROWDATA	pCrowData;
	DWORD		dwID;

	EMCROW emCrow; 
	if ( pNetMsg->dwFindMob == 1 ) emCrow = CROW_MOB;
	else if ( pNetMsg->dwFindMob == 2 ) emCrow = CROW_NPC;

	map<DWORD, int> vecCrowDataList;
	for ( i = 0; i < MAXCROW; i++ )
	{
		pCrow = NULL;
		pCrowData = NULL;
		
		pCrow = pCHAR->m_pLandMan->GetCrow( i );
		if( pCrow == NULL ) continue;

		pCrowData = pCrow->m_pCrowData;
		if( pCrowData == NULL ) continue;
		if( pNetMsg->dwFindMob !=  0 )
		{
			if( pCrowData->m_emCrow != emCrow ) continue;
			if( pCrowData->m_emCrow != emCrow ) continue;
		}


		dwID = pCrow->m_sNativeID.dwID;
		
		if( vecCrowDataList.count( dwID ) == 0 )
		{
			vecCrowDataList.insert( std::make_pair( dwID, 1 ) );
		}
		else
		{
			vecCrowDataList[dwID] += 1;
		}
		
	}

	map<DWORD, int>::iterator iter = vecCrowDataList.begin();
	map<DWORD, int>::iterator iter_end = vecCrowDataList.end();

	GLMSG::SNET_GM_PRINT_CROWLIST_FB NetMsgFB;
	NetMsgFB.emCrow = emCrow;

	for ( ; iter != iter_end; ++iter )
	{
		SNATIVEID sID = iter->first;
		int nNum = iter->second;

		
		NetMsgFB.mID	= sID.wMainID;
		NetMsgFB.sID	= sID.wSubID;
		NetMsgFB.wNum = nNum;		

		GLGaeaServer::GetInstance().SENDTOCLIENT ( pCHAR->m_dwClientID, &NetMsgFB );
	}



	return TRUE;
}

BOOL GLGaeaServer::GMCtrolMobGen ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_MOB_GEN_FLD *pNetMsg )
{
	if ( !m_bEmulator && dwClientID != m_dwAgentSlot ) return FALSE;

	SNATIVEID sID(pNetMsg->wMainID,pNetMsg->wSubID);
	PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( sID );
	if ( !pCrowData )	return FALSE;

	PGLCHAR pCHAR = GetChar ( pNetMsg->dwGaeaID );
	if ( !pCHAR )				return FALSE;
	/*if ( !pCHAR->m_pLandMan )	return FALSE;
	pCHAR->m_pLandMan->DropCrow ( sID, pNetMsg->wPosX, pNetMsg->wPosY );*/
	GLLandMan* pLandMan = GetByMapID ( pCHAR->m_sMapID );
	if( !pLandMan ) return FALSE;

	if ( pCrowData->m_emCrow == CROW_MATERIAL ) 
		pLandMan->DropMaterial ( sID, pNetMsg->wPosX, pNetMsg->wPosY );
	else	
		pLandMan->DropCrow ( sID, pNetMsg->wPosX, pNetMsg->wPosY );

	return TRUE;
}

BOOL GLGaeaServer::GMCtrolMobGenEx ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_MOB_GEN_EX_FLD *pNetMsg)
{
	if ( !m_bEmulator && dwClientID != m_dwAgentSlot ) return FALSE;

	SNATIVEID sMobID(pNetMsg->wMobMID,pNetMsg->wMobSID);
	PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( sMobID );
	if ( !pCrowData )	return FALSE;

	const SNATIVEID sMapID(pNetMsg->wMapMID,pNetMsg->wMapSID);
	GLLandMan* pLandMan = GetByMapID ( sMapID );
	if ( !pLandMan )	return FALSE;

	D3DXVECTOR3 vBasePos(0,0,0), vGenPos(0,0,0);
	GLMapAxisInfo& MapAxisInfo = pLandMan->GetMapAxisInfo ();
	MapAxisInfo.MapPos2MiniPos ( pNetMsg->wPosX, pNetMsg->wPosY, vBasePos.x, vBasePos.z );
	vBasePos.x -= (float)pNetMsg->wRange*50;
	vBasePos.z -= (float)pNetMsg->wRange*50;
	DWORD wRange = pNetMsg->wRange*100;

	if ( wRange < pNetMsg->wNum ) return FALSE;

	std::vector<DWORD> vecRandPosX, vecRandPosZ;
	for ( DWORD i = 0; i < wRange; ++i ) vecRandPosX.push_back(i);
	std::random_shuffle ( vecRandPosX.begin(), vecRandPosX.end() );
	for ( DWORD i = 0; i < wRange; ++i ) vecRandPosZ.push_back(i);
	std::random_shuffle ( vecRandPosZ.begin(), vecRandPosZ.end() );

	for ( DWORD i = 0; i < pNetMsg->wNum; ++i )
	{
		vGenPos.x = (float)(vBasePos.x + vecRandPosX[i]);
		vGenPos.z = (float)(vBasePos.z + vecRandPosZ[i]);

		if ( pCrowData->m_emCrow == CROW_MATERIAL ) 
		{
			DWORD dwGlobID = pLandMan->DropMaterial ( sMobID, vGenPos.x, vGenPos.z );
			pLandMan->m_listDelGlobIDExMat.push_back(dwGlobID);
		}
		else 
		{
			DWORD dwGlobID = pLandMan->DropCrow ( sMobID, vGenPos.x, vGenPos.z );
			pLandMan->m_listDelGlobIDEx.push_back(dwGlobID);
		}
	}

	return TRUE;
}


BOOL GLGaeaServer::GMCtrolMobDelEx ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_MOB_DEL_EX_FLD* pNetMsg )
{
	if ( !m_bEmulator && dwClientID != m_dwAgentSlot ) return FALSE;

	SNATIVEID sMobID(pNetMsg->wMobMID,pNetMsg->wMobSID);
	PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( sMobID );
	if ( !pCrowData )	return FALSE;

	const SNATIVEID sMapID(pNetMsg->wMapMID,pNetMsg->wMapSID);
	GLLandMan* pLandMan = GetByMapID ( sMapID );
	if ( !pLandMan )	return FALSE;

	if ( pCrowData->m_emCrow == CROW_MATERIAL ) 
	{
		LISTDELMATERIALID_ITER iter = pLandMan->m_listDelGlobIDExMat.begin();
		LISTDELMATERIALID_ITER iter_end = pLandMan->m_listDelGlobIDExMat.end();
		for ( ;iter != iter_end; ) 
		{
			LISTDELMATERIALID_ITER iter_cur = iter++;
			PGLMATERIAL pMaterial = pLandMan->GetMaterial ( *iter_cur );
			if ( !pMaterial ) continue;

			if ( pMaterial->m_sNativeID == sMobID ) 
			{
				pLandMan->DropOutMaterial ( *iter_cur );
				pLandMan->m_listDelGlobIDExMat.erase(iter_cur);
			}
		}
	}
	else
	{
		LISTDELCROWID_ITER iter = pLandMan->m_listDelGlobIDEx.begin();
		LISTDELCROWID_ITER iter_end = pLandMan->m_listDelGlobIDEx.end();
		for ( ;iter != iter_end; ) 
		{
			LISTDELCROWID_ITER iter_cur = iter++;
			PGLCROW pCrow = pLandMan->GetCrow ( *iter_cur );
			if ( !pCrow ) continue;

			if ( pCrow->m_sNativeID == sMobID ) 
			{
				pLandMan->DropOutCrow ( *iter_cur );
				pLandMan->m_listDelGlobIDEx.erase(iter_cur);
			}
		}
	}

	return TRUE;
}

BOOL GLGaeaServer::GMCtrolMobDel ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_MOB_DEL_FLD *pNetMsg )
{
	if ( !m_bEmulator )
	{
		if ( dwClientID != m_dwAgentSlot ) return FALSE;
	}

	PGLCHAR pCHAR = GetChar ( pNetMsg->dwGaeaID );
	if ( !pCHAR )				return FALSE;

	GLLandMan* pLandMan = GetByMapID ( pCHAR->m_sMapID );
	if( !pLandMan ) return FALSE;

	if ( pNetMsg->sMobID == NATIVEID_NULL() ) 
	{
		if ( pNetMsg->bMaterial ) 
		{
			return pLandMan->DropOutMaterial();
		}
		else
		{
			return pLandMan->DropOutCrow();
		}
	}
	else
	{
		PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( pNetMsg->sMobID );
		if ( !pCrowData )	return FALSE;

		if ( pCrowData->m_emCrow == CROW_MATERIAL ) 
		{
			LISTDELMATERIALID_ITER iter = pLandMan->m_listDelGlobIDMat.begin();
			LISTDELMATERIALID_ITER iter_end = pLandMan->m_listDelGlobIDMat.end();			

			for ( ;iter != iter_end; ) 
			{
				LISTDELMATERIALID_ITER iter_cur = iter++;
				PGLMATERIAL pMaterial = pLandMan->GetMaterial ( *iter_cur );
				if ( !pMaterial ) continue;

				if ( pMaterial->m_sNativeID == pNetMsg->sMobID ) 
				{
					pLandMan->DropOutMaterial ( *iter_cur );
				}
			}
		}
		else
		{
			LISTDELCROWID_ITER iter = pLandMan->m_listDelGlobID.begin();
			LISTDELCROWID_ITER iter_end = pLandMan->m_listDelGlobID.end();

			for ( ;iter != iter_end; ) 
			{
				LISTDELCROWID_ITER iter_cur = iter++;
				PGLCROW pCrow = pLandMan->GetCrow ( *iter_cur );
				if ( !pCrow ) continue;

				if ( pCrow->m_sNativeID == pNetMsg->sMobID ) 
				{
					pLandMan->DropOutCrow ( *iter_cur );
				}
			}
		}

	}

	return TRUE;
}

BOOL GLGaeaServer::GMCtrolMobLevel ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_MOB_LEV *pNetMsg )
{
	if ( !m_bEmulator )
	{
		if ( dwClientID != m_dwAgentSlot ) return FALSE;
	}
	
	SNATIVEID nidMAP = SNATIVEID(pNetMsg->wMAP_MID,pNetMsg->wMAP_SID);
	if ( nidMAP==SNATIVEID(false) )		return FALSE;

	GLLandMan *pLandMan = GetByMapID ( nidMAP );
	if ( !pLandMan )					return FALSE;

	pLandMan->LoadMobSchManEx ( pNetMsg->szLevelFile );

	return TRUE;
}

BOOL GLGaeaServer::GMCtrolMobLevelClear ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_MOB_LEV_CLEAR *pNetMsg )
{
	if ( !m_bEmulator )
	{
		if ( dwClientID != m_dwAgentSlot ) return FALSE;
	}

	SNATIVEID nidMAP = SNATIVEID(pNetMsg->wMAP_MID,pNetMsg->wMAP_SID);
	if ( nidMAP==SNATIVEID(false) )		return FALSE;

	GLLandMan *pLandMan = GetByMapID ( nidMAP );
	if ( !pLandMan )					return FALSE;

	pLandMan->ClearMobSchManEx ();

	return TRUE;
}

BOOL GLGaeaServer::GMCtrolChatBlockFld ( GLMSG::SNET_GM_CHAT_BLOCK_FLD *pNetMsg )
{
	PGLCHAR pCHAR = GetCharID ( pNetMsg->dwCHARID );
	if ( !pCHAR )	return FALSE;

	if ( pNetMsg->dwBLOCK_MINUTE!=0 )
	{
		//	�����ð� ����.
		pCHAR->m_tCHATBLOCK;

		CTime cBLOCK = CTime::GetCurrentTime();
		CTimeSpan cSPAN(0,0,pNetMsg->dwBLOCK_MINUTE,0);
		cBLOCK += cSPAN;

		pCHAR->m_tCHATBLOCK = cBLOCK.GetTime();
	}
	else
	{
		//	�����ð� ����.
		pCHAR->m_tCHATBLOCK = 0;
	}

	return TRUE;
}

BOOL GLGaeaServer::UserCtrolCharInfoFld ( GLMSG::SNET_USER_CHAR_INFO_FLD *pNetMsg )
{
	PGLCHAR pCHAR = GetCharID ( pNetMsg->dwCHARID );
	if ( !pCHAR )	return FALSE;

	int nPosX(0), nPosY(0);
	if ( pCHAR->m_pLandMan )
	{
		pCHAR->m_pLandMan->GetMapAxisInfo().Convert2MapPos ( pCHAR->m_vPos.x, pCHAR->m_vPos.z, nPosX, nPosY );
	}

	GLCLUB *pCLUB = GetClubMan().GetClub ( pCHAR->m_dwGuild );

	GLMSG::SNET_USER_CHAR_INFO_FLD_FB NetMsgFB;
	if ( pCLUB )	StringCchCopy ( NetMsgFB.szCLUB, CHAR_SZNAME, pCLUB->m_szName );
	else			StringCchCopy ( NetMsgFB.szCLUB, CHAR_SZNAME, "void" );
	NetMsgFB.dwCLIENTID_FB = pNetMsg->dwCLIENTID_FB;
	SENDTOAGENT ( &NetMsgFB );

	return TRUE;
}

BOOL GLGaeaServer::GMCtrolCharInfoFld ( GLMSG::SNET_GM_CHAR_INFO_FLD *pNetMsg )
{
	PGLCHAR pCHAR = GetCharID ( pNetMsg->dwCHARID );
	if ( !pCHAR )	return FALSE;
	
	int nPosX(0), nPosY(0);
	if ( pCHAR->m_pLandMan )
	{
		pCHAR->m_pLandMan->GetMapAxisInfo().Convert2MapPos ( pCHAR->m_vPos.x, pCHAR->m_vPos.z, nPosX, nPosY );
	}

	GLCLUB *pCLUB = GetClubMan().GetClub ( pCHAR->m_dwGuild );

	GLMSG::SNET_GM_CHAR_INFO_FLD_FB NetMsgFB;
	NetMsgFB.dwCHARID = pCHAR->m_dwCharID;
	NetMsgFB.nidMAP = pCHAR->m_sMapID;
	NetMsgFB.wLEVEL = pCHAR->m_wLevel;
	NetMsgFB.wPOSX = nPosX;
	NetMsgFB.wPOSY = nPosY;
	NetMsgFB.sHP = pCHAR->m_sHP;
	NetMsgFB.sMP = pCHAR->m_sMP;
	NetMsgFB.sSP = pCHAR->m_sSP;
	NetMsgFB.sEXP = pCHAR->m_sExperience;
	NetMsgFB.sCP = pCHAR->m_sCombatPoint; /*combatpoint logic, Juver, 2017/05/29 */
	if ( pCLUB )	StringCchCopy ( NetMsgFB.szCLUB, CHAR_SZNAME, pCLUB->m_szName );
	else			StringCchCopy ( NetMsgFB.szCLUB, CHAR_SZNAME, "void" );
	NetMsgFB.dwCLIENTID_FB = pNetMsg->dwCLIENTID_FB;
	SENDTOAGENT ( &NetMsgFB );
	
	return TRUE;
}

BOOL GLGaeaServer::RequestViewAllPlayer ( GLMSG::SNET_GM_VIEWALLPLAYER_FLD_REQ *pNetMsg )
{
	PGLCHAR pCHAR = GetChar ( pNetMsg->dwID );
	if ( !pCHAR )						 return FALSE;
	if ( pCHAR->m_dwUserLvl < USER_GM3 ) return FALSE;

	GLMSG::SNET_GM_VIEWALLPLAYER_FLD_FB NetMsgFB;

	LANDQUADTREE* pLandTree = pCHAR->m_pLandMan->GetLandTree();
	if ( !pLandTree )					return FALSE;

	// ���� ũ�⸦ ���´�.
	D3DXVECTOR3 vMax, vMin;
	pCHAR->m_pLandMan->GetNaviMeshAABB ( vMax, vMin );

	LANDQUADNODE* pQuadNode = NULL;
	BOUDRECT bRect( (int)vMax.x,(int)vMax.z,(int)vMin.x,(int)vMin.z );
	pLandTree->FindNodes ( bRect, pLandTree->GetRootNode(), &pQuadNode );

    DWORD dwPlayerNum = 0;
	for ( ; pQuadNode; pQuadNode = pQuadNode->pFindNext )
	{
		GLLandNode *pLandNode = pQuadNode->pData;
		if( !pLandNode ) continue;

		GASSERT(pLandTree->GetCellNum()>pQuadNode->dwCID&&"�ε����� ������� �ʰ��Ͽ����ϴ�.");

		//	Note : - Char -
		//
		GLCHARNODE *pCharCur = pLandNode->m_PCList.m_pHead;
		for ( ; pCharCur; pCharCur = pCharCur->pNext )
		{
			PGLCHAR pGLChar = pCharCur->Data;
			if( !pGLChar ) continue;
			if ( pGLChar->m_dwClientID == pCHAR->m_dwClientID )	continue;

			// ���⼭ ��Ŷ�� �Ǿ �����ָ� �ȴ�
			GLMSG::SNET_GM_VIEWALLPLAYER_FLD_FB NetMsgFB;
			NetMsgFB.dwCHARID = pGLChar->m_dwCharID;
			StringCchCopy ( NetMsgFB.szCHARNAME, CHAR_SZNAME, pGLChar->m_szName );

			SENDTOCLIENT ( pCHAR->m_dwClientID, &NetMsgFB );
			dwPlayerNum++;
		}
	}
	NetMsgFB.dwPlayerNum = dwPlayerNum;
	SENDTOCLIENT ( pCHAR->m_dwClientID, &NetMsgFB );

	return TRUE;

	/*
	GLLandNode* pLandNode = NULL;
	GLCHARNODE* pCharCur = NULL;
	PGLCHAR pGLChar = NULL;
	// GLMSG::SNET_GM_VIEWALLPLAYER_FLD_FB NetMsgFB;
	for ( ; pQuadNode; pQuadNode = pQuadNode->pFindNext )
	{
		pLandNode = pQuadNode->pData;

		GASSERT(pLandTree->GetCellNum()>pQuadNode->dwCID&&"Index over cell size");

		//	Note : - Char -
		//
		pCharCur = pLandNode->m_PCList.m_pHead;
		for ( ; pCharCur; pCharCur = pCharCur->pNext )
		{
			pGLChar = pCharCur->Data;
			if ( pGLChar->m_dwClientID == pCHAR->m_dwClientID )	continue;

			// ���⼭ ��Ŷ�� �Ǿ �����ָ� �ȴ�			
			NetMsgFB.dwCHARID = pGLChar->m_dwCharID;
			StringCchCopy ( NetMsgFB.szCHARNAME, CHAR_SZNAME, pGLChar->m_szName );

			SENDTOCLIENT ( pCHAR->m_dwClientID, &NetMsgFB );
		}
	}
	return TRUE;
	*/
}


BOOL GLGaeaServer::RequestShopInfo( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_GM_SHOP_INFO_REQ *pNetMsg )
{

	LISTSEARCHSHOP_ITER iter	 = m_listSearchShop.begin();

	PGLCHAR pMyChar = GetChar ( dwGaeaID );
	if( pMyChar == NULL ) return TRUE;	
	if( pMyChar ->m_dwUserLvl < USER_GM3 ) return TRUE;


	TEXTCONSOLEMSG_WRITE( "[INFO]GM Command Shop Info Start" );


	int iMsgCount = 0;	

	for( ; iter != m_listSearchShop.end(); ++iter  )
	{	
		PGLCHAR pChar = GetChar ( *iter );
		if( pChar == NULL || pChar->m_sPMarket.IsOpen() == FALSE || pChar->m_sMapID.wMainID != 22 || pChar->m_sMapID.wSubID != 0  ) 
		{
			m_listSearchShop.erase( iter-- );		
			continue;
		}

		MAPSEARCH mapSearchItem = pChar->m_sPMarket.GetSearchItemList();
		if( pNetMsg->bBasicInfo )
		{
			MAPSEARCH_ITER iterItem		= mapSearchItem.begin();
			for( ; iterItem != mapSearchItem.end(); ++iterItem )
			{
				CString strItemInfo;
				SNATIVEID		sItemID	   = iterItem->first;
				SSEARCHITEMDATA searchItem = iterItem->second;			
				//					�������̵� MID SID ����
				strItemInfo.Format( "%d, %d, %d, %d, %I64d\n", pMyChar->GetUserID(), pMyChar->GetCharID(), sItemID.wMainID, sItemID.wSubID, searchItem.llPRICE );		

				GLMSG::SNET_GM_SHOP_INFO_FB NetMsgFB;
				NetMsgFB.bBasicInfo = pNetMsg->bBasicInfo;
				StringCbCopy(NetMsgFB.szBasicInfo, MAX_CHAR_LENGTH+1, strItemInfo.GetString());		

				if( iMsgCount == 0 ) NetMsgFB.bSTATE = GLMSG::SNET_GM_SHOP_INFO_FB::FIRST;
				else				 NetMsgFB.bSTATE = GLMSG::SNET_GM_SHOP_INFO_FB::MIDDLE;
				iMsgCount++;
				ReserveMessage( dwClientID, dwGaeaID, iMsgCount/100, &NetMsgFB );
			}
		}else{
			CString strItemInfo;
			if( mapSearchItem.find(pNetMsg->sSearchItemID.dwID) == mapSearchItem.end() ) continue; 
			SSEARCHITEMDATA searchItem = mapSearchItem[pNetMsg->sSearchItemID.dwID];

			strItemInfo.Format( "%d, %d, %d, %d, %I64d, %d, %f, %d, %f, %d, %f, %d, %f, %d, %d, %d, %d, %d, %d, %d\n", 
													   pMyChar->GetUserID(),			// ����   NUM
													   pMyChar->GetCharID(),			// ĳ���� NUM
													   pNetMsg->sSearchItemID.wMainID,  // ������ MID
													   pNetMsg->sSearchItemID.wSubID,	// ������ SID
													   searchItem.llPRICE,				// ������ ����

													   searchItem.RandOptTYPE1,			// �����ɼ� 1 Ÿ��
													   searchItem.fRandOptValue1,		// �����ɼ� 1 ��
													   searchItem.RandOptTYPE2,			// �����ɼ� 2 Ÿ��
													   searchItem.fRandOptValue2,		// �����ɼ� 2 ��
													   searchItem.RandOptTYPE3,			// �����ɼ� 3 Ÿ��
													   searchItem.fRandOptValue3,		// �����ɼ� 3 ��
													   searchItem.RandOptTYPE4,			// �����ɼ� 4 Ÿ��
													   searchItem.fRandOptValue4,		// �����ɼ� 4 ��

													   searchItem.cDAMAGE,				// ���� ���� ���
													   searchItem.cDEFENSE,				// ��� ���� ���

													   searchItem.cRESIST_FIRE,				//	����(ȭ) ���� ���.
													   searchItem.cRESIST_ICE,				//	����(��) ���� ���.
													   searchItem.cRESIST_ELEC,				//	����(��) ���� ���.
													   searchItem.cRESIST_POISON,			//	����(��) ���� ���.
													   searchItem.cRESIST_SPIRIT );			//	����(��) ���� ���.

			GLMSG::SNET_GM_SHOP_INFO_FB NetMsgFB;
			NetMsgFB.bBasicInfo = pNetMsg->bBasicInfo;
			StringCbCopy(NetMsgFB.szBasicInfo, MAX_CHAR_LENGTH+1, strItemInfo.GetString());	

			if( iMsgCount == 0 ) NetMsgFB.bSTATE = GLMSG::SNET_GM_SHOP_INFO_FB::FIRST;
			else				 NetMsgFB.bSTATE = GLMSG::SNET_GM_SHOP_INFO_FB::MIDDLE;
			iMsgCount++;
			ReserveMessage( dwClientID, dwGaeaID, iMsgCount/100, &NetMsgFB );
		}
	}

	GLMSG::SNET_GM_SHOP_INFO_FB NetMsgFB;
	if( iMsgCount == 0 )
		sprintf( NetMsgFB.szBasicInfo, "Not Found Item Count" );
	else
		sprintf( NetMsgFB.szBasicInfo, "Found Item Count : %d", iMsgCount );
	NetMsgFB.bSTATE = GLMSG::SNET_GM_SHOP_INFO_FB::END;
	ReserveMessage( dwClientID, dwGaeaID, iMsgCount/100, &NetMsgFB );

	TEXTCONSOLEMSG_WRITE( "[INFO]GM Command Shop Info End" );

	return TRUE;
}


BOOL GLGaeaServer::RequestConfrontPartyChkMbr ( GLMSG::SNETPC_CONFRONTPTY_CHECKMBR2_FLD *pNetMsg )
{
	const SNATIVEID &sMapID = pNetMsg->sMapID;

	GLLandMan* pLandMan = GetByMapID ( sMapID );
	if ( !pLandMan )	return FALSE;

	GLPARTY_FIELD* pPartyA = GetParty ( pNetMsg->dwPARTY_A );
	if ( !pPartyA )		return FALSE;

	GLPARTY_FIELD* pPartyB = GetParty ( pNetMsg->dwPARTY_B );
	if ( !pPartyB )		return FALSE;

	GLChar* pCharA = GetChar(pPartyA->m_dwMASTER);
	GLChar* pCharB = GetChar(pPartyB->m_dwMASTER);

	if ( !pCharA || !pCharB )
	{
		GLMSG::SNETPC_REQ_CONFRONT_FB NetMsgFB;
		NetMsgFB.emTYPE = EMCONFT_PARTY;
		NetMsgFB.dwID = pNetMsg->dwPARTY_A;
		NetMsgFB.emFB = EMCONFRONT_FAIL;

		if ( pCharA )	GLGaeaServer::GetInstance().SENDTOAGENT ( pCharA->m_dwClientID, &NetMsgFB );
		if ( pCharB )	GLGaeaServer::GetInstance().SENDTOAGENT ( pCharB->m_dwClientID, &NetMsgFB );

		return FALSE;
	}

	GLMSG::SNETPC_CONFRONTPTY_CHECKMBR2_AGT NetMsgCheckMbr;
	NetMsgCheckMbr.sMapID = pNetMsg->sMapID;
	NetMsgCheckMbr.dwPARTY_A = pNetMsg->dwPARTY_A;
	NetMsgCheckMbr.dwPARTY_B = pNetMsg->dwPARTY_B;
	NetMsgCheckMbr.sOption = pNetMsg->sOption;
	NetMsgCheckMbr.vPosition = ( pCharA->m_vPos + pCharB->m_vPos ) / 2.0f;
	
	bool bSCHOOL_CONFT_LEVEL(true);
	int nCheck(0);
	GLPARTY_FIELD::MEMBER_ITER iter, end;
	
	iter = pPartyA->m_cMEMBER.begin();
	end = pPartyA->m_cMEMBER.end();
	for ( ; iter!=end; ++iter )
	{
		GLChar* pMember = GetChar ( (*iter).first );
		if ( !pMember )								continue;
		if ( pNetMsg->sMapID!=pMember->m_sMapID )	continue;
		
		if ( pMember->m_wLevel < GLCONST_CHAR::wCONFRONT_SCHOOL_LIMIT_LEVEL )		bSCHOOL_CONFT_LEVEL = false;

		D3DXVECTOR3 vDistance = pMember->m_vPos - NetMsgCheckMbr.vPosition;
		float fDistance = D3DXVec3Length ( &vDistance );

		if ( fDistance < GLCONST_CHAR::fCONFRONT_PY_DIST )
		{
			NetMsgCheckMbr.dwPARTY_A_MEMBER[nCheck++] = pMember->m_dwGaeaID;
		}
	}

	nCheck = 0;
	iter = pPartyB->m_cMEMBER.begin();
	end = pPartyB->m_cMEMBER.end();
	for ( ; iter!=end; ++iter )
	{
		GLChar* pMember = GetChar ( (*iter).first );
		if ( !pMember )								continue;
		if ( pNetMsg->sMapID!=pMember->m_sMapID )	continue;

		if ( pMember->m_wLevel < GLCONST_CHAR::wCONFRONT_SCHOOL_LIMIT_LEVEL )		bSCHOOL_CONFT_LEVEL = false;
		
		D3DXVECTOR3 vDistance = pMember->m_vPos - NetMsgCheckMbr.vPosition;
		float fDistance = D3DXVec3Length ( &vDistance );

		if ( fDistance < GLCONST_CHAR::fCONFRONT_PY_DIST )
		{
			NetMsgCheckMbr.dwPARTY_B_MEMBER[nCheck++] = pMember->m_dwGaeaID;
		}
	}

	NetMsgCheckMbr.sOption.bSCHOOL = bSCHOOL_CONFT_LEVEL;

	SENDTOAGENT	( &NetMsgCheckMbr );

	return TRUE;
}

BOOL GLGaeaServer::RequestConfrontParty ( GLMSG::SNETPC_CONFRONTPTY_START2_FLD *pNetMsg )
{
	const SNATIVEID &sMapID = pNetMsg->sMapID;

	GLLandMan* pLandMan = GetByMapID ( sMapID );
	if ( !pLandMan )	return FALSE;

	GLPARTY_FIELD* pPartyA = GetParty ( pNetMsg->dwPARTY_A );
	if ( !pPartyA )		return FALSE;

	GLPARTY_FIELD* pPartyB = GetParty ( pNetMsg->dwPARTY_B );
	if ( !pPartyB )		return FALSE;

	GLChar* pCharA = GetChar(pPartyA->m_dwMASTER);
	GLChar* pCharB = GetChar(pPartyB->m_dwMASTER);

	if ( !pCharA || !pCharB )
	{
		GLMSG::SNETPC_REQ_CONFRONT_FB NetMsgFB;
		NetMsgFB.emTYPE = EMCONFT_PARTY;
		NetMsgFB.dwID = pNetMsg->dwPARTY_A;
		NetMsgFB.emFB = EMCONFRONT_FAIL;

		if ( pCharA )	GLGaeaServer::GetInstance().SENDTOAGENT ( pCharA->m_dwClientID, &NetMsgFB );
		if ( pCharB )	GLGaeaServer::GetInstance().SENDTOAGENT ( pCharB->m_dwClientID, &NetMsgFB );

		return FALSE;
	}

	D3DXVECTOR3 vPosition = pNetMsg->vPos;

	//	Note : �б� ���.
	WORD wSCHOOL_A(0);
	WORD wSCHOOL_B(0);
	if ( pNetMsg->sOption.bSCHOOL )
	{
		wSCHOOL_A = pCharA->m_wSchool;
		wSCHOOL_B = pCharB->m_wSchool;
	}

	//	Note : ù��° ��Ƽ���鿡 '��Ƽ���' ���� ����.
	//
	pPartyA->m_sconftOPTION = pNetMsg->sOption;
	pPartyA->m_dwconftPARTYID = pNetMsg->dwPARTY_B;
	
	GLMSG::SNETPC_CONFRONTPTY_START2_CLT sNetMsgClt;
	sNetMsgClt.dwconftPARTY = pNetMsg->dwPARTY_B;
	memcpy ( sNetMsgClt.dwCONFRONT_MY_MEMBER, pNetMsg->dwPARTY_A_MEMBER, sizeof(DWORD)*MAXPARTY );
	memcpy ( sNetMsgClt.dwCONFRONT_TAR_MEMBER, pNetMsg->dwPARTY_B_MEMBER, sizeof(DWORD)*MAXPARTY );
	sNetMsgClt.vPosition = vPosition;
	sNetMsgClt.sOption = pNetMsg->sOption;
	sNetMsgClt.sOption.wMY_SCHOOL = wSCHOOL_A;
	sNetMsgClt.sOption.wTAR_SCHOOL = wSCHOOL_B;

	if ( pCharB )
	{
		StringCbCopy(sNetMsgClt.szName, CHR_ID_LENGTH+1, pCharB->m_szName);		
	}

	int i(0);
	DWORD dwMember = 0;
	GLChar* pChar = NULL;
	for ( i=0; i<MAXPARTY; ++i )
	{
		dwMember = pNetMsg->dwPARTY_A_MEMBER[i];
		if ( GAEAID_NULL==dwMember )	continue;
		pChar = GetChar ( dwMember );
		if ( !pChar )
		{
			continue;
		}
		else
		{
			pPartyA->MBR_CONFRONT_JOIN ( dwMember );

			pChar->m_sCONFTING.RESET();
			pChar->m_sCONFTING.emTYPE = EMCONFT_PARTY;
			pChar->m_sCONFTING.dwTAR_ID = pPartyA->m_dwconftPARTYID;
			pChar->m_sCONFTING.vPosition = vPosition;
			pChar->m_sCONFTING.sOption = pNetMsg->sOption;

			//	Note : Ŭ���̾�Ʈ�鿡 '��Ƽ���' ���� ����.
			//
			SENDTOCLIENT ( pChar->m_dwClientID, &sNetMsgClt );

			//	Note : ���������� ����.
			//
			pChar->DelPlayHostile();

			//	Note : ��� ü�� ���.
			//
			pChar->DoConftStateBackup();
		}
	}


	//	Note : �ι��� ��Ƽ���鿡 '��Ƽ���' ���� ����.
	//
	pPartyB->m_sconftOPTION = pNetMsg->sOption;
	pPartyB->m_dwconftPARTYID = pNetMsg->dwPARTY_A;

	sNetMsgClt.dwconftPARTY = pNetMsg->dwPARTY_A;
	memcpy ( sNetMsgClt.dwCONFRONT_MY_MEMBER, pNetMsg->dwPARTY_B_MEMBER, sizeof(DWORD)*MAXPARTY );
	memcpy ( sNetMsgClt.dwCONFRONT_TAR_MEMBER, pNetMsg->dwPARTY_A_MEMBER, sizeof(DWORD)*MAXPARTY );
	sNetMsgClt.vPosition = vPosition;
	sNetMsgClt.sOption = pNetMsg->sOption;
	sNetMsgClt.sOption.wMY_SCHOOL = wSCHOOL_B;
	sNetMsgClt.sOption.wTAR_SCHOOL = wSCHOOL_A;

	if ( pCharA )
	{
		StringCbCopy(sNetMsgClt.szName, CHR_ID_LENGTH+1, pCharA->m_szName);
	}

	for ( i=0; i<MAXPARTY; ++i )
	{
		dwMember = pNetMsg->dwPARTY_B_MEMBER[i];
		if ( GAEAID_NULL==dwMember )	continue;
		pChar = GetChar ( dwMember );
		if ( !pChar )
		{
			continue;
		}
		else
		{
			pPartyB->MBR_CONFRONT_JOIN ( dwMember );
			
			pChar->m_sCONFTING.RESET();
			pChar->m_sCONFTING.emTYPE = EMCONFT_PARTY;
			pChar->m_sCONFTING.dwTAR_ID = pPartyB->m_dwconftPARTYID;
			pChar->m_sCONFTING.vPosition = vPosition;
			pChar->m_sCONFTING.sOption = pNetMsg->sOption;

			//	Note : Ŭ���̾�Ʈ�鿡 '��Ƽ���' ���� ����.
			//
			SENDTOCLIENT ( pChar->m_dwClientID, &sNetMsgClt );

			//	Note : ���������� ����.
			//
			pChar->DelPlayHostile();

			//	Note : ��� ü�� ���.
			//
			pChar->DoConftStateBackup();
		}
	}

	DWORD dwLevelA = pPartyA->GETAVER_CONFT_LEVEL();
	DWORD dwLevelB = pPartyB->GETAVER_CONFT_LEVEL();

	return TRUE;
}

BOOL GLGaeaServer::RequestConfrontPartyEnd ( GLMSG::SNETPC_CONFRONTPTY_END2_FLD *pNetMsg )
{
	if ( pNetMsg->dwPARTY_A!=PARTY_NULL && pNetMsg->dwPARTY_B!=PARTY_NULL )
	{
		GLPARTY_FIELD *pPartyA = GetParty(pNetMsg->dwPARTY_A);
		GLPARTY_FIELD *pPartyB = GetParty(pNetMsg->dwPARTY_B);

		//	Note : ��Ƽ ����� ��Ƽ�� ����� �κ��� ������ ���� ó��.
		//
		if ( pPartyA && pPartyB )
		{
			DWORD dwLevelA = pPartyA->m_dwconftMEMLEVEL;
			DWORD dwLevelB = pPartyB->m_dwconftMEMLEVEL;
			const SCONFT_OPTION sOption = pPartyB->m_sconftOPTION;

			float fLIFE_P_A(0), fLIFE_P_B(0);

			if ( GLCONST_CHAR::nCONFRONT_WIN_LP!=0 && GLCONST_CHAR::nCONFRONT_LOSS_LP )
			{
				if ( pNetMsg->emEND_A==EMCONFRONT_END_PWIN )
				{
					fLIFE_P_A = GLCONST_CHAR::nCONFRONT_WIN_LP + (dwLevelB-dwLevelA)/2.0f
						+ (sOption.fTAR_HP_RATE-sOption.fHP_RATE);
					LIMIT ( fLIFE_P_A, 20.0f, 0.0f );

					fLIFE_P_B = GLCONST_CHAR::nCONFRONT_LOSS_LP - (dwLevelB-dwLevelA)/2.0f
						- (sOption.fTAR_HP_RATE-sOption.fHP_RATE);
					LIMIT ( fLIFE_P_B, 0.0f, -20.0f );
				}
				else if ( pNetMsg->emEND_B==EMCONFRONT_END_PWIN )
				{
					fLIFE_P_B = GLCONST_CHAR::nCONFRONT_WIN_LP + (dwLevelA-dwLevelB)/2.0f
						+ (sOption.fHP_RATE-sOption.fTAR_HP_RATE);
					LIMIT ( fLIFE_P_B, 20.0f, 0.0f );

					fLIFE_P_A = GLCONST_CHAR::nCONFRONT_LOSS_LP - (dwLevelA-dwLevelB)/2.0f
						- (sOption.fHP_RATE-sOption.fTAR_HP_RATE);
					LIMIT ( fLIFE_P_A, 0.0f, -20.0f );
				}
			}

			pPartyA->RECEIVELIVINGPOINT ( (int)fLIFE_P_A );
			pPartyB->RECEIVELIVINGPOINT ( (int)fLIFE_P_B );
		}
		else
		{
			if ( pPartyA )
			{
				if ( pNetMsg->emEND_A==EMCONFRONT_END_PWIN )	pPartyA->RECEIVELIVINGPOINT ( 0 );
				else											pPartyA->RECEIVELIVINGPOINT ( 0 );
			}

			if ( pPartyB )
			{
				if ( pNetMsg->emEND_B==EMCONFRONT_END_PWIN )	pPartyB->RECEIVELIVINGPOINT ( 0 );
				else											pPartyB->RECEIVELIVINGPOINT ( 0 );
			}
		}
	}

	if ( pNetMsg->dwPARTY_A!=PARTY_NULL )
	{
		GLPARTY_FIELD *pParty = GetParty(pNetMsg->dwPARTY_A);
		if ( pParty )	pParty->RESET_CONFRONT(pNetMsg->emEND_A);
	}

	if ( pNetMsg->dwPARTY_B!=PARTY_NULL )
	{
		GLPARTY_FIELD *pParty = GetParty(pNetMsg->dwPARTY_B);
		if ( pParty )	pParty->RESET_CONFRONT(pNetMsg->emEND_B);
	}

	return TRUE;
}

BOOL GLGaeaServer::RequestConfrontClubChkMbr ( GLMSG::SNET_CONFRONTCLB_CHECKMBR_FLD *pNetMsg )
{
	const SNATIVEID &sMapID = pNetMsg->sMapID;

	GLLandMan* pLandMan = GetByMapID ( sMapID );
	if ( !pLandMan )	return FALSE;

	GLCLUB* pClubA = m_cClubMan.GetClub ( pNetMsg->dwCLUB_A );
	if ( !pClubA )		return FALSE;

	GLCLUB* pClubB = m_cClubMan.GetClub ( pNetMsg->dwCLUB_B );
	if ( !pClubB )		return FALSE;

	GLChar* pCharA = GetCharID(pClubA->m_dwMasterID);
	GLChar* pCharB = GetCharID(pClubB->m_dwMasterID);

	if ( !pCharA || !pCharB )
	{
		GLMSG::SNETPC_REQ_CONFRONT_FB NetMsgFB;
		NetMsgFB.emTYPE = EMCONFT_GUILD;
		NetMsgFB.dwID = pNetMsg->dwCLUB_A;
		NetMsgFB.emFB = EMCONFRONT_FAIL;

		if ( pCharA )	GLGaeaServer::GetInstance().SENDTOAGENT ( pCharA->m_dwClientID, &NetMsgFB );
		if ( pCharB )	GLGaeaServer::GetInstance().SENDTOAGENT ( pCharB->m_dwClientID, &NetMsgFB );

		return FALSE;
	}

	GLMSG::SNET_CONFRONTCLB_CHECKMBR_AGT NetMsgCheckMbr;
	NetMsgCheckMbr.sMapID = pNetMsg->sMapID;
	NetMsgCheckMbr.dwCLUB_A = pNetMsg->dwCLUB_A;
	NetMsgCheckMbr.dwCLUB_B = pNetMsg->dwCLUB_B;
	NetMsgCheckMbr.sOption = pNetMsg->sOption;
	NetMsgCheckMbr.vPosition = ( pCharA->m_vPos + pCharB->m_vPos ) / 2.0f;
	
	bool bSCHOOL_CONFT_LEVEL(true);
	CLUBMEMBERS_ITER pos, end;
	
	pos = pClubA->m_mapMembers.begin();
	end = pClubA->m_mapMembers.end();
	for ( ; pos!=end && NetMsgCheckMbr.dwCLUB_A_NUM < EMMAX_CLUB_NUM; ++pos )
	{
		GLChar* pMember = GetCharID ( (*pos).first );
		if ( !pMember )								continue;
		if ( pNetMsg->sMapID!=pMember->m_sMapID )	continue;
		
		D3DXVECTOR3 vDistance = pMember->m_vPos - NetMsgCheckMbr.vPosition;
		float fDistance = D3DXVec3Length ( &vDistance );

		if ( fDistance < GLCONST_CHAR::fCONFRONT_CLB_DIST )
		{
			NetMsgCheckMbr.dwCLUB_A_MEMBER[NetMsgCheckMbr.dwCLUB_A_NUM++] = pMember->m_dwCharID;
		}
	}

	pos = pClubB->m_mapMembers.begin();
	end = pClubB->m_mapMembers.end();
	for ( ; pos!=end && NetMsgCheckMbr.dwCLUB_B_NUM < EMMAX_CLUB_NUM; ++pos )
	{
		GLChar* pMember = GetCharID ( (*pos).first );
		if ( !pMember )								continue;
		if ( pNetMsg->sMapID!=pMember->m_sMapID )	continue;

		D3DXVECTOR3 vDistance = pMember->m_vPos - NetMsgCheckMbr.vPosition;
		float fDistance = D3DXVec3Length ( &vDistance );

		if ( fDistance < GLCONST_CHAR::fCONFRONT_CLB_DIST )
		{
			NetMsgCheckMbr.dwCLUB_B_MEMBER[NetMsgCheckMbr.dwCLUB_B_NUM++] = pMember->m_dwCharID;
		}
	}

	SENDTOAGENT	( &NetMsgCheckMbr );

	return TRUE;
}

BOOL GLGaeaServer::RequestConfrontClub ( GLMSG::SNETPC_CONFRONTCLB_START2_FLD *pNetMsg )
{
	const SNATIVEID &sMapID = pNetMsg->sMapID;

	GLLandMan* pLandMan = GetByMapID ( sMapID );
	if ( !pLandMan )	return FALSE;

	GLCLUB* pClubA = m_cClubMan.GetClub ( pNetMsg->dwCLUB_A );
	if ( !pClubA )		return FALSE;

	GLCLUB* pClubB = m_cClubMan.GetClub ( pNetMsg->dwCLUB_B );
	if ( !pClubB )		return FALSE;

	GLChar* pCharA = GetCharID(pClubA->m_dwMasterID);
	GLChar* pCharB = GetCharID(pClubB->m_dwMasterID);

	if ( !pCharA || !pCharB )
	{
		GLMSG::SNETPC_REQ_CONFRONT_FB NetMsgFB;
		NetMsgFB.emTYPE = EMCONFT_GUILD;
		NetMsgFB.dwID = pNetMsg->dwCLUB_A;
		NetMsgFB.emFB = EMCONFRONT_FAIL;

		if ( pCharA )	GLGaeaServer::GetInstance().SENDTOAGENT ( pCharA->m_dwClientID, &NetMsgFB );
		if ( pCharB )	GLGaeaServer::GetInstance().SENDTOAGENT ( pCharB->m_dwClientID, &NetMsgFB );

		return FALSE;
	}

	D3DXVECTOR3 vPosition = pNetMsg->vPos;

	//	Note : ù��° Ŭ�����鿡 'Ŭ�����' ���� ����.
	//
	pClubA->m_conftOPT = pNetMsg->sOption;
	pClubA->m_dwconftCLUB = pNetMsg->dwCLUB_B;
	
	GLMSG::SNETPC_CONFRONTCLB_START2_CLT sNetMsgClt;
	sNetMsgClt.dwconftCLUB = pNetMsg->dwCLUB_B;
	sNetMsgClt.dwCLUB_MY_NUM = pNetMsg->dwCLUB_A_NUM;
	memcpy ( sNetMsgClt.dwCONFRONT_MY_MEMBER, pNetMsg->dwCLUB_A_MEMBER, sizeof(DWORD)*EMMAX_CLUB_NUM );
	sNetMsgClt.dwCLUB_TAR_NUM = pNetMsg->dwCLUB_B_NUM;
	memcpy ( sNetMsgClt.dwCONFRONT_TAR_MEMBER, pNetMsg->dwCLUB_B_MEMBER, sizeof(DWORD)*EMMAX_CLUB_NUM );
	sNetMsgClt.vPosition = vPosition;
	sNetMsgClt.sOption = pNetMsg->sOption;

	StringCchCopy ( sNetMsgClt.szName, sizeof(char)*(CHR_ID_LENGTH+1), pClubB->m_szName );

	DWORD i(0);
	DWORD dwMember = 0;
	GLChar* pChar = NULL;
	for ( i=0; i<pNetMsg->dwCLUB_A_NUM; ++i )
	{
		dwMember = pNetMsg->dwCLUB_A_MEMBER[i];
		pChar = GetCharID ( dwMember );
		if ( !pChar )
		{
			continue;
		}
		else
		{
			pClubA->ADDCONFT_MEMBER ( dwMember );

			pChar->m_sCONFTING.RESET();
			pChar->m_sCONFTING.emTYPE = EMCONFT_GUILD;
			pChar->m_sCONFTING.dwTAR_ID = pClubA->m_dwconftCLUB;
			pChar->m_sCONFTING.vPosition = vPosition;
			pChar->m_sCONFTING.sOption = pNetMsg->sOption;

			//	Note : Ŭ���̾�Ʈ�鿡 'Ŭ�����' ���� ����.
			//
			SENDTOCLIENT ( pChar->m_dwClientID, &sNetMsgClt );

			//	Note : ���������� ����.
			//
			pChar->DelPlayHostile();

			//	Note : ��� ü�� ���.
			//
			pChar->DoConftStateBackup();
		}
	}
	pClubA->SETCONFT_MEMBER ();

	//	Note : �ι��� Ŭ�����鿡 'Ŭ�����' ���� ����.
	//
	pClubB->m_conftOPT = pNetMsg->sOption;
	pClubB->m_dwconftCLUB = pNetMsg->dwCLUB_A;

	sNetMsgClt.dwconftCLUB = pNetMsg->dwCLUB_A;

	sNetMsgClt.dwCLUB_MY_NUM = pNetMsg->dwCLUB_B_NUM;
	memcpy ( sNetMsgClt.dwCONFRONT_MY_MEMBER, pNetMsg->dwCLUB_B_MEMBER, sizeof(DWORD)*EMMAX_CLUB_NUM );
	sNetMsgClt.dwCLUB_TAR_NUM = pNetMsg->dwCLUB_A_NUM;
	memcpy ( sNetMsgClt.dwCONFRONT_TAR_MEMBER, pNetMsg->dwCLUB_A_MEMBER, sizeof(DWORD)*EMMAX_CLUB_NUM );
	sNetMsgClt.vPosition = vPosition;
	sNetMsgClt.sOption = pNetMsg->sOption;

	StringCchCopy ( sNetMsgClt.szName, sizeof(char)*(CHR_ID_LENGTH+1), pClubA->m_szName );

	for ( i=0; i<pNetMsg->dwCLUB_B_NUM; ++i )
	{
		dwMember = pNetMsg->dwCLUB_B_MEMBER[i];
		pChar = GetCharID ( dwMember );
		if ( !pChar )					
		{
			continue;
		}
		else
		{
			pClubB->ADDCONFT_MEMBER ( dwMember );
			
			pChar->m_sCONFTING.RESET();
			pChar->m_sCONFTING.emTYPE = EMCONFT_GUILD;
			pChar->m_sCONFTING.dwTAR_ID = pClubB->m_dwconftCLUB;
			pChar->m_sCONFTING.vPosition = vPosition;
			pChar->m_sCONFTING.sOption = pNetMsg->sOption;

			//	Note : Ŭ���̾�Ʈ�鿡 'Ŭ�����' ���� ����.
			//
			SENDTOCLIENT ( pChar->m_dwClientID, &sNetMsgClt );

			//	Note : ���������� ����.
			//
			pChar->DelPlayHostile();

			//	Note : ��� ü�� ���.
			//
			pChar->DoConftStateBackup();
		}
	}
	pClubB->SETCONFT_MEMBER ();

	return TRUE;
}

BOOL GLGaeaServer::RequestConfrontClubEnd ( GLMSG::SNETPC_CONFRONTCLB_END2_FLD *pNetMsg )
{
	if ( pNetMsg->dwCLUB_A!=CLUB_NULL && pNetMsg->dwCLUB_B!=CLUB_NULL )
	{
		GLCLUB *pClubA = m_cClubMan.GetClub(pNetMsg->dwCLUB_A);
		GLCLUB *pClubB = m_cClubMan.GetClub(pNetMsg->dwCLUB_B);

		if ( pClubA && pClubB )
		{
			float fLIFE_P_A(0), fLIFE_P_B(0);

			if ( pNetMsg->emEND_A==EMCONFRONT_END_CWIN )
			{
				fLIFE_P_A = (float) GLCONST_CHAR::nCONFRONT_CLB_WIN_LP;
			}
			else if ( pNetMsg->emEND_A==EMCONFRONT_END_CLOSS )
			{
				fLIFE_P_A = (float) GLCONST_CHAR::nCONFRONT_CLB_LOSS_LP;
			}

			if ( pNetMsg->emEND_B==EMCONFRONT_END_CWIN )
			{
				fLIFE_P_B = (float) GLCONST_CHAR::nCONFRONT_CLB_WIN_LP;
			}
			else if ( pNetMsg->emEND_B==EMCONFRONT_END_CLOSS )
			{
				fLIFE_P_B = (float) GLCONST_CHAR::nCONFRONT_CLB_LOSS_LP;
			}

			{
				CLUBCONFT_ITER pos = pClubA->m_setConftSTART.begin();
				CLUBCONFT_ITER end = pClubA->m_setConftSTART.end();
				for ( ; pos!=end; ++pos )
				{
					PGLCHAR pCHAR = GetCharID ( *pos );
					if ( pCHAR )
					{
						pCHAR->ReceiveLivingPoint ( (int)fLIFE_P_A );

						if ( pCHAR->m_sCONFTING.IsCONFRONTING() )
						{
							if ( pCHAR->m_sCONFTING.emTYPE==EMCONFT_GUILD )
							{
								// ������� �� �����ð� ����
								pCHAR->DoConftStateRestore();
								pCHAR->m_sCONFTING.RESET();
								pCHAR->m_sCONFTING.SETPOWERFULTIME ( 10.0f ); 
							}
						}
					}
				}
			}

			{
				CLUBCONFT_ITER pos = pClubB->m_setConftSTART.begin();
				CLUBCONFT_ITER end = pClubB->m_setConftSTART.end();
				for ( ; pos!=end; ++pos )
				{
					PGLCHAR pCHAR = GetCharID ( *pos );
					if ( pCHAR )
					{
						pCHAR->ReceiveLivingPoint ( (int)fLIFE_P_B );

						if ( pCHAR->m_sCONFTING.IsCONFRONTING() )
						{
							if ( pCHAR->m_sCONFTING.emTYPE==EMCONFT_GUILD )
							{
								// ������� �� �����ð� ����
								pCHAR->DoConftStateRestore();
								pCHAR->m_sCONFTING.RESET();
								pCHAR->m_sCONFTING.SETPOWERFULTIME ( 10.0f ); 
							}
						}
					}
				}
			}
		}

		if ( pClubA )
		{
			//	Note : Ŭ�� ��� ���Ḧ Ŭ���̾�Ʈ�鿡 �˸�.
			//
			GLMSG::SNETPC_CONFRONTPTY_END2_CLT NetMsgClt;
			NetMsgClt.emEND = pNetMsg->emEND_A;
			
			GLGaeaServer::GetInstance().SENDTOCLUBCLIENT ( pClubA->m_dwID, &NetMsgClt );

			if ( pNetMsg->emEND_A==EMCONFRONT_END_PWIN || pNetMsg->emEND_A==EMCONFRONT_END_PLOSS )
			{
				GLMSG::SNETPC_CONFRONT_END2_CLT_BRD NetMsgBrd;
				NetMsgBrd.emEND = pNetMsg->emEND_A;

				CLUBMEMBERS_ITER pos = pClubA->m_mapMembers.begin();
				CLUBMEMBERS_ITER end = pClubA->m_mapMembers.end();
				for ( ; pos!=end; ++pos )
				{
					PGLCHAR pCHAR = GetCharID ( (*pos).first );
					if( !pCHAR ) continue;

					NetMsgBrd.dwGaeaID = pCHAR->m_dwGaeaID;
					pCHAR->SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
				}
			}

			pClubA->CLEARCONFT();
		}

		if ( pClubB )
		{
			//	Note : Ŭ�� ��� ���Ḧ Ŭ���̾�Ʈ�鿡 �˸�.
			//
			GLMSG::SNETPC_CONFRONTPTY_END2_CLT NetMsgClt;
			NetMsgClt.emEND = pNetMsg->emEND_B;
			
			GLGaeaServer::GetInstance().SENDTOCLUBCLIENT ( pClubB->m_dwID, &NetMsgClt );

			if ( pNetMsg->emEND_B==EMCONFRONT_END_PWIN || pNetMsg->emEND_B==EMCONFRONT_END_PLOSS )
			{
				GLMSG::SNETPC_CONFRONT_END2_CLT_BRD NetMsgBrd;
				NetMsgBrd.emEND = pNetMsg->emEND_B;

				CLUBMEMBERS_ITER pos = pClubB->m_mapMembers.begin();
				CLUBMEMBERS_ITER end = pClubB->m_mapMembers.end();
				for ( ; pos!=end; ++pos )
				{
					PGLCHAR pCHAR = GetCharID ( (*pos).first );
					if( !pCHAR ) continue;

					NetMsgBrd.dwGaeaID = pCHAR->m_dwGaeaID;
					pCHAR->SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
				}
			}

			pClubB->CLEARCONFT();
		}
	}

	return TRUE;
}

BOOL GLGaeaServer::ServerClubDeathMatchReady ( GLMSG::SNET_CLUB_DEATHMATCH_READY_FLD *pNetMsg )
{
	GLClubDeathMatchFieldMan::GetInstance().ReadyBattle ( pNetMsg->dwID );

	return TRUE;}

BOOL GLGaeaServer::ServerClubDeathMatchStart ( GLMSG::SNET_CLUB_DEATHMATCH_START_FLD *pNetMsg )
{
	m_bClubDMStarted = true;
	GLClubDeathMatchFieldMan::GetInstance().BeginBattle ( pNetMsg->dwID );

	return TRUE;
}

BOOL GLGaeaServer::ServerClubDeathMatchEnd ( GLMSG::SNET_CLUB_DEATHMATCH_END_FLD *pNetMsg )
{
	m_bClubDMStarted = false;
	GLClubDeathMatchFieldMan::GetInstance().EndBattle ( pNetMsg->dwID );

	return TRUE;
}

BOOL GLGaeaServer::ServerClubBattleStart ( GLMSG::SNET_CLUB_BATTLE_START_FLD *pNetMsg )
{
	m_bClubBattleStarted = true;
	GLGuidanceFieldMan::GetInstance().BeginBattle ( pNetMsg->dwID );

	return TRUE;
}

BOOL GLGaeaServer::ServerClubBattleEnd ( GLMSG::SNET_CLUB_BATTLE_END_FLD *pNetMsg )
{
	m_bClubBattleStarted = false;
	GLGuidanceFieldMan::GetInstance().EndBattle ( pNetMsg->dwID );

	return TRUE;
}

BOOL GLGaeaServer::ServerClubCertify ( GLMSG::SNET_CLUB_CERTIFIED_FLD *pNetMsg )
{
	GLGuidanceFieldMan::GetInstance().ChangeGuidClub ( pNetMsg->dwID, pNetMsg->dwCLUBID );

	return TRUE;
}

BOOL GLGaeaServer::ServerClubGuidCommission ( GLMSG::SNET_CLUB_GUID_COMMISSION_FLD *pNetMsg )
{
	GLGuidanceFieldMan::GetInstance().ChangeCommission ( pNetMsg->dwID, pNetMsg->fCommission );

	return TRUE;
}

BOOL GLGaeaServer::ServerClubNotice ( GLMSG::SNET_CLUB_NOTICE_FLD *pNetMsg )
{
	GLCLUB* pCLUB = m_cClubMan.GetClub ( pNetMsg->dwCLUB_ID );
	if ( !pCLUB )		return FALSE;

	StringCchCopy ( pCLUB->m_szNotice, EMCLUB_NOTICE_LEN+1, pNetMsg->szNotice );

	//	Note : Ŭ���� ������ �ʵ� ������ �ݿ�.
	GLMSG::SNET_CLUB_NOTICE_CLT NetMsgClt;
	StringCchCopy ( NetMsgClt.szNotice, EMCLUB_NOTICE_LEN+1, pNetMsg->szNotice );
	GLGaeaServer::GetInstance().SENDTOCLUBCLIENT ( pCLUB->m_dwID, &NetMsgClt );

	return TRUE;
}

BOOL GLGaeaServer::ServerClubStorageGetDB ( GLMSG::SNET_CLUB_STORAGE_GET_DB *pNetMsg )
{
	if ( !pNetMsg->m_pStream )						return FALSE;

	GLCLUB *pCLUB(NULL);
	PGLCHAR pCHAR(NULL);

	pCLUB = GetClubMan().GetClub ( pNetMsg->m_dwClubID );
	if ( !pCLUB )									goto  _END;

	pCHAR = GetCharID ( pNetMsg->m_dwCharID );
	if ( !pCHAR )									goto _END;

	if ( pCHAR->m_dwGuild != pCLUB->m_dwID )		goto _END;
	if ( pCHAR->m_dwCharID != pCLUB->m_dwMasterID )	goto _END;

	pCLUB->m_bVALID_STORAGE = TRUE;
	pCLUB->m_lnStorageMoney = pNetMsg->m_lnStorageMoney;
	pCLUB->SETSTORAGE_BYBUF ( *pNetMsg->m_pStream );

	pCHAR->MsgClubStorage ();

_END:
	SAFE_DELETE(pNetMsg->m_pStream);

	return TRUE;
}

BOOL GLGaeaServer::ServerClubInComeDn ( DWORD dwClientID, GLMSG::SNET_CLUB_INCOME_DN *pNetMsg )
{
	if ( dwClientID != m_dwAgentSlot )				return FALSE;

	GLCLUB *pCLUB = GetClubMan().GetClub ( pNetMsg->dwClubID );
	if ( !pCLUB )									return FALSE;

	PGLCHAR pCHAR = GetCharID ( pCLUB->m_dwMasterID );
	if ( !pCHAR )									return FALSE;
	if ( !pCLUB->m_bVALID_STORAGE )					return FALSE;

	pCLUB->m_lnStorageMoney += pNetMsg->lnInComeMoney;

	GLITEMLMT::GetInstance().ReqMoneyExc (	ID_CLUB, 0, 
											ID_CLUB, pNetMsg->dwClubID, 
											pNetMsg->lnInComeMoney, 
											EMITEM_ROUTE_CLUBINCOME );

	CDebugSet::ToFile ( "club_income_gaea.txt", "ServerClubInComeDn, ClubID[%u], MasterID[%u], IncomeMoney[%I64d]",
						pNetMsg->dwClubID, pCLUB->m_dwMasterID, pNetMsg->lnInComeMoney );

	//	Note : ��� ����.
	//GLMSG::SNET_CLUB_INCOME_FB NetMsgFB;
	//NetMsgFB.dwClubID = pCLUB->m_dwID;
	//NetMsgFB.lnInComeMoney = pNetMsg->lnInComeMoney;
	//SENDTOAGENT ( &NetMsgFB );

	//	Note : Ŭ���忡�� �뺸.
	GLMSG::SNET_CLUB_STORAGE_UPDATE_MONEY	NetMsgClt;
	NetMsgClt.lnMoney = pCLUB->m_lnStorageMoney;
	SENDTOCLIENT ( pCHAR->m_dwClientID, &NetMsgClt );

	return TRUE;
}

BOOL GLGaeaServer::ServerClubAllianceAddFld ( GLMSG::SNET_CLUB_ALLIANCE_ADD_FLD *pNetMsg )
{
	GLCLUB *pCLUB_CHIEF = GetClubMan().GetClub ( pNetMsg->dwChiefClubID );
	if ( !pCLUB_CHIEF )									return FALSE;

	GLCLUB *pCLUB_INDIAN = GetClubMan().GetClub ( pNetMsg->dwIndianClubID );
	if ( !pCLUB_INDIAN )								return FALSE;

	//	���� �Ἲ �� ���� Ŭ�� �߰�.
	pCLUB_CHIEF->m_dwAlliance = pCLUB_CHIEF->m_dwID;
	pCLUB_INDIAN->m_dwAlliance = pCLUB_CHIEF->m_dwID;
	
	pCLUB_INDIAN->m_dwAllianceBattleWin = pCLUB_CHIEF->m_dwAllianceBattleWin;
	pCLUB_INDIAN->m_dwAllianceBattleLose = pCLUB_CHIEF->m_dwAllianceBattleLose;
	pCLUB_INDIAN->m_dwAllianceBattleDraw = pCLUB_CHIEF->m_dwAllianceBattleDraw;

	pCLUB_CHIEF->AddAlliance ( pCLUB_CHIEF->m_dwID, pCLUB_CHIEF->m_szName );
	pCLUB_CHIEF->AddAlliance ( pCLUB_INDIAN->m_dwID, pCLUB_INDIAN->m_szName );
	//pCLUB_INDIAN->AddAlliance ( pCLUB_CHIEF->m_dwID, pCLUB_CHIEF->m_szName );
	//pCLUB_INDIAN->AddAlliance ( pCLUB_INDIAN->m_dwID, pCLUB_INDIAN->m_szName );

/*
	if (pCLUB_CHIEF->AddAlliance ( pCLUB_CHIEF->m_dwID, pCLUB_CHIEF->m_szName ) == true)
	{
		if (pCLUB_CHIEF->AddAlliance ( pCLUB_INDIAN->m_dwID, pCLUB_INDIAN->m_szName ) == true)
		{
			//	�߰��Ǵ� Ŭ���� ���� ����.
			pCLUB_INDIAN->m_dwAlliance = pCLUB_CHIEF->m_dwID;
			return TRUE;
		} // if (pCLUB_CHIEF->AddAlliance ( pCLUB_INDIAN->m_dwID, pCLUB_INDIAN->m_szName ) == true)
		else
		{
			return FALSE;
		}
	} // if (pCLUB_CHIEF->AddAlliance ( pCLUB_CHIEF->m_dwID, pCLUB_CHIEF->m_szName ) == true)
	else
	{
		return FALSE;
	}
*/
	return TRUE;
}

BOOL GLGaeaServer::ServerClubAllianceDel ( GLMSG::SNET_CLUB_ALLIANCE_DEL_FLD *pNetMsg )
{
	GLCLUB *pCLUB_CHIEF = GetClubMan().GetClub ( pNetMsg->dwAlliance );
	if ( !pCLUB_CHIEF )									return FALSE;

	GLCLUB *pCLUB_INDIAN = GetClubMan().GetClub ( pNetMsg->dwDelClubID );
	if ( !pCLUB_INDIAN )								return FALSE;

	CLUB_ALLIANCE_ITER del = pCLUB_CHIEF->m_setAlliance.find ( GLCLUBALLIANCE(pNetMsg->dwDelClubID,"") );
	if ( pCLUB_CHIEF->m_setAlliance.end()==del )		return FALSE;

	//	���Ϳ��� ����.
	pCLUB_CHIEF->m_setAlliance.erase ( del );

	//	���ܵǴ� Ŭ�� ���� ����.
	pCLUB_INDIAN->ClearAlliance();

	return TRUE;
}

BOOL GLGaeaServer::ServerClubAllianceDis ( GLMSG::SNET_CLUB_ALLIANCE_DIS_FLD *pNetMsg )
{
	GLCLUB *pCLUB_CHIEF = GetClubMan().GetClub ( pNetMsg->dwChiefClubID );
	if ( !pCLUB_CHIEF )									return FALSE;

	//	���͵鿡�� ���� ��ü �˸�.
	CLUB_ALLIANCE_ITER pos = pCLUB_CHIEF->m_setAlliance.begin();
	CLUB_ALLIANCE_ITER end = pCLUB_CHIEF->m_setAlliance.end();
	for ( ; pos!=end; ++pos )
	{
		const GLCLUBALLIANCE &sALLIANCE = *pos;
		if ( sALLIANCE.m_dwID==pCLUB_CHIEF->m_dwID )	continue;	//	Ŭ������ ���߿� ����.

		GLCLUB *pCLUB_IND = GetClubMan().GetClub ( sALLIANCE.m_dwID );
		if ( !pCLUB_IND )	continue;

		pCLUB_IND->ClearAlliance();
	}

	//	Ŭ������ ���� ���� ����.
	pCLUB_CHIEF->ClearAlliance();

	return TRUE;
}

BOOL GLGaeaServer::ServerClubBattleBeginFld ( GLMSG::SNET_CLUB_BATTLE_BEGIN_FLD *pNetMsg )
{
	GLCLUB *pCLUB_CHIEF = GetClubMan().GetClub ( pNetMsg->dwChiefClubID );
	if ( !pCLUB_CHIEF )									return FALSE;

	GLCLUB *pCLUB_INDIAN = GetClubMan().GetClub ( pNetMsg->dwIndianClubID );
	if ( !pCLUB_INDIAN )								return FALSE;

	GLCLUBBATTLE sClubBattle;
	
	sClubBattle.m_tStartTime = pNetMsg->tStartTime;
	sClubBattle.m_tEndTime = pNetMsg->tEndTime;
	sClubBattle.m_bAlliance = pNetMsg->bAlliance;

	if( pNetMsg->bAlliance )
	{
		sClubBattle.m_dwCLUBID = pNetMsg->dwIndianClubID;
		StringCchCopy ( sClubBattle.m_szClubName, CHAR_SZNAME, pCLUB_INDIAN->m_szName );

		CLUB_ALLIANCE_ITER pos = pCLUB_CHIEF->m_setAlliance.begin();
		CLUB_ALLIANCE_ITER end = pCLUB_CHIEF->m_setAlliance.end();
		for ( ; pos!=end; ++pos )
		{
			const GLCLUBALLIANCE &sALLIANCE = *pos;

			GLCLUB *pCLUB = GetClubMan().GetClub ( sALLIANCE.m_dwID );
			if ( !pCLUB ) continue;

			pCLUB->ADDBATTLECLUB( sClubBattle );
		}

		
		sClubBattle.m_dwCLUBID = pNetMsg->dwChiefClubID;
		StringCchCopy ( sClubBattle.m_szClubName, CHAR_SZNAME, pCLUB_CHIEF->m_szName );	

		pos = pCLUB_INDIAN->m_setAlliance.begin();
		end = pCLUB_INDIAN->m_setAlliance.end();
		for ( ; pos!=end; ++pos )
		{
			const GLCLUBALLIANCE &sALLIANCE = *pos;

			GLCLUB *pCLUB = GetClubMan().GetClub ( sALLIANCE.m_dwID );
			if ( !pCLUB ) continue;

			pCLUB->ADDBATTLECLUB( sClubBattle );
		}

	}
	else
	{
		sClubBattle.m_dwCLUBID = pNetMsg->dwIndianClubID;
		StringCchCopy ( sClubBattle.m_szClubName, CHAR_SZNAME, pCLUB_INDIAN->m_szName );
		pCLUB_CHIEF->ADDBATTLECLUB( sClubBattle );

		sClubBattle.m_dwCLUBID = pNetMsg->dwChiefClubID;
		StringCchCopy ( sClubBattle.m_szClubName, CHAR_SZNAME, pCLUB_CHIEF->m_szName );	
		pCLUB_INDIAN->ADDBATTLECLUB( sClubBattle );
	}	

	return TRUE;
}

BOOL GLGaeaServer::ServerClubBattleOverFld ( GLMSG::SNET_CLUB_BATTLE_OVER_FLD *pNetMsg )
{
	GLCLUB *pCLUB_CHIEF = GetClubMan().GetClub ( pNetMsg->dwWinClubID );
	if ( !pCLUB_CHIEF )									return FALSE;

	GLCLUB *pCLUB_INDIAN = GetClubMan().GetClub ( pNetMsg->dwLoseClubID );
	if ( !pCLUB_INDIAN )								return FALSE;

	GLCLUBBATTLE* pClubBattle = pCLUB_CHIEF->GetClubBattle( pNetMsg->dwLoseClubID );
	if ( !pClubBattle ) return FALSE;

	if ( pClubBattle->m_bAlliance ) 
	{

		// Ŭ�� ��Ʋ ���� ���� ���� ����
		DelPlayHostileAllianceBattle( pNetMsg->dwWinClubID, pNetMsg->dwLoseClubID );

		// �ڽ� Ŭ��
		CLUB_ALLIANCE_ITER pos = pCLUB_CHIEF->m_setAlliance.begin();
		CLUB_ALLIANCE_ITER end = pCLUB_CHIEF->m_setAlliance.end();
		for ( ; pos!=end; ++pos )
		{
			const GLCLUBALLIANCE &sALLIANCE = *pos;
			GLCLUB *pCLUB = GetClubMan().GetClub ( sALLIANCE.m_dwID );
			if ( !pCLUB ) continue;		
		
			switch( pNetMsg->emFB )
			{
				case EMCLUB_BATTLE_OVER_DRAW:	pCLUB->m_dwAllianceBattleDraw++;			break;
				case EMCLUB_BATTLE_OVER_WIN:	pCLUB->m_dwAllianceBattleWin++;		break;
				case EMCLUB_BATTLE_OVER_LOSE:										break;
				case EMCLUB_BATTLE_OVER_ARMISTICE:	pCLUB->m_dwAllianceBattleDraw++;	break;
				case EMCLUB_BATTLE_OVER_SUBMISSION:	pCLUB->m_dwAllianceBattleWin++;	break;
				case EMCLUB_BATTLE_OVER_TARSUBMISSION:								break;
			}

			pCLUB->DELBATTLECLUB( pNetMsg->dwLoseClubID );
		}


		// �ڽ� Ŭ��
		pos = pCLUB_INDIAN->m_setAlliance.begin();
		end = pCLUB_INDIAN->m_setAlliance.end();
		for ( ; pos!=end; ++pos )
		{
			const GLCLUBALLIANCE &sALLIANCE = *pos;
			GLCLUB *pCLUB = GetClubMan().GetClub ( sALLIANCE.m_dwID );
			if ( !pCLUB ) continue;		
		
			switch( pNetMsg->emFB )
			{
				case EMCLUB_BATTLE_OVER_DRAW:	pCLUB->m_dwAllianceBattleDraw++;			break;
				case EMCLUB_BATTLE_OVER_WIN:	pCLUB->m_dwAllianceBattleLose++;		break;
				case EMCLUB_BATTLE_OVER_LOSE:										break;
				case EMCLUB_BATTLE_OVER_ARMISTICE:	pCLUB->m_dwAllianceBattleDraw++;	break;
				case EMCLUB_BATTLE_OVER_SUBMISSION:	pCLUB->m_dwAllianceBattleLose++;	break;
				case EMCLUB_BATTLE_OVER_TARSUBMISSION:								break;
			}

			pCLUB->DELBATTLECLUB( pNetMsg->dwWinClubID );
		}


	}
	else
	{
		switch( pNetMsg->emFB )
		{
			case EMCLUB_BATTLE_OVER_DRAW:
				{
					pCLUB_CHIEF->m_dwBattleDraw++;
					pCLUB_INDIAN->m_dwBattleDraw++;
				}
				break;
			case EMCLUB_BATTLE_OVER_WIN:
				{
					pCLUB_CHIEF->m_dwBattleWin++;
					pCLUB_INDIAN->m_dwBattleLose++;
				}
				break;
			case EMCLUB_BATTLE_OVER_LOSE:			
				break;
			case EMCLUB_BATTLE_OVER_ARMISTICE:
				{
					pCLUB_CHIEF->m_dwBattleDraw++;
					pCLUB_INDIAN->m_dwBattleDraw++;
				}
				break;
			case EMCLUB_BATTLE_OVER_SUBMISSION:
				{
					pCLUB_CHIEF->m_dwBattleWin++;
					pCLUB_INDIAN->m_dwBattleLose++;
				}
				break;
			case EMCLUB_BATTLE_OVER_TARSUBMISSION:
				break;
		}


		// Ŭ�� ��Ʋ ���� ���� ���� ����
		DelPlayHostileClubBattle( pNetMsg->dwWinClubID, pNetMsg->dwLoseClubID );

		pCLUB_CHIEF->DELBATTLECLUB( pNetMsg->dwLoseClubID );
		pCLUB_INDIAN->DELBATTLECLUB( pNetMsg->dwWinClubID );	

	}


	return TRUE;

}

BOOL GLGaeaServer::ServerClubBattleKillUpdate( GLMSG::SNET_CLUB_BATTLE_KILL_UPDATE_FLD* pNetMsg )
{
	GLCLUB *pMyClub = GetClubMan().GetClub ( pNetMsg->dwClubID );
	if ( !pMyClub )	return FALSE;

	GLCLUBBATTLE* pBattle = pMyClub->GetClubBattle( pNetMsg->dwBattleClubID );
	if ( !pBattle ) return FALSE;

	pBattle->m_wKillPoint = pNetMsg->wKillPoint;
	pBattle->m_wDeathPoint = pNetMsg->wDeathPoint;

	return TRUE;
}

BOOL GLGaeaServer::ServerClubBattleLastKillUpdate( GLMSG::SNET_CLUB_BATTLE_LAST_KILL_UPDATE_FLD *pNetMsg )
{
	GLCLUB *pMyClub = GetClubMan().GetClub ( pNetMsg->dwClubID_A );
	if ( !pMyClub )	return FALSE;

	GLCLUB *pTarClub = GetClubMan().GetClub ( pNetMsg->dwClubID_B );
	if ( !pTarClub ) return FALSE;

	GLCLUBBATTLE* pBattle = pMyClub->GetClubBattle( pNetMsg->dwClubID_B );
	if ( !pBattle ) return FALSE;

	GLMSG::SNET_CLUB_BATTLE_LAST_KILL_UPDATE_AGT NetMsg;
	NetMsg.dwClubID_A = pNetMsg->dwClubID_A;
	NetMsg.dwClubID_B = pNetMsg->dwClubID_B;
	NetMsg.wKillPoint = pBattle->m_wKillPointTemp;
	NetMsg.wDeathPoint = pBattle->m_wDeathPointTemp;

	SENDTOAGENT( &NetMsg );

	return TRUE;
}

BOOL GLGaeaServer::ServerLevelEventEnd( GLMSG::SNET_LEVEL_EVENT_END_FLD *pNetMsg )
{
	SNATIVEID nidMAP( pNetMsg->dwMapID );
	GLLandMan *pLandMan = GetByMapID( nidMAP );
	if( pLandMan )
	{
		SNATIVEID nidMAP( pNetMsg->dwHallMapID );

		GLMapList::FIELDMAP_ITER iter = m_MapList.find ( nidMAP.dwID );
		if( iter==m_MapList.end() ) return FALSE;

		const SMAPNODE *pMapNode = &(*iter).second;

		GLMapAxisInfo sMapAxisInfo;
		sMapAxisInfo.LoadFile ( pMapNode->strFile.c_str() );

		GLMSG::SNETPC_REQ_RECALL_AG NetMsgAg;
		NetMsgAg.sMAPID = nidMAP;
		NetMsgAg.dwGATEID = UINT_MAX;
		
		GLCHARNODE* pCharNode = pLandMan->m_GlobPCList.m_pHead;
		for ( ; pCharNode; pCharNode = pCharNode->pNext )
		{
			PGLCHAR pChar = pCharNode->Data;
			if( !pChar ) continue;

			int nRandNum = rand() % MAX_EVENT_POS;
			GLLEV_POS levPos = pNetMsg->levPos[nRandNum];

			D3DXVECTOR3 vPOS(0,0,0); 
			sMapAxisInfo.MapPos2MiniPos ( levPos.m_dwPOS_X, levPos.m_dwPOS_Y, vPOS.x, vPOS.z );

			NetMsgAg.vPOS = vPOS;

			SENDTOAGENT( pChar->m_dwClientID, &NetMsgAg );

			// ���� �������ش�
			DropOutPET ( pChar->m_dwPetGUID, true, true );
			//DropOutSummon ( pChar->m_dwSummonGUID, true );
			SaveVehicle( pChar->m_dwClientID, pChar->m_dwGaeaID, true );

			/*skill summon, Juver, 2017/10/09 */
			for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
				DropOutSummon ( pChar->m_dwSummonGUID_FLD[i] );
		}
	}

	return TRUE;
}

BOOL GLGaeaServer::ServerLevelEventWarning( GLMSG::SNET_LEVEL_EVENT_WARNING_FLD * pNetMsg )
{
	SNATIVEID nidMAP( pNetMsg->dwMapID );
	GLLandMan *pLandMan = GetByMapID( nidMAP );
	if( pLandMan )
	{
		GLMSG::SNET_GM_WARNING_MSG_BRD NetMsgBrd;
		NetMsgBrd.bOn = pNetMsg->bOn;

		GLCHARNODE* pCharNode = pLandMan->m_GlobPCList.m_pHead;
		for ( ; pCharNode; pCharNode = pCharNode->pNext )
		{
			PGLCHAR pChar = pCharNode->Data;
			if( !pChar ) continue;

			SENDTOCLIENT( pChar->m_dwClientID, &NetMsgBrd );
		}
	}

	return TRUE;
}

BOOL GLGaeaServer::ServerLevelEventCountdown( GLMSG::SNET_LEVEL_EVENT_COUNTDOWN_FLD * pNetMsg )
{
	SNATIVEID nidMAP( pNetMsg->dwMapID );
	GLLandMan *pLandMan = GetByMapID( nidMAP );
	if( pLandMan )
	{
		GLMSG::SNET_GM_COUNTDOWN_MSG_BRD NetMsgBrd;
		NetMsgBrd.nCount = pNetMsg->nCount;

		GLCHARNODE* pCharNode = pLandMan->m_GlobPCList.m_pHead;
		for ( ; pCharNode; pCharNode = pCharNode->pNext )
		{
			PGLCHAR pChar = pCharNode->Data;
			if( !pChar ) continue;

			SENDTOCLIENT( pChar->m_dwClientID, &NetMsgBrd );
		}
	}

	return TRUE;
}

BOOL GLGaeaServer::RequestChargedItem2Inven ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_CHARGED_ITEM2_INVEN *pNetMsg )
{
	SITEM *pITEM(NULL);
	WORD wPosX(0), wPosY(0);
	SITEMCUSTOM sITEM_NEW;
	CTime cTIME = CTime::GetCurrentTime();
	MAPSHOP_ITER pos;
	GLMSG::SNET_CHARGED_ITEM_DEL NetMsgDel;
	GLMSG::SNETPC_INVEN_INSERT NetMsgInven;

	GLChar *pChar = GetChar(dwGaeaID);
	if ( !pChar )									goto _DB_RESTORE;

	if ( pNetMsg->dwUserID != pChar->GetUserID() )	goto _DB_RESTORE;

	pITEM = GLItemMan::GetInstance().GetItem ( pNetMsg->nidITEM );
	if ( !pITEM )									goto _DB_RESTORE;

	BOOL bOK = pChar->m_cInventory.FindInsrtable ( pITEM->sBasicOp.wInvenSizeX, pITEM->sBasicOp.wInvenSizeY, wPosX, wPosY );
	if ( !bOK )										goto _DB_RESTORE;

	//	������ �߻�.
	sITEM_NEW.sNativeID = pNetMsg->nidITEM;
	sITEM_NEW.tBORNTIME = cTIME.GetTime();

	//	�������� ��� Ƚ�� ǥ��. ( �Ҹ�ǰ�� ��� x ��, �Ϲ� ��ǰ 1 )
	sITEM_NEW.wTurnNum = pITEM->GETAPPLYNUM();

	// ������ ����Ʈ���� �Է��� ���� ��� ���� ( ���� )
	sITEM_NEW.cDAMAGE = (BYTE)pITEM->sBasicOp.wGradeAttack;
	sITEM_NEW.cDEFENSE = (BYTE)pITEM->sBasicOp.wGradeDefense;

	// �Ϻ������� ���� �����ۿ� ���� �ɼ� ���� ( ���� )
#ifdef JP_PARAM
	if( sITEM_NEW.GENERATE_RANDOM_OPT() )
	{
		GLITEMLMT::GetInstance().ReqRandomItem( sITEM_NEW );
	}
#endif

	//	�߻��� ���� ���.
	sITEM_NEW.cGenType = EMGEN_BILLING;
	sITEM_NEW.cChnID = (BYTE)GLGaeaServer::GetInstance().GetServerChannel();
	sITEM_NEW.cFieldID = (BYTE)GLGaeaServer::GetInstance().GetFieldSvrID();
	sITEM_NEW.lnGenNum = GLITEMLMT::GetInstance().RegItemGen ( sITEM_NEW.sNativeID, EMGEN_BILLING );

	/*item color, Juver, 2018/01/08 */
	SITEM* pitem_data_check = GLItemMan::GetInstance().GetItem( sITEM_NEW.sNativeID );
	if ( pitem_data_check && pitem_data_check->sBasicOp.bItemColor )
	{
		sITEM_NEW.wColor1 = pitem_data_check->sBasicOp.wItemColor1;
		sITEM_NEW.wColor2 = pitem_data_check->sBasicOp.wItemColor2;
	}

	pChar->m_cInventory.InsertItem ( sITEM_NEW, wPosX, wPosY );

	/*activity system, Juver, 2017/11/03 */
	pChar->DoActivityTakeItem( sITEM_NEW.sNativeID, sITEM_NEW.wTurnNum );

	//	�ι꿡 ������ �־��ִ� �޽���.
	//
	SINVENITEM *pINVENITEM = pChar->m_cInventory.GetItem ( wPosX, wPosY );
	if ( !pINVENITEM ) return FALSE;
	NetMsgInven.Data = *pINVENITEM;
	GLGaeaServer::GetInstance().SENDTOCLIENT(pChar->m_dwClientID,&NetMsgInven);

	//	Note :�������� ���� ���� ��� ���.
	//
	GLITEMLMT::GetInstance().ReqItemRoute ( sITEM_NEW, ID_CHAR, 0, ID_CHAR, pChar->m_dwCharID, EMITEM_ROUTE_CHAR, sITEM_NEW.wTurnNum );

	//	Note : ������ ��Ͽ��� ����.
	pos = pChar->m_mapCharged.find ( pNetMsg->szPurKey );
	if ( pos!=pChar->m_mapCharged.end() )
	{
		pChar->m_mapCharged.erase ( pos );
	}

	//	Note : Ŭ���̾�Ʈ�� ���Ե� ��� ����Ʈ���� ����.
	//
	NetMsgDel.dwID = pNetMsg->dwInvenPos;
	SENDTOCLIENT ( pChar->m_dwClientID, &NetMsgDel );

	return TRUE;

_DB_RESTORE:
	if ( GetDBMan() )
	{
		CPurchaseItem_Restore *pDbAction = new CPurchaseItem_Restore(pNetMsg->szPurKey);
		GetDBMan()->AddJob ( pDbAction );
	}

	return FALSE;
}

BOOL GLGaeaServer::RequestConftSPtyExp ( GLMSG::SNET_CONFT_SPTY_EXP_FLD *pNetMsg )
{
	GLCHARNODE *pCharNode = m_GaeaPCList.m_pHead;
	for ( ; pCharNode; pCharNode=pCharNode->pNext )
	{
		GLChar *pChar = pCharNode->Data;

		if ( pChar->m_wSchool != pNetMsg->wSCHOOL )	continue;

		int nExp = int ( pChar->m_sExperience.lnMax * pNetMsg->fEXP_RATE );
		pChar->ReceiveExp ( nExp, true );
	}

	return TRUE;
}

BOOL GLGaeaServer::RequestGenItemFieldFB ( GLMSG::SNET_REQ_GENITEM_FLD_FB * pNetMsg )
{
	GLLandMan* pLandMan = GetByMapID ( pNetMsg->sMAPID );
	if ( !pLandMan )	return FALSE;

	pLandMan->RequestGenItemFieldFB ( pNetMsg );

	return TRUE;
}

// *****************************************************
// Desc: ��ȯ�� ������ ��� ó��
// *****************************************************
BOOL GLGaeaServer::RequestInvenTeleport ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNETPC_REQ_INVEN_TELEPORT* pNetMsg )
{
	GLChar* pChar = GetChar(dwGaeaID);
	if ( !pChar )	return FALSE;

	GLMSG::SNETPC_REQ_TELEPORT_FB	NetMsgFB;

	//	�ɸ��� ���� ���°� �ƴ� ���.
	if ( !pChar->IsValidBody() )
	{
		NetMsgFB.emFB = EMREQ_TELEPORT_FB_CONDITION;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	//	��� ������ ���.
	if ( pChar->m_sCONFTING.IsCONFRONTING() )
	{
		NetMsgFB.emFB = EMREQ_TELEPORT_FB_CONDITION;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	//	�ŷ����� ���.
	if ( pChar->m_sTrade.Valid() )
	{
		NetMsgFB.emFB = EMREQ_TELEPORT_FB_CONDITION;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	//	Note : pk ����� ������ ��� �̻��� ��� ��ȯ ī���� ����� ���´�.
	//
	DWORD dwPK_LEVEL = pChar->GET_PK_LEVEL();
	if ( dwPK_LEVEL != UINT_MAX && dwPK_LEVEL>GLCONST_CHAR::dwPK_RECALL_ENABLE_LEVEL )
	{
		NetMsgFB.emFB = EMREQ_TELEPORT_FB_PK_LEVEL;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return E_FAIL;
	}

	//	�κ��� ������ Ȯ��.
	SINVENITEM *pINVENITEM = pChar->m_cInventory.FindPosItem ( pNetMsg->wPosX, pNetMsg->wPosY );
	if ( !pINVENITEM )
	{
		NetMsgFB.emFB = EMREQ_TELEPORT_FB_ITEM;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	if ( pChar->CheckCoolTime( pINVENITEM->sItemCustom.sNativeID ) )	return FALSE;

	//	������ ���� ������.
	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pINVENITEM->sItemCustom.sNativeID );
	if ( !pItem || pItem->sBasicOp.emItemType!=ITEM_TELEPORT_CARD || pItem->sDrugOp.emDrug!=ITEM_DRUG_CALL_TELEPORT )
	{
		NetMsgFB.emFB = EMREQ_TELEPORT_FB_ITEM;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	SNATIVEID sMAPID(false);
	D3DXVECTOR3 vPOS(0.0f,0.0f,0.0f);


	
	sMAPID = pItem->sBasicOp.sSubID;
//	vPOS = pChar->m_vLastCallPos;	

	if ( sMAPID==SNATIVEID(false) )
	{
		NetMsgFB.emFB = EMREQ_TELEPORT_FB_FAIL;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	GLMapList::FIELDMAP_ITER iter = m_MapList.find ( sMAPID.dwID );
	if ( iter==m_MapList.end() )
	{
		NetMsgFB.emFB = EMREQ_TELEPORT_FB_FAIL;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	//	Note : ������ȯ ��ġ ����. ( �ñͳ� �б��� ��� )
	//
	pChar->SaveLastCall ();

	//	Note : ������ȯ ��ġ DB�� ����.
	//
	CSetLastCallPos *pDbAction = new CSetLastCallPos ( pChar->m_dwCharID, pChar->m_sLastCallMapID.dwID, pChar->m_vLastCallPos );
	if ( GetDBMan() )
	{
		GetDBMan()->AddJob ( pDbAction );
	}

	const SMAPNODE *pMapNode = &(*iter).second;
	GLMapAxisInfo sMapAxisInfo;
	sMapAxisInfo.LoadFile ( pMapNode->strFile.c_str() );
	sMapAxisInfo.MapPos2MiniPos (  pItem->sBasicOp.wPosX, pItem->sBasicOp.wPosY, vPOS.x, vPOS.z );

	// �������� �˻�
	if ( pChar->m_dwUserLvl < USER_GM3 )
	{
		EMREQFAIL emReqFail(EMREQUIRE_COMPLETE);
		const SLEVEL_REQUIRE &sRequire = pMapNode->sLEVEL_REQUIRE;
		emReqFail = sRequire.ISCOMPLETE ( pChar ); 
		if ( emReqFail != EMREQUIRE_COMPLETE )
		{
			NetMsgFB.emFB = EMREQ_TELEPORT_FB_JOINCON;
			SENDTOCLIENT ( pChar->m_dwClientID, &NetMsgFB );
			
			return FALSE;
		}
	}

	// PET
	// ���̵��� Pet ����
	DropOutPET ( pChar->m_dwPetGUID, true, true );
	//DropOutSummon ( pChar->m_dwSummonGUID, true );
	SaveVehicle( pChar->m_dwClientID, pChar->m_dwGaeaID, true );

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		DropOutSummon ( pChar->m_dwSummonGUID_FLD[i] );

	//	Note : �ٸ� �ʵ� ������ ���.
	if ( pMapNode->dwFieldSID!=m_dwFieldSvrID )
	{
		GLMSG::SNETPC_REQ_TELEPORT_AG NetMsgAg;
		NetMsgAg.sMAPID = sMAPID;
		NetMsgAg.vPOS = vPOS;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pChar->m_dwClientID, &NetMsgAg );
	}
	//	Note : ���� �ʵ� ������ ���.
	else
	{
		SNATIVEID sCurMapID = pChar->m_sMapID;

		BOOL bOK = RequestInvenTeleportThisSvr ( pChar, sMAPID, vPOS );
		if ( !bOK )
		{
			NetMsgFB.emFB = EMREQ_TELEPORT_FB_FAIL;
			SENDTOCLIENT ( pChar->m_dwClientID, &NetMsgFB );
			return FALSE;
		}

		// ������ �����̻� ����
		if ( sCurMapID != pChar->m_sMapID )
		{
			for ( int i=0; i<EMBLOW_MULTI; ++i )		pChar->DISABLEBLOW ( i );
			for ( int i=0; i<SKILLFACT_SIZE; ++i )		pChar->DISABLESKEFF ( i );
			pChar->DISABLEALLLANDEFF();
		}

		pChar->ResetAction();

		//	Note : �� �̵� ������ �˸�.
		//
		NetMsgFB.emFB = EMREQ_TELEPORT_FB_OK;
		NetMsgFB.sMAPID = sMAPID;
		NetMsgFB.vPOS = pChar->m_vPos;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pChar->m_dwClientID, &NetMsgFB );
	}

	//	[�ڽſ���] �ι� ������ �Ҹ��Ŵ.
	pChar->DoDrugInvenItem ( pNetMsg->wPosX, pNetMsg->wPosY, true );

	return TRUE;
}

BOOL GLGaeaServer::RequestInvenTeleportThisSvr ( GLChar* pPC, SNATIVEID sMAPID, D3DXVECTOR3 vPOS )
{
	if ( !pPC )		return FALSE;
	
	GLLandMan* pLandMan = NULL;
	D3DXVECTOR3 vStartPos;

	pLandMan = GetByMapID ( sMAPID );
	if ( !pLandMan )	return FALSE;

	vStartPos = vPOS;	

	//	Note : �ڽ��� �� �ֺ� ���� ����.
	pPC->ResetViewAround ();

	//	Note : ���� �ʿ� �ִ� ���� ��� ����.
	//
	if ( pPC->m_pLandNode )
		pPC->m_pLandMan->m_GlobPCList.DELNODE ( pPC->m_pLandNode );

	if ( pPC->m_pQuadNode && pPC->m_pCellNode )
		pPC->m_pQuadNode->pData->m_PCList.DELNODE ( pPC->m_pCellNode );

	//	Note : ���ο� �� ��ġ�� ������̼� �ʱ�ȭ.
	//
	pPC->SetNavi ( pLandMan->GetNavi(), vStartPos );

	//	Note : ���ο� ���� ID �ο�.
	//
	pPC->m_sMapID = sMAPID;

	pPC->m_pLandMan = pLandMan;
	pPC->m_pLandNode = pLandMan->m_GlobPCList.ADDHEAD ( pPC );

	//	Note : GLLandMan�� ���� ����ϴ� �۾�.
	//			
	//RegistChar ( pPC ); --> ( GLGaeaServer::RequestLandIn ()�� ȣ��� ������ ������. )

	pPC->m_dwCeID = 0;
	pPC->m_pQuadNode = NULL;
	pPC->m_pCellNode = NULL;

	return TRUE;
}

// *****************************************************
// Desc: �б�, �ñ�, ���� ī�� ��� ó��
// *****************************************************
BOOL GLGaeaServer::RequestInvenRecall ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNETPC_REQ_INVEN_RECALL *pNetMsg )
{
	GLChar* pChar = GetChar(dwGaeaID);
	if ( !pChar )	return FALSE;

	GLMSG::SNETPC_REQ_RECALL_FB	NetMsgFB;

	//	�ɸ��� ���� ���°� �ƴ� ���.
	if ( !pChar->IsValidBody() )
	{
		NetMsgFB.emFB = EMREQ_RECALL_FB_CONDITION;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	//	��� ������ ���.
	if ( pChar->m_sCONFTING.IsCONFRONTING() )
	{
		NetMsgFB.emFB = EMREQ_RECALL_FB_CONDITION;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	//	�ŷ����� ���.
	if ( pChar->m_sTrade.Valid() )
	{
		NetMsgFB.emFB = EMREQ_RECALL_FB_CONDITION;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	//	Note : pk ����� ������ ��� �̻��� ��� ��ȯ ī���� ����� ���´�.
	//
	DWORD dwPK_LEVEL = pChar->GET_PK_LEVEL();
	if ( dwPK_LEVEL != UINT_MAX && dwPK_LEVEL>GLCONST_CHAR::dwPK_RECALL_ENABLE_LEVEL )
	{
		NetMsgFB.emFB = EMREQ_RECALL_FB_PK_LEVEL;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return E_FAIL;
	}

	//	�κ��� ������ Ȯ��.
	SINVENITEM *pINVENITEM = pChar->m_cInventory.FindPosItem ( pNetMsg->wPosX, pNetMsg->wPosY );
	if ( !pINVENITEM )
	{
		NetMsgFB.emFB = EMREQ_RECALL_FB_ITEM;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	if ( pChar->CheckCoolTime( pINVENITEM->sItemCustom.sNativeID ) )	return FALSE;


	//	������ ���� ������.
	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pINVENITEM->sItemCustom.sNativeID );
	if ( !pItem || pItem->sBasicOp.emItemType!=ITEM_RECALL )
	{
		NetMsgFB.emFB = EMREQ_RECALL_FB_ITEM;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	SNATIVEID sMAPID(false);
	DWORD dwGATEID(UINT_MAX);
	D3DXVECTOR3 vPOS(FLT_MAX,FLT_MAX,FLT_MAX);

	WORD wSchool = pChar->m_wSchool;

	switch ( pItem->sDrugOp.emDrug )
	{
	default:
		NetMsgFB.emFB = EMREQ_RECALL_FB_ITEM;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;

	case ITEM_DRUG_CALL_SCHOOL:
		sMAPID = GLCONST_CHAR::nidSTARTMAP[wSchool];
		dwGATEID = GLCONST_CHAR::dwSTARTGATE[wSchool];
		break;

	case ITEM_DRUG_CALL_REGEN:
		sMAPID = pChar->m_sStartMapID;
		dwGATEID = pChar->m_dwStartGate;
		break;

	case ITEM_DRUG_CALL_LASTCALL:
		if ( pChar->m_sLastCallMapID==SNATIVEID(false) )
		{
			NetMsgFB.emFB = EMREQ_RECALL_FB_NOTLASTCALL;
			SENDTOCLIENT ( dwClientID, &NetMsgFB );
			return FALSE;
		}

		sMAPID = pChar->m_sLastCallMapID;
		dwGATEID = UINT_MAX;
		vPOS = pChar->m_vLastCallPos;
		
		//	Note : ���� ��ȯ ��ġ ����.
		//
		pChar->m_sLastCallMapID = SNATIVEID(false);

		GLMSG::SNETPC_UPDATE_LASTCALL NetMsg;
		NetMsg.sLastCallMapID = SNATIVEID(false);
		SENDTOCLIENT( dwClientID, &NetMsg );

		break;
	};

	if ( sMAPID==SNATIVEID(false) )
	{
		NetMsgFB.emFB = EMREQ_RECALL_FB_FAIL;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	GLMapList::FIELDMAP_ITER iter = m_MapList.find ( sMAPID.dwID );
	if ( iter==m_MapList.end() )
	{
		NetMsgFB.emFB = EMREQ_RECALL_FB_FAIL;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	//	Note : ������ȯ ��ġ ����. ( �ñͳ� �б��� ��� )
	//
	if ( pItem->sDrugOp.emDrug==ITEM_DRUG_CALL_SCHOOL || pItem->sDrugOp.emDrug==ITEM_DRUG_CALL_REGEN )
	{
		pChar->SaveLastCall ();

		//	Note : ������ȯ ��ġ DB�� ����.
		//
		CSetLastCallPos *pDbAction = new CSetLastCallPos ( pChar->m_dwCharID, pChar->m_sLastCallMapID.dwID, pChar->m_vLastCallPos );
		if ( GetDBMan() )
		{
			GetDBMan()->AddJob ( pDbAction );
		}
	}

	const SMAPNODE *pMapNode = &(*iter).second;

	// �������� �˻�
	if ( pChar->m_dwUserLvl < USER_GM3 )
	{
		EMREQFAIL emReqFail(EMREQUIRE_COMPLETE);
		const SLEVEL_REQUIRE &sRequire = pMapNode->sLEVEL_REQUIRE;
		emReqFail = sRequire.ISCOMPLETE ( pChar ); 
		if ( emReqFail != EMREQUIRE_COMPLETE )
		{
			NetMsgFB.emFB = EMREQ_RECALL_FB_JOINCON;
			SENDTOCLIENT ( pChar->m_dwClientID, &NetMsgFB );

			// �ñ͸� ���� �����������
			// ������ġ ���� (���п��� ���۸�����)
			if ( pItem->sDrugOp.emDrug == ITEM_DRUG_CALL_REGEN )
			{
				pChar->ReSetStartMap();
			}

			return FALSE;
		}

		// MapList �� Move flag �� �����Ǿ� ���� ������
		// ������ȯī��� �̵��Ұ���
		if ( !pMapNode->bMove && pItem->sDrugOp.emDrug == ITEM_DRUG_CALL_LASTCALL )
		{
			NetMsgFB.emFB = EMREQ_RECALL_FB_IMMOVABLE;
			SENDTOCLIENT ( pChar->m_dwClientID, &NetMsgFB );
			return FALSE;
		}
	}

	//	Ŭ��������ġ�� �϶� 
	if ( pMapNode->bCDMZone ) 
	{
		GLLandMan* pLandMan = GetByMapID ( sMAPID );

		if ( !pLandMan )
		{
			NetMsgFB.emFB = EMREQ_RECALL_FB_JOINCON;
			SENDTOCLIENT ( pChar->m_dwClientID, &NetMsgFB );
			pChar->ReSetStartMap();
			return FALSE;
		}

		if ( GLCONST_CHAR::bPKLESS )
		{
			NetMsgFB.emFB = EMREQ_RECALL_FB_JOINCON;
			SENDTOCLIENT ( pChar->m_dwClientID, &NetMsgFB );
			pChar->ReSetStartMap();
			return FALSE;
		}

		GLClubDeathMatch *pCDM = GLClubDeathMatchFieldMan::GetInstance().Find ( pLandMan->m_dwClubMapID );
		if ( pCDM && !pCDM->IsEnterMap( pChar->m_dwGuild ) )
		{
			NetMsgFB.emFB = EMREQ_RECALL_FB_JOINCON;
			SENDTOCLIENT ( pChar->m_dwClientID, &NetMsgFB );
			pChar->ReSetStartMap();
			return FALSE;
		}

		GLCLUB* pClub = GLGaeaServer::GetInstance().GetClubMan().GetClub ( pChar->m_dwGuild );		
		// Ŭ����Ʋ �������϶��� ������.
		if ( !pClub || pClub->GetAllBattleNum() > 0 || !pClub->IsMemberFlgCDM(pChar->m_dwCharID) )
		{
			NetMsgFB.emFB = EMREQ_RECALL_FB_JOINCON;
			SENDTOCLIENT ( pChar->m_dwClientID, &NetMsgFB );
			pChar->ReSetStartMap();
			return FALSE;
		}
	}

	/*pvp tyranny, Juver, 2017/08/25 */
	if ( pMapNode->bPVPTyrannyZone )
	{
		NetMsgFB.emFB = EMREQ_RECALL_FB_JOINCON;
		SENDTOCLIENT ( pChar->m_dwClientID, &NetMsgFB );
		pChar->ReSetStartMap();
		return FALSE;
	}

	/*school wars, Juver, 2018/01/19 */
	if ( pMapNode->bPVPSchoolWarsZone )
	{
		NetMsgFB.emFB = EMREQ_RECALL_FB_JOINCON;
		SENDTOCLIENT ( pChar->m_dwClientID, &NetMsgFB );
		pChar->ReSetStartMap();
		return FALSE;
	}

	/*pvp capture the flag, Juver, 2018/01/31 */
	if ( pMapNode->bPVPCaptureTheFlagZone )
	{
		NetMsgFB.emFB = EMREQ_RECALL_FB_JOINCON;
		SENDTOCLIENT ( pChar->m_dwClientID, &NetMsgFB );
		pChar->ReSetStartMap();
		return FALSE;
	}

	// PET
	// ���̵��� Pet ����
	DropOutPET ( pChar->m_dwPetGUID, true, true );
	//DropOutSummon ( pChar->m_dwSummonGUID, true );
	SaveVehicle( pChar->m_dwClientID, pChar->m_dwGaeaID, true );

	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		DropOutSummon ( pChar->m_dwSummonGUID_FLD[i] );

	//	Note : �ٸ� �ʵ� ������ ���.
	if ( pMapNode->dwFieldSID!=m_dwFieldSvrID )
	{
		GLMSG::SNETPC_REQ_RECALL_AG NetMsgAg;
		NetMsgAg.sMAPID = sMAPID;
		NetMsgAg.dwGATEID = dwGATEID;
		NetMsgAg.vPOS = vPOS;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pChar->m_dwClientID, &NetMsgAg );
	}
	//	Note : ���� �ʵ� ������ ���.
	else
	{
		SNATIVEID sCurMapID = pChar->m_sMapID;

		BOOL bOK = RequestInvenRecallThisSvr ( pChar, sMAPID, dwGATEID, vPOS );
		if ( !bOK )
		{
			NetMsgFB.emFB = EMREQ_RECALL_FB_FAIL;
			SENDTOCLIENT ( pChar->m_dwClientID, &NetMsgFB );
			return FALSE;
		}

		// ������ �����̻� ����
		if ( sCurMapID != pChar->m_sMapID )
		{
			for ( int i=0; i<EMBLOW_MULTI; ++i )		pChar->DISABLEBLOW ( i );
			for ( int i=0; i<SKILLFACT_SIZE; ++i )		pChar->DISABLESKEFF ( i );
			pChar->DISABLEALLLANDEFF();
		}

		pChar->ResetAction();

		//	Note : �� �̵� ������ �˸�.
		//
		NetMsgFB.emFB = EMREQ_RECALL_FB_OK;
		NetMsgFB.sMAPID = sMAPID;
		NetMsgFB.vPOS = pChar->m_vPos;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pChar->m_dwClientID, &NetMsgFB );
	}

	//	[�ڽſ���] �ι� ������ �Ҹ��Ŵ.
	pChar->DoDrugInvenItem ( pNetMsg->wPosX, pNetMsg->wPosY, true );

	return TRUE;
}

// *****************************************************
// Desc: ��ȯ��û ó��(���� �ʵ弭���� ����)
// *****************************************************
BOOL GLGaeaServer::RequestInvenRecallThisSvr ( GLChar* pPC, SNATIVEID sMAPID, DWORD dwGATEID, D3DXVECTOR3 vPOS )
{
	if ( !pPC )		return FALSE;
	
	GLLandMan* pLandMan = NULL;
	DxLandGateMan* pGateMan = NULL;
	PDXLANDGATE pGate = NULL;
	D3DXVECTOR3 vStartPos;

	pLandMan = GetByMapID ( sMAPID );
	if ( !pLandMan )	return FALSE;

	if ( dwGATEID!=UINT_MAX )
	{
		pGateMan = &pLandMan->GetLandGateMan ();
		pGate = pGateMan->FindLandGate ( dwGATEID );
		if ( !pGate )		return FALSE;

		vStartPos = pGate->GetGenPos ( DxLandGate::GEN_RENDUM );
	}
	else
	{
		vStartPos = vPOS;
	}

	//	Note : �ڽ��� �� �ֺ� ���� ����.
	pPC->ResetViewAround ();

	//	Note : ���� �ʿ� �ִ� ���� ��� ����.
	//
	if ( pPC->m_pLandNode )
		pPC->m_pLandMan->m_GlobPCList.DELNODE ( pPC->m_pLandNode );

	if ( pPC->m_pQuadNode && pPC->m_pCellNode )
		pPC->m_pQuadNode->pData->m_PCList.DELNODE ( pPC->m_pCellNode );

	//	Note : ���ο� �� ��ġ�� ������̼� �ʱ�ȭ.
	//
	pPC->SetNavi ( pLandMan->GetNavi(), vStartPos );

	//	Note : ���ο� ���� ID �ο�.
	//
	pPC->m_sMapID = sMAPID;

	pPC->m_vPos	  = vStartPos;

	pPC->m_pLandMan = pLandMan;
	pPC->m_pLandNode = pLandMan->m_GlobPCList.ADDHEAD ( pPC );

	//	Note : GLLandMan�� ���� ����ϴ� �۾�.
	//			
	//RegistChar ( pPC ); --> ( GLGaeaServer::RequestLandIn ()�� ȣ��� ������ ������. )

	pPC->m_dwCeID = 0;
	pPC->m_pQuadNode = NULL;
	pPC->m_pCellNode = NULL;

	return TRUE;
}

BOOL GLGaeaServer::RequestBus ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNETPC_REQ_BUS *pNetMsg )
{
	GLChar* pChar = GetChar(dwGaeaID);
	if ( !pChar )	return FALSE;

	GLMSG::SNETPC_REQ_BUS_FB	NetMsgFB;

	//	�ɸ��� ���� ���°� �ƴ� ���.
	if ( !pChar->IsValidBody() )
	{
		NetMsgFB.emFB = EMBUS_TAKE_CONDITION;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	//	��� ������ ���.
	if ( pChar->m_sCONFTING.IsCONFRONTING() )
	{
		NetMsgFB.emFB = EMBUS_TAKE_CONDITION;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	//	�ŷ����� ���.
	if ( pChar->m_sTrade.Valid() )
	{
		NetMsgFB.emFB = EMBUS_TAKE_CONDITION;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	//	Note : pk ����� ������ ��� �̻��� ��� ���� ����� ���´�.
	//
	//DWORD dwPK_LEVEL = pChar->GET_PK_LEVEL();
	//if ( dwPK_LEVEL != UINT_MAX && dwPK_LEVEL>GLCONST_CHAR::dwPK_RECALL_ENABLE_LEVEL )
	//{
	//	NetMsgFB.emFB = EMBUS_TAKE_PK_LEVEL;
	//	SENDTOCLIENT ( dwClientID, &NetMsgFB );
	//	return FALSE;
	//}

	PGLCROW pCROW(NULL);
	if ( pChar->m_pLandMan )	pCROW = pChar->m_pLandMan->GetCrow ( pNetMsg->dwNPC_ID );
	if ( !pCROW )
	{
		NetMsgFB.emFB = EMBUS_TAKE_FAIL;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	if ( !pCROW->m_pCrowData->m_sNpcTalkDlg.IsBUSSTATION() )
	{
		NetMsgFB.emFB = EMBUS_TAKE_FAIL;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	//	�κ��� ������ Ȯ��.
	SINVENITEM *pINVENITEM = pChar->m_cInventory.FindPosItem ( pNetMsg->wPosX, pNetMsg->wPosY );
	if ( !pINVENITEM )
	{
		NetMsgFB.emFB = EMBUS_TAKE_TICKET;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	if ( pChar->CheckCoolTime( pINVENITEM->sItemCustom.sNativeID ) ) return FALSE;

	//	������ ���� ������.
	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pINVENITEM->sItemCustom.sNativeID );
	if ( !pItem || pItem->sBasicOp.emItemType!=ITEM_TICKET )
	{
		NetMsgFB.emFB = EMBUS_TAKE_TICKET;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	SNATIVEID sMAPID(false);
	DWORD dwGATEID(UINT_MAX);

	//	Note : ������ id�� ��Ȯ���� �˻�.
	//
	SSTATION* pSTATION = GLBusStation::GetInstance().GetStation ( pNetMsg->dwSTATION_ID );
	if ( !pSTATION )
	{
		NetMsgFB.emFB = EMBUS_TAKE_FAIL;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	//	map id.
	sMAPID.dwID = pSTATION->dwMAPID;

	//	gate id.
	dwGATEID = pSTATION->dwGATEID;


	if ( sMAPID==SNATIVEID(false) )
	{
		NetMsgFB.emFB = EMBUS_TAKE_FAIL;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	GLMapList::FIELDMAP_ITER iter = m_MapList.find ( sMAPID.dwID );
	if ( iter==m_MapList.end() )
	{
		NetMsgFB.emFB = EMBUS_TAKE_FAIL;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	const SMAPNODE *pMapNode = &(*iter).second;

	// �������� �˻�
	if ( pChar->m_dwUserLvl < USER_GM3 )
	{
		EMREQFAIL emReqFail(EMREQUIRE_COMPLETE);
		const SLEVEL_REQUIRE &sRequire = pMapNode->sLEVEL_REQUIRE;
		emReqFail = sRequire.ISCOMPLETE ( pChar ); 
		if ( emReqFail != EMREQUIRE_COMPLETE )
		{
			//	ž�� ������ ����.
			NetMsgFB.emFB = EMBUS_TAKE_CONDITION;
			SENDTOCLIENT ( dwClientID, &NetMsgFB );
			return FALSE;
		}
	}

	//	[�ڽſ���] �ι� ������ �Ҹ��Ŵ.
	pChar->DoDrugInvenItem ( pNetMsg->wPosX, pNetMsg->wPosY, true );

	//	Note : ž�� ���� �˸�.
	NetMsgFB.emFB = EMBUS_TAKE_OK;
	SENDTOCLIENT ( dwClientID, &NetMsgFB );

	// PET
	// ���̵��� Pet ����
	DropOutPET ( pChar->m_dwPetGUID,false,true );
	//DropOutSummon ( pChar->m_dwSummonGUID, false );
	SetActiveVehicle( pChar->m_dwClientID, pChar->m_dwGaeaID, false );
	
	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		DropOutSummon ( pChar->m_dwSummonGUID_FLD[i] );

	//	Note : �ٸ� �ʵ� ������ ���.
	if ( pMapNode->dwFieldSID!=m_dwFieldSvrID )
	{
		GLMSG::SNETPC_REQ_RECALL_AG NetMsgAg;
		NetMsgAg.sMAPID = sMAPID;
		NetMsgAg.dwGATEID = dwGATEID;
		NetMsgAg.vPOS = D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX);
		GLGaeaServer::GetInstance().SENDTOAGENT ( pChar->m_dwClientID, &NetMsgAg );
	}
	else
	{
		GLMSG::SNETPC_REQ_RECALL_FB	NetMsgFB;
		SNATIVEID sCurMapID = pChar->m_sMapID;

		BOOL bOK = RequestInvenRecallThisSvr ( pChar, sMAPID, dwGATEID, D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX) );
		if ( !bOK )
		{
			NetMsgFB.emFB = EMREQ_RECALL_FB_FAIL;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( pChar->m_dwClientID, &NetMsgFB );
			return FALSE;
		}

		// ������ �����̻� ����
		if ( sCurMapID != pChar->m_sMapID )
		{
			for ( int i=0; i<EMBLOW_MULTI; ++i )		pChar->DISABLEBLOW ( i );
			for ( int i=0; i<SKILLFACT_SIZE; ++i )		pChar->DISABLESKEFF ( i );
			pChar->DISABLEALLLANDEFF();
		}

		pChar->ResetAction();

		//	Note : �� �̵� ������ �˸�.
		//
		NetMsgFB.emFB = EMREQ_RECALL_FB_OK;
		NetMsgFB.sMAPID = sMAPID;
		NetMsgFB.vPOS = pChar->m_vPos;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pChar->m_dwClientID, &NetMsgFB );
	}

	return TRUE;
}

BOOL GLGaeaServer::RequestTaxi ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNETPC_REQ_TAXI *pNetMsg )
{
	GLChar* pChar = GetChar(dwGaeaID);
	if ( !pChar )	return FALSE;

	GLMSG::SNETPC_REQ_TAXI_FB	NetMsgFB;

	//	�ɸ��� ���� ���°� �ƴ� ���.
	if ( !pChar->IsValidBody() )
	{
		NetMsgFB.emFB = EMTAXI_TAKE_CONDITION;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	//	��� ������ ���.
	if ( pChar->m_sCONFTING.IsCONFRONTING() )
	{
		NetMsgFB.emFB = EMTAXI_TAKE_CONDITION;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	//	�ŷ����� ���.
	if ( pChar->m_sTrade.Valid() )
	{
		NetMsgFB.emFB = EMTAXI_TAKE_CONDITION;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}
	
	// ������ �˻�
	STAXI_MAP* pTaxiMap = GLTaxiStation::GetInstance().GetTaxiMap ( pNetMsg->dwSelectMap );
	if ( !pTaxiMap )
	{
		NetMsgFB.emFB = EMTAXI_TAKE_MAPFAIL;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	// ������ �˻�
	STAXI_STATION* pStation = pTaxiMap->GetStation( pNetMsg->dwSelectStop );
	if ( !pStation )
	{
		NetMsgFB.emFB = EMTAXI_TAKE_STATIONFAIL;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}


	//	�κ��� ������ Ȯ��.
	SINVENITEM *pINVENITEM = pChar->m_cInventory.FindPosItem ( pNetMsg->wPosX, pNetMsg->wPosY );
	if ( !pINVENITEM )
	{
		NetMsgFB.emFB = EMTAXI_TAKE_TICKET;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	if ( pChar->CheckCoolTime( pINVENITEM->sItemCustom.sNativeID ) )	return FALSE;


	//	������ ���� ������.
	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pINVENITEM->sItemCustom.sNativeID );
	if ( !pItem || pItem->sBasicOp.emItemType!=ITEM_TAXI_CARD )
	{
		NetMsgFB.emFB = EMTAXI_TAKE_TICKET;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	//	�����ݾ� �˻�
	DWORD dwCurMapID = pChar->m_sMapID.dwID;
	LONGLONG lnCharge = GLTaxiStation::GetInstance().GetBasicCharge();

	if ( pStation->dwMAPID != dwCurMapID ) lnCharge += pStation->dwMapCharge;
	
	volatile float fSHOP_RATE = pChar->GET_PK_SHOP2BUY();
	volatile float fSHOP_2RATE = fSHOP_RATE + pChar->m_pLandMan->m_fCommissionRate;
	volatile float fSHOP_2RATE_C = fSHOP_2RATE * 0.01f;
	lnCharge = (LONGLONG)(lnCharge * fSHOP_2RATE_C);

	if ( pChar->m_lnMoney < lnCharge )
	{
		NetMsgFB.emFB = EMTAXI_TAKE_MONEY;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return S_FALSE;
	}

	SNATIVEID sMAPID(false);
	DWORD dwNPCID(UINT_MAX);

	//	map id.
	sMAPID.dwID = pStation->dwMAPID;

	//	gate id.
	dwNPCID = pStation->dwNPCID;

	// �ʰ˻�
	if ( sMAPID==SNATIVEID(false) )
	{
		NetMsgFB.emFB = EMTAXI_TAKE_MAPFAIL;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	GLMapList::FIELDMAP_ITER iter = m_MapList.find ( sMAPID.dwID );
	if ( iter==m_MapList.end() )
	{
		NetMsgFB.emFB = EMTAXI_TAKE_MAPFAIL;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	const SMAPNODE *pMapNode = &(*iter).second;

	// �������� �˻�
	if ( pChar->m_dwUserLvl < USER_GM3 )
	{
		EMREQFAIL emReqFail(EMREQUIRE_COMPLETE);
		const SLEVEL_REQUIRE &sRequire = pMapNode->sLEVEL_REQUIRE;
		emReqFail = sRequire.ISCOMPLETE ( pChar ); 
		if ( emReqFail != EMREQUIRE_COMPLETE )
		{
			//	ž�� ������ ����.
			NetMsgFB.emFB = EMTAXI_TAKE_CONDITION;
			SENDTOCLIENT ( dwClientID, &NetMsgFB );
			return FALSE;
		}
	}

	//	NPC �˻�

	PGLCROW pCROW(NULL);
	D3DXVECTOR3 vPOS(FLT_MAX,FLT_MAX,FLT_MAX);
	DWORD dwGATEID( UINT_MAX );

	if ( pMapNode->dwFieldSID==m_dwFieldSvrID )
	{
		GLLandMan* pToLandMan = GetByMapID( sMAPID );

		if ( pToLandMan )
		{
			SNATIVEID sNpcID( dwNPCID );
			if ( pStation->wPosX != 0 && pStation->wPosY != 0 ) 
				pCROW = pToLandMan->GetCrowID ( sNpcID, pStation->wPosX, pStation->wPosY );
			else 
				pCROW = pToLandMan->GetCrowID ( sNpcID );
			
			if ( !pCROW )
			{
				NetMsgFB.emFB = EMTAXI_TAKE_NPCFAIL;
				SENDTOCLIENT ( dwClientID, &NetMsgFB );
				return FALSE;
			}

			
			D3DXVECTOR3 vDIR = pCROW->m_vDir;
			vDIR = vDIR * 20.0f;
			
			vPOS = pCROW->GetPosition();
			vPOS += vDIR;
		}	
	}
	else
	{
		vPOS = pNetMsg->vPos;
	}



	//	[�ڽſ���] �ι� ������ �Ҹ��Ŵ.
	pChar->DoDrugInvenItem ( pNetMsg->wPosX, pNetMsg->wPosY, true );

	//	[�ڽſ���] ���ݾ��� �Ҹ��Ŵ
	pChar->CheckMoneyUpdate( pChar->m_lnMoney, lnCharge, FALSE, "Taxi Charge Money" );
	pChar->m_bMoneyUpdate = TRUE;
	pChar->m_lnMoney -= lnCharge;

	//	Note : ž�� ���� �˸�.
	NetMsgFB.emFB = EMTAXI_TAKE_OK;
	SENDTOCLIENT ( dwClientID, &NetMsgFB );

	GLMSG::SNETPC_UPDATE_MONEY NetMsgMoney;
	NetMsgMoney.lnMoney = pChar->m_lnMoney;
	/*dmk14 ingame web*/
	NetMsgMoney.dwPremium = pChar->m_dwPremiumPoints;
	NetMsgMoney.dwCombat = pChar->m_dwCombatPoints;
	SENDTOCLIENT ( dwClientID, &NetMsgMoney );
	
	// PET
	// ���̵��� Pet ����
	DropOutPET ( pChar->m_dwPetGUID,false,true );
	//DropOutSummon ( pChar->m_dwSummonGUID, false );
	SetActiveVehicle( pChar->m_dwClientID, pChar->m_dwGaeaID, false );
	
	/*skill summon, Juver, 2017/10/09 */
	for ( int i=0; i<SKILL_SUMMON_MAX_CLIENT_NUM; ++i )
		DropOutSummon ( pChar->m_dwSummonGUID_FLD[i] );

	//	Note : �ٸ� �ʵ� ������ ���.
	if ( pMapNode->dwFieldSID!=m_dwFieldSvrID )
	{
		GLMSG::SNETPC_REQ_RECALL_AG NetMsgAg;
		NetMsgAg.sMAPID = sMAPID;
		NetMsgAg.dwGATEID = dwGATEID;
		NetMsgAg.vPOS = vPOS;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pChar->m_dwClientID, &NetMsgAg );
	}
	else
	{
		GLMSG::SNETPC_REQ_RECALL_FB	NetMsgFB;
		SNATIVEID sCurMapID = pChar->m_sMapID;

		BOOL bOK = RequestInvenRecallThisSvr ( pChar, sMAPID, dwGATEID, vPOS );
		if ( !bOK )
		{
			NetMsgFB.emFB = EMREQ_RECALL_FB_FAIL;
			GLGaeaServer::GetInstance().SENDTOCLIENT ( pChar->m_dwClientID, &NetMsgFB );
			return FALSE;
		}

		// ������ �����̻� ����
		if ( sCurMapID != pChar->m_sMapID )
		{
			for ( int i=0; i<EMBLOW_MULTI; ++i )		pChar->DISABLEBLOW ( i );
			for ( int i=0; i<SKILLFACT_SIZE; ++i )		pChar->DISABLESKEFF ( i );
			pChar->DISABLEALLLANDEFF();
		}

		pChar->ResetAction();

		//	Note : �� �̵� ������ �˸�.
		//
		NetMsgFB.emFB = EMREQ_RECALL_FB_OK;
		NetMsgFB.sMAPID = sMAPID;
		NetMsgFB.vPOS = pChar->m_vPos;
		GLGaeaServer::GetInstance().SENDTOAGENT ( pChar->m_dwClientID, &NetMsgFB );
	}

	return TRUE;
}

BOOL GLGaeaServer::RequestTaxiNpcPos ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNETPC_REQ_TAXI_NPCPOS *pNetMsg )
{
	GLMSG::SNETPC_REQ_TAXI_NPCPOS_FB NetMsgFB;

	// ������ �˻�
	STAXI_MAP* pTaxiMap = GLTaxiStation::GetInstance().GetTaxiMap ( pNetMsg->dwSelectMap );
	if ( !pTaxiMap ) 		return FALSE;

	// ������ �˻�
	STAXI_STATION* pStation = pTaxiMap->GetStation( pNetMsg->dwSelectStop );
	if ( !pStation ) return FALSE;

	

	SNATIVEID sMAPID(pStation->dwMAPID);

	// �ʰ˻�
	if ( sMAPID==SNATIVEID(false) )	return FALSE;

	GLLandMan* pToLandMan = GetByMapID( sMAPID );
	
	if ( !pToLandMan )	return FALSE;
	
	SNATIVEID sNpcID( pStation->dwNPCID );
	PGLCROW pCROW;

	if ( pStation->wPosX != 0 && pStation->wPosY != 0 ) 
		pCROW = pToLandMan->GetCrowID ( sNpcID, pStation->wPosX, pStation->wPosY );
	else 
		pCROW = pToLandMan->GetCrowID ( sNpcID );


	if ( !pCROW )
	{
		NetMsgFB.vPos = D3DXVECTOR3(FLT_MAX,FLT_MAX,FLT_MAX);
		NetMsgFB.dwGaeaID = pNetMsg->dwGaeaID;
		SENDTOAGENT( dwClientID, &NetMsgFB );
		return FALSE;
	}

	D3DXVECTOR3 vPos = pCROW->GetPosition();
	D3DXVECTOR3 vDir = pCROW->m_vDir;
	vDir = vDir * 20.0f;
	vPos += vDir;

	NetMsgFB.dwSelectMap = pNetMsg->dwSelectMap;
	NetMsgFB.dwSelectStop = pNetMsg->dwSelectStop;
	NetMsgFB.wPosX = pNetMsg->wPosX;
	NetMsgFB.wPosY = pNetMsg->wPosY;
	NetMsgFB.dwGaeaID = pNetMsg->dwGaeaID;
	NetMsgFB.vPos = vPos;

	SENDTOAGENT( dwClientID, &NetMsgFB );


	return TRUE;

}

// *****************************************************
// Desc: ģ���̵��� ģ����ġ ��ȯ (Agent->Field)
// *****************************************************
BOOL GLGaeaServer::Request2FriendCK ( GLMSG::SNETPC_2_FRIEND_CK *pNetMsg )
{
	GLMSG::SNETPC_2_FRIEND_AG NetMsgAG;
	NetMsgAG.dwCOMMAND_CHARID = pNetMsg->dwCOMMAND_CHARID;
	NetMsgAG.dwFRIEND_CHARID = pNetMsg->dwFRIEND_CHARID;
	NetMsgAG.wItemPosX = pNetMsg->wItemPosX;
	NetMsgAG.wItemPosY = pNetMsg->wItemPosY;

	PGLCHAR pFRIEND = GetCharID ( pNetMsg->dwFRIEND_CHARID );
	if ( !pFRIEND )
	{
		NetMsgAG.emFB = EM2FRIEND_FB_FRIEND_CONDITION;
		SENDTOAGENT ( &NetMsgAG );
		return FALSE;
	}

	if ( !pFRIEND->IsValidBody() || pFRIEND->m_pLandMan==NULL )
	{
		NetMsgAG.emFB = EM2FRIEND_FB_FRIEND_CONDITION;
		SENDTOAGENT ( &NetMsgAG );
		return FALSE;
	}

	GLMapList::FIELDMAP_ITER iter = m_MapList.find ( pFRIEND->m_sMapID.dwID );
	if ( iter==m_MapList.end() )
	{
		NetMsgAG.emFB = EM2FRIEND_FB_FAIL;
		SENDTOAGENT ( &NetMsgAG );
		return FALSE;
	}

	// MapList �� Move flag �� �����Ǿ� ���� ������
	// ģ������ī��� �̵��Ұ���
	SMAPNODE sMapNode = (*iter).second;
	if ( !sMapNode.bMove )
	{
		NetMsgAG.emFB = EM2FRIEND_FB_IMMOVABLE;
		SENDTOAGENT ( &NetMsgAG );
		return FALSE;
	}
		
	BOOL bCoB(FALSE);
	DWORD dwCoB(0);
	D3DXVECTOR3 vCoB(0,0,0);
	
	D3DXVECTOR3 vec2POS = pFRIEND->m_vPos;
	D3DXMATRIX matRotY;
	for ( float fAngle=0.0f; fAngle<D3DX_PI*2; fAngle += D3DX_PI/4.0f )
	{
		D3DXVECTOR3 vecDIR(1,0,0);
		D3DXMatrixRotationY ( &matRotY, fAngle );

		D3DXVec3TransformCoord ( &vecDIR, &vecDIR, &matRotY );
		D3DXVECTOR3 vPos = pFRIEND->m_vPos + vecDIR*10.0f;

		NavigationMesh* pNavi = pFRIEND->m_pLandMan->GetNavi();

		pNavi->IsCollision ( D3DXVECTOR3(vPos.x,vPos.y+20.0f,vPos.z), D3DXVECTOR3(vPos.x,vPos.y-30.0f,vPos.z), vCoB, dwCoB, bCoB );
		if ( bCoB )
		{
			vec2POS = vCoB;
			break;
		}
	}

	//	Note : ��ġ ��ȯ.
	NetMsgAG.emFB = EM2FRIEND_FB_OK;
	NetMsgAG.sFriendMapID = pFRIEND->m_sMapID;
	NetMsgAG.vFriendPos = vec2POS;
	SENDTOAGENT ( &NetMsgAG );

	return TRUE;
}

// *****************************************************
// Desc: �̵��� �ɸ����� ��ġ ��ȯ
// *****************************************************
BOOL GLGaeaServer::RequestMove2CharPos ( DWORD dwClientID, GLMSG::SNETPC_GM_MOVE2CHAR_POS *pNetMsg )
{
	if ( dwClientID != m_dwAgentSlot ) return FALSE;

	// ������Ʈ�� ȸ���� �޼���
	GLMSG::SNETPC_GM_MOVE2CHAR_AG NetMsgAG;
	NetMsgAG.dwCOMMAND_CHARID = pNetMsg->dwCOMMAND_CHARID;
	NetMsgAG.dwTO_CHARID = pNetMsg->dwTO_CHARID;

	// �̵��� �ɸ��� ��ȿ��
	PGLCHAR pTO_CHAR = GetCharID ( pNetMsg->dwTO_CHARID );
	if ( !pTO_CHAR )
	{
		NetMsgAG.emFB = EMGM_MOVE2CHAR_FB_TO_CONDITION;
		SENDTOAGENT ( &NetMsgAG );
		return FALSE;
	}

	// �̵��� ���� ��ȿ��
	if ( pTO_CHAR->m_pLandMan==NULL )
	{
		NetMsgAG.emFB = EMGM_MOVE2CHAR_FB_TO_CONDITION;
		SENDTOAGENT ( &NetMsgAG );
		return FALSE;
	}

	BOOL bCoB(FALSE);
	DWORD dwCoB(0);
	D3DXVECTOR3 vCoB(0,0,0);
	
	D3DXVECTOR3 vec2POS = pTO_CHAR->m_vPos;
	D3DXMATRIX matRotY;
	for ( float fAngle=0.0f; fAngle<D3DX_PI*2; fAngle += D3DX_PI/4.0f )
	{
		D3DXVECTOR3 vecDIR(1,0,0);
		D3DXMatrixRotationY ( &matRotY, fAngle );

		D3DXVec3TransformCoord ( &vecDIR, &vecDIR, &matRotY );
		D3DXVECTOR3 vPos = pTO_CHAR->m_vPos + vecDIR*10.0f;

		NavigationMesh* pNavi = pTO_CHAR->m_pLandMan->GetNavi();

		pNavi->IsCollision ( D3DXVECTOR3(vPos.x,vPos.y+20.0f,vPos.z), D3DXVECTOR3(vPos.x,vPos.y-30.0f,vPos.z), vCoB, dwCoB, bCoB );
		if ( bCoB )
		{
			vec2POS = vCoB;
			break;
		}
	}

	//	Note : ��ġ ��ȯ.
	NetMsgAG.emFB = EMGM_MOVE2CHAR_FB_OK;
	NetMsgAG.sToMapID = pTO_CHAR->m_sMapID;
	NetMsgAG.vToPos = vec2POS;
	SENDTOAGENT ( &NetMsgAG );

	return TRUE;
}

BOOL GLGaeaServer::RequestClubRank2Fld ( GLMSG::SNET_CLUB_RANK_2FLD *pNetMsg )
{
	GLCLUB *pCLUB = m_cClubMan.GetClub ( pNetMsg->dwClubID );
	if ( !pCLUB )								return FALSE;
	if ( pNetMsg->dwRank > (GLCONST_CHAR::dwMAX_CLUBRANK-1) )	return FALSE;

	pCLUB->m_dwRank = pNetMsg->dwRank;

	GLMSG::SNET_CLUB_RANK_2CLT NetMsg;
	NetMsg.dwRank = pCLUB->m_dwRank;

	//	Note : Ŭ���̾�Ʈ�鿡�� �˸�.
	//
	PGLCHAR pMASTER = GetCharID ( pCLUB->m_dwMasterID );
	if ( pMASTER )
	{
		LONGLONG lnPAY = GLCONST_CHAR::sCLUBRANK[NetMsg.dwRank].m_dwPay;

		pMASTER->CheckMoneyUpdate( pMASTER->m_lnMoney, lnPAY, FALSE, "Club Rank Pay." );
		pMASTER->m_bMoneyUpdate = TRUE;

		if ( pMASTER->m_lnMoney >= lnPAY )
		{
			pMASTER->m_lnMoney -= lnPAY;
		}
		else
		{
			pMASTER->m_lnMoney = 0;
		}


		//	Note : �ݾ� �α�.
		//
		GLITEMLMT::GetInstance().ReqMoneyExc( ID_CHAR, pMASTER->m_dwCharID, ID_CLUB, 0, -lnPAY, EMITEM_ROUTE_DELETE );
		GLITEMLMT::GetInstance().ReqMoneyExc( ID_CHAR, pMASTER->m_dwCharID, ID_CHAR, 0, pMASTER->m_lnMoney, EMITEM_ROUTE_CHAR );

		//	�ݾ� ��ȭ Ŭ���̾�Ʈ�� �˷���.
		GLMSG::SNETPC_UPDATE_MONEY NetMsg;
		NetMsg.lnMoney = pMASTER->m_lnMoney;
		/*dmk14 ingame web*/
		NetMsg.dwPremium = pMASTER->m_dwPremiumPoints;
		NetMsg.dwCombat = pMASTER->m_dwCombatPoints;
		GLGaeaServer::GetInstance().SENDTOCLIENT ( pMASTER->m_dwClientID, &NetMsg );

		//	Note : �����Ϳ��� ����.
		SENDTOCLIENT ( pMASTER->m_dwClientID, &NetMsg );

		//	Note : Ŭ�� ��ũ ��ȭ �α� ���.
		GLITEMLMT::GetInstance().ReqAction
		(
			pMASTER->m_dwCharID,		//	�����.
			EMLOGACT_CLUB_LEV_UP,		//	����.
			ID_CLUB, pCLUB->m_dwID,		//	����.
			0,							//	exp.
			0,							//	bright.
			0,							//	life.
			(int)-lnPAY					//	money.
		);
	}

	CLUBMEMBERS_ITER pos = pCLUB->m_mapMembers.begin();
	CLUBMEMBERS_ITER end = pCLUB->m_mapMembers.end();
	PGLCHAR pMEMBER = NULL;
	for ( ; pos!=end; ++pos )
	{
		pMEMBER = GetCharID ( (*pos).first );
		if ( pMEMBER )
		{
			//	Note : �������.
			SENDTOCLIENT ( pMEMBER->m_dwClientID, &NetMsg );
		}
	}

	return TRUE;
}

BOOL GLGaeaServer::RequestClubInfo ( GLMSG::SNET_CLUB_INFO_2FLD *pNetMsg )
{
	//	Note : Ŭ�� ����.
	//
	GLCLUB cCLUB;
	cCLUB.m_dwID = pNetMsg->dwClubID;
	StringCchCopy ( cCLUB.m_szName, CHAR_SZNAME, pNetMsg->szClubName );
	
	cCLUB.m_dwMasterID = pNetMsg->dwMasterID;
	StringCchCopy ( cCLUB.m_szMasterName, CHAR_SZNAME, pNetMsg->szMasterName );
	cCLUB.m_dwCDCertifior = pNetMsg->dwCDCertifior;

	cCLUB.m_dwRank = pNetMsg->dwRank;
	cCLUB.m_dwMarkVER = pNetMsg->dwMarkVER;

	cCLUB.m_tOrganize = pNetMsg->tOrganize;
	cCLUB.m_tDissolution = pNetMsg->tDissolution;

	for ( DWORD i=0; i<pNetMsg->dwMemberNum; ++i )
	{
		GLCLUBMEMBER cMEMBER;
		cMEMBER.dwID = pNetMsg->aMembers[i];
		StringCchCopy ( cMEMBER.m_szName, CHAR_SZNAME, pNetMsg->szMemberName[i] );
		cCLUB.m_mapMembers.insert ( std::make_pair(cMEMBER.dwID,cMEMBER) );
	}

	m_cClubMan.AddClub ( cCLUB );

	//	Note : Ŭ�� ������, ��������� Ŭ�� �� ���� ����.
	//
	GLMSG::SNET_CLUB_INFO_2CLT NetMsg2Clt;
	NetMsg2Clt.dwClubID = cCLUB.m_dwID;
	StringCchCopy ( NetMsg2Clt.szClubName, CHAR_SZNAME, cCLUB.m_szName );

	NetMsg2Clt.dwMasterID = cCLUB.m_dwMasterID;
	StringCchCopy ( NetMsg2Clt.szMasterName, CHAR_SZNAME, cCLUB.m_szMasterName );
	NetMsg2Clt.dwCDCertifior = cCLUB.m_dwCDCertifior;

	NetMsg2Clt.dwMarkVER = cCLUB.m_dwMarkVER;
	NetMsg2Clt.dwRank = cCLUB.m_dwRank;

	NetMsg2Clt.tOrganize = cCLUB.m_tOrganize;
	NetMsg2Clt.tDissolution = cCLUB.m_tDissolution;

	GLMSG::SNET_CLUB_MEMBER_2CLT NetMsgMember;
	for ( int i=0; i<EMMAXCLUBMEMBER_NEW; ++i )
	{
		if ( pNetMsg->aMembers[i]==0 )	continue;
		NetMsgMember.ADDMEMBER ( pNetMsg->aMembers[i], NULL, pNetMsg->szMemberName[i] );
	}

	//	Note : ������, ��� �ֺ� ����鿡�� Ŭ�� ���� ���� ����.
	GLMSG::SNET_CLUB_INFO_BRD NetMsgBrd;
	NetMsgBrd.dwClubID = cCLUB.m_dwID;
	NetMsgBrd.dwMarkVer = cCLUB.m_dwMarkVER;

	//	Note : ������, ������� ����.
	//
	PGLCHAR pMASTER = GetCharID ( cCLUB.m_dwMasterID );
	if ( pMASTER )
	{
		//	Note : �����Ϳ��� ����.
		pMASTER->m_dwGuild = cCLUB.m_dwID;

		StringCchCopy ( pMASTER->m_szNick, CHAR_SZNAME, cCLUB.m_szName );
		StringCchCopy ( NetMsg2Clt.szNickName, CHAR_SZNAME, pMASTER->m_szNick );
		SENDTOCLIENT ( pMASTER->m_dwClientID, &NetMsg2Clt );

		SENDTOCLIENT ( pMASTER->m_dwClientID, &NetMsgMember );

		//	Note : �������� �ֺ� �������.
		NetMsgBrd.dwGaeaID = pMASTER->m_dwGaeaID;
		StringCchCopy ( NetMsgBrd.szNickName, CHAR_SZNAME, pMASTER->m_szNick );
		pMASTER->SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
	}

	PGLCHAR pMEMBER = NULL;
	for ( DWORD i=0; i<pNetMsg->dwMemberNum; ++i )
	{
		pMEMBER = GetCharID ( pNetMsg->aMembers[i] );
		if ( pMEMBER )
		{
			//	Note : �������.
			pMEMBER->m_dwGuild = cCLUB.m_dwID;

			StringCchCopy ( pMEMBER->m_szNick, CHAR_SZNAME, cCLUB.m_szName );
			StringCchCopy ( NetMsg2Clt.szNickName, CHAR_SZNAME, pMEMBER->m_szNick );
			SENDTOCLIENT ( pMEMBER->m_dwClientID, &NetMsg2Clt );

			SENDTOCLIENT ( pMEMBER->m_dwClientID, &NetMsgMember );

			//	Note : ����� �ֺ�����.
			NetMsgBrd.dwGaeaID = pMEMBER->m_dwGaeaID;
			StringCchCopy ( NetMsgBrd.szNickName, CHAR_SZNAME, pMEMBER->m_szNick );
			pMEMBER->SendMsgViewAround ( (NET_MSG_GENERIC*) &NetMsgBrd );
		}
	}

	return TRUE;
}

// *****************************************************
// Desc: Ŭ������ ����
// *****************************************************
BOOL GLGaeaServer::RequestClubDissolution2Fld ( GLMSG::SNET_CLUB_DISSOLUTION_2FLD *pNetMsg )
{
	GLCLUB *pCLUB = m_cClubMan.GetClub ( pNetMsg->dwClubID );
	if ( !pCLUB )		return FALSE;

	pCLUB->DoDissolution ( pNetMsg->tDissolution );

	//	Note : Ŭ�� ������, ��������� Ŭ�� �� ���� ����.
	//
	GLMSG::SNET_CLUB_INFO_DISSOLUTION NetMsg2Clt;
	NetMsg2Clt.tDissolution = pCLUB->m_tDissolution;

	//	Note : ������, ������� ����
	//
	PGLCHAR pMASTER = GetCharID ( pCLUB->m_dwMasterID );
	CTime tDisTime( pNetMsg->tDissolution );
	if ( pMASTER )
	{
		// �⵵�� 1970���̸� Ŭ�� ��ü ���.
		if ( tDisTime.GetYear() != 1970 )
		{
			//	Note : ����ġ�� �����Ų��.
			__int64 nDECEXP;
			nDECEXP = __int64 ( GLOGICEX::GLDIE_DECEXP(pMASTER->GETLEVEL()) * 0.01f * pMASTER->GET_LEVELUP_EXP() );
			nDECEXP = __int64 ( nDECEXP * pMASTER->GET_PK_DECEXP_RATE()*0.01f );
			pMASTER->SET_DECEXP ( nDECEXP );
			
			// �׾��� ��츸 ���ŵǰ� �ϱ� ���Ͽ� �̸� �����س��� �ٽ� �����Ѵ�.
			LONGLONG nReExp = pMASTER->m_lnReExp;
			
			pMASTER->ReBirthDecExp ();
			
			pMASTER->m_lnReExp = nReExp;
		}

		//	Note : �����Ϳ��� ����.
		SENDTOCLIENT ( pMASTER->m_dwClientID, &NetMsg2Clt );
	}

	CLUBMEMBERS_ITER pos = pCLUB->m_mapMembers.begin();
	CLUBMEMBERS_ITER end = pCLUB->m_mapMembers.end();
	PGLCHAR pMEMBER = NULL;
	for ( ; pos!=end; ++pos )
	{
		pMEMBER = GetCharID ( (*pos).first );
		if ( pMEMBER )
		{
			//	Note : �������.
			SENDTOCLIENT ( pMEMBER->m_dwClientID, &NetMsg2Clt );
		}
	}

	return TRUE;
}

BOOL GLGaeaServer::RequestClubDel2Fld ( GLMSG::SNET_CLUB_DEL_2FLD *pNetMsg )
{
	GLCLUB *pCLUB = m_cClubMan.GetClub ( pNetMsg->dwClubID );
	if ( !pCLUB )		return FALSE;
	
	PGLCHAR pCHAR = GetCharID ( pCLUB->m_dwMasterID );
	if ( pCHAR )
	{
		//	Note : Ŭ�� ���� ����.
		pCHAR->m_dwGuild = CLUB_NULL;

		//	Note : Ŭ���̾�Ʈ�� Ŭ�� ���� ����.
		//
		GLMSG::SNET_CLUB_DEL_2CLT NetMsgClt;
		SENDTOCLIENT ( pCHAR->m_dwClientID, &NetMsgClt );

		//	Note : �ֺ� ������� �˸�.
		GLMSG::SNET_CLUB_DEL_BRD NetMsgBrd;
		NetMsgBrd.dwGaeaID = pCHAR->m_dwGaeaID;
		pCHAR->SendMsgViewAround ( (NET_MSG_GENERIC*)&NetMsgBrd );
	}
	
	CLUBMEMBERS_ITER pos = pCLUB->m_mapMembers.begin();
	CLUBMEMBERS_ITER end = pCLUB->m_mapMembers.end();
	PGLCHAR pMember = NULL;
	GLMSG::SNET_CLUB_DEL_2CLT NetMsgClt;
	GLMSG::SNET_CLUB_DEL_BRD NetMsgBrd;
	for ( ; pos!=end; ++pos )
	{
		const GLCLUBMEMBER &cMEMBER = (*pos).second;
		pMember = GetCharID ( cMEMBER.dwID );
		if ( !pMember )	
		{
			continue;
		}
		else
		{
			//	Note : Ŭ�� ���� ����.
			pMember->m_dwGuild = CLUB_NULL;

			//	Note : Ŭ���̾�Ʈ�� Ŭ�� ���� ����.
			//
			SENDTOCLIENT ( pMember->m_dwClientID, &NetMsgClt );

			//	Note : �ֺ� ������� �˸�.			
			NetMsgBrd.dwGaeaID = pMember->m_dwGaeaID;
			pMember->SendMsgViewAround ( (NET_MSG_GENERIC*)&NetMsgBrd );
		}
	}

	//	Note : Ŭ�� ����.
	m_cClubMan.DelClub ( pNetMsg->dwClubID );

	return TRUE;
}

BOOL GLGaeaServer::RequestClubAdd2Fld ( GLMSG::SNET_CLUB_MEMBER_ADD_2FLD *pNetMsg )
{
	GLCLUB *pCLUB = m_cClubMan.GetClub ( pNetMsg->dwClubID );
	if ( !pCLUB )		return FALSE;

	pCLUB->AddMember ( pNetMsg->dwMember, pNetMsg->szMember, NULL );

	GLMSG::SNET_CLUB_MEMBER_2CLT NetMsgMember;
	NetMsgMember.ADDMEMBER ( pNetMsg->dwMember, NULL, pNetMsg->szMember );

	PGLCHAR pMASTER = GetCharID ( pCLUB->m_dwMasterID );
	if ( pMASTER )
	{
		//	Note : Ŭ���̾�Ʈ�� �ű� ��� ���� ����.
		SENDTOCLIENT ( pMASTER->m_dwClientID, &NetMsgMember );
	}
	
	CLUBMEMBERS_ITER pos = pCLUB->m_mapMembers.begin();
	CLUBMEMBERS_ITER end = pCLUB->m_mapMembers.end();
	PGLCHAR pMember = NULL;
	for ( ; pos!=end; ++pos )
	{
		const GLCLUBMEMBER &cMEMBER = (*pos).second;
		if ( cMEMBER.dwID==pNetMsg->dwMember )	continue;		//	�ڱ� �ڽ��� ��� ����.

		pMember = GetCharID ( cMEMBER.dwID );
		if ( !pMember )							continue;		//	������� ���� �� ��츸.

		//	Note : Ŭ���̾�Ʈ�� �ű� ��� ���� ����.
		SENDTOCLIENT ( pMember->m_dwClientID, &NetMsgMember );
	}

	//	Note : �űԷ� �����ϰԵ� ��� ����.
	PGLCHAR pNEW_MEMBER = GetCharID ( pNetMsg->dwMember );
	if ( pNEW_MEMBER )
	{
		pNEW_MEMBER->m_dwGuild = pCLUB->m_dwID;

		//	Note : Ŭ�� ���� ����.
		pNEW_MEMBER->MsgClubInfo ();

		//	Note : �ֺ� ������� �˸�.
		GLMSG::SNET_CLUB_INFO_BRD NetMsgBrd;
		NetMsgBrd.dwGaeaID = pNEW_MEMBER->m_dwGaeaID;
		NetMsgBrd.dwClubID = pNEW_MEMBER->m_dwGuild;
		NetMsgBrd.dwMarkVer = pCLUB->m_dwMarkVER;
		StringCchCopy( NetMsgBrd.szNickName, CHAR_SZNAME, pNEW_MEMBER->m_szNick );
		pNEW_MEMBER->SendMsgViewAround ( (NET_MSG_GENERIC*)&NetMsgBrd );
	}

	return TRUE;
}

BOOL GLGaeaServer::RequestClubDel ( GLMSG::SNET_CLUB_MEMBER_DEL_2FLD *pNetMsg )
{
	GLCLUB *pCLUB = m_cClubMan.GetClub ( pNetMsg->dwClub );
	if ( !pCLUB )		return FALSE;

	pCLUB->SetMemberCDMFlag(pNetMsg->dwMember, FALSE);

	//	Note : Ŭ�� ������� ����.
	pCLUB->DelMember ( pNetMsg->dwMember );

	PGLCHAR pCHAR = GetCharID ( pNetMsg->dwMember );
	if ( pCHAR )
	{
		//	Ŭ�� ���� ����.
		pCHAR->m_dwGuild = CLUB_NULL;

		// Ŭ��Ż��ð� ����
		pCHAR->m_tSECEDE = pNetMsg->tSECEDE;

		//	�ڽſ��� Ŭ�� ���� �˸�.
		GLMSG::SNET_CLUB_DEL_2CLT NetMsgInfo;
		SENDTOCLIENT ( pCHAR->m_dwClientID, &NetMsgInfo );

		//	Note : �ֺ� ������� �˸�.
		GLMSG::SNET_CLUB_DEL_BRD NetMsgBrd;
		NetMsgBrd.dwGaeaID = pCHAR->m_dwGaeaID;
		pCHAR->SendMsgViewAround ( (NET_MSG_GENERIC*)&NetMsgBrd );
	}


	//	Note : ���� �޽���.
	GLMSG::SNET_CLUB_MEMBER_DEL_2CLT NetMsgClt;
	NetMsgClt.dwClub = pNetMsg->dwClub;
	NetMsgClt.dwMember = pNetMsg->dwMember;

	PGLCHAR pMASTER = GetCharID ( pCLUB->m_dwMasterID );
	if ( pMASTER )
	{
		//	Note : Ŭ���̾�Ʈ�� ���� ��� ����.
		SENDTOCLIENT ( pMASTER->m_dwClientID, &NetMsgClt );
	}
	
	CLUBMEMBERS_ITER pos = pCLUB->m_mapMembers.begin();
	CLUBMEMBERS_ITER end = pCLUB->m_mapMembers.end();
	PGLCHAR pMember = NULL;
	for ( ; pos!=end; ++pos )
	{
		const GLCLUBMEMBER &cMEMBER = (*pos).second;
		if ( cMEMBER.dwID==pNetMsg->dwMember )	continue;		//	�ڱ� �ڽ��� ��� ����.

		pMember = GetCharID ( cMEMBER.dwID );
		if ( !pMember )
		{
			continue;		//	������� ���� �� ��츸.
		}
		else
		{
			//	Note : Ŭ���̾�Ʈ�� ���� ��� ����.
			SENDTOCLIENT ( pMember->m_dwClientID, &NetMsgClt );
		}
	}

	return TRUE;
}

BOOL GLGaeaServer::RequestClubMarkChange ( GLMSG::SNET_CLUB_MARK_CHANGE_2FLD *pNetMsg )
{
	GLCLUB *pCLUB = m_cClubMan.GetClub ( pNetMsg->dwClub );
	if ( !pCLUB )		return FALSE;

	pCLUB->m_dwMarkVER = pNetMsg->dwMarkVER;
	memcpy ( pCLUB->m_aryMark, pNetMsg->aryMark, sizeof(DWORD)*EMCLUB_MARK_SX*EMCLUB_MARK_SY );

	GLMSG::SNET_CLUB_INFO_MARK_BRD NetMsgBrd;
	NetMsgBrd.dwMarkVer = pCLUB->m_dwMarkVER;
	
	CLUBMEMBERS_ITER pos = pCLUB->m_mapMembers.begin();
	CLUBMEMBERS_ITER end = pCLUB->m_mapMembers.end();
	for ( ; pos!=end; ++pos )
	{
		const GLCLUBMEMBER &cMEMBER = (*pos).second;
		PGLCHAR pMember = GetCharID ( cMEMBER.dwID );
		if ( !pMember )	continue;

		NetMsgBrd.dwGaeaID = pMember->m_dwGaeaID;

		//	Note : �ֺ� ������� �˸�.
		pMember->SendMsgViewAround ( (NET_MSG_GENERIC*)&NetMsgBrd );
	}

	return TRUE;
}

BOOL GLGaeaServer::RequestClubSubMaster ( GLMSG::SNET_CLUB_SUBMASTER_FLD *pNetMsg )
{
	GLCLUB *pCLUB = m_cClubMan.GetClub ( pNetMsg->dwClubID );
	if ( !pCLUB )		return FALSE;

	pCLUB->SetMemberFlag ( pNetMsg->dwCharID, pNetMsg->dwFlags );
	pCLUB->m_dwCDCertifior = pNetMsg->dwCDCertifior;

	DWORD dwCHARID = pNetMsg->dwCharID;
	PGLCHAR pSUB = GetCharID ( dwCHARID );
	if ( pSUB )
	{
		GLMSG::SNETPC_UPDATE_FLAGS_BRD NetMsgBrd;
		NetMsgBrd.dwGaeaID = pSUB->m_dwGaeaID;
		if ( pCLUB->IsMemberFlgCDCertify(dwCHARID) )	NetMsgBrd.dwFLAGS |= SDROP_CHAR::CLUB_CD;
		else											NetMsgBrd.dwFLAGS = NULL;

		pSUB->SendMsgViewAround ( (NET_MSG_GENERIC*)&NetMsgBrd );
	}

	dwCHARID = pCLUB->m_dwMasterID;
	PGLCHAR pMASTER = GetCharID ( dwCHARID );
	if ( pMASTER )
	{
		GLMSG::SNETPC_UPDATE_FLAGS_BRD NetMsgBrd;
		NetMsgBrd.dwGaeaID = pMASTER->m_dwGaeaID;
		if ( pCLUB->IsMemberFlgCDCertify(dwCHARID) )	NetMsgBrd.dwFLAGS |= SDROP_CHAR::CLUB_CD;
		else											NetMsgBrd.dwFLAGS = NULL;

		pMASTER->SendMsgViewAround ( (NET_MSG_GENERIC*)&NetMsgBrd );
	}

	return TRUE;
}

BOOL GLGaeaServer::RequestClubAuthority ( GLMSG::SNET_CLUB_AUTHORITY_FLD *pNetMsg )
{
	GLCLUB *pCLUB = m_cClubMan.GetClub ( pNetMsg->dwClubID );
	if ( !pCLUB )		return FALSE;
	
	if ( pNetMsg->dwCharID == GAEAID_NULL ) return FALSE;

	GLCLUBMEMBER* pMember = pCLUB->GetMember( pNetMsg->dwCharID );
	if ( !pMember ) return FALSE;

	pCLUB->m_dwMasterID = pNetMsg->dwCharID;
	StringCchCopy( pCLUB->m_szMasterName, CHAR_SZNAME, pMember->m_szName );

	GLMSG::SNET_CLUB_AUTHORITY_CLT	NetMsgClt;
	GLMSG::SNET_CLUB_AUTHORITY_BRD	NetMsgBrd;

	NetMsgClt.dwCharID = pCLUB->m_dwMasterID;
	NetMsgBrd.dwCharID = pCLUB->m_dwMasterID;

	CLUBMEMBERS_ITER pos = pCLUB->m_mapMembers.begin();
	CLUBMEMBERS_ITER end = pCLUB->m_mapMembers.end();
	for ( ; pos!=end; ++pos )
	{
		const GLCLUBMEMBER &cMEMBER = (*pos).second;
		PGLCHAR pMember = GetCharID ( cMEMBER.dwID );
		if ( !pMember )	continue;

		SENDTOCLIENT ( pMember->m_dwClientID, &NetMsgClt );

		NetMsgBrd.dwGaeaID = pMember->m_dwGaeaID;

		//	Note : �ֺ� ������� �˸�.
		pMember->SendMsgViewAround ( (NET_MSG_GENERIC*)&NetMsgBrd );
	}

	return TRUE;
}

BOOL GLGaeaServer::ReqClubMemberRename ( GLMSG::SNET_CLUB_MEMBER_RENAME_FLD* pNetMsg )
{
	//GLCLUB* pCLUB = m_cClubMan.GetClub ( pNetMsg->dwClubID );
	GLCLUB* pCLUB = GetClubMan().GetClub ( pNetMsg->dwClubID );

	// Ŭ���� ���������� Ŭ������ �̸� ����
	if ( pCLUB )
	{
		GLCLUBMEMBER* pClubMember = pCLUB->GetMember ( pNetMsg->dwCharID );

		if ( pCLUB->IsMaster ( pNetMsg->dwCharID ) )
		{
			StringCchCopy ( pCLUB->m_szMasterName , CHAR_SZNAME, pNetMsg->szName );
		}
		if ( pClubMember )
		{
			StringCchCopy ( pClubMember->m_szName , CHAR_SZNAME, pNetMsg->szName );
		}
	}

	PGLCHAR pCHAR = GLGaeaServer::GetInstance().GetCharID ( pNetMsg->dwCharID );
	if ( pCHAR )
	{
		GLGaeaServer::GetInstance().ChangeNameMap ( pCHAR, pCHAR->m_szName, pNetMsg->szName );
	}
    
	return TRUE;
}

void GLGaeaServer::CheckMarketState( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNETPC_MARKETCHECK *pNetMsg )
{
	if ( dwGaeaID == GAEAID_NULL )		return;

	PGLCHAR pChar = GetChar ( dwGaeaID );

	if( pChar == NULL )
	{
		GetConsoleMsg()->Write ( LOG_TEXT_CONSOLE, "%%%% Could not found Character..%d %%%%", dwGaeaID );
	}
}

BOOL GLGaeaServer::RequestNpcRecall( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_INVEN_NPC_RECALL *pNetMsg )
{
		
	GLChar* pChar = GetChar(dwGaeaID);
	if ( !pChar )	return FALSE;

	GLMSG::SNET_INVEN_NPC_RECALL_FB	NetMsgFB;


	//	�κ��� ������ Ȯ��.
	SINVENITEM *pINVENITEM = pChar->m_cInventory.FindPosItem ( pNetMsg->wPosX, pNetMsg->wPosY );
	if ( !pINVENITEM )
	{
		NetMsgFB.emFB = EMREQ_NPC_RECALL_FB_NOITEM;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	if ( pChar->CheckCoolTime( pINVENITEM->sItemCustom.sNativeID ) )	return FALSE;


	//	������ ���� ������.
	SITEM* pItem = GLItemMan::GetInstance().GetItem ( pINVENITEM->sItemCustom.sNativeID );
	if ( !pItem || pItem->sBasicOp.emItemType!=ITEM_NPC_RECALL )
	{
		NetMsgFB.emFB = EMREQ_NPC_RECALL_FB_NOITEM;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}
	
	SNATIVEID sID(pItem->sBasicOp.sSubID );
	PCROWDATA pCrowData = GLCrowDataMan::GetInstance().GetCrowData ( sID );
	if ( !pCrowData )
	{		
		NetMsgFB.emFB = EMREQ_NPC_RECALL_FB_NPC;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	GLLandMan* pLandMan = GetByMapID ( pChar->m_sMapID );
	if( !pLandMan )
	{
		NetMsgFB.emFB = EMREQ_NPC_RECALL_FB_FAIL;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	DWORD dwMobID = pLandMan->DropCrow ( sID, pChar->m_vPos.x, pChar->m_vPos.z );

	if ( dwMobID == UINT_MAX ) 
	{
		NetMsgFB.emFB = EMREQ_NPC_RECALL_FB_POS;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	PGLCROW pCrow = pLandMan->GetCrow( dwMobID );
    if ( !pCrow ) 
	{
		NetMsgFB.emFB = EMREQ_NPC_RECALL_FB_POS;
		SENDTOCLIENT ( dwClientID, &NetMsgFB );
		return FALSE;
	}

	pCrow->m_bNpcRecall = true;
	pCrow->m_dwCallCharID = pChar->m_dwCharID;
	pCrow->m_dwCallUserID = pChar->GetUserID();
	pCrow->m_tGenTime = CTime::GetCurrentTime().GetTime();

	//	[�ڽſ���] �ι� ������ �Ҹ��Ŵ.
	pChar->DoDrugInvenItem ( pNetMsg->wPosX, pNetMsg->wPosY, true );


	NetMsgFB.emFB = EMREQ_NPC_RECALL_FB_OK;
	SENDTOCLIENT ( dwClientID, &NetMsgFB );

	return TRUE;
}

BOOL GLGaeaServer::RequestNpcCommission( GLMSG::SNET_INVEN_NPC_COMMISSION_FB* pNetMsg )
{
	if ( pNetMsg->emFB == EMREQ_NPC_COMMISSION_CHAR )
	{
		GLChar* pChar = GLGaeaServer::GetInstance().GetCharID( pNetMsg->dwCharID );
		if ( !pChar ) return FALSE;
		
		pChar->UpdateNpcCommission( pNetMsg->lnCommission );

	}
	else if ( pNetMsg->emFB == EMREQ_NPC_COMMISSION_DB )
	{
		//	DB�� â���Ӵ� UPDATE
		SaveNpcCommissionDB( pNetMsg->dwCharID, pNetMsg->dwUserID, pNetMsg->lnCommission );	
	}

	return TRUE;
}

//	Note : �޽��� ó���� GLGaeaServer::GetInstance().FrameMove () �Լ���
//		�ۿ��� ó���ϴ°� ��Ģ���� �Ѵ�.
//
HRESULT GLGaeaServer::MsgProcess ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID )
{
	/////////////////////////////////////////////////////////////////////////////
	// AntiSpam Illegal
	if (!IsEmulatorMode())
	{
		PGLCHAR pChar1 = GetChar(dwGaeaID);
		if (pChar1 && pChar1->IsSpam()) return E_FAIL;
	}
	/////////////////////////////////////////////////////////////////////////////
	switch ( nmg->nType )
	{
		/////////////////////////////////////////////////////////////////////////////
	// LG-7 CWRanking
	case NET_MSG_GCTRL_REQ_CW_RANKING:
	{
		PGLCHAR pChar = GetChar(dwGaeaID);
		if (!pChar)		return E_FAIL;

		PGLLANDMAN pLandMan = GetByMapID(pChar->m_sMapID);
		if (!pLandMan)	return E_FAIL;

		if (pLandMan->m_bGuidBattleMap || pLandMan->m_bGuidBattleMapHall)
		{
			GLGuidance* pGuidance = GLGuidanceFieldMan::GetInstance().Find(pLandMan->m_dwClubMapID);
			if (pGuidance && pGuidance->IsBattle())		pGuidance->ReqCWRanking(dwClientID);
			else										return E_FAIL;
		}
	}
	break;
	case NET_MSG_GCTRL_REQ_MY_CW_RANKING:
	{
		PGLCHAR pChar = GetChar(dwGaeaID);
		if (!pChar)		return E_FAIL;

		PGLLANDMAN pLandMan = GetByMapID(pChar->m_sMapID);
		if (!pLandMan)	return E_FAIL;

		if (pLandMan->m_bGuidBattleMap || pLandMan->m_bGuidBattleMapHall)
		{
			GLGuidance* pGuidance = GLGuidanceFieldMan::GetInstance().Find(pLandMan->m_dwClubMapID);
			if (pGuidance && pGuidance->IsBattle())		pGuidance->ReqMyCWRanking(dwClientID, pChar->m_dwCharID);
			else										return E_FAIL;
		}
	}
	break;
	/////////////////////////////////////////////////////////////////////////////
	case NET_MSG_CHAT:
		if ( dwGaeaID == GAEAID_NULL )		return E_FAIL;
		ChatMsgProc ( nmg, dwClientID, dwGaeaID );
		break;

	case NET_MSG_CHAT_LOUDSPEAKER:

	case NET_MSG_GCTRL_REQ_READY:

	case NET_MSG_GCTRL_ACTSTATE:
	case NET_MSG_GCTRL_MOVESTATE:
	case NET_MSG_GCTRL_GOTO:
	case NET_MSG_VIETNAM_TIME_REQ:
	case NET_MSG_GCTRL_ATTACK:
	case NET_MSG_GCTRL_ATTACK_CANCEL:
	
	case NET_MSG_GCTRL_REQ_FIELD_TO_INVEN:
	case NET_MSG_GCTRL_REQ_FIELD_TO_HOLD:

	case NET_MSG_GCTRL_REQ_INVEN_TO_HOLD:
	case NET_MSG_GCTRL_REQ_HOLD_ARRAY:

	case NET_MSG_GCTRL_REQ_VNGAIN_TO_HOLD:
	case NET_MSG_GCTRL_REQ_VNGAIN_EX_HOLD:
	case NET_MSG_GCTRL_REQ_HOLD_TO_VNGAIN:
	case NET_MSG_GCTRL_REQ_VNGAIN_INVEN_RESET:
	case NET_MSG_GCTRL_REQ_VNINVEN_TO_INVEN:

	case NET_MSG_GCTRL_REQ_INVEN_EX_HOLD:
	case NET_MSG_GCTRL_REQ_INVEN_TO_SLOT:

	case NET_MSG_GCTRL_REQ_INVEN_SPLIT:

	case NET_MSG_GCTRL_REQ_SLOT_TO_HOLD:
	case NET_MSG_GCTRL_REQ_SLOT_EX_HOLD:

	case NET_MSG_GCTRL_REQ_HOLD_TO_FIELD:
	case NET_MSG_GCTRL_REQ_HOLD_TO_INVEN:
	case NET_MSG_GCTRL_REQ_HOLD_TO_SLOT:
	case NET_MSG_GCTRL_REQ_SLOT_CHANGE:

	case NET_MSG_GCTRL_REQ_BUY_FROM_NPC:
	case NET_MSG_GCTRL_REQ_SALE_TO_NPC:

	case NET_MSG_GCTRL_REQ_REBIRTH:
	case NET_MSG_GCTRL_REQ_LEVELUP:
	case NET_MSG_GCTRL_REQ_STATSUP:

	case NET_MSG_GCTRL_FIRECRACKER:
	case NET_MSG_GCTRL_REQ_INVENDRUG:
	case NET_MSG_GCTRL_REQ_DAMAGE_OVERLAP:
	case NET_MSG_GCTRL_REQ_SUICIDE_CMD:
	case NET_MSG_GCTRL_INVEN_BOXOPEN:
	case NET_MSG_GCTRL_INVEN_DISGUISE:
	case NET_MSG_GCTRL_INVEN_CLEANSER:
	case NET_MSG_GCTRL_INVEN_GMITEM:

	/*Codex System, Jhoniex 2/14/2024 */
	case NET_MSG_GCTRL_REGISTER_CODEX:

	case NET_MSG_GCTRL_CHARGED_ITEM_GET:
	case NET_MSG_GCTRL_GET_CHARGEDITEM_FROMDB:

	case NET_MSG_GCTRL_REQ_LEARNSKILL_NONINVEN:
	
	case NET_MSG_GCTRL_REQ_LEARNSKILL:
	case NET_MSG_GCTRL_REQ_SKILLUP:

	case NET_MSG_GCTRL_REQ_SKILL:
	case NET_MSG_GCTRL_SKILL_CANCEL:

	case NET_MSG_GCTRL_REQ_SKILLQ_ACTIVE:
	case NET_MSG_GCTRL_REQ_SKILLQ_SET:
	case NET_MSG_GCTRL_REQ_SKILLQ_RESET:

	case NET_MSG_GCTRL_REQ_ACTIONQ_SET:
	case NET_MSG_GCTRL_REQ_ACTIONQ_RESET:

	case NET_MSG_GCTRL_REQ_GETSTORAGE:
	case NET_MSG_GCTRL_REQ_STORAGEDRUG:
	case NET_MSG_GCTRL_REQ_STORAGESKILL:

	case NET_MSG_GCTRL_REQ_STORAGE_TO_HOLD:
	case NET_MSG_GCTRL_REQ_STORAGE_EX_HOLD:
	case NET_MSG_GCTRL_REQ_HOLD_TO_STORAGE:
	case NET_MSG_GCTRL_STORAGE_SAVE_MONEY:
	case NET_MSG_GCTRL_STORAGE_DRAW_MONEY:

	case NET_MSG_GCTRL_REQ_MONEY_TO_FIELD:
	case NET_MSG_GCTRL_REQ_STORAGE_SPLIT:
	case NET_MSG_GCTRL_INVEN_GRINDING:
	case NET_MSG_GCTRL_INVEN_RESET_SKST:

	case NET_MSG_GCTRL_REGEN_GATE:
	case NET_MSG_GCTRL_CURE:
	case NET_MSG_GCTRL_REVIVE:
	case NET_MSG_GCTRL_2_FRIEND_FLD:
	case NET_MSG_GM_MOVE2CHAR_FLD:

	case NET_MSG_GCTRL_GETEXP_RECOVERY:
	case NET_MSG_GCTRL_RECOVERY:
	case NET_MSG_GCTRL_GETEXP_RECOVERY_NPC:
	case NET_MSG_GCTRL_RECOVERY_NPC:

	case NET_MSG_GCTRL_CHARRESET:
	case NET_MSG_GCTRL_INVEN_CHARCARD:
	case NET_MSG_GCTRL_INVEN_STORAGECARD:
	case NET_MSG_GCTRL_INVEN_INVENLINE:
	case NET_MSG_GCTRL_INVEN_STORAGEOPEN:
	case NET_MSG_GCTRL_INVEN_REMODELOPEN:
	case NET_MSG_GCTRL_INVEN_GARBAGEOPEN:
	case NET_MSG_GCTRL_INVEN_STORAGECLOSE:
	case NET_MSG_GCTRL_INVEN_PREMIUMSET:
	case NET_MSG_GCTRL_INVEN_RANDOMBOXOPEN:
	case NET_MSG_GCTRL_INVEN_DISJUNCTION:

	case NET_MSG_GCTRL_INVEN_HAIR_CHANGE:
	case NET_MSG_GCTRL_INVEN_FACE_CHANGE:
	case NET_MSG_GCTRL_INVEN_HAIRSTYLE_CHANGE:
	case NET_MSG_GCTRL_INVEN_HAIRCOLOR_CHANGE:
	case NET_MSG_GCTRL_INVEN_FACESTYLE_CHANGE:
	case NET_MSG_GCTRL_INVEN_GENDER_CHANGE:
	case NET_MSG_GCTRL_INVEN_RENAME:
	case NET_MSG_GCTRL_INVEN_RENAME_FROM_DB:

	case NET_MSG_GCTRL_CARD_BRIGHT:
	//CNDev | 2-7-2020 | e-point card system
	case NET_MSG_GCTRL_CARD_EPOINT:
	case NET_MSG_GCTRL_CARD_CONTRIBUTION:
	case NET_MSG_GCTRL_INVEN_VIETNAM_INVENGET:

	case NET_MSG_GCTRL_CONFRONT_START2_FLD:
	case NET_MSG_GCTRL_CONFRONT_END2_FLD:

	case NET_MSG_GCTRL_NPC_ITEM_TRADE:

	case NET_MSG_GCTRL_REQ_GESTURE:

	case NET_MSG_GCTRL_REQ_QUEST_START:
	case NET_MSG_GCTRL_QUEST_PROG_NPCTALK:
	case NET_MSG_GCTRL_QUEST_PROG_GIVEUP:
	case NET_MSG_GCTRL_QUEST_PROG_READ:
	case NET_MSG_GCTRL_REQ_QUEST_COMPLETE:

	case NET_MSG_GCTRL_PMARKET_TITLE:
	case NET_MSG_GCTRL_PMARKET_REGITEM:
	case NET_MSG_GCTRL_PMARKET_DISITEM:
	case NET_MSG_GCTRL_PMARKET_OPEN:
	case NET_MSG_GCTRL_PMARKET_CLOSE:
	case NET_MSG_GCTRL_PMARKET_ITEM_INFO:
	case NET_MSG_GCTRL_PMARKET_BUY:

	case NET_MSG_GCTRL_CLUB_NEW:
	case NET_MSG_GCTRL_CLUB_RANK:
	case NET_MSG_GCTRL_CLUB_NEW_2FLD:
	case NET_MSG_GCTRL_CLUB_MEMBER_REQ:
	case NET_MSG_GCTRL_CLUB_MEMBER_REQ_ANS:
	case NET_MSG_GCTRL_CLUB_MEMBER_NICK:

	case NET_MSG_GCTRL_CLUB_CD_CERTIFY:
	case NET_MSG_GCTRL_CLUB_GETSTORAGE:
	case NET_MSG_GCTRL_CLUB_STORAGE_TO_HOLD:
	case NET_MSG_GCTRL_CLUB_STORAGE_EX_HOLD:
	case NET_MSG_GCTRL_CLUB_HOLD_TO_STORAGE:
	case NET_MSG_GCTRL_CLUB_STORAGE_SPLIT:
	case NET_MSG_GCTRL_CLUB_STORAGE_SAVE_MONEY:
	case NET_MSG_GCTRL_CLUB_STORAGE_DRAW_MONEY:

	case NET_MSG_GCTRL_CLUB_INCOME_RENEW:

	case NET_MSG_REBUILD_RESULT:	// ITEMREBUILD_MARK
	case NET_MSG_GCTRL_GARBAGE_RESULT:
	case NET_MSG_REBUILD_MOVE_ITEM:
	/*dmk14 reform*/
	case NET_MSG_REBUILD_MOVE_ITEMSEALED:
	case NET_MSG_REBUILD_INPUT_MONEY:

	case NET_MSG_SMS_PHONE_NUMBER:
	case NET_MSG_SMS_PHONE_NUMBER_FROM_DB:
	case NET_MSG_SMS_SEND:
	case NET_MSG_SMS_SEND_FROM_DB:

	case NET_MSG_MGAME_ODDEVEN:

	// PET
		/*dual pet skill, Juver, 2017/12/28 */
	case NET_MSG_PET_GETRIGHTOFITEM_A:
	case NET_MSG_PET_GETRIGHTOFITEM_B:

	case NET_MSG_PET_REQ_GIVEFOOD:
	case NET_MSG_PET_REQ_GETFULL_FROMDB_FB:
	case NET_MSG_PET_REQ_PETREVIVEINFO:

		/*dual pet skill, Juver, 2017/12/29 */
	case NET_MSG_PET_REQ_ENABLE_DUAL_SKILL:

	// vehicle
	case NET_MSG_VEHICLE_REQ_SLOT_EX_HOLD:
	case NET_MSG_VEHICLE_REQ_HOLD_TO_SLOT:
	case NET_MSG_VEHICLE_REQ_SLOT_TO_HOLD:
	case NET_MSG_VEHICLE_REMOVE_SLOTITEM:
	case NET_MSG_VEHICLE_REQ_GIVE_BATTERY:
	case NET_MSG_VEHICLE_REQ_GET_BATTERY_FROMDB_FB:

	case NET_MSG_GCTRL_ITEMSHOPOPEN:

	case NET_MSG_REQ_ATTEND_REWARD_FLD:
	case NET_MSG_GCTRL_INVEN_ITEM_MIX:
	case NET_MSG_REQ_GATHERING:
	case NET_MSG_REQ_GATHERING_CANCEL:

		/*itemfood system, Juver, 2017/05/25 */
	case NET_MSG_GCTRL_INVEN_CONSUME_FOOD: 
	case NET_MSG_GCTRL_INVEN_UNLOCK_FOOD:

		/*game stats, Juver, 2017/06/21 */
	case NET_MSG_GCTRL_PING_PACKET: 

		 /*rightclick wear/unwear, Juver, 2017/06/24 */
	case NET_MSG_GCTRL_REQ_INVEN_TO_WEAR:
	case NET_MSG_GCTRL_REQ_WEAR_TO_INVEN:

		/*npc shop, Juver, 2017/07/27 */
	case NET_MSG_GCTRL_NPCSHOP_PURCHASE_MONEY:

		 /*vehicle booster system, Juver, 2017/08/10 */
	case NET_MSG_VEHICLE_REQ_ENABLE_BOOSTER:
	case NET_MSG_GCTRL_REQ_VEHICLE_BOOSTER:

		/*item exchange, Juver, 2017/10/13 */
	case NET_MSG_GCTRL_NPC_ITEM_EXCHANGE_TRADE:

		/*product item, Juver, 2017/10/18 */
	case NET_MSG_GCTRL_ITEM_COMPOUND_START:
	case NET_MSG_GCTRL_ITEM_COMPOUND_CANCEL:

		/*activity system, Juver, 2017/11/05 */
	case NET_MSG_GCTRL_CHARACTER_BADGE_CHANGE:

		/*charinfoview , Juver, 2017/11/11 */
	case NET_MSG_GCTRL_REQ_CHARINFO:

		/*bike color , Juver, 2017/11/13 */
	case NET_MSG_VEHICLE_REQ_CHANGE_COLOR:

		/*rv card, Juver, 2017/11/25 */
	case NET_MSG_GCTRL_INVEN_RANDOM_OPTION_CHANGE:

		/*nondrop card, Juver, 2017/11/26 */
	case NET_MSG_GCTRL_INVEN_NONDROP_CARD:

		/*regional chat, Juver, 2017/12/06 */
	case NET_MSG_GCTRL_REGIONAL_CHAT_PAY:

		/*change scale card, Juver, 2018/01/04 */
	case NET_MSG_GCTRL_INVEN_SCALE_CHANGE:

		/*item color, Juver, 2018/01/10 */
	case NET_MSG_GCTRL_INVEN_ITEMCOLOR_CHANGE:

		/*item wrapper, Juver, 2018/01/11 */
	case NET_MSG_GCTRL_INVEN_WRAP:
	case NET_MSG_GCTRL_INVEN_UNWRAP:

		/*change school card, Juver, 2018/01/12 */
	case NET_MSG_GCTRL_INVEN_CHANGE_SCHOOL:

		/*equipment lock, Juver, 2018/01/13 */
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_ENABLE:
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_ENABLE_FROM_DB:
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_INPUT:
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_INPUT_FROM_DB:
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER:
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_FROM_DB:
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE:
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_CHANGE_FROM_DB:
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE:
	case NET_MSG_GCTRL_INVEN_EQUIPMENT_LOCK_RECOVER_DELETE_FROM_DB:

		/*storage lock, Juver, 2018/01/13 */
	case NET_MSG_GCTRL_STORAGE_LOCK_ENABLE:
	case NET_MSG_GCTRL_STORAGE_LOCK_ENABLE_FROM_DB:
	case NET_MSG_GCTRL_STORAGE_LOCK_INPUT:
	case NET_MSG_GCTRL_STORAGE_LOCK_INPUT_FROM_DB:
	case NET_MSG_GCTRL_STORAGE_LOCK_RECOVER:
	case NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_FROM_DB:
	case NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_CHANGE:
	case NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_CHANGE_FROM_DB:
	case NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_DELETE:
	case NET_MSG_GCTRL_STORAGE_LOCK_RECOVER_DELETE_FROM_DB:

		/*inventory lock, Juver, 2018/01/13 */
	case NET_MSG_GCTRL_INVENTORY_LOCK_ENABLE:
	case NET_MSG_GCTRL_INVENTORY_LOCK_ENABLE_FROM_DB:
	case NET_MSG_GCTRL_INVENTORY_LOCK_INPUT:
	case NET_MSG_GCTRL_INVENTORY_LOCK_INPUT_FROM_DB:
	case NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER:
	case NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_FROM_DB:
	case NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_CHANGE:
	case NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_CHANGE_FROM_DB:
	case NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_DELETE:
	case NET_MSG_GCTRL_INVENTORY_LOCK_RECOVER_DELETE_FROM_DB:

		/*item transfer card, Juver, 2018/01/18 */
	case NET_MSG_GCTRL_INVEN_ITEM_TRANSFER:

		/* car, cart color, Juver, 2018/02/14 */
	case NET_MSG_VEHICLE_REQ_CHANGE_CAR_COLOR:

		/* booster all vehicle, Juver, 2018/02/14 */
	case NET_MSG_ALLVEHICLE_REQ_ENABLE_BOOSTER:
	case NET_MSG_GCTRL_MYSTERY_BOX: //Mystery Box
	case NET_MSG_HP_CHECK:
		/*hackshield implementation, Juver, 2018/06/21 */
	case NET_MSG_GCTRL_HS_CALLBACK:
	case NET_MSG_GCTRL_HS_HEARTBEAT:
		/*dmk14 ingame web*/
	case NET_MSG_GCTRL_REQ_BUY_FROM_ITEMMALL:
	/*dmk14 remove active buff selection*/
	case NET_MSG_GCTRL_REQ_DISABLESKILLEFF:
	case NET_MSG_GCTRL_REQ_DISABLEITEMFACT:
	case NET_MSG_GCTRL_REQ_STATSUPCMD:
	case NET_MSG_GCTRL_REMOVE_SKILL_EFFECT_REQ:
	case NET_MSG_GM_CAMERA_ONOFF:
	case NET_MSG_RESET_STATS_REQ:
	case NET_MSG_GCTRL_INVEN_REFORM_WEAPON: //Weapon Skill Reform Card - SqueeeAK
	//PANDORA
	case NET_MSG_PANDORA_BOX_REFRESH_RESULT:
    case NET_MSG_PANDORA_BOX_OPEN_REQ:
    case NET_MSG_PANDORA_BOX_BUY_ITEM:
    case NET_MSG_PANDORA_BOX_CLEAR_RESULTS_REQ:
		{
			if ( dwGaeaID == GAEAID_NULL )		return E_FAIL;

			PGLCHAR pChar = GetChar ( dwGaeaID );
			if ( !pChar )	return S_FALSE;
			pChar->MsgProcess ( nmg );
		}
		break;

	case NET_MSG_GCTRL_NPC_RECALL:
		RequestNpcRecall( dwClientID, dwGaeaID, (GLMSG::SNET_INVEN_NPC_RECALL*) nmg );
		break;

	case NET_MSG_GCTRL_NPC_COMMISSION_FB:
		RequestNpcCommission( (GLMSG::SNET_INVEN_NPC_COMMISSION_FB*) nmg );
		break;

	case NET_MSG_MARKET_CHECKTIME:
		CheckMarketState( dwClientID, dwGaeaID, (GLMSG::SNETPC_MARKETCHECK*) nmg );
		break;

	case NET_MSG_GCTRL_CHARGED_ITEM2_INVEN:
		RequestChargedItem2Inven ( dwClientID, dwGaeaID, (GLMSG::SNET_CHARGED_ITEM2_INVEN*) nmg );
		break;

	case NET_MSG_GCTRL_CONFRONTSPTY_EXP_FLD:
		RequestConftSPtyExp ( (GLMSG::SNET_CONFT_SPTY_EXP_FLD*) nmg );
		break;

	case NET_MSG_GCTRL_REQ_GATEOUT_REQ:
		RequestGateOutReq ( dwClientID, dwGaeaID, (GLMSG::SNETREQ_GATEOUT_REQ *) nmg );
		break;

	case NET_MSG_GCTRL_CREATE_INSTANT_MAP_REQ:
		RequestCreateInstantMapReq ( dwClientID, dwGaeaID, (GLMSG::SNETREQ_CREATE_INSTANT_MAP_REQ *) nmg );
		break;
		 
	case NET_MSG_GCTRL_FIELDSVR_OUT:
		if ( dwGaeaID == GAEAID_NULL )		return E_FAIL;
		RequestFieldSvrOut ( dwClientID, dwGaeaID, (GLMSG::SNETPC_FIELDSVR_OUT *) nmg );
		break;

	case NET_MSG_GCTRL_REBIRTH_OUT:
		if ( dwGaeaID == GAEAID_NULL )		return E_FAIL;
		RequestReBirthOut ( dwClientID, dwGaeaID, (GLMSG::SNETPC_REBIRTH_OUT *) nmg );
		break;

	case NET_MSG_GCTRL_REQ_GATEOUT:
		RequestMoveMapPC ( dwClientID, dwGaeaID, (GLMSG::SNETREQ_GATEOUT *) nmg );
		break;

	case NET_MSG_GCTRL_CREATE_INSTANT_MAP_FLD:
		RequestCreateInstantMap ( dwClientID, dwGaeaID, (GLMSG::SNETREQ_CREATE_INSTANT_MAP_FLD *) nmg );
		break;

	case NET_MSG_GCTRL_REQ_LANDIN:
		if ( dwGaeaID == GAEAID_NULL )		return E_FAIL;
		RequestLandIn ( dwClientID, dwGaeaID, (GLMSG::SNETREQ_LANDIN *) nmg );
		break;

	case NET_MSG_GCTRL_TRADE:
		if ( dwGaeaID == GAEAID_NULL )		return E_FAIL;
		RequestTrade ( dwClientID, dwGaeaID, (GLMSG::SNET_TRADE *) nmg );
		break;

	case NET_MSG_GCTRL_TRADE_TAR_ANS:
		if ( dwGaeaID == GAEAID_NULL )		return E_FAIL;
		RequestTradeTarAns ( dwClientID, dwGaeaID, (GLMSG::SNET_TRADE_TAR_ANS *) nmg );
		break;

	case NET_MSG_GCTRL_TRADE_MONEY:
		if ( dwGaeaID == GAEAID_NULL )		return E_FAIL;
		RequestTradeMoney ( dwClientID, dwGaeaID, (GLMSG::SNET_TRADE_MONEY *) nmg );
		break;

	case NET_MSG_GCTRL_TRADE_ITEM_REGIST:
		if ( dwGaeaID == GAEAID_NULL )		return E_FAIL;
		RequestTradeItemResist ( dwClientID, dwGaeaID, (GLMSG::SNET_TRADE_ITEM_REGIST *) nmg );
		break;

	case NET_MSG_GCTRL_TRADE_ITEM_REMOVE:
		if ( dwGaeaID == GAEAID_NULL )		return E_FAIL;
		RequestTradeItemReMove ( dwClientID, dwGaeaID, (GLMSG::SNET_TRADE_ITEM_REMOVE *) nmg );
		break;

	case NET_MSG_GCTRL_TRADE_AGREE:
		if ( dwGaeaID == GAEAID_NULL )		return E_FAIL;
		RequestTradeAgree ( dwClientID, dwGaeaID, (GLMSG::SNET_TRADE_AGREE *) nmg );
		break;

	case NET_MSG_GCTRL_TRADE_CANCEL:
		if ( dwGaeaID == GAEAID_NULL )		return E_FAIL;
		RequestTradeCancel ( dwClientID, dwGaeaID, (GLMSG::SNET_TRADE_CANCEL *) nmg );
		break;

		/*trade lock, Juver, 2018/01/02 */
	case NET_MSG_GCTRL_TRADE_LOCK:
		if ( dwGaeaID == GAEAID_NULL )		return E_FAIL;
		RequestTradeLock ( dwClientID, dwGaeaID, (GLMSG::SNET_TRADE_LOCK *) nmg );
		break;

	case NET_MSG_FIELDSVR_CHARCHK:
		RequestFieldSvrCharChk ( dwClientID, (GLMSG::SNETPC_FIELDSVR_CHARCHK *) nmg );
		break;

	case NET_MSG_REQ_MUST_LEAVE_MAP:
		RequestMustLeaveMap ( dwClientID, dwGaeaID, (GLMSG::SNETPC_REQ_MUST_LEAVE_MAP *) nmg );
		break;

	case NET_MSG_GCTRL_REQ_RECALL:
		RequestInvenRecall ( dwClientID, dwGaeaID, (GLMSG::SNETPC_REQ_INVEN_RECALL *) nmg );
		break;

	case NET_MSG_GCTRL_REQ_TELEPORT:
		RequestInvenTeleport ( dwClientID, dwGaeaID, (GLMSG::SNETPC_REQ_INVEN_TELEPORT *) nmg );
		break;

	case NET_MSG_GCTRL_REQ_BUS:
		RequestBus ( dwClientID, dwGaeaID, (GLMSG::SNETPC_REQ_BUS *) nmg );
		break;

	case NET_MSG_GCTRL_REQ_TAXI:
		RequestTaxi ( dwClientID, dwGaeaID, (GLMSG::SNETPC_REQ_TAXI *) nmg );
		break;
	
	case NET_MSG_GCTRL_REQ_TAXI_NPCPOS:
		RequestTaxiNpcPos ( dwClientID, dwGaeaID, (GLMSG::SNETPC_REQ_TAXI_NPCPOS *) nmg );
		break;

	case NET_MSG_GCTRL_2_FRIEND_CK:
		Request2FriendCK ( (GLMSG::SNETPC_2_FRIEND_CK *)nmg );
		break;

	case NET_MSG_GM_MOVE2CHAR_POS:
		RequestMove2CharPos ( dwClientID, (GLMSG::SNETPC_GM_MOVE2CHAR_POS *)nmg );
		break;

	case NET_MSG_GM_VIEWALLPLAYER_FLD_REQ:
		RequestViewAllPlayer ( (GLMSG::SNET_GM_VIEWALLPLAYER_FLD_REQ *)nmg );
		break;

	case NET_MSG_GM_SHOP_INFO_REQ:
		RequestShopInfo ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_SHOP_INFO_REQ *)nmg ); 
		break;

	case NET_MSG_GCTRL_PMARKET_SEARCH_ITEM:
		ReqSearchShopItem ( dwClientID, dwGaeaID, (GLMSG::SNETPC_PMARKET_SEARCH_ITEM *)nmg );
		break;

	case NET_MSG_GCTRL_PMARKET_SEARCH_ITEM_RESULT_REQ:
		//ReqSearchResultShopItem ( dwClientID, dwGaeaID, (GLMSG::SNETPC_PMARKET_SEARCH_ITEM_RESULT_REQ *)nmg );
		break;

	case NET_MSG_GCTRL_ACTIVE_VEHICLE_CONFIRM:
		ReqActiveVehicleConfirm( dwClientID, dwGaeaID, (GLMSG::SNETPC_ACTIVE_VEHICLE_CONFIRM*) nmg );
		break;

	/* anti-zoomout cheat fix, SeiferXIII 2020/05/17 */
	case NET_MSG_ZOOMOUT_CHEAT_DETECTED:
		ReqZoomOutCheatDetected( dwClientID, dwGaeaID, (GLMSG::SNETPC_ZOOMOUT_CHEAT_DETECTED*) nmg );
		break;

	case NET_MSG_SPEEDHACK_DETECTED:
		GerberSecDetSpdHack(dwClientID, dwGaeaID);
		break;

	//case NET_MSG_HACKSHIELD_HEARTBEAT:
		//GerberHackShieldHeartbeat( dwClientID, dwGaeaID );
		//break;

	case NET_MSG_GCTRL_PARTY_FNEW:
	case NET_MSG_GCTRL_PARTY_ADD:
	case NET_MSG_GCTRL_PARTY_DEL:
	case NET_MSG_GCTRL_PARTY_DISSOLVE:
	case NET_MSG_GCTRL_PARTY_MBR_MOVEMAP:
	case NET_MSG_GCTRL_PARTY_MBR_RENAME_FLD:
	case NET_MSG_GCTRL_PARTY_MASTER_RENEW:
	case NET_MSG_GCTRL_PARTY_AUTHORITY:
		{
			m_cPartyFieldMan.MsgProcess ( nmg, dwClientID, dwGaeaID );
		}
		break;

	case NET_MSG_GCTRL_CLUB_RANK_2FLD:
		RequestClubRank2Fld ( (GLMSG::SNET_CLUB_RANK_2FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_INFO_2FLD:
		RequestClubInfo ( (GLMSG::SNET_CLUB_INFO_2FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_DISSOLUTION_2FLD:
		RequestClubDissolution2Fld ( (GLMSG::SNET_CLUB_DISSOLUTION_2FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_DEL_2FLD:
		RequestClubDel2Fld ( (GLMSG::SNET_CLUB_DEL_2FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_MEMBER_ADD_2FLD:
		RequestClubAdd2Fld ( (GLMSG::SNET_CLUB_MEMBER_ADD_2FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_MEMBER_DEL_2FLD:
		RequestClubDel ( (GLMSG::SNET_CLUB_MEMBER_DEL_2FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_MARK_CHANGE_2FLD:
		RequestClubMarkChange ( (GLMSG::SNET_CLUB_MARK_CHANGE_2FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_SUBMASTER_FLD:
		RequestClubSubMaster ( (GLMSG::SNET_CLUB_SUBMASTER_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_AUTHORITY_FLD:
		RequestClubAuthority( (GLMSG::SNET_CLUB_AUTHORITY_FLD *)nmg );

	case NET_MSG_GCTRL_CONFRONTPTY_CHECKMBR_FLD:
		RequestConfrontPartyChkMbr ( (GLMSG::SNETPC_CONFRONTPTY_CHECKMBR2_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CONFRONTPTY_START2_FLD:
		RequestConfrontParty ( (GLMSG::SNETPC_CONFRONTPTY_START2_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CONFRONTPTY_END2_FLD:
		RequestConfrontPartyEnd ( (GLMSG::SNETPC_CONFRONTPTY_END2_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CONFRONTCLB_CHECKMBR_FLD:
		RequestConfrontClubChkMbr ( (GLMSG::SNET_CONFRONTCLB_CHECKMBR_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CONFRONTCLB_START2_FLD:
		RequestConfrontClub ( (GLMSG::SNETPC_CONFRONTCLB_START2_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CONFRONTCLB_END2_FLD:
		RequestConfrontClubEnd ( (GLMSG::SNETPC_CONFRONTCLB_END2_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_DEATHMATCH_READY_FLD:
		ServerClubDeathMatchReady ( (GLMSG::SNET_CLUB_DEATHMATCH_READY_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_DEATHMATCH_START_FLD:
		ServerClubDeathMatchStart ( (GLMSG::SNET_CLUB_DEATHMATCH_START_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_DEATHMATCH_END_FLD:
		ServerClubDeathMatchEnd ( (GLMSG::SNET_CLUB_DEATHMATCH_END_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_BATTLE_START_FLD:
		ServerClubBattleStart ( (GLMSG::SNET_CLUB_BATTLE_START_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_BATTLE_END_FLD:
		ServerClubBattleEnd ( (GLMSG::SNET_CLUB_BATTLE_END_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_CERTIFIED_FLD:
		ServerClubCertify ( (GLMSG::SNET_CLUB_CERTIFIED_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_COMMISSION_FLD:
		ServerClubGuidCommission ( (GLMSG::SNET_CLUB_GUID_COMMISSION_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_NOTICE_FLD:
		ServerClubNotice ( (GLMSG::SNET_CLUB_NOTICE_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_STORAGE_GET_DB:
		ServerClubStorageGetDB ( (GLMSG::SNET_CLUB_STORAGE_GET_DB *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_INCOME_DN:
		ServerClubInComeDn ( dwClientID, (GLMSG::SNET_CLUB_INCOME_DN *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_ALLIANCE_ADD_FLD:
		ServerClubAllianceAddFld ( (GLMSG::SNET_CLUB_ALLIANCE_ADD_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_ALLIANCE_DEL_FLD:
		ServerClubAllianceDel ( (GLMSG::SNET_CLUB_ALLIANCE_DEL_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_ALLIANCE_DIS_FLD:
		ServerClubAllianceDis ( (GLMSG::SNET_CLUB_ALLIANCE_DIS_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_CLUB_BATTLE_BEGIN_FLD:
		ServerClubBattleBeginFld( ( GLMSG::SNET_CLUB_BATTLE_BEGIN_FLD*) nmg );
        break;

	case NET_MSG_GCTRL_CLUB_BATTLE_OVER_FLD:
		ServerClubBattleOverFld( ( GLMSG::SNET_CLUB_BATTLE_OVER_FLD*) nmg );
		break;
	case NET_MSG_GCTRL_CLUB_BATTLE_KILL_UPDATE_FLD:
		ServerClubBattleKillUpdate( (GLMSG::SNET_CLUB_BATTLE_KILL_UPDATE_FLD*) nmg );
		break;

	case NET_MSG_GCTRL_CLUB_BATTLE_LAST_KILL_UPDATE_FLD:
		ServerClubBattleLastKillUpdate( (GLMSG::SNET_CLUB_BATTLE_LAST_KILL_UPDATE_FLD*) nmg );
		break;

	case NET_MSG_GCTRL_CLUB_MBR_RENAME_FLD:
		ReqClubMemberRename ( (GLMSG::SNET_CLUB_MEMBER_RENAME_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_LEVEL_EVENT_END_FLD:
		ServerLevelEventEnd( (GLMSG::SNET_LEVEL_EVENT_END_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_LEVEL_EVENT_WARNING_FLD:
		ServerLevelEventWarning( (GLMSG::SNET_LEVEL_EVENT_WARNING_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_LEVEL_EVENT_COUNTDOWN_FLD:
		ServerLevelEventCountdown( (GLMSG::SNET_LEVEL_EVENT_COUNTDOWN_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_SLOT_EVENT_GEN_FLD:
		ServerSlotEventGen( (GLMSG::SNET_SLOT_EVENT_GEN_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_REQ_GENITEM_FLD_FB:
		RequestGenItemFieldFB ( (GLMSG::SNET_REQ_GENITEM_FLD_FB *)nmg );
		break;

	case NET_MSG_GCTRL_FIELDINFO_RESET:
		ServerFieldInfoReset ( (GLMSG::SNET_FIELDINFO_RESET *)nmg );
		break;

	case NET_MSG_GCTRL_SCHOOLFREEPK_FLD:
		ServerSchoolFreePk ( (GLMSG::SNETPC_SCHOOLFREEPK_FLD *)nmg );
		break;

	case NET_MSG_GCTRL_PERIOD:
		ServerCtrlPeriod ( (GLMSG::SNET_PERIOD *) nmg );
		break;

	case NET_MSG_SERVER_CTRL_WEATHER:
		ServerCtrlWeather ( (GLMSG::SNETSERVER_CTRL_WEATHER *) nmg );
		break;

	case NET_MSG_SERVER_CTRL_WEATHER2:
		ServerCtrlWeather2 ( (GLMSG::SNETSERVER_CTRL_WEATHER2 *) nmg );
		break;

	case NET_MSG_SERVER_CTRL_TIME:
		ServerCtrlTime ( (GLMSG::SNETSERVER_CTRL_TIME *) nmg );
		break;

	case NET_MSG_SERVER_CTRL_MONTH:
		ServerCtrlMonth ( (GLMSG::SNETSERVER_CTRL_MONTH *) nmg );
		break;

	case NET_MSG_SERVER_CTRL_GENITEMHOLD:
		ServerCtrlGenItemHold ( (GLMSG::SNETSERVER_CTRL_GENITEMHOLD *) nmg );
		break;

	case NET_MSG_SERVER_PLAYERKILLING_MODE:
		if ( dwClientID != m_dwAgentSlot ) break;
		ServerCtrlPlayerKillingMode ( (GLMSG::SNET_SERVER_PLAYERKILLING_MODE *) nmg );
		break;

	case NET_MSG_CYBERCAFECLASS_UPDATE:
		CyberCafeClassUpdate  ( dwClientID, dwGaeaID, (GLMSG::SNET_CYBERCAFECLASS_UPDATE *) nmg );
		break;

	case NET_MSG_GCTRL_DROP_OUT_FORCED:
		ServerCtrlCharDropOutForced ( (GLMSG::SNET_DROP_OUT_FORCED *)nmg );
		break;

	case NET_MSG_GM_MOVE2MAPPOS_FLD:
		GMCtrolMove2MapPos ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_MOVE2MAPPOS_FLD*) nmg );
		break;

	case NET_MSG_GM_WHERE_NPC_FLD:
		GMCtrlWhereNpc ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_WHERE_NPC_FLD*) nmg );
		break;

	case NET_MSG_GM_WARNING_MSG_FLD:
		GMCtrlWarningMSG ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_WARNING_MSG_FLD*) nmg );
		break;

	case NET_MSG_GM_WHERE_PC_POS:
		GMCtrolWherePcPos ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_WHERE_PC_POS*) nmg );
		break;

	case NET_MSG_GM_BIGHEAD:
		GMCtrolBigHead ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_BIGHEAD*) nmg );
		break;

	case NET_MSG_GM_BIGHAND:
		GMCtrolBigHand ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_BIGHAND*) nmg );
		break;

	case NET_MSG_GM_EVENT_EX:
		GMCtrolEventEx ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_EVENT_EX*) nmg );			
		break;

	case NET_MSG_GM_EVENT_EX_END:			
		GMCtrolEventExEnd ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_EVENT_EX_END*) nmg  );		
		break;

	case NET_MSG_GM_LIMIT_EVENT_BEGIN:
		GMCtrolLimitEventBegin ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_LIMIT_EVENT_BEGIN*) nmg );			
		break;

	case NET_MSG_GM_LIMIT_EVENT_TIME_RESET:
		GMCtrolLimitEventTimeReset  ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_LIMIT_EVENT_TIME_RESET*) nmg );			
		break;

	case NET_MSG_GM_LIMIT_EVENT_END:			
		GMCtrolLimitEventEnd ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_LIMIT_EVENT_END*) nmg  );		
		break;

	case NET_MSG_GM_MOB_GEN_FLD:
		GMCtrolMobGen ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_MOB_GEN_FLD*) nmg );
		break;

	case NET_MSG_GM_MOB_GEN_EX_FLD:
		GMCtrolMobGenEx ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_MOB_GEN_EX_FLD*) nmg );
		break;

	case NET_MSG_GM_PRINT_CROWLIST_FLD:
		GMCtrolMoPrintCrowList ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_PRINT_CROWLIST_FLD*) nmg );
		break;

	case NET_MSG_GM_MOB_DEL_EX_FLD:
		GMCtrolMobDelEx ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_MOB_DEL_EX_FLD*) nmg );
		break;

	case NET_MSG_GM_MOB_DEL_FLD:
		GMCtrolMobDel ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_MOB_DEL_FLD*) nmg );
		break;

	case NET_MSG_GM_MOB_LEVEL:
		GMCtrolMobLevel ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_MOB_LEV*) nmg );
		break;

	case NET_MSG_GM_MOB_LEVEL_CLEAR:
		GMCtrolMobLevelClear ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_MOB_LEV_CLEAR*) nmg );
		break;

	case NET_MSG_GM_CHAT_BLOCK_FLD:
		if ( dwClientID != m_dwAgentSlot ) break;
		GMCtrolChatBlockFld ( (GLMSG::SNET_GM_CHAT_BLOCK_FLD*) nmg );
		break;

	case NET_MSG_USER_CHAR_INFO_FLD:
		if ( dwClientID != m_dwAgentSlot ) break;
		UserCtrolCharInfoFld ( (GLMSG::SNET_USER_CHAR_INFO_FLD*) nmg );
		break;

	case NET_MSG_GM_CHAR_INFO_FLD:
		if ( dwClientID != m_dwAgentSlot ) break;
		GMCtrolCharInfoFld ( (GLMSG::SNET_GM_CHAR_INFO_FLD*) nmg );
		break;

	case NET_MSG_GM_FREEPK:
		GMCtrolFreePK ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_FREEPK*) nmg );
		break;

	case NET_MSG_GM_MOVE2GATE_FLD:
		GMCtrolMove2Gate ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_MOVE2GATE_FLD *) nmg );
		break;

	case NET_MSG_GCTRL_SERVERTEST:
		if( dwClientID != 0 ) break;
		ReserveServerStop ();
		break;

	case NET_MSG_GM_KICK_USER_PROC_FLD:
		GMKicUser ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_KICK_USER_PROC_FLD *) nmg );
		break;

	case NET_MSG_GM_SHOWMETHEMONEY_FLD:
		GMCtrolShowMeTheMoney ( dwClientID, dwGaeaID, (GLMSG::SNET_GM_SHOWMETHEMONEY_FLD *) nmg );
		break;

		/*private market set, Juver, 2018/01/02 */
	case NET_MSG_GM_SET_PRIVATE_MARKET:
		GMCtrolSetPrivateMarket( dwClientID, dwGaeaID, (GLMSG::SNET_GM_SET_PRIVATE_MARKET*) nmg );
		break;

		/*megaphone set, Juver, 2018/01/02 */
	case NET_MSG_GM_SET_MEGAPHONE:
		GMCtrolSetMegaPhone( dwClientID, dwGaeaID, (GLMSG::SNET_GM_SET_MEGAPHONE*) nmg );
		break;

#if defined(VN_PARAM) //vietnamtest%%%
	case NET_MSG_VIETNAM_ALLINITTIME:
		VietnamAllInitTime ( (GLMSG::SNETPC_VIETNAM_ALLINITTIME *)nmg );
		break;
#endif

	case NET_MSG_GM_EVENT_ITEM_GEN:
	case NET_MSG_GM_EVENT_ITEM_GEN_END:
	case NET_MSG_GM_EVENT_MONEY_GEN:
	case NET_MSG_GM_EVENT_MONEY_GEN_END:
	case NET_MSG_GM_EVENT_EXP:
	case NET_MSG_GM_EVENT_EXP_END:
		m_cEventProc.MsgProcess ( nmg, dwClientID, dwGaeaID );
		break;

	// Ż��
	case NET_MSG_GCTRL_ACTIVE_VEHICLE:
		ReqActiveVehicle ( dwClientID, dwGaeaID, (GLMSG::SNETPC_ACTIVE_VEHICLE*) nmg );
		break;
	case NET_MSG_GCTRL_GET_VEHICLE:
		ReqGetVehicle( dwClientID, dwGaeaID, (GLMSG::SNETPC_GET_VEHICLE*) nmg );
		break;
	case NET_MSG_VEHICLE_CREATE_FROMDB_FB:
		CreateVehicleOnDB( dwClientID, dwGaeaID, (GLMSG::SNET_VEHICLE_CREATE_FROMDB_FB*) nmg );
		break;
	case NET_MSG_VEHICLE_GET_FROMDB_FB:
		GetVehicleInfoFromDB( dwClientID, dwGaeaID, (GLMSG::SNET_VEHICLE_GET_FROMDB_FB*) nmg );
		break;	
	case NET_MSG_VEHICLE_GET_FROMDB_ERROR:
		GetVehicleInfoFromDBError( dwClientID, dwGaeaID, (GLMSG::SNET_VEHICLE_GET_FROMDB_ERROR*) nmg );
		break;	
	case NET_MSG_VEHICLE_REQ_ITEM_INFO:
		GetVehicleItemInfo ( dwClientID, dwGaeaID, (GLMSG::SNET_VEHICLE_REQ_ITEM_INFO*) nmg );
		break;

	// ��Ȱ ��ų ��� ����
	case NET_NON_REBIRTH_REQ:
		SetNonRebirth ( dwClientID, dwGaeaID, (GLMSG::SNET_NON_REBIRTH_REQ*) nmg );
		break;
	// QBox On/Off �ɼ�
	case NET_QBOX_OPTION_REQ_FLD:
		ReqQBoxEnable ( dwClientID, dwGaeaID, (GLMSG::SNET_QBOX_OPTION_REQ_FLD*) nmg );
		break;

		/*charinfoview , Juver, 2017/11/12 */
	case NET_PRIVATE_STATS_REQ:
		SetPrivateStats( dwClientID, dwGaeaID, (GLMSG::SNET_PRIVATE_STATS_REQ*) nmg );
		break;

	case NET_DISTRADE_REQ:
		SetDisTrade( dwClientID, dwGaeaID, (GLMSG::SNET_DISTRADE_REQ*) nmg );
		break;

	case NET_MSG_GCTRL_CLUB_DEATHMATCH_RANKING_REQ:
		ReqClubDeathMatchRanking ( dwClientID, dwGaeaID, (GLMSG::SNET_CLUB_DEATHMATCH_RANKING_REQ*) nmg );
		break;

	/*dmk14 ingame web*/
	case NET_MSG_GCTRL_GET_ITEMMALL_FROMDB:
		ReqItemMallList( dwClientID, dwGaeaID, (GLMSG::SNET_GET_ITEMMALL_FROMDB*) nmg );
		break;

//----------------------------------------------------------------------------------------------------------------
// Message Processing about PET (START)
//----------------------------------------------------------------------------------------------------------------

	// =======  Message Processing about MyPET From Owner (START)  ======= //

	case NET_MSG_PET_REQ_USECARD:								
		RequestUsePETCARD ( dwClientID, dwGaeaID, ( GLMSG::SNETPET_REQ_USEPETCARD* ) nmg );
		break;

	case NET_MSG_PET_REQ_UNUSECARD:
		{
			GLMSG::SNETPET_REQ_UNUSEPETCARD* pNetMsg = ( GLMSG::SNETPET_REQ_UNUSEPETCARD* ) nmg;
			PGLPETFIELD pPet = GetPET ( pNetMsg->dwGUID );
			if ( pPet && pPet->GetPetID () == pNetMsg->dwPetID )
			{
                DropOutPET ( pNetMsg->dwGUID, false, false );
			}
		}
		break;

	case NET_MSG_PET_REQ_REVIVE:
		RequestRevivePet ( dwClientID, dwGaeaID, ( GLMSG::SNETPET_REQ_REVIVE* ) nmg );
		break;

	case NET_MSG_PET_REQ_REVIVE_FROMDB_FB:
		RevivePet ( dwClientID, dwGaeaID, ( GLMSG::SNETPET_REQ_REVIVE_FROMDB_FB* ) nmg );
		break;

	case NET_MSG_CREATE_PET_FROMDB_FB:
		CreatePETOnDB ( dwClientID, dwGaeaID, ( GLMSG::SNETPET_CREATEPET_FROMDB_FB* ) nmg );
		break;

	case NET_MSG_GET_PET_FROMDB_FB:
		GetPETInfoFromDB ( dwClientID, dwGaeaID, ( GLMSG::SNETPET_GETPET_FROMDB_FB* ) nmg );
		break;
	case NET_MSG_GET_PET_FROMDB_ERROR:
		GetPETInfoFromDBError ( dwClientID, dwGaeaID, ( GLMSG::SNETPET_GETPET_FROMDB_ERROR* ) nmg );
		break;

	case NET_MSG_PET_REQ_PETCARDINFO:
		{
			GLMSG::SNETPET_REQ_PETCARDINFO* pNetMsg = ( GLMSG::SNETPET_REQ_PETCARDINFO* ) nmg;

			PGLCHAR pChar = GetChar ( dwGaeaID );
			if ( !pChar ) break;

			// �� �������� Ȯ��
			if ( pNetMsg->dwPetID <= 0 || pChar->m_dwCharID <= 0 )
			{
				GetConsoleMsg()->Write ( LOG_TEXT_CONSOLE, "Invalid ID, PetID %d CharID %d dwClientID %d", 
										 pNetMsg->dwPetID, pChar->m_dwCharID, dwClientID );
				break;
			}

			// �ŷ��� ���� ��ī�� �����̸�
			if ( pNetMsg->bTrade )
			{
				PGLPET pPetInfo = new GLPET ();

				// DB�۾� ���н� Ȥ�� ������ �޸� ���������
				CGetPet* pDbAction = new CGetPet ( pPetInfo, pNetMsg->dwPetID, dwClientID, pChar->m_dwCharID, 
												   0, 0, false, false, true );
				m_pDBMan->AddJob ( pDbAction );
			}
			// �κ��丮,â���� ���� ��ī�� �����̸�
			else
			{
				PGLPET pPetInfo = new GLPET ();

				// DB�۾� ���н� Ȥ�� ������ �޸� ���������
				CGetPet* pDbAction = new CGetPet ( pPetInfo, pNetMsg->dwPetID, dwClientID, pChar->m_dwCharID, 
												   0, 0, false, true );
				m_pDBMan->AddJob ( pDbAction );
			}
		}
		break;

	// =======  Message Processing about MyPET From Owner (END)  ======= //

	// =======  Message Processing about MyPET From PET (START)  ======= //

	case NET_MSG_PET_REQ_GOTO:
	case NET_MSG_PET_REQ_STOP:
	case NET_MSG_PET_REQ_UPDATE_MOVE_STATE:

		/*dual pet skill, Juver, 2017/12/27 */
	case NET_MSG_PET_REQ_SKILLCHANGE_A:
	case NET_MSG_PET_REQ_SKILLCHANGE_B:

	case NET_MSG_PET_REQ_RENAME:
	case NET_MSG_PET_REQ_RENAME_FROMDB_FB:
	case NET_MSG_PET_REQ_CHANGE_COLOR:
	case NET_MSG_PET_REQ_CHANGE_STYLE:
	case NET_MSG_PET_REQ_SLOT_EX_HOLD:
	case NET_MSG_PET_REQ_HOLD_TO_SLOT:
	case NET_MSG_PET_REQ_SLOT_TO_HOLD:
	case NET_MSG_PET_REQ_LEARNSKILL:
	case NET_MSG_PET_REQ_FUNNY:
	case NET_MSG_PET_REMOVE_SLOTITEM:
	case NET_MSG_PET_PETSKINPACKOPEN:
		{
			PGLCHAR pOwner = GetChar ( dwGaeaID );
			if ( pOwner )
			{
				PGLPETFIELD pPet = GetPET ( pOwner->m_dwPetGUID );
				if ( pPet )
				{
					pPet->MsgProcess ( nmg );
				}
			}
		}
		break;

	// =======  Message Processing about MyPET From PET (END)  ======= //

	//----------------------------------------------------------------------------------------------------------------
	// Message Processing about SUMMON (START)
	//----------------------------------------------------------------------------------------------------------------
	// =======  Message Processing about MySUMMON From Owner (END)  ======= //

	// =======  Message Processing about MySUMMON From SUMMON (START)  ======= //

	case NET_MSG_SUMMON_REQ_GOTO:
		{
			/*skill summon, Juver, 2017/10/09 */
			GLMSG::SNET_SUMMON_REQ_GOTO* pNetMsg = ( GLMSG::SNET_SUMMON_REQ_GOTO* ) nmg;
			PGLSUMMONFIELD pSummon = GetSummon ( pNetMsg->dwGUID );
			if ( pSummon )
			{
				pSummon->MsgProcess ( nmg );
			}
		}break;

	case NET_MSG_SUMMON_REQ_STOP:
		{
			/*skill summon, Juver, 2017/10/09 */
			GLMSG::SNET_SUMMON_REQ_STOP* pNetMsg = ( GLMSG::SNET_SUMMON_REQ_STOP* ) nmg;
			PGLSUMMONFIELD pSummon = GetSummon ( pNetMsg->dwGUID );
			if ( pSummon )
			{
				pSummon->MsgProcess ( nmg );
			}
		}break;

	case NET_MSG_SUMMON_REQ_UPDATE_MOVE_STATE:
		{
			/*skill summon, Juver, 2017/10/09 */
			GLMSG::SNET_SUMMON_REQ_UPDATE_MOVESTATE* pNetMsg = ( GLMSG::SNET_SUMMON_REQ_UPDATE_MOVESTATE* ) nmg;
			PGLSUMMONFIELD pSummon = GetSummon ( pNetMsg->dwGUID );
			if ( pSummon )
			{
				pSummon->MsgProcess ( nmg );
			}
		}
		break;


	// =======  Message Processing about MySUMMON From SUMMON (END)  ======= //

	/*event map move, Juver, 2017/08/25 */
	case NET_MSG_GCTRL_REQ_EVENT_MOVEMAP_FLD:
		{
			GLMSG::SNETPC_REQ_EVENT_MOVEMAP_FLD* pNetMsg = ( GLMSG::SNETPC_REQ_EVENT_MOVEMAP_FLD* ) nmg;
			EventMoveMap( pNetMsg->dwCharID, pNetMsg->nidMAP, pNetMsg->wPosX, pNetMsg->wPosY, pNetMsg->bCurMapCheck, pNetMsg->bCalculatePos );
		}break;

	case NET_MSG_DROPCHAR_TOFIELD:
		MsgDropCharFromAgent ( dwClientID, dwGaeaID, (GLMSG::SNET_DROPCHAR_TOFIELD *) nmg );
		break;

		/*pvp tyranny, Juver, 2017/08/25 */
	case NET_MSG_GCTRL_TYRANNY_A2FC_STATE_REGISTER:
	case NET_MSG_GCTRL_TYRANNY_A2FC_STATE_BATTLE:
	case NET_MSG_GCTRL_TYRANNY_A2FC_STATE_REWARD:
	case NET_MSG_GCTRL_TYRANNY_A2FC_STATE_ENDED:
	case NET_MSG_GCTRL_TYRANNY_A2FC_NEXTSCHED:
	case NET_MSG_GCTRL_TYRANNY_A2F_PLAYER_DATA:
	case NET_MSG_GCTRL_TYRANNY_A2F_TOWER_CAPTURE:
	case NET_MSG_GCTRL_TYRANNY_A2F_TOWER_WINNER:

	case NET_MSG_GCTRL_TYRANNY_C2F_REVIVE_REQ:
		{
			GLPVPTyrannyField::GetInstance().MsgProcess( nmg, dwClientID, dwGaeaID );
		}break;

		/*pvp tyranny, Juver, 2017/08/25 */
	case NET_MSG_GCTRL_TYRANNY_A2F_MAP_MOVE:
		{
			GLMSG::SNETPC_TYRANNY_A2F_MAP_MOVE* pNetMsg = ( GLMSG::SNETPC_TYRANNY_A2F_MAP_MOVE* ) nmg;
			TyrannyMoveMap( pNetMsg->dwCharID, pNetMsg->nidMAP, pNetMsg->vPos, pNetMsg->bCurMapCheck );
		}break;

		/*pvp tyranny, Juver, 2017/08/25 */
	case NET_MSG_GCTRL_TYRANNY_A2F_BUFF_RESET:
		{
			TyrannyResetBuff ( (GLMSG::SNETPC_TYRANNY_A2F_BUFF_RESET *)nmg );
		}break;

		/*pvp tyranny, Juver, 2017/08/25 */
	case NET_MSG_GCTRL_TYRANNY_A2F_BUFF_REWARD:
		{
			TyrannyRewardBuff ( (GLMSG::SNETPC_TYRANNY_A2F_BUFF_REWARD *)nmg );
		}break;

		/*pvp tyranny, Juver, 2017/08/25 */
	case NET_MSG_GCTRL_TYRANNY_A2F_BUFF_BATTLE:
		{
			TyrannyBattleBuff ( (GLMSG::SNETPC_TYRANNY_A2F_BUFF_BATTLE *)nmg );
		}break;

		/*school wars, Juver, 2018/01/19 */
	case NET_MSG_GCTRL_SCHOOLWARS_A2FC_STATE_REGISTER:
	case NET_MSG_GCTRL_SCHOOLWARS_A2FC_STATE_BATTLE:
	case NET_MSG_GCTRL_SCHOOLWARS_A2FC_STATE_REWARD:
	case NET_MSG_GCTRL_SCHOOLWARS_A2FC_STATE_ENDED:
	case NET_MSG_GCTRL_SCHOOLWARS_A2FC_NEXTSCHED:
	case NET_MSG_GCTRL_SCHOOLWARS_A2F_PLAYER_DATA:
	case NET_MSG_GCTRL_SCHOOLWARS_A2F_SCORE_UPDATE:
	case NET_MSG_GCTRL_SCHOOLWARS_C2F_REVIVE_REQ:
	case NET_MSG_GCTRL_SCHOOLWARS_A2F_SCORE_WINNER:
		{
			GLPVPSchoolWarsField::GetInstance().MsgProcess( nmg, dwClientID, dwGaeaID );
		}break;

		/*school wars, Juver, 2018/01/19 */
	case NET_MSG_GCTRL_SCHOOLWARS_A2F_MAP_MOVE:
		{
			GLMSG::SNETPC_SCHOOLWARS_A2F_MAP_MOVE* pNetMsg = ( GLMSG::SNETPC_SCHOOLWARS_A2F_MAP_MOVE* ) nmg;
			SchoolWarsMoveMap( pNetMsg->dwCharID, pNetMsg->nidMAP, pNetMsg->vPos, pNetMsg->bCurMapCheck );
		}break;

		/*pvp capture the flag, Juver, 2018/01/24 */
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_STATE_REGISTER:
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_STATE_BATTLE:
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_STATE_REWARD:
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_STATE_ENDED:
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_NEXTSCHED:
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2F_PLAYER_DATA:
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2F_CAPTURE_UPDATE:
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_C2F_REVIVE_REQ:
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2F_WINNER:

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_C2F_FLAG_ACTION_CAPTURE:
		{
			GLPVPCaptureTheFlagField::GetInstance().MsgProcess( nmg, dwClientID, dwGaeaID );
		}break;

		/*pvp capture the flag, Juver, 2018/01/29 */
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2F_PLAYER_TEAM:
		{
			CaptureTheFlagPlayerTeam ( (GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_PLAYER_TEAM *)nmg );
		}break;

		/*pvp capture the flag, Juver, 2018/01/30 */
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2F_MAP_MOVE:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_MAP_MOVE* pNetMsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_MAP_MOVE* ) nmg;
			CaptureTheFlagMoveMap( pNetMsg->dwCharID, pNetMsg->nidMAP, pNetMsg->vPos, pNetMsg->bCurMapCheck );
		}break;

	default:
		{
			PGLCHAR pChar = GetChar ( dwGaeaID );
			if ( pChar )
			{
				GetConsoleMsg()->Write ( LOG_CONSOLE,
			                    "[WARNING]GLGaeaServer::MsgProcess illigal message(%d) Name(%s)",
								nmg->nType, pChar->m_szName );

				pChar->SetSpam();
			}
			else
			{
				GetConsoleMsg()->Write ( LOG_CONSOLE,
			                    "[WARNING]GLGaeaServer::MsgProcess illigal message(%d)",
			                    nmg->nType);
			}
			GLMSG::SNETPC_REQ_CLOSE_CLIENT_AGT NetMsg;
			NetMsg.dwGaeaID = dwGaeaID;
			SENDTOAGENT(&NetMsg);
		}
		break;
	};

	return S_OK;
}

BOOL GLGaeaServer::MsgDropCharFromAgent( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_DROPCHAR_TOFIELD* pNetMsgFld )
{
	if ( dwClientID != m_dwAgentSlot ) return FALSE;

	PGLCHAR pTARCHAR = GetCharID ( pNetMsgFld->dwCHARID );
	if ( !pTARCHAR ) return FALSE;

	if( GetConsoleMsg() )
	{
		GetConsoleMsg()->Write( "FieldServer AutoDrop Stuck Char (ID:%d NAME:%s ControlID:%d)", pTARCHAR->GetCharID(), pTARCHAR->GetCharData2().m_szName, pTARCHAR->GetCtrlID() );
	}

	ReserveDropOutPC ( pTARCHAR->GetCtrlID() );
	return TRUE;
}
/*trade lock, Juver, 2018/01/02 */
BOOL GLGaeaServer::RequestTradeLock ( DWORD dwClientID, DWORD dwGaeaID, GLMSG::SNET_TRADE_LOCK *pNetMsg )
{
	PGLCHAR pMyChar = GetChar ( dwGaeaID );
	if ( !pMyChar )									return FALSE;
	if ( !pMyChar->m_sTrade.Valid() )				return FALSE;

	DWORD dwTargetID = pMyChar->m_sTrade.GetTarget();
	PGLCHAR pTarChar = GetChar ( dwTargetID );

	if ( !pTarChar || pTarChar->m_sTrade.GetTarget()!=dwGaeaID )
	{
		pMyChar->m_sTrade.Reset ();

		GLMSG::SNET_TRADE_CANCEL_TAR NetMsg;
		SENDTOAGENT ( pMyChar->m_dwClientID, &NetMsg );

		return FALSE;
	}

	pMyChar->m_sTrade.SetLock();

	GLMSG::SNET_TRADE_LOCK_TAR NetMsgAgreeTar;
	NetMsgAgreeTar.dwGaeaID = dwGaeaID;
	NetMsgAgreeTar.emFB = EMTRADE_LOCK_OK;

	SENDTOAGENT ( pMyChar->m_dwClientID, &NetMsgAgreeTar );
	SENDTOAGENT ( pTarChar->m_dwClientID, &NetMsgAgreeTar );

	return TRUE;
}

BOOL GLGaeaServer::ServerSlotEventGen( GLMSG::SNET_SLOT_EVENT_GEN_FLD * pNetMsg )
{

	SNATIVEID nidMAP( pNetMsg->dwMapID );
	GLLandMan *pLandMan = GetByMapID( nidMAP );

	if( pLandMan )
	{
		if( pNetMsg->bRoll )
		{
			GLMSG::SNET_SLOT_EVENT_GEN_BRD NetMsgBrd;
			NetMsgBrd.wMin = pNetMsg->wMin;
			NetMsgBrd.wMax = pNetMsg->wMax;
			NetMsgBrd.wWin = 0;
			NetMsgBrd.bRoll = true;
			
			m_sItemSlotReward = SNATIVEID(pNetMsg->dwMid,pNetMsg->dwSid);
			m_wSlotMin = pNetMsg->wMin;
			m_wSlotMax = pNetMsg->wMax;
			m_wMaxChar = pNetMsg->wMaxChar;

			SITEM* pItem = GLItemMan::GetInstance().GetItem( m_sItemSlotReward );
			if ( !pItem ) return FALSE;

			const char *szItem = pItem->GetName();
			CString strTEXT;
			strTEXT.Format ( "Slot event begin! slot range %d-%d, max char per slot: %d, reward: %s", m_wSlotMin, m_wSlotMax, m_wMaxChar, szItem );
			GLMSG::SNET_SERVER_GENERALCHAT NetMsgKill;
			NetMsgKill.SETTEXT ( strTEXT.GetString() );
			SENDTOAGENT ( &NetMsgKill );

			GLCHARNODE* pCharNode = pLandMan->m_GlobPCList.m_pHead;
			for ( ; pCharNode; pCharNode = pCharNode->pNext )
			{
				PGLCHAR pChar = pCharNode->Data;
				if( !pChar ) continue;

				SENDTOCLIENT( pChar->m_dwClientID, &NetMsgBrd );
			}
		}
		else if( !pNetMsg->bRoll )
		{
			if( m_wWinner < 1 || m_wWinner > 100 )	return FALSE;

			GLMSG::SNET_SLOT_EVENT_GEN_BRD NetMsgBrd;
			NetMsgBrd.wWin = m_wWinner;
			NetMsgBrd.bRoll = false;

			SITEM* pItem = GLItemMan::GetInstance().GetItem( m_sItemSlotReward );
			if ( !pItem ) return FALSE;
	
			SITEMCUSTOM sITEM_NEW;
			sITEM_NEW.sNativeID = m_sItemSlotReward;
			sITEM_NEW.tBORNTIME = CTime::GetCurrentTime().GetTime();
			sITEM_NEW.cGenType = EMGEN_SYSTEM;
			sITEM_NEW.cChnID = (BYTE)GLGaeaServer::GetInstance().GetServerChannel();
			sITEM_NEW.cFieldID = (BYTE)GLGaeaServer::GetInstance().GetFieldSvrID();
			sITEM_NEW.lnGenNum = GLITEMLMT::GetInstance().RegItemGen ( sITEM_NEW.sNativeID, (EMITEMGEN)sITEM_NEW.cGenType );
	
			CItemDrop cDropItem;
			cDropItem.sItemCustom = sITEM_NEW;

			DWORD dwCharCounter = 0;

			GLCHARNODE* pCharNodeCounter = pLandMan->m_GlobPCList.m_pHead;
			for ( ; pCharNodeCounter; pCharNodeCounter = pCharNodeCounter->pNext )
			{
				PGLCHAR pChar = pCharNodeCounter->Data;
				if( !pChar ) continue;
				if ( pChar->GetSlotNumber() == NetMsgBrd.wWin && !pChar->IsDie() )
					dwCharCounter++;
			}

			if( dwCharCounter == 0 )
			{
				GLMSG::SNET_SERVER_GENERALCHAT NetMsgKill;
				CString strTEXT;
				strTEXT.Format ( "We don't have a winner on slot number %d", NetMsgBrd.wWin );
				NetMsgKill.SETTEXT ( strTEXT.GetString() );
				SENDTOAGENT ( &NetMsgKill );
			}
			else if( dwCharCounter <= m_wMaxChar )
			{
				GLMSG::SNET_SERVER_GENERALCHAT NetMsgKill;
				CString strTEXT;
				strTEXT.Format ( "%d lucky winner in slot number %d", dwCharCounter, NetMsgBrd.wWin );
				NetMsgKill.SETTEXT ( strTEXT.GetString() );
				SENDTOAGENT ( &NetMsgKill );

				GLCHARNODE* pCharNode = pLandMan->m_GlobPCList.m_pHead;
				for ( ; pCharNode; pCharNode = pCharNode->pNext )
				{
					PGLCHAR pChar = pCharNode->Data;
					if( !pChar ) continue;

					if ( pChar->GetSlotNumber() == NetMsgBrd.wWin && pChar->IsInsertToInvenEx ( &cDropItem ) && !pChar->IsDie() )
					{
						const char *szMAP_NAME = pChar->m_szName;
						CString strTEXT;
						strTEXT.Format ( "%s", szMAP_NAME );
						NetMsgKill.SETTEXT ( strTEXT.GetString() );
						SENDTOAGENT ( &NetMsgKill );

						pChar->InsertToInvenEx ( &cDropItem );
										
						GLITEMLMT::GetInstance().ReqItemRoute ( sITEM_NEW, ID_CHAR, 0, ID_CHAR, pChar->m_dwCharID, 
							EMITEM_ROUTE_SYSTEM, sITEM_NEW.wTurnNum );
					}
				}
			}
			else
			{
				GLMSG::SNET_SERVER_GENERALCHAT NetMsgKill;
				CString strTEXT;
				strTEXT.Format ( "Slot event round void on slot number %d, max char per slot: %d, count: %d", NetMsgBrd.wWin, m_wMaxChar, dwCharCounter );
				NetMsgKill.SETTEXT ( strTEXT.GetString() );
				SENDTOAGENT ( &NetMsgKill );
			}

			SENDTOCLIENT_ONMAP( pNetMsg->dwMapID, &NetMsgBrd );

			//reset slot info
			m_sItemSlotReward = NATIVEID_NULL();
			m_wSlotMin = 0;
			m_wSlotMax = 0;
			m_wWinner = 0;
			m_wMaxChar = 0;
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// LG-7 GlobalRanking
BOOL GLGaeaServer::ReqGlobalRankingKillUpdate(STOP_RANK_KILL sTopKill, BOOL bWIN)
{
	GLMSG::SNETPC_REQ_GLOBAL_RANKING_KILL_UPDATE NetMsg;
	NetMsg.sTopKill	= sTopKill;
	NetMsg.bWIN		= bWIN;
	SENDTOAGENT(&NetMsg);

	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////


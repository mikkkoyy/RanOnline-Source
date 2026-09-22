#include "StdAfx.h"
#include "GLPVPSchoolWarsAgent.h"
#include "GLAgentServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void GLPVPSchoolWarsAgent::QueueMoveIn()
{
	//queue to move players in
	m_vecPlayerMove.clear();
	m_fMoveTimer = 0.0f;
	m_fMoveTimerTotal = 0.0f;

	for( SCHOOLWARS_REGISTER_DATA_MAP_ITER iterplayer = m_mapPlayerDataSG.begin(); iterplayer != m_mapPlayerDataSG.end(); ++ iterplayer ){
		const SCHOOLWARS_REGISTER_DATA& splayerdata = (*iterplayer).second;

		PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( splayerdata.dwInfoCharID );
		if ( !pchar )	continue;
		if ( pchar->m_sCurMapID == m_sMap )	continue;

		m_vecPlayerMove.push_back( splayerdata );
	}

	for( SCHOOLWARS_REGISTER_DATA_MAP_ITER iterplayer = m_mapPlayerDataMP.begin(); iterplayer != m_mapPlayerDataMP.end(); ++ iterplayer ){
		const SCHOOLWARS_REGISTER_DATA& splayerdata = (*iterplayer).second;

		PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( splayerdata.dwInfoCharID );
		if ( !pchar )	continue;
		if ( pchar->m_sCurMapID == m_sMap )	continue;

		m_vecPlayerMove.push_back( splayerdata );
	}

	for( SCHOOLWARS_REGISTER_DATA_MAP_ITER iterplayer = m_mapPlayerDataPHX.begin(); iterplayer != m_mapPlayerDataPHX.end(); ++ iterplayer ){
		const SCHOOLWARS_REGISTER_DATA& splayerdata = (*iterplayer).second;

		PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( splayerdata.dwInfoCharID );
		if ( !pchar )	continue;
		if ( pchar->m_sCurMapID == m_sMap )	continue;

		m_vecPlayerMove.push_back( splayerdata );
	}

	m_bMoveIn = true;
}

void GLPVPSchoolWarsAgent::QueueMoveOut()
{
	//queue to move players out
	m_vecPlayerMove.clear();
	m_fMoveTimer = 0.0f;
	m_fMoveTimerTotal = 0.0f;

	for( SCHOOLWARS_REGISTER_DATA_MAP_ITER iterplayer = m_mapPlayerDataSG.begin(); iterplayer != m_mapPlayerDataSG.end(); ++ iterplayer ){
		const SCHOOLWARS_REGISTER_DATA& splayerdata = (*iterplayer).second;

		PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( splayerdata.dwInfoCharID );
		if ( !pchar )	continue;
		if ( pchar->m_sCurMapID == m_sOutMap )	continue;

		m_vecPlayerMove.push_back( splayerdata );
	}

	for( SCHOOLWARS_REGISTER_DATA_MAP_ITER iterplayer = m_mapPlayerDataMP.begin(); iterplayer != m_mapPlayerDataMP.end(); ++ iterplayer ){
		const SCHOOLWARS_REGISTER_DATA& splayerdata = (*iterplayer).second;

		PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( splayerdata.dwInfoCharID );
		if ( !pchar )	continue;
		if ( pchar->m_sCurMapID == m_sOutMap )	continue;

		m_vecPlayerMove.push_back( splayerdata );
	}

	for( SCHOOLWARS_REGISTER_DATA_MAP_ITER iterplayer = m_mapPlayerDataPHX.begin(); iterplayer != m_mapPlayerDataPHX.end(); ++ iterplayer ){
		const SCHOOLWARS_REGISTER_DATA& splayerdata = (*iterplayer).second;

		PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( splayerdata.dwInfoCharID );
		if ( !pchar )	continue;
		if ( pchar->m_sCurMapID == m_sOutMap )	continue;

		m_vecPlayerMove.push_back( splayerdata );
	}

	m_bMoveOut = true;
}

void GLPVPSchoolWarsAgent::MapMoveInPlayer()
{
	if ( m_vecPlayerMove.empty() )	return;

	SCHOOLWARS_REGISTER_DATA sdata = m_vecPlayerMove[0];

	DWORD dwCharID = sdata.dwInfoCharID;
	SNATIVEID sMapid = m_sMap;
	WORD wGateID = m_wSchoolGate[sdata.wInfoSchool];

	m_vecPlayerMove.erase(m_vecPlayerMove.begin());

	PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( dwCharID );
	if ( pchar )
	{
		GLAGLandMan* plandman = GLAgentServer::GetInstance().GetByMapID ( sMapid );
		if ( plandman )
		{
			DxLandGateMan* pGateMan = plandman->GetLandGateMan ();
			if ( pGateMan )
			{
				PDXLANDGATE pGate = pGateMan->FindLandGate ( wGateID );
				if ( pGate )
				{
					D3DXVECTOR3 vGenPos = pGate->GetGenPos ( DxLandGate::GEN_RENDUM );

					GLMSG::SNETPC_SCHOOLWARS_A2F_MAP_MOVE net_msg_field;
					net_msg_field.nidMAP = sMapid;
					net_msg_field.vPos = vGenPos;
					net_msg_field.bCurMapCheck = TRUE;
					net_msg_field.dwCharID = pchar->m_dwCharID;
					GLAgentServer::GetInstance().SENDTOFIELDSVR ( pchar->m_nChannel, pchar->m_dwCurFieldSvr, &net_msg_field );
					return;
				}
			}
		}
	}

	CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "GLPVPSchoolWarsAgent char:%d failure to move map:[%d~%d] gate%d", dwCharID, sMapid.wMainID, sMapid.wSubID, wGateID );
}

void GLPVPSchoolWarsAgent::MapMoveOutPlayer()
{
	if ( m_vecPlayerMove.empty() )	return;

	SCHOOLWARS_REGISTER_DATA sdata = m_vecPlayerMove[0];

	DWORD dwCharID = sdata.dwInfoCharID;
	SNATIVEID sMapid = m_sOutMap;
	WORD wGateID = m_wOutGate;

	m_vecPlayerMove.erase(m_vecPlayerMove.begin());

	PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( dwCharID );
	if ( pchar )
	{
		GLAGLandMan* plandman = GLAgentServer::GetInstance().GetByMapID ( sMapid );
		if ( plandman )
		{
			DxLandGateMan* pGateMan = plandman->GetLandGateMan ();
			if ( pGateMan )
			{
				PDXLANDGATE pGate = pGateMan->FindLandGate ( wGateID );
				if ( pGate )
				{
					D3DXVECTOR3 vGenPos = pGate->GetGenPos ( DxLandGate::GEN_RENDUM );

					GLMSG::SNETPC_SCHOOLWARS_A2F_MAP_MOVE net_msg_field;
					net_msg_field.nidMAP = sMapid;
					net_msg_field.vPos = vGenPos;
					net_msg_field.bCurMapCheck = TRUE;
					net_msg_field.dwCharID = pchar->m_dwCharID;
					GLAgentServer::GetInstance().SENDTOFIELDSVR ( pchar->m_nChannel, pchar->m_dwCurFieldSvr, &net_msg_field );
					return;
				}
			}
		}
	}

	CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "GLPVPSchoolWarsAgent char:%d failure to move map:[%d~%d] gate%d", dwCharID, sMapid.wMainID, sMapid.wSubID, wGateID );
}

void GLPVPSchoolWarsAgent::PlayerRejoinBattle( DWORD dwCharID )
{
	PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( dwCharID );
	if ( !pchar )	return;

	GLMSG::SNETPC_SCHOOLWARS_A2C_REJOIN_FB netmsgfb;

	//can only rejoin if battle running
	if ( m_emState != SCHOOLWARS_STATE_BATTLE )
	{
		netmsgfb.emFB = SCHOOLWARS_REJOIN_FB_NOTPOSSIBLE;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}

	//already inside map no need to rejoin
	if ( pchar->m_sCurMapID == m_sMap )
	{
		netmsgfb.emFB = SCHOOLWARS_REJOIN_FB_INBATTLE;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}

	SCHOOLWARS_REGISTER_DATA* pplayerdata = PlayerDataGet( pchar->m_dwCharID );
	if ( !pplayerdata )
	{	
		//not registered
		netmsgfb.emFB = SCHOOLWARS_REJOIN_FB_NOT_REGISTERED;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}

	GLAGLandMan* plandman = GLAgentServer::GetInstance().GetByMapID ( m_sMap );
	if ( plandman )
	{
		DxLandGateMan* pGateMan = plandman->GetLandGateMan ();
		if ( pGateMan )
		{
			PDXLANDGATE pGate = pGateMan->FindLandGate ( m_wSchoolGate[pplayerdata->wInfoSchool] );
			if ( pGate )
			{
				D3DXVECTOR3 vGenPos = pGate->GetGenPos ( DxLandGate::GEN_RENDUM );

				GLMSG::SNETPC_SCHOOLWARS_A2F_MAP_MOVE net_msg_field;
				net_msg_field.nidMAP = m_sMap;
				net_msg_field.vPos = vGenPos;
				net_msg_field.bCurMapCheck = TRUE;
				net_msg_field.dwCharID = pchar->m_dwCharID;
				GLAgentServer::GetInstance().SENDTOFIELDSVR ( pchar->m_nChannel, pchar->m_dwCurFieldSvr, &net_msg_field );
				return;
			}
		}
	}

	CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "GLPVPSchoolWarsAgent PlayerRejoinBattle failed  [%d][%s] map[%d~%d] gate[%d]", 
		pplayerdata->dwInfoCharID, pplayerdata->szInfoCharName, m_sMap.wMainID, m_sMap.wSubID, m_wSchoolGate[pplayerdata->wInfoSchool] );
}


void GLPVPSchoolWarsAgent::MapExit( DWORD dwCharID )
{
	if ( m_emState != SCHOOLWARS_STATE_REWARD && m_emState != SCHOOLWARS_STATE_ENDED )	return;

	PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( dwCharID );
	if ( !pchar )	return;

	SCHOOLWARS_REGISTER_DATA* pdata = PlayerDataGet( dwCharID );
	if ( !pdata	)	return;

	GLAGLandMan* plandman = GLAgentServer::GetInstance().GetByMapID ( m_sOutMap );
	if ( plandman )
	{
		DxLandGateMan* pGateMan = plandman->GetLandGateMan ();
		if ( pGateMan )
		{
			PDXLANDGATE pGate = pGateMan->FindLandGate ( m_wOutGate );
			if ( pGate )
			{
				D3DXVECTOR3 vGenPos = pGate->GetGenPos ( DxLandGate::GEN_RENDUM );

				GLMSG::SNETPC_SCHOOLWARS_A2F_MAP_MOVE net_msg_field;
				net_msg_field.nidMAP = m_sOutMap;
				net_msg_field.vPos = vGenPos;
				net_msg_field.bCurMapCheck = TRUE;
				net_msg_field.dwCharID = pchar->m_dwCharID;
				GLAgentServer::GetInstance().SENDTOFIELDSVR ( pchar->m_nChannel, pchar->m_dwCurFieldSvr, &net_msg_field );
				return;
			}
		}
	}

	CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "GLPVPSchoolWarsAgent MapExit failed  [%d][%s] map[%d~%d] gate[%d]", 
		pdata->dwInfoCharID, pdata->szInfoCharName, m_sOutMap.wMainID, m_sOutMap.wSubID, m_wOutGate );
}
#include "StdAfx.h"
#include "GLPVPCaptureTheFlagAgent.h"
#include "GLAgentServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void GLPVPCaptureTheFlagAgent::QueueMoveIn()
{
	//queue to move players in
	m_vecPlayerMove.clear();
	m_fMoveTimer = 0.0f;
	m_fMoveTimerTotal = 0.0f;

	for( CAPTURE_THE_FLAG_REGISTER_DATA_MAP_ITER iterplayer = m_mapPlayerDataA.begin(); iterplayer != m_mapPlayerDataA.end(); ++ iterplayer ){
		const CAPTURE_THE_FLAG_REGISTER_DATA& splayerdata = (*iterplayer).second;

		PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( splayerdata.dwInfoCharID );
		if ( !pchar )	continue;
		if ( pchar->m_sCurMapID == m_sMap )	continue;
		if ( splayerdata.wInfoTeam >= CAPTURE_THE_FLAG_TEAM_SIZE )	continue;

		m_vecPlayerMove.push_back( splayerdata );
	}

	for( CAPTURE_THE_FLAG_REGISTER_DATA_MAP_ITER iterplayer = m_mapPlayerDataB.begin(); iterplayer != m_mapPlayerDataB.end(); ++ iterplayer ){
		const CAPTURE_THE_FLAG_REGISTER_DATA& splayerdata = (*iterplayer).second;

		PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( splayerdata.dwInfoCharID );
		if ( !pchar )	continue;
		if ( pchar->m_sCurMapID == m_sMap )	continue;
		if ( splayerdata.wInfoTeam >= CAPTURE_THE_FLAG_TEAM_SIZE )	continue;

		m_vecPlayerMove.push_back( splayerdata );
	}

	m_bMoveIn = true;
}

void GLPVPCaptureTheFlagAgent::QueueMoveOut()
{
	//queue to move players out
	m_vecPlayerMove.clear();
	m_fMoveTimer = 0.0f;
	m_fMoveTimerTotal = 0.0f;

	for( CAPTURE_THE_FLAG_REGISTER_DATA_MAP_ITER iterplayer = m_mapPlayerDataA.begin(); iterplayer != m_mapPlayerDataA.end(); ++ iterplayer ){
		const CAPTURE_THE_FLAG_REGISTER_DATA& splayerdata = (*iterplayer).second;

		PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( splayerdata.dwInfoCharID );
		if ( !pchar )	continue;
		if ( pchar->m_sCurMapID == m_sOutMap )	continue;
		if ( splayerdata.wInfoTeam >= CAPTURE_THE_FLAG_TEAM_SIZE )	continue;

		m_vecPlayerMove.push_back( splayerdata );
	}

	for( CAPTURE_THE_FLAG_REGISTER_DATA_MAP_ITER iterplayer = m_mapPlayerDataB.begin(); iterplayer != m_mapPlayerDataB.end(); ++ iterplayer ){
		const CAPTURE_THE_FLAG_REGISTER_DATA& splayerdata = (*iterplayer).second;

		PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( splayerdata.dwInfoCharID );
		if ( !pchar )	continue;
		if ( pchar->m_sCurMapID == m_sOutMap )	continue;
		if ( splayerdata.wInfoTeam >= CAPTURE_THE_FLAG_TEAM_SIZE )	continue;

		m_vecPlayerMove.push_back( splayerdata );
	}

	m_bMoveOut = true;
}

void GLPVPCaptureTheFlagAgent::MapMoveInPlayer()
{
	if ( m_vecPlayerMove.empty() )	return;

	CAPTURE_THE_FLAG_REGISTER_DATA sdata = m_vecPlayerMove[0];

	DWORD dwCharID = sdata.dwInfoCharID;
	SNATIVEID sMapid = m_sMap;
	WORD wGateID = m_wTeamGate[sdata.wInfoTeam];

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

					GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_MAP_MOVE net_msg_field;
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

	CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagAgent char:%d failure to move map:[%d~%d] gate%d", dwCharID, sMapid.wMainID, sMapid.wSubID, wGateID );
}

void GLPVPCaptureTheFlagAgent::MapMoveOutPlayer()
{
	if ( m_vecPlayerMove.empty() )	return;

	CAPTURE_THE_FLAG_REGISTER_DATA sdata = m_vecPlayerMove[0];

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

					GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_MAP_MOVE net_msg_field;
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

	CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagAgent char:%d failure to move map:[%d~%d] gate%d", dwCharID, sMapid.wMainID, sMapid.wSubID, wGateID );
}

void GLPVPCaptureTheFlagAgent::PlayerRejoinBattle( DWORD dwCharID )
{
	PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( dwCharID );
	if ( !pchar )	return;

	GLMSG::SNETPC_CAPTURE_THE_FLAG_A2C_REJOIN_FB netmsgfb;

	//can only rejoin if battle running
	if ( m_emState != CAPTURE_THE_FLAG_STATE_BATTLE )
	{
		netmsgfb.emFB = CAPTURE_THE_FLAG_REJOIN_FB_NOTPOSSIBLE;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}

	//already inside map no need to rejoin
	if ( pchar->m_sCurMapID == m_sMap )
	{
		netmsgfb.emFB = CAPTURE_THE_FLAG_REJOIN_FB_INBATTLE;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}

	CAPTURE_THE_FLAG_REGISTER_DATA* pplayerdata = PlayerDataGet( pchar->m_dwCharID );
	if ( !pplayerdata )
	{	
		//not registered
		netmsgfb.emFB = CAPTURE_THE_FLAG_REJOIN_FB_NOT_REGISTERED;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}

	if ( pplayerdata->wInfoTeam >= CAPTURE_THE_FLAG_TEAM_SIZE )
	{
		netmsgfb.emFB = CAPTURE_THE_FLAG_REJOIN_FB_INVALID_TEAM;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}

	GLAgentServer::GetInstance().CaptureTheFlagRemoveParty( dwCharID );

	GLAGLandMan* plandman = GLAgentServer::GetInstance().GetByMapID ( m_sMap );
	if ( plandman )
	{
		DxLandGateMan* pGateMan = plandman->GetLandGateMan ();
		if ( pGateMan )
		{
			PDXLANDGATE pGate = pGateMan->FindLandGate ( m_wTeamGate[pplayerdata->wInfoTeam] );
			if ( pGate )
			{
				D3DXVECTOR3 vGenPos = pGate->GetGenPos ( DxLandGate::GEN_RENDUM );

				GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_MAP_MOVE net_msg_field;
				net_msg_field.nidMAP = m_sMap;
				net_msg_field.vPos = vGenPos;
				net_msg_field.bCurMapCheck = TRUE;
				net_msg_field.dwCharID = pchar->m_dwCharID;
				GLAgentServer::GetInstance().SENDTOFIELDSVR ( pchar->m_nChannel, pchar->m_dwCurFieldSvr, &net_msg_field );
				return;
			}
		}
	}

	CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagAgent PlayerRejoinBattle failed  [%d][%s] map[%d~%d] gate[%d]", 
		pplayerdata->dwInfoCharID, pplayerdata->szInfoCharName, m_sMap.wMainID, m_sMap.wSubID, m_wTeamGate[pplayerdata->wInfoTeam] );
}


void GLPVPCaptureTheFlagAgent::MapExit( DWORD dwCharID )
{
	if ( m_emState != CAPTURE_THE_FLAG_STATE_REWARD && m_emState != CAPTURE_THE_FLAG_STATE_ENDED )	return;

	PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( dwCharID );
	if ( !pchar )	return;

	CAPTURE_THE_FLAG_REGISTER_DATA* pdata = PlayerDataGet( dwCharID );
	if ( !pdata	)	return;

	GLAgentServer::GetInstance().CaptureTheFlagRemoveParty( dwCharID );

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

				GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_MAP_MOVE net_msg_field;
				net_msg_field.nidMAP = m_sOutMap;
				net_msg_field.vPos = vGenPos;
				net_msg_field.bCurMapCheck = TRUE;
				net_msg_field.dwCharID = pchar->m_dwCharID;
				GLAgentServer::GetInstance().SENDTOFIELDSVR ( pchar->m_nChannel, pchar->m_dwCurFieldSvr, &net_msg_field );
				return;
			}
		}
	}

	CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagAgent MapExit failed  [%d][%s] map[%d~%d] gate[%d]", 
		pdata->dwInfoCharID, pdata->szInfoCharName, m_sOutMap.wMainID, m_sOutMap.wSubID, m_wOutGate );
}
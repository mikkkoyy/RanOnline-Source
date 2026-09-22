#include "StdAfx.h"
#include "GLPVPCaptureTheFlagField.h"
#include "GLGaeaServer.h"
#include "GLLandMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void GLPVPCaptureTheFlagField::FlagCrowSetHide( WORD wTeam, bool bHide )
{
	GLLandMan* pland = GLGaeaServer::GetInstance().GetByMapID ( m_sMap );
	if ( !pland )	return;

	if ( wTeam == CAPTURE_THE_FLAG_TEAM_SIZE )
	{
		PGLCROW pcrow_a = pland->GetCrowID( m_sFlagCrow[CAPTURE_THE_FLAG_TEAM_A] );
		if ( pcrow_a )
		{
			pcrow_a->CaptureTheFlagSetHide( bHide );
			m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_A].dwID = pcrow_a->m_dwGlobID;
			m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_A].emCrow = pcrow_a->GETCROW();
			m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_A].vPos = pcrow_a->GetPosition();
		}
		
		PGLCROW pcrow_b = pland->GetCrowID( m_sFlagCrow[CAPTURE_THE_FLAG_TEAM_B] );
		if ( pcrow_b )
		{
			pcrow_b->CaptureTheFlagSetHide( bHide );
			m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_B].dwID = pcrow_b->m_dwGlobID;
			m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_B].emCrow = pcrow_b->GETCROW();
			m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_B].vPos = pcrow_b->GetPosition();
		}
	}
	else if ( wTeam == CAPTURE_THE_FLAG_TEAM_A )
	{
		PGLCROW pcrow_a = pland->GetCrowID( m_sFlagCrow[CAPTURE_THE_FLAG_TEAM_A] );
		if ( pcrow_a )
		{
			pcrow_a->CaptureTheFlagSetHide( bHide );
			m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_A].dwID = pcrow_a->m_dwGlobID;
			m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_A].emCrow = pcrow_a->GETCROW();
			m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_A].vPos = pcrow_a->GetPosition();
		}
	}
	else if ( wTeam == CAPTURE_THE_FLAG_TEAM_B )
	{
		PGLCROW pcrow_b = pland->GetCrowID( m_sFlagCrow[CAPTURE_THE_FLAG_TEAM_B] );
		if ( pcrow_b )
		{
			pcrow_b->CaptureTheFlagSetHide( bHide );
			m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_B].dwID = pcrow_b->m_dwGlobID;
			m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_B].emCrow = pcrow_b->GETCROW();
			m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_B].vPos = pcrow_b->GetPosition();
		}
	}
}

void GLPVPCaptureTheFlagField::FlagToHold( DWORD dwCharID, DWORD dwGlobID )
{
	if ( !IsBattle() )	return;

	CAPTURE_THE_FLAG_PLAYER_DATA* pplayer_data = PlayerDataGet( dwCharID );
	PGLCHAR pchar = GLGaeaServer::GetInstance().GetCharID( dwCharID );
	if ( !pplayer_data || !pchar )	return;

	if ( pplayer_data->wInfoTeam != pchar->m_wCaptureTheFlagTeam  )	return;

	GLLandMan* pland = GLGaeaServer::GetInstance().GetByMapID ( m_sMap );
	if ( !pland )	return;

	PGLCROW pcrow = pland->GetCrow( dwGlobID );
	if ( !pcrow )	return;

	if ( !pcrow->m_pCrowData )		return;

	if ( pcrow->m_pCrowData->m_emCTFType != EMNPC_CTF_FLAG_TYPE_A && pcrow->m_pCrowData->m_emCTFType != EMNPC_CTF_FLAG_TYPE_B )	
		return;

	if ( pcrow->m_pCrowData->m_emCTFType == EMNPC_CTF_FLAG_TYPE_A  && pplayer_data->wInfoTeam != CAPTURE_THE_FLAG_TEAM_B )
		return;

	if ( pcrow->m_pCrowData->m_emCTFType == EMNPC_CTF_FLAG_TYPE_B  &&  pplayer_data->wInfoTeam != CAPTURE_THE_FLAG_TEAM_A )
		return;

	if ( m_bFlagHold[pplayer_data->wInfoTeam] )	return;

	if ( pcrow->m_pCrowData->m_emCTFType == EMNPC_CTF_FLAG_TYPE_A )
	{
		FlagCrowSetHide( CAPTURE_THE_FLAG_TEAM_A, true );
		m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_A].dwID = pchar->m_dwGaeaID;
		m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_A].emCrow = pchar->GETCROW();
		m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_A].vPos = pchar->GetPosition();
	}
	else if ( pcrow->m_pCrowData->m_emCTFType == EMNPC_CTF_FLAG_TYPE_B )
	{
		FlagCrowSetHide( CAPTURE_THE_FLAG_TEAM_B, true );
		m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_B].dwID = pchar->m_dwGaeaID;
		m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_B].emCrow = pchar->GETCROW();
		m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_B].vPos = pchar->GetPosition();
	}

	pchar->PVPCaptureTheFlagHoldFlag( true );

	m_bFlagHold[pplayer_data->wInfoTeam] = TRUE;
	m_fHoldTimer[pplayer_data->wInfoTeam] = CAPTURE_THE_FLAG_HOLD_MAX_TIME;

	GLMSG::SNETPC_CAPTURE_THE_FLAG_F2A_FLAG_HOLD netmsgagent;
	netmsgagent.wTeam = pplayer_data->wInfoTeam;
	netmsgagent.bHold = m_bFlagHold[pplayer_data->wInfoTeam];
	GLGaeaServer::GetInstance().SENDTOAGENT( &netmsgagent );

	GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_FLAG_HOLD netmsgclient;
	netmsgclient.wTeam = pplayer_data->wInfoTeam;
	netmsgclient.emStatus = CAPTURE_THE_FLAG_FLAG_HOLD_STATUS_CAPTURE;
	StringCchCopy ( netmsgclient.szName, CAPTURE_THE_FLAG_CHARNAME_STRING_SIZE, pplayer_data->szInfoCharName );
	for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
		netmsgclient.bFlagHold[i] = m_bFlagHold[i];
	GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMap.dwID, &netmsgclient );
}

void GLPVPCaptureTheFlagField::FlagToBase( DWORD dwCharID, DWORD dwGateID )
{
	if ( !IsBattle() )	return;

	CAPTURE_THE_FLAG_PLAYER_DATA* pplayer_data = PlayerDataGet( dwCharID );
	PGLCHAR pchar = GLGaeaServer::GetInstance().GetCharID( dwCharID );
	if ( !pplayer_data || !pchar )	return;

	if ( pplayer_data->wInfoTeam != pchar->m_wCaptureTheFlagTeam  )	return;
	if ( m_wTeamGate[pplayer_data->wInfoTeam] != dwGateID )			return;
	if ( m_bFlagHold[pplayer_data->wInfoTeam] == FALSE )			return;
	if ( m_bFlagHold[CAPTURE_THE_FLAG_TEAM_A] && 
		m_bFlagHold[CAPTURE_THE_FLAG_TEAM_B] )						return;

	if ( pplayer_data->wInfoTeam == CAPTURE_THE_FLAG_TEAM_B )
		FlagCrowSetHide( CAPTURE_THE_FLAG_TEAM_A, false );
	else if ( pplayer_data->wInfoTeam == CAPTURE_THE_FLAG_TEAM_A )
		FlagCrowSetHide( CAPTURE_THE_FLAG_TEAM_B, false );

	pchar->PVPCaptureTheFlagHoldFlag( false );

	pplayer_data->wScoreFlag ++;

	GLMSG::SNETPC_CAPTURE_THE_FLAG_F2A_CAPTURE_UPDATE netmsgagent_point;
	netmsgagent_point.wTeam = pplayer_data->wInfoTeam;
	GLGaeaServer::GetInstance().SENDTOAGENT( &netmsgagent_point );

	m_bFlagHold[pplayer_data->wInfoTeam] = FALSE;
	m_fHoldTimer[pplayer_data->wInfoTeam] = 0.0f;

	GLMSG::SNETPC_CAPTURE_THE_FLAG_F2A_FLAG_HOLD netmsgagent;
	netmsgagent.wTeam = pplayer_data->wInfoTeam;
	netmsgagent.bHold = m_bFlagHold[pplayer_data->wInfoTeam];
	GLGaeaServer::GetInstance().SENDTOAGENT( &netmsgagent );

	GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_FLAG_HOLD netmsgclient;
	netmsgclient.wTeam = pplayer_data->wInfoTeam;
	netmsgclient.emStatus = CAPTURE_THE_FLAG_FLAG_HOLD_STATUS_SCORE;
	StringCchCopy ( netmsgclient.szName, CAPTURE_THE_FLAG_CHARNAME_STRING_SIZE, pplayer_data->szInfoCharName );
	for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
		netmsgclient.bFlagHold[i] = m_bFlagHold[i];
	GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMap.dwID, &netmsgclient );
}

void GLPVPCaptureTheFlagField::FlagTimeOut( WORD wTeam )
{
	if ( !IsBattle() )	return;

	if ( wTeam >= CAPTURE_THE_FLAG_TEAM_SIZE )	return;
	if ( m_bFlagHold[wTeam] == FALSE )			return;

	if ( wTeam == CAPTURE_THE_FLAG_TEAM_B )
		FlagCrowSetHide( CAPTURE_THE_FLAG_TEAM_A, false );
	else if ( wTeam == CAPTURE_THE_FLAG_TEAM_A )
		FlagCrowSetHide( CAPTURE_THE_FLAG_TEAM_B, false );

	m_bFlagHold[wTeam] = FALSE;
	m_fHoldTimer[wTeam] = 0.0f;

	GLMSG::SNETPC_CAPTURE_THE_FLAG_F2A_FLAG_HOLD netmsgagent;
	netmsgagent.wTeam = wTeam;
	netmsgagent.bHold = m_bFlagHold[wTeam];
	GLGaeaServer::GetInstance().SENDTOAGENT( &netmsgagent );

	GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_FLAG_HOLD netmsgclient;
	netmsgclient.wTeam = wTeam;
	netmsgclient.emStatus = CAPTURE_THE_FLAG_FLAG_HOLD_STATUS_TIMEOUT;
	for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
		netmsgclient.bFlagHold[i] = m_bFlagHold[i];
	GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMap.dwID, &netmsgclient );
}

void GLPVPCaptureTheFlagField::FlagResetBase( DWORD dwCharID, CAPTURE_THE_FLAG_FLAG_HOLD_STATUS emStatus )
{
	if ( !IsBattle() )	return;

	CAPTURE_THE_FLAG_PLAYER_DATA* pplayer_data = PlayerDataGet( dwCharID );
	PGLCHAR pchar = GLGaeaServer::GetInstance().GetCharID( dwCharID );
	if ( !pplayer_data || !pchar )	return;

	if ( pplayer_data->wInfoTeam != pchar->m_wCaptureTheFlagTeam  )	return;
	if ( m_bFlagHold[pplayer_data->wInfoTeam] == FALSE )			return;

	if ( pplayer_data->wInfoTeam == CAPTURE_THE_FLAG_TEAM_B )
		FlagCrowSetHide( CAPTURE_THE_FLAG_TEAM_A, false );
	else if ( pplayer_data->wInfoTeam == CAPTURE_THE_FLAG_TEAM_A )
		FlagCrowSetHide( CAPTURE_THE_FLAG_TEAM_B, false );

	pchar->PVPCaptureTheFlagHoldFlag( false );

	m_bFlagHold[pplayer_data->wInfoTeam] = FALSE;
	m_fHoldTimer[pplayer_data->wInfoTeam] = 0.0f;

	GLMSG::SNETPC_CAPTURE_THE_FLAG_F2A_FLAG_HOLD netmsgagent;
	netmsgagent.wTeam = pplayer_data->wInfoTeam;
	netmsgagent.bHold = m_bFlagHold[pplayer_data->wInfoTeam];
	GLGaeaServer::GetInstance().SENDTOAGENT( &netmsgagent );

	GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_FLAG_HOLD netmsgclient;
	netmsgclient.wTeam = pplayer_data->wInfoTeam;
	netmsgclient.emStatus = emStatus;
	StringCchCopy ( netmsgclient.szName, CAPTURE_THE_FLAG_CHARNAME_STRING_SIZE, pplayer_data->szInfoCharName );
	for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
		netmsgclient.bFlagHold[i] = m_bFlagHold[i];
	GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMap.dwID, &netmsgclient  );
}

void GLPVPCaptureTheFlagField::FlagLocationUpdate()
{
	GLLandMan* pland = GLGaeaServer::GetInstance().GetByMapID ( m_sMap );
	if ( !pland )	return;

	for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
	{
		GLACTOR* pactor = GLGaeaServer::GetInstance().GetTarget( pland, m_sFlagTarget[i] );
		if ( pactor )
			m_sFlagTarget[i].vPos = pactor->GetPosition();
	}

	GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_FLAG_POSITION netmsgclient;
	for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
		netmsgclient.sFlagTarget[i] = m_sFlagTarget[i];
	GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMap.dwID, &netmsgclient  );
}


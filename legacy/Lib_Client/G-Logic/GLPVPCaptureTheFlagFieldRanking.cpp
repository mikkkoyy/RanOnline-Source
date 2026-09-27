#include "StdAfx.h"
#include "GLPVPCaptureTheFlagField.h"
#include "GLGaeaServer.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void GLPVPCaptureTheFlagField::InsertPlayerData( CAPTURE_THE_FLAG_REGISTER_DATA* pPlayerData )
{
	if ( !pPlayerData )			return;
	if ( pPlayerData->dwInfoCharID == CAPTURE_THE_FLAG_PLAYER_NULL )
	{
		CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagField invalid ID:%d %s", pPlayerData->dwInfoCharID, pPlayerData->szInfoCharName );
		return;
	}

	CAPTURE_THE_FLAG_PLAYER_DATA* pdata = PlayerDataGet( pPlayerData->dwInfoCharID );
	if ( pdata )
	{
		CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagField char:%d %s already exist", pPlayerData->dwInfoCharID, pPlayerData->szInfoCharName );
		return;
	}

	if ( pPlayerData->wInfoSchool >= CAPTURE_THE_FLAG_INFO_SCHOOL_SIZE )
	{
		CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagField char:%d %s invalid school %d", pPlayerData->dwInfoCharID, pPlayerData->szInfoCharName, pPlayerData->wInfoSchool );
		return;
	}

	if ( pPlayerData->wInfoTeam >= CAPTURE_THE_FLAG_TEAM_SIZE )
	{
		CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagField char:%d %s invalid team %d", pPlayerData->dwInfoCharID, pPlayerData->szInfoCharName, pPlayerData->wInfoTeam );
		return;
	}

	if ( pPlayerData->wInfoClass >= GLCI_NUM_8CLASS )
	{
		CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagField char:%d %s invalid class %d", pPlayerData->dwInfoCharID, pPlayerData->szInfoCharName, pPlayerData->wInfoClass );
		return;
	}

	CAPTURE_THE_FLAG_PLAYER_DATA sdata;
	sdata.dwInfoCharID	= pPlayerData->dwInfoCharID;
	sdata.wInfoClass	= pPlayerData->wInfoClass;
	sdata.wInfoSchool	= pPlayerData->wInfoSchool;
	sdata.wInfoTeam		= pPlayerData->wInfoTeam;
	StringCchCopy( sdata.szInfoCharName, CAPTURE_THE_FLAG_CHARNAME_STRING_SIZE, pPlayerData->szInfoCharName );

	switch ( sdata.wInfoTeam )
	{
	case CAPTURE_THE_FLAG_TEAM_A:
		{
			m_mapPlayerDataA.insert( std::make_pair( sdata.dwInfoCharID, sdata ) );
		}break;

	case CAPTURE_THE_FLAG_TEAM_B:
		{
			m_mapPlayerDataB.insert( std::make_pair( sdata.dwInfoCharID, sdata ) );
		}break;
	};
}

void GLPVPCaptureTheFlagField::ScoreKill( DWORD dwCharKilled, DWORD dwCharKiller )
{
	if ( m_emState != CAPTURE_THE_FLAG_STATE_BATTLE )	return;

	CAPTURE_THE_FLAG_PLAYER_DATA* pplayerkilled = PlayerDataGet( dwCharKilled );
	CAPTURE_THE_FLAG_PLAYER_DATA* pplayerkiller = PlayerDataGet( dwCharKiller );

	if ( pplayerkilled && pplayerkiller )
	{
		pplayerkilled->wScoreDeath ++;
		pplayerkiller->wScoreKill ++;

		GLMSG::SNETPC_CAPTURE_THE_FLAG_F2A_KILL_UPDATE netmsgagent;
		netmsgagent.wTeamKilled = pplayerkilled->wInfoTeam;
		netmsgagent.wTeamKiller = pplayerkiller->wInfoTeam;
		GLGaeaServer::GetInstance().SENDTOAGENT( &netmsgagent );
	}
}

void GLPVPCaptureTheFlagField::ScoreHeal( DWORD dwChar, DWORD dwHeal )
{
	if ( m_emState != CAPTURE_THE_FLAG_STATE_BATTLE )	return;

	CAPTURE_THE_FLAG_PLAYER_DATA* pplayerdata = PlayerDataGet( dwChar );
	if ( pplayerdata )
	{
		pplayerdata->dwScoreHeal += dwHeal;
	}
}

void GLPVPCaptureTheFlagField::ScoreResu( DWORD dwChar )
{
	if ( m_emState != CAPTURE_THE_FLAG_STATE_BATTLE )	return;

	CAPTURE_THE_FLAG_PLAYER_DATA* pplayerdata = PlayerDataGet( dwChar );
	if ( pplayerdata )
	{
		pplayerdata->wScoreResu ++;
	}
}

void GLPVPCaptureTheFlagField::ScoreCalculate( CAPTURE_THE_FLAG_PLAYER_DATA_MAP& mapData, WORD wTeamWinner )
{
	for( CAPTURE_THE_FLAG_PLAYER_DATA_MAP_ITER iterplayer = mapData.begin(); iterplayer != mapData.end(); ++ iterplayer )
	{
		CAPTURE_THE_FLAG_PLAYER_DATA& splayer_data = (*iterplayer).second;

		//calculate score
		float ftotal_score = 0.0f;
		ftotal_score += float( splayer_data.wScoreFlag * m_fFlagVar );
		ftotal_score += float( splayer_data.dwScoreHeal * m_fHealVar );
		ftotal_score += float( splayer_data.wScoreKill * m_fKillVar );
		ftotal_score += float( splayer_data.wScoreResu * m_fResuVar );
		splayer_data.dwScoreTotal = (DWORD)ftotal_score;

		//calculate points
		if ( splayer_data.dwScoreTotal > 0 )
		{
			//base point
			WORD wpoint_base = m_wBasePointLoss;
			if ( wTeamWinner != CAPTURE_THE_FLAG_TEAM_SIZE &&
				splayer_data.wInfoTeam == wTeamWinner )
			{
				wpoint_base = m_wBasePointWin;
			}

			//point gained from score
			wpoint_base += WORD( splayer_data.dwScoreTotal * m_fPointVar );

			//point limit
			if ( wpoint_base >= m_wMaxRewardPoint )
				wpoint_base = m_wMaxRewardPoint;

			splayer_data.wRewardPoint = wpoint_base;
		}
		else
		{
			//no reward point for no score
			splayer_data.wRewardPoint = 0;
		}
	}
}

void GLPVPCaptureTheFlagField::CalculateRankingTeam( )
{
	{	//a rank
		CAPTURE_THE_FLAG_PLAYER_DATA_VEC vecdata;
		vecdata.reserve( m_mapPlayerDataA.size() );

		for( CAPTURE_THE_FLAG_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataA.begin(); iterplayer != m_mapPlayerDataA.end(); ++ iterplayer ){
			const CAPTURE_THE_FLAG_PLAYER_DATA& splayerdata = (*iterplayer).second;
			vecdata.push_back( splayerdata );
		}

		std::sort( vecdata.begin(), vecdata.end() );	

		for( size_t i=0; i<vecdata.size(); ++i )
			vecdata[i].wRankTeam = i+1;

		for( size_t i=0; i<vecdata.size(); ++i )
		{
			const CAPTURE_THE_FLAG_PLAYER_DATA& splayer_data = vecdata[i];

			CAPTURE_THE_FLAG_PLAYER_DATA* prank_player = PlayerDataGet( splayer_data.dwInfoCharID );
			if ( prank_player && prank_player->dwInfoCharID == splayer_data.dwInfoCharID )
				prank_player->wRankTeam = splayer_data.wRankTeam;
		}
	}

	{	//b rank
		CAPTURE_THE_FLAG_PLAYER_DATA_VEC vecdata;
		vecdata.reserve( m_mapPlayerDataB.size() );

		for( CAPTURE_THE_FLAG_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataB.begin(); iterplayer != m_mapPlayerDataB.end(); ++ iterplayer ){
			const CAPTURE_THE_FLAG_PLAYER_DATA& splayerdata = (*iterplayer).second;
			vecdata.push_back( splayerdata );
		}

		std::sort( vecdata.begin(), vecdata.end() );	

		for( size_t i=0; i<vecdata.size(); ++i )
			vecdata[i].wRankTeam = i+1;

		for( size_t i=0; i<vecdata.size(); ++i )
		{
			const CAPTURE_THE_FLAG_PLAYER_DATA& splayer_data = vecdata[i];

			CAPTURE_THE_FLAG_PLAYER_DATA* prank_player = PlayerDataGet( splayer_data.dwInfoCharID );
			if ( prank_player && prank_player->dwInfoCharID == splayer_data.dwInfoCharID )
				prank_player->wRankTeam = splayer_data.wRankTeam;
		}
	}
}

void GLPVPCaptureTheFlagField::CalculateRankingAll()
{
	CAPTURE_THE_FLAG_PLAYER_DATA_VEC vecdata;
	vecdata.reserve( m_mapPlayerDataA.size() + m_mapPlayerDataB.size() );

	for( CAPTURE_THE_FLAG_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataA.begin(); iterplayer != m_mapPlayerDataA.end(); ++ iterplayer ){
		const CAPTURE_THE_FLAG_PLAYER_DATA& splayerdata = (*iterplayer).second;
		vecdata.push_back( splayerdata );
	}

	for( CAPTURE_THE_FLAG_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataB.begin(); iterplayer != m_mapPlayerDataB.end(); ++ iterplayer ){
		const CAPTURE_THE_FLAG_PLAYER_DATA& splayerdata = (*iterplayer).second;
		vecdata.push_back( splayerdata );
	}

	std::sort( vecdata.begin(), vecdata.end() );	

	for( size_t i=0; i<vecdata.size(); ++i )
		vecdata[i].wRankAll = i+1;

	for( size_t i=0; i<vecdata.size(); ++i )
	{
		const CAPTURE_THE_FLAG_PLAYER_DATA& splayer_data = vecdata[i];
		CAPTURE_THE_FLAG_PLAYER_DATA* prank_player = PlayerDataGet( splayer_data.dwInfoCharID );
		if ( prank_player && prank_player->dwInfoCharID == splayer_data.dwInfoCharID )
			prank_player->wRankAll = splayer_data.wRankAll;
	}
}

void GLPVPCaptureTheFlagField::RankingSend()
{
	CAPTURE_THE_FLAG_PLAYER_DATA_VEC vecdata;
	vecdata.reserve( m_mapPlayerDataA.size() + m_mapPlayerDataB.size() );

	for( CAPTURE_THE_FLAG_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataA.begin(); iterplayer != m_mapPlayerDataA.end(); ++ iterplayer ){
		const CAPTURE_THE_FLAG_PLAYER_DATA& splayerdata = (*iterplayer).second;
		vecdata.push_back( splayerdata );
	}

	for( CAPTURE_THE_FLAG_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataB.begin(); iterplayer != m_mapPlayerDataB.end(); ++ iterplayer ){
		const CAPTURE_THE_FLAG_PLAYER_DATA& splayerdata = (*iterplayer).second;
		vecdata.push_back( splayerdata );
	}

	GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_RANKING_UPDATE	netmsg_ranking;
	GLMSG::SNETPC_CAPTURE_THE_FLAG_F2A_RANKINFO_PC		netmsg_ranking_agent;

	WORD wNum = 0;
	for ( size_t i=0; i<vecdata.size(); ++i )
	{
		const CAPTURE_THE_FLAG_PLAYER_DATA& splayer_data = vecdata[i];
		netmsg_ranking.ADDRANK( splayer_data );
		if ( netmsg_ranking.wRankNum == CAPTURE_THE_FLAG_PACKET_RANK_NUM )
		{
			GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMap.dwID, &netmsg_ranking );
			wNum += netmsg_ranking.wRankNum;
			netmsg_ranking.Reset();
		}

		if ( splayer_data.wRankAll > 0 && splayer_data.wRankAll <= CAPTURE_THE_FLAG_MINI_RANKING_NUM )
			netmsg_ranking_agent.ADDRANK( splayer_data );
	}

	if ( netmsg_ranking.wRankNum != 0 )
	{
		GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMap.dwID, &netmsg_ranking );
		wNum += netmsg_ranking.wRankNum;
		netmsg_ranking.Reset();
	}

	GLGaeaServer::GetInstance().GetConsoleMsg()->Write( "GLPVPCaptureTheFlagField Send Rank num: %u", wNum );
	CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagField Send Rank num: %u", wNum );

	if ( netmsg_ranking_agent.wRankNum > 0 )	
	{
		GLGaeaServer::GetInstance().SENDTOAGENT( &netmsg_ranking_agent );
		GLGaeaServer::GetInstance().GetConsoleMsg()->Write( "GLPVPCaptureTheFlagField Send to agent rank num: %u", netmsg_ranking_agent.wRankNum );
		CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagField Send to agent rank num: %u", netmsg_ranking_agent.wRankNum );
		netmsg_ranking_agent.wRankNum = 0;
	}

	GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_RANKING_END	netmsg_ranking_end;
	netmsg_ranking_end.wPlayerNum[CAPTURE_THE_FLAG_TEAM_A] = (WORD)m_mapPlayerDataA.size();
	netmsg_ranking_end.wPlayerNum[CAPTURE_THE_FLAG_TEAM_B] = (WORD)m_mapPlayerDataB.size();
	GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMap.dwID, &netmsg_ranking_end );
}
#include "StdAfx.h"
#include "GLPVPSchoolWarsField.h"
#include "GLGaeaServer.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void GLPVPSchoolWarsField::InsertPlayerData( SCHOOLWARS_REGISTER_DATA* pPlayerData )
{
	if ( !pPlayerData )			return;
	if ( pPlayerData->dwInfoCharID == SCHOOLWARS_PLAYER_NULL )
	{
		CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "GLPVPSchoolWarsField invalid ID:%d %s", pPlayerData->dwInfoCharID, pPlayerData->szInfoCharName );
		return;
	}

	SCHOOLWARS_PLAYER_DATA* pdata = PlayerDataGet( pPlayerData->dwInfoCharID );
	if ( pdata )
	{
		CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "GLPVPSchoolWarsField char:%d %s already exist", pPlayerData->dwInfoCharID, pPlayerData->szInfoCharName );
		return;
	}

	if ( pPlayerData->wInfoSchool >= SCHOOLWARS_SCHOOL_SIZE )
	{
		CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "GLPVPSchoolWarsField char:%d %s invalid school %d", pPlayerData->dwInfoCharID, pPlayerData->szInfoCharName, pPlayerData->wInfoSchool );
		return;
	}

	if ( pPlayerData->wInfoClass >= GLCI_NUM_8CLASS )
	{
		CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "GLPVPSchoolWarsField char:%d %s invalid class %d", pPlayerData->dwInfoCharID, pPlayerData->szInfoCharName, pPlayerData->wInfoClass );
		return;
	}

	SCHOOLWARS_PLAYER_DATA sdata;
	sdata.dwInfoCharID	= pPlayerData->dwInfoCharID;
	sdata.wInfoClass	= pPlayerData->wInfoClass;
	sdata.wInfoSchool	= pPlayerData->wInfoSchool;
	StringCchCopy( sdata.szInfoCharName, SCHOOLWARS_CHARNAME_STRING_SIZE, pPlayerData->szInfoCharName );

	switch ( sdata.wInfoSchool )
	{
	case SCHOOLWARS_SCHOOL_SG:
		{
			m_mapPlayerDataSG.insert( std::make_pair( sdata.dwInfoCharID, sdata ) );
		}break;

	case SCHOOLWARS_SCHOOL_MP:
		{
			m_mapPlayerDataMP.insert( std::make_pair( sdata.dwInfoCharID, sdata ) );
		}break;

	case SCHOOLWARS_SCHOOL_PHX:
		{
			m_mapPlayerDataPHX.insert( std::make_pair( sdata.dwInfoCharID, sdata ) );
		}break;
	};
}

void GLPVPSchoolWarsField::ScoreKill( DWORD dwCharKilled, DWORD dwCharKiller )
{
	if ( m_emState != SCHOOLWARS_STATE_BATTLE )	return;

	SCHOOLWARS_PLAYER_DATA* pplayerkilled = PlayerDataGet( dwCharKilled );
	SCHOOLWARS_PLAYER_DATA* pplayerkiller = PlayerDataGet( dwCharKiller );

	if ( pplayerkilled && pplayerkiller )
	{
		pplayerkilled->wScoreDeath ++;
		pplayerkiller->wScoreKill ++;

		GLMSG::SNETPC_SCHOOLWARS_F2A_SCORE_UPDATE netmsgagent;
		netmsgagent.wSchoolKilled = pplayerkilled->wInfoSchool;
		netmsgagent.wSchoolKiller = pplayerkiller->wInfoSchool;
		netmsgagent.dwCharID = dwCharKiller;
		GLGaeaServer::GetInstance().SENDTOAGENT( &netmsgagent );
	}
}

void GLPVPSchoolWarsField::ScoreHeal( DWORD dwChar, DWORD dwHeal )
{
	if ( m_emState != SCHOOLWARS_STATE_BATTLE )	return;

	SCHOOLWARS_PLAYER_DATA* pplayerdata = PlayerDataGet( dwChar );
	if ( pplayerdata )
	{
		pplayerdata->dwScoreHeal += dwHeal;
	}
}

void GLPVPSchoolWarsField::ScoreResu( DWORD dwChar )
{
	if ( m_emState != SCHOOLWARS_STATE_BATTLE )	return;

	SCHOOLWARS_PLAYER_DATA* pplayerdata = PlayerDataGet( dwChar );
	if ( pplayerdata )
	{
		pplayerdata->wScoreResu ++;
	}
}

void GLPVPSchoolWarsField::ScoreCalculate(SCHOOLWARS_PLAYER_DATA_MAP& mapData, WORD wSchoolWinner )
{
	for( SCHOOLWARS_PLAYER_DATA_MAP_ITER iterplayer = mapData.begin(); iterplayer != mapData.end(); ++ iterplayer )
	{
		SCHOOLWARS_PLAYER_DATA& splayer_data = (*iterplayer).second;

		//calculate score
		float ftotal_score = 0.0f;
		ftotal_score += float( splayer_data.dwScoreHeal * m_fHealVar );
		ftotal_score += float( splayer_data.wScoreKill * m_fKillVar );
		ftotal_score += float( splayer_data.wScoreResu * m_fResuVar );
		splayer_data.dwScoreTotal = (DWORD)ftotal_score;

		//calculate points
		if ( splayer_data.dwScoreTotal > 0 )
		{
			//base point
			WORD wpoint_base = m_wBasePointLoss;
			if ( wSchoolWinner != SCHOOLWARS_SCHOOL_SIZE &&
				splayer_data.wInfoSchool == wSchoolWinner )
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

void GLPVPSchoolWarsField::CalculateRankingSchool( )
{

	{	//sg rank
		SCHOOLWARS_PLAYER_DATA_VEC vecdata;
		vecdata.reserve( m_mapPlayerDataSG.size() );

		for( SCHOOLWARS_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataSG.begin(); iterplayer != m_mapPlayerDataSG.end(); ++ iterplayer ){
			const SCHOOLWARS_PLAYER_DATA& splayerdata = (*iterplayer).second;
			vecdata.push_back( splayerdata );
		}

		std::sort( vecdata.begin(), vecdata.end() );	

		for( size_t i=0; i<vecdata.size(); ++i )
			vecdata[i].wRankSchool = i+1;

		for( size_t i=0; i<vecdata.size(); ++i )
		{
			const SCHOOLWARS_PLAYER_DATA& splayer_data = vecdata[i];

			SCHOOLWARS_PLAYER_DATA* prank_player = PlayerDataGet( splayer_data.dwInfoCharID );
			if ( prank_player && prank_player->dwInfoCharID == splayer_data.dwInfoCharID )
				prank_player->wRankSchool = splayer_data.wRankSchool;
		}
	}

	{	//mp rank
		SCHOOLWARS_PLAYER_DATA_VEC vecdata;
		vecdata.reserve( m_mapPlayerDataMP.size() );

		for( SCHOOLWARS_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataMP.begin(); iterplayer != m_mapPlayerDataMP.end(); ++ iterplayer ){
			const SCHOOLWARS_PLAYER_DATA& splayerdata = (*iterplayer).second;
			vecdata.push_back( splayerdata );
		}

		std::sort( vecdata.begin(), vecdata.end() );	

		for( size_t i=0; i<vecdata.size(); ++i )
			vecdata[i].wRankSchool = i+1;

		for( size_t i=0; i<vecdata.size(); ++i )
		{
			const SCHOOLWARS_PLAYER_DATA& splayer_data = vecdata[i];

			SCHOOLWARS_PLAYER_DATA* prank_player = PlayerDataGet( splayer_data.dwInfoCharID );
			if ( prank_player && prank_player->dwInfoCharID == splayer_data.dwInfoCharID )
				prank_player->wRankSchool = splayer_data.wRankSchool;
		}
	}

	{	//phx rank
		SCHOOLWARS_PLAYER_DATA_VEC vecdata;
		vecdata.reserve( m_mapPlayerDataPHX.size() );

		for( SCHOOLWARS_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataPHX.begin(); iterplayer != m_mapPlayerDataPHX.end(); ++ iterplayer ){
			const SCHOOLWARS_PLAYER_DATA& splayerdata = (*iterplayer).second;
			vecdata.push_back( splayerdata );
		}

		std::sort( vecdata.begin(), vecdata.end() );	

		for( size_t i=0; i<vecdata.size(); ++i )
			vecdata[i].wRankSchool = i+1;

		for( size_t i=0; i<vecdata.size(); ++i )
		{
			const SCHOOLWARS_PLAYER_DATA& splayer_data = vecdata[i];

			SCHOOLWARS_PLAYER_DATA* prank_player = PlayerDataGet( splayer_data.dwInfoCharID );
			if ( prank_player && prank_player->dwInfoCharID == splayer_data.dwInfoCharID )
				prank_player->wRankSchool = splayer_data.wRankSchool;
		}
	}
}

void GLPVPSchoolWarsField::CalculateRankingAll()
{
	SCHOOLWARS_PLAYER_DATA_VEC vecdata;
	vecdata.reserve( m_mapPlayerDataSG.size() + m_mapPlayerDataMP.size() + m_mapPlayerDataPHX.size() );

	for( SCHOOLWARS_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataSG.begin(); iterplayer != m_mapPlayerDataSG.end(); ++ iterplayer ){
		const SCHOOLWARS_PLAYER_DATA& splayerdata = (*iterplayer).second;
		vecdata.push_back( splayerdata );
	}

	for( SCHOOLWARS_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataMP.begin(); iterplayer != m_mapPlayerDataMP.end(); ++ iterplayer ){
		const SCHOOLWARS_PLAYER_DATA& splayerdata = (*iterplayer).second;
		vecdata.push_back( splayerdata );
	}

	for( SCHOOLWARS_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataPHX.begin(); iterplayer != m_mapPlayerDataPHX.end(); ++ iterplayer ){
		const SCHOOLWARS_PLAYER_DATA& splayerdata = (*iterplayer).second;
		vecdata.push_back( splayerdata );
	}

	std::sort( vecdata.begin(), vecdata.end() );	

	for( size_t i=0; i<vecdata.size(); ++i )
		vecdata[i].wRankAll = i+1;

	for( size_t i=0; i<vecdata.size(); ++i )
	{
		const SCHOOLWARS_PLAYER_DATA& splayer_data = vecdata[i];
		SCHOOLWARS_PLAYER_DATA* prank_player = PlayerDataGet( splayer_data.dwInfoCharID );
		if ( prank_player && prank_player->dwInfoCharID == splayer_data.dwInfoCharID )
			prank_player->wRankAll = splayer_data.wRankAll;
	}
}

void GLPVPSchoolWarsField::RankingSend()
{
	SCHOOLWARS_PLAYER_DATA_VEC vecdata;
	vecdata.reserve( m_mapPlayerDataSG.size() + m_mapPlayerDataMP.size() + m_mapPlayerDataPHX.size() );

	for( SCHOOLWARS_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataSG.begin(); iterplayer != m_mapPlayerDataSG.end(); ++ iterplayer ){
		const SCHOOLWARS_PLAYER_DATA& splayerdata = (*iterplayer).second;
		vecdata.push_back( splayerdata );
	}

	for( SCHOOLWARS_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataMP.begin(); iterplayer != m_mapPlayerDataMP.end(); ++ iterplayer ){
		const SCHOOLWARS_PLAYER_DATA& splayerdata = (*iterplayer).second;
		vecdata.push_back( splayerdata );
	}

	for( SCHOOLWARS_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataPHX.begin(); iterplayer != m_mapPlayerDataPHX.end(); ++ iterplayer ){
		const SCHOOLWARS_PLAYER_DATA& splayerdata = (*iterplayer).second;
		vecdata.push_back( splayerdata );
	}

	GLMSG::SNETPC_SCHOOLWARS_F2C_RANKING_UPDATE	netmsg_ranking;
	GLMSG::SNETPC_SCHOOLWARS_F2A_RANKINFO_PC	netmsg_ranking_agent;

	WORD wNum = 0;
	for ( size_t i=0; i<vecdata.size(); ++i )
	{
		const SCHOOLWARS_PLAYER_DATA& splayer_data = vecdata[i];
		netmsg_ranking.ADDRANK( splayer_data );
		if ( netmsg_ranking.wRankNum == SCHOOLWARS_PACKET_RANK_NUM )
		{
			GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMap.dwID, &netmsg_ranking );
			wNum += netmsg_ranking.wRankNum;
			netmsg_ranking.Reset();
		}

		if ( splayer_data.wRankAll > 0 && splayer_data.wRankAll <= SCHOOLWARS_MINI_RANKING_NUM )
			netmsg_ranking_agent.ADDRANK( splayer_data );
	}

	if ( netmsg_ranking.wRankNum != 0 )
	{
		GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMap.dwID, &netmsg_ranking );
		wNum += netmsg_ranking.wRankNum;
		netmsg_ranking.Reset();
	}

	GLGaeaServer::GetInstance().GetConsoleMsg()->Write( "GLPVPSchoolWarsField Send Rank num: %u", wNum );
	CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "GLPVPSchoolWarsField Send Rank num: %u", wNum );

	if ( netmsg_ranking_agent.wRankNum > 0 )	
	{
		GLGaeaServer::GetInstance().SENDTOAGENT( &netmsg_ranking_agent );
		GLGaeaServer::GetInstance().GetConsoleMsg()->Write( "GLPVPSchoolWarsField Send to agent rank num: %u", netmsg_ranking_agent.wRankNum );
		CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "GLPVPSchoolWarsField Send to agent rank num: %u", netmsg_ranking_agent.wRankNum );
		netmsg_ranking_agent.wRankNum = 0;
	}

	GLMSG::SNETPC_SCHOOLWARS_F2C_RANKING_END	netmsg_ranking_end;
	netmsg_ranking_end.wPlayerNum[SCHOOLWARS_SCHOOL_SG] = (WORD)m_mapPlayerDataSG.size();
	netmsg_ranking_end.wPlayerNum[SCHOOLWARS_SCHOOL_MP] = (WORD)m_mapPlayerDataMP.size();
	netmsg_ranking_end.wPlayerNum[SCHOOLWARS_SCHOOL_PHX] = (WORD)m_mapPlayerDataPHX.size();
	GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMap.dwID, &netmsg_ranking_end );
}

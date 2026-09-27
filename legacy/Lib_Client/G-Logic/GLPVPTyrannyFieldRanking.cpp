#include "StdAfx.h"
#include "GLPVPTyrannyField.h"
#include "GLGaeaServer.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void GLPVPTyrannyField::InsertPlayerData( TYRANNY_REGISTER_DATA* pPlayerData )
{
	if ( !pPlayerData )			return;
	if ( pPlayerData->dwInfoCharID == TYRANNY_PLAYER_NULL )
	{
		CDebugSet::ToFileWithTime( "_pvptyranny.txt", "GLPVPTyrannyField invalid ID:%d %s", pPlayerData->dwInfoCharID, pPlayerData->szInfoCharName );
		return;
	}

	TYRANNY_PLAYER_DATA* pdata = PlayerDataGet( pPlayerData->dwInfoCharID );
	if ( pdata )
	{
		CDebugSet::ToFileWithTime( "_pvptyranny.txt", "GLPVPTyrannyField char:%d %s already exist", pPlayerData->dwInfoCharID, pPlayerData->szInfoCharName );
		return;
	}
		
	if ( pPlayerData->wInfoSchool >= TYRANNY_SCHOOL_SIZE )
	{
		CDebugSet::ToFileWithTime( "_pvptyranny.txt", "GLPVPTyrannyField char:%d %s invalid school %d", pPlayerData->dwInfoCharID, pPlayerData->szInfoCharName, pPlayerData->wInfoSchool );
		return;
	}

	if ( pPlayerData->wInfoClass >= GLCI_NUM_8CLASS )
	{
		CDebugSet::ToFileWithTime( "_pvptyranny.txt", "GLPVPTyrannyField char:%d %s invalid class %d", pPlayerData->dwInfoCharID, pPlayerData->szInfoCharName, pPlayerData->wInfoClass );
		return;
	}

	TYRANNY_PLAYER_DATA sdata;
	sdata.dwInfoCharID	= pPlayerData->dwInfoCharID;
	sdata.wInfoClass	= pPlayerData->wInfoClass;
	sdata.wInfoSchool	= pPlayerData->wInfoSchool;
	StringCchCopy( sdata.szInfoCharName, TYRANNY_CHARNAME_STRING_SIZE, pPlayerData->szInfoCharName );

	switch ( sdata.wInfoSchool )
	{
	case TYRANNY_SCHOOL_SG:
		{
			m_mapPlayerDataSG.insert( std::make_pair( sdata.dwInfoCharID, sdata ) );
		}break;

	case TYRANNY_SCHOOL_MP:
		{
			m_mapPlayerDataMP.insert( std::make_pair( sdata.dwInfoCharID, sdata ) );
		}break;

	case TYRANNY_SCHOOL_PHX:
		{
			m_mapPlayerDataPHX.insert( std::make_pair( sdata.dwInfoCharID, sdata ) );
		}break;
	};
}

void GLPVPTyrannyField::ScoreKill( DWORD dwCharKilled, DWORD dwCharKiller )
{
	if ( m_emState != TYRANNY_STATE_BATTLE )	return;

	TYRANNY_PLAYER_DATA* pplayerkilled = PlayerDataGet( dwCharKilled );
	TYRANNY_PLAYER_DATA* pplayerkiller = PlayerDataGet( dwCharKiller );

	if ( pplayerkilled && pplayerkiller )
	{
		pplayerkilled->wScoreDeath ++;
		pplayerkiller->wScoreKill ++;

		GLMSG::SNETPC_TYRANNY_F2A_TOWER_SCORE netmsgagent;
		netmsgagent.wSchoolKilled = pplayerkilled->wInfoSchool;
		netmsgagent.wSchoolKiller = pplayerkiller->wInfoSchool;
		GLGaeaServer::GetInstance().SENDTOAGENT( &netmsgagent );

		//CDebugSet::ToFileWithTime( "_pvptyrannyscore.txt", "player:[%u]%s add death", pplayerkilled->dwInfoCharID, pplayerkilled->szInfoCharName );
		//CDebugSet::ToFileWithTime( "_pvptyrannyscore.txt", "player:[%u]%s add kill", pplayerkiller->dwInfoCharID, pplayerkiller->szInfoCharName );
	}
}

void GLPVPTyrannyField::ScoreDamage( DWORD dwChar, DWORD dwDamage )
{
	if ( m_emState != TYRANNY_STATE_BATTLE )	return;

	TYRANNY_PLAYER_DATA* pplayerdata = PlayerDataGet( dwChar );
	if ( pplayerdata )
	{
		pplayerdata->dwScoreDamage += dwDamage;
		//CDebugSet::ToFileWithTime( "_pvptyrannyscore.txt", "player:[%u]%s add damage %u", pplayerdata->dwInfoCharID, pplayerdata->szInfoCharName, dwDamage );
	}
}

void GLPVPTyrannyField::ScoreHeal( DWORD dwChar, DWORD dwHeal )
{
	if ( m_emState != TYRANNY_STATE_BATTLE )	return;

	TYRANNY_PLAYER_DATA* pplayerdata = PlayerDataGet( dwChar );
	if ( pplayerdata )
	{
		pplayerdata->dwScoreHeal += dwHeal;
		//CDebugSet::ToFileWithTime( "_pvptyrannyscore.txt", "player:[%u]%s add heal %u", pplayerdata->dwInfoCharID, pplayerdata->szInfoCharName, dwHeal );
	}
}

void GLPVPTyrannyField::ScoreResu( DWORD dwChar )
{
	if ( m_emState != TYRANNY_STATE_BATTLE )	return;

	TYRANNY_PLAYER_DATA* pplayerdata = PlayerDataGet( dwChar );
	if ( pplayerdata )
	{
		pplayerdata->wScoreResu ++;
		//CDebugSet::ToFileWithTime( "_pvptyrannyscore.txt", "player:[%u]%s add resu", pplayerdata->dwInfoCharID, pplayerdata->szInfoCharName );
	}
}

void GLPVPTyrannyField::ScoreCalculate(TYRANNY_PLAYER_DATA_MAP& mapData, WORD wSchoolWinner )
{
	for( TYRANNY_PLAYER_DATA_MAP_ITER iterplayer = mapData.begin(); iterplayer != mapData.end(); ++ iterplayer )
	{
		TYRANNY_PLAYER_DATA& splayer_data = (*iterplayer).second;

		//calculate score
		float ftotal_score = 0.0f;
		ftotal_score += float( splayer_data.dwScoreDamage * m_fDamageVar );
		ftotal_score += float( splayer_data.dwScoreHeal * m_fHealVar );
		ftotal_score += float( splayer_data.wScoreKill * m_fKillVar );
		ftotal_score += float( splayer_data.wScoreResu * m_fResuVar );
		splayer_data.dwScoreTotal = (DWORD)ftotal_score;

		//calculate points
		if ( splayer_data.dwScoreTotal > 0 )
		{
			//base point
			WORD wpoint_base = m_wBasePointLoss;
			if ( wSchoolWinner != TYRANNY_SCHOOL_SIZE &&
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

void GLPVPTyrannyField::CalculateRankingSchool( )
{
	
	{	//sg rank
		TYRANNY_PLAYER_DATA_VEC vecdata;
		vecdata.reserve( m_mapPlayerDataSG.size() );

		for( TYRANNY_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataSG.begin(); 
			iterplayer != m_mapPlayerDataSG.end(); ++ iterplayer ){
			const TYRANNY_PLAYER_DATA& splayerdata = (*iterplayer).second;
			vecdata.push_back( splayerdata );
		}

		std::sort( vecdata.begin(), vecdata.end() );	

		for( size_t i=0; i<vecdata.size(); ++i )
			vecdata[i].wRankSchool = i+1;

		for( size_t i=0; i<vecdata.size(); ++i )
		{
			const TYRANNY_PLAYER_DATA& splayer_data = vecdata[i];

			TYRANNY_PLAYER_DATA* prank_player = PlayerDataGet( splayer_data.dwInfoCharID );
			if ( prank_player && prank_player->dwInfoCharID == splayer_data.dwInfoCharID )
				prank_player->wRankSchool = splayer_data.wRankSchool;
		}
	}

	{	//mp rank
		TYRANNY_PLAYER_DATA_VEC vecdata;
		vecdata.reserve( m_mapPlayerDataMP.size() );

		for( TYRANNY_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataMP.begin();
			iterplayer != m_mapPlayerDataMP.end(); ++ iterplayer ){
			const TYRANNY_PLAYER_DATA& splayerdata = (*iterplayer).second;
			vecdata.push_back( splayerdata );
		}

		std::sort( vecdata.begin(), vecdata.end() );	

		for( size_t i=0; i<vecdata.size(); ++i )
			vecdata[i].wRankSchool = i+1;

		for( size_t i=0; i<vecdata.size(); ++i )
		{
			const TYRANNY_PLAYER_DATA& splayer_data = vecdata[i];

			TYRANNY_PLAYER_DATA* prank_player = PlayerDataGet( splayer_data.dwInfoCharID );
			if ( prank_player && prank_player->dwInfoCharID == splayer_data.dwInfoCharID )
				prank_player->wRankSchool = splayer_data.wRankSchool;
		}
	}
	
	{	//phx rank
		TYRANNY_PLAYER_DATA_VEC vecdata;
		vecdata.reserve( m_mapPlayerDataPHX.size() );

		for( TYRANNY_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataPHX.begin(); 
			iterplayer != m_mapPlayerDataPHX.end(); ++ iterplayer ){
			const TYRANNY_PLAYER_DATA& splayerdata = (*iterplayer).second;
			vecdata.push_back( splayerdata );
		}

		std::sort( vecdata.begin(), vecdata.end() );	

		for( size_t i=0; i<vecdata.size(); ++i )
			vecdata[i].wRankSchool = i+1;

		for( size_t i=0; i<vecdata.size(); ++i )
		{
			const TYRANNY_PLAYER_DATA& splayer_data = vecdata[i];

			TYRANNY_PLAYER_DATA* prank_player = PlayerDataGet( splayer_data.dwInfoCharID );
			if ( prank_player && prank_player->dwInfoCharID == splayer_data.dwInfoCharID )
				prank_player->wRankSchool = splayer_data.wRankSchool;
		}
	}
}

void GLPVPTyrannyField::CalculateRankingAll()
{
	TYRANNY_PLAYER_DATA_VEC vecdata;
	vecdata.reserve( m_mapPlayerDataSG.size() + m_mapPlayerDataMP.size() + m_mapPlayerDataPHX.size() );

	for( TYRANNY_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataSG.begin(); 
		iterplayer != m_mapPlayerDataSG.end(); ++ iterplayer ){
		const TYRANNY_PLAYER_DATA& splayerdata = (*iterplayer).second;
		vecdata.push_back( splayerdata );
	}

	for( TYRANNY_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataMP.begin(); 
		iterplayer != m_mapPlayerDataMP.end(); ++ iterplayer ){
		const TYRANNY_PLAYER_DATA& splayerdata = (*iterplayer).second;
		vecdata.push_back( splayerdata );
	}

	for( TYRANNY_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataPHX.begin(); 
		iterplayer != m_mapPlayerDataPHX.end(); ++ iterplayer ){
		const TYRANNY_PLAYER_DATA& splayerdata = (*iterplayer).second;
		vecdata.push_back( splayerdata );
	}

	std::sort( vecdata.begin(), vecdata.end() );	

	for( size_t i=0; i<vecdata.size(); ++i )
		vecdata[i].wRankAll = i+1;

	for( size_t i=0; i<vecdata.size(); ++i )
	{
		const TYRANNY_PLAYER_DATA& splayer_data = vecdata[i];

		TYRANNY_PLAYER_DATA* prank_player = PlayerDataGet( splayer_data.dwInfoCharID );
		if ( prank_player && prank_player->dwInfoCharID == splayer_data.dwInfoCharID )
			prank_player->wRankAll = splayer_data.wRankAll;
	}
}

void GLPVPTyrannyField::RankingSend()
{
	TYRANNY_PLAYER_DATA_VEC vecdata;
	vecdata.reserve( m_mapPlayerDataSG.size() + m_mapPlayerDataMP.size() + m_mapPlayerDataPHX.size() );

	for( TYRANNY_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataSG.begin(); 
		iterplayer != m_mapPlayerDataSG.end(); ++ iterplayer ){
			const TYRANNY_PLAYER_DATA& splayerdata = (*iterplayer).second;
			vecdata.push_back( splayerdata );
	}

	for( TYRANNY_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataMP.begin(); 
		iterplayer != m_mapPlayerDataMP.end(); ++ iterplayer ){
			const TYRANNY_PLAYER_DATA& splayerdata = (*iterplayer).second;
			vecdata.push_back( splayerdata );
	}

	for( TYRANNY_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataPHX.begin(); 
		iterplayer != m_mapPlayerDataPHX.end(); ++ iterplayer ){
			const TYRANNY_PLAYER_DATA& splayerdata = (*iterplayer).second;
			vecdata.push_back( splayerdata );
	}

	GLMSG::SNETPC_TYRANNY_F2C_RANKING_UPDATE	netmsg_ranking;
	GLMSG::SNETPC_TYRANNY_F2A_RANKINFO_PC	netmsg_ranking_agent;

	WORD wNum = 0;
	for ( size_t i=0; i<vecdata.size(); ++i )
	{
		const TYRANNY_PLAYER_DATA& splayer_data = vecdata[i];
		netmsg_ranking.ADDRANK( splayer_data );
		if ( netmsg_ranking.wRankNum == TYRANNY_PACKET_RANK_NUM )
		{
			GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMap.dwID, &netmsg_ranking );
			wNum += netmsg_ranking.wRankNum;
			netmsg_ranking.Reset();
		}

		if ( splayer_data.wRankAll > 0 && splayer_data.wRankAll <= TYRANNY_MINI_RANKING_NUM )
			netmsg_ranking_agent.ADDRANK( splayer_data );
	}

	if ( netmsg_ranking.wRankNum != 0 )
	{
		GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMap.dwID, &netmsg_ranking );
		wNum += netmsg_ranking.wRankNum;
		netmsg_ranking.Reset();
	}

	GLGaeaServer::GetInstance().GetConsoleMsg()->Write( "GLPVPTyrannyField Send Rank num: %u", wNum );
	CDebugSet::ToFileWithTime( "_pvptyranny.txt", "GLPVPTyrannyField Send Rank num: %u", wNum );

	if ( netmsg_ranking_agent.wRankNum > 0 )	
	{
		GLGaeaServer::GetInstance().SENDTOAGENT( &netmsg_ranking_agent );
		GLGaeaServer::GetInstance().GetConsoleMsg()->Write( "GLPVPTyrannyField Send to agent rank num: %u", netmsg_ranking_agent.wRankNum );
		CDebugSet::ToFileWithTime( "_pvptyranny.txt", "GLPVPTyrannyField Send to agent rank num: %u", netmsg_ranking_agent.wRankNum );
		netmsg_ranking_agent.wRankNum = 0;
	}

	GLMSG::SNETPC_TYRANNY_F2C_RANKING_END	netmsg_ranking_end;
	netmsg_ranking_end.wPlayerNum[TYRANNY_SCHOOL_SG] = (WORD)m_mapPlayerDataSG.size();
	netmsg_ranking_end.wPlayerNum[TYRANNY_SCHOOL_MP] = (WORD)m_mapPlayerDataMP.size();
	netmsg_ranking_end.wPlayerNum[TYRANNY_SCHOOL_PHX] = (WORD)m_mapPlayerDataPHX.size();
	GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMap.dwID, &netmsg_ranking_end );
}
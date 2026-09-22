#include "StdAfx.h"
#include "GLPVPSchoolWarsAgent.h"
#include "GLAgentServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SCHOOLWARS_REGISTER_DATA* GLPVPSchoolWarsAgent::PlayerDataGet( DWORD dwCharID )
{
	if ( dwCharID == SCHOOLWARS_PLAYER_NULL ) return NULL;

	{
		SCHOOLWARS_REGISTER_DATA_MAP_ITER iterdata = m_mapPlayerDataSG.find( dwCharID );
		if( iterdata != m_mapPlayerDataSG.end() ){
			return &(*iterdata).second;
		}
	}

	{
		SCHOOLWARS_REGISTER_DATA_MAP_ITER iterdata = m_mapPlayerDataMP.find( dwCharID );
		if( iterdata != m_mapPlayerDataMP.end() ){
			return &(*iterdata).second;
		}
	}

	{
		SCHOOLWARS_REGISTER_DATA_MAP_ITER iterdata = m_mapPlayerDataPHX.find( dwCharID );
		if( iterdata != m_mapPlayerDataPHX.end() ){
			return &(*iterdata).second;
		}
	}

	return NULL;
};

SCHOOLWARS_REGISTER_DATA_QUEUE* GLPVPSchoolWarsAgent::PlayerQueueGet( DWORD dwCharID )
{
	if ( dwCharID == SCHOOLWARS_PLAYER_NULL ) return NULL;

	for( size_t i=0; i<m_vecPlayerDataQueueSG.size(); ++i ){
		SCHOOLWARS_REGISTER_DATA_QUEUE sdata = m_vecPlayerDataQueueSG[i];
		if ( sdata.sPlayerData.dwInfoCharID == dwCharID ){
			return &m_vecPlayerDataQueueSG[i];
		}
	}

	for( size_t i=0; i<m_vecPlayerDataQueueMP.size(); ++i ){
		SCHOOLWARS_REGISTER_DATA_QUEUE sdata = m_vecPlayerDataQueueMP[i];
		if ( sdata.sPlayerData.dwInfoCharID == dwCharID ){
			return &m_vecPlayerDataQueueMP[i];
		}
	}

	for( size_t i=0; i<m_vecPlayerDataQueuePHX.size(); ++i ){
		SCHOOLWARS_REGISTER_DATA_QUEUE sdata = m_vecPlayerDataQueuePHX[i];
		if ( sdata.sPlayerData.dwInfoCharID == dwCharID ){
			return &m_vecPlayerDataQueuePHX[i];
		}
	}

	return NULL;
};


BOOL GLPVPSchoolWarsAgent::IsPlayerFull( SCHOOLWARS_REGISTER_DATA_MAP& mapData  )
{
	//unlimited team members
	if ( m_wPlayerLimit == 0 )	return FALSE;

	if ( mapData.size() >= m_wPlayerLimit )
		return TRUE;

	return FALSE;
}

BOOL GLPVPSchoolWarsAgent::IsPlayerReg( SCHOOLWARS_REGISTER_DATA_MAP& mapData, DWORD dwCharID )
{
	if ( dwCharID == SCHOOLWARS_PLAYER_NULL )	return FALSE;

	SCHOOLWARS_REGISTER_DATA_MAP_ITER iterdata = mapData.find( dwCharID );
	if( iterdata != mapData.end() ){
		return TRUE;
	}

	return FALSE;
}

BOOL GLPVPSchoolWarsAgent::IsPlayerQueue( SCHOOLWARS_REGISTER_DATA_QUEUE_VEC& vecData, DWORD dwCharID )
{
	if ( dwCharID == SCHOOLWARS_PLAYER_NULL )	return FALSE;

	for( size_t i=0; i<vecData.size(); ++i ){
		const SCHOOLWARS_REGISTER_DATA_QUEUE sdata = vecData[i];
		if ( sdata.sPlayerData.dwInfoCharID == dwCharID ){
			return TRUE;
			break;
		}
	}

	return FALSE;
}

BOOL GLPVPSchoolWarsAgent::InsertQueue( SCHOOLWARS_REGISTER_DATA_QUEUE_VEC& vecData, DWORD dwCharID, const char* szName, WORD wClass, WORD wSchool )
{
	if ( !szName )							return FALSE;
	if ( dwCharID == SCHOOLWARS_PLAYER_NULL )	return FALSE;
	if ( wClass >= GLCI_NUM_8CLASS )		return FALSE;
	if ( wSchool >= SCHOOLWARS_SCHOOL_SIZE )	return FALSE;

	SCHOOLWARS_REGISTER_DATA_QUEUE sdata;
	sdata.sPlayerData.dwInfoCharID = dwCharID;
	sdata.sPlayerData.wInfoClass = wClass;
	sdata.sPlayerData.wInfoSchool = wSchool;
	sdata.wQueueNum = (WORD)vecData.size() + 1;
	StringCchCopy( sdata.sPlayerData.szInfoCharName, SCHOOLWARS_CHARNAME_STRING_SIZE, szName );
	vecData.push_back( sdata );

	return TRUE;
}

BOOL GLPVPSchoolWarsAgent::InsertRegister( SCHOOLWARS_REGISTER_DATA_MAP& mapData, DWORD dwCharID, const char* szName, WORD wClass, WORD wSchool )
{
	if ( !szName )							return FALSE;
	if ( dwCharID == SCHOOLWARS_PLAYER_NULL )	return FALSE;
	if ( wClass >= GLCI_NUM_8CLASS )		return FALSE;
	if ( wSchool >= SCHOOLWARS_SCHOOL_SIZE )	return FALSE;

	SCHOOLWARS_REGISTER_DATA sdata;
	sdata.dwInfoCharID = dwCharID;
	sdata.wInfoClass = wClass;
	sdata.wInfoSchool = wSchool;
	StringCchCopy( sdata.szInfoCharName, SCHOOLWARS_CHARNAME_STRING_SIZE, szName );
	mapData.insert( std::make_pair( sdata.dwInfoCharID, sdata ) );

	return TRUE;
}

BOOL GLPVPSchoolWarsAgent::DeleteRegister( DWORD dwCharID, SCHOOLWARS_REGISTER_DATA_MAP& mapData  )
{
	if ( dwCharID == SCHOOLWARS_PLAYER_NULL )	return FALSE;

	SCHOOLWARS_REGISTER_DATA_MAP_ITER iterdata = mapData.find( dwCharID );
	if( iterdata != mapData.end() ){
		mapData.erase( iterdata );
		return TRUE;
	}

	return FALSE;
}

BOOL GLPVPSchoolWarsAgent::DeleteQueue( DWORD dwCharID, SCHOOLWARS_REGISTER_DATA_QUEUE_VEC& vecData )
{
	if ( dwCharID == SCHOOLWARS_PLAYER_NULL )	return FALSE;

	for( size_t i=0; i<vecData.size(); ++i ){
		SCHOOLWARS_REGISTER_DATA_QUEUE sdata = vecData[i];
		if ( sdata.sPlayerData.dwInfoCharID == dwCharID ){
			vecData.erase( vecData.begin() + i );
			return TRUE;
			break;
		}
	}

	return FALSE;
}

void GLPVPSchoolWarsAgent::ProcessQueueData( SCHOOLWARS_REGISTER_DATA_MAP& mapData, SCHOOLWARS_REGISTER_DATA_QUEUE_VEC& vecData )
{
	if ( IsPlayerFull( mapData ) )	return;

	if ( vecData.size() <= 0 )	return;

	//get data from the very first vector entry
	SCHOOLWARS_REGISTER_DATA_QUEUE sdata = vecData[0];
	PGLCHARAG pcharmove = GLAgentServer::GetInstance().GetCharID( sdata.sPlayerData.dwInfoCharID );
	if ( pcharmove ){
		//insert to player data
		mapData.insert( std::make_pair( sdata.sPlayerData.dwInfoCharID, sdata.sPlayerData ) );

		//inform player moved from queue to registered
		GLMSG::SNETPC_SCHOOLWARS_A2C_QUEUE_MOVED netmsgclient;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pcharmove->m_dwClientID, &netmsgclient );
	}

	//erase the entry from vector
	vecData.erase(vecData.begin());
}

void GLPVPSchoolWarsAgent::UpdateQueuePosition( SCHOOLWARS_REGISTER_DATA_QUEUE_VEC& vecData )
{
	if ( vecData.size() <= 0 )	return;

	//inform players of their current position in queue
	for( size_t i=0; i<vecData.size(); ++i ){
		SCHOOLWARS_REGISTER_DATA_QUEUE &sdata = vecData[i];
		sdata.wQueueNum = i + 1;

		PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( sdata.sPlayerData.dwInfoCharID );
		if ( !pchar ) continue;

		GLMSG::SNETPC_SCHOOLWARS_A2C_QUEUE_UPDATE netmsgclient;
		netmsgclient.wQueueNum = sdata.wQueueNum;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgclient );
	}
}

void GLPVPSchoolWarsAgent::ProcessQueueDataAll( SCHOOLWARS_REGISTER_DATA_MAP& mapData, SCHOOLWARS_REGISTER_DATA_QUEUE_VEC& vecData )
{
	//try to move the remaining queued players to registered
	if ( !IsPlayerFull( mapData ) ){

		while(1){

			ProcessQueueData( mapData, vecData );

			if ( IsPlayerFull( mapData ) )
				break;

			if ( vecData.size() <= 0 )
				break;

			Sleep(0);
		}
	}

	//remove queued players
	for( size_t i=0; i<vecData.size(); ++i ){
		SCHOOLWARS_REGISTER_DATA_QUEUE sdata = vecData[i];

		PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( sdata.sPlayerData.dwInfoCharID );
		if ( !pchar ) continue;

		GLMSG::SNETPC_SCHOOLWARS_A2C_REGISTER_FB netmsgfb;
		netmsgfb.emFB = SCHOOLWARS_REGISTER_FB_CANCEL_REG;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
	}

	//clear
	vecData.clear();
}

void GLPVPSchoolWarsAgent::RequestPlayerRegister( DWORD dwCharID, WORD wLevel, LONGLONG llContri, BOOL bRegister )
{	
	PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( dwCharID );
	if ( !pchar )	return;

	GLMSG::SNETPC_SCHOOLWARS_A2C_REGISTER_FB netmsgfb;

	//invalid charid
	if ( pchar->m_dwCharID != dwCharID ){
		netmsgfb.emFB = SCHOOLWARS_REGISTER_FB_INVALID_CHARID;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}

	//registration not running
	/*if ( m_emState != SCHOOLWARS_STATE_REGISTER ){
		netmsgfb.emFB = SCHOOLWARS_REGISTER_FB_NOTPOSSIBLE;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}*/

	//register only available in channel 0 
	if ( pchar->m_nChannel != 0 ){
		netmsgfb.emFB = SCHOOLWARS_REGISTER_FB_INVALID_CHANNEL;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}

	//player level too low
	if ( wLevel < m_wLevelReq ){
		netmsgfb.emFB = SCHOOLWARS_REGISTER_FB_LEVEL_LOW;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}

	//contri low
	if ( llContri < m_llContriReq ){
		netmsgfb.emFB = SCHOOLWARS_REGISTER_FB_CONTRI_LOW;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}

	if ( bRegister )
	{
		if ( pchar->m_wSchool == SCHOOLWARS_SCHOOL_SG )
			PlayerRegister( pchar->m_dwCharID, m_mapPlayerDataSG, m_vecPlayerDataQueueSG );
		else if ( pchar->m_wSchool == SCHOOLWARS_SCHOOL_MP )
			PlayerRegister( pchar->m_dwCharID, m_mapPlayerDataMP, m_vecPlayerDataQueueMP );
		else if ( pchar->m_wSchool == SCHOOLWARS_SCHOOL_PHX )
			PlayerRegister( pchar->m_dwCharID, m_mapPlayerDataPHX, m_vecPlayerDataQueuePHX );
		else
		{
			netmsgfb.emFB = SCHOOLWARS_REGISTER_FB_INVALID_SCHOOL;
			GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
			return;
		}

		SCHOOLWARS_REGISTER_DATA* pdataregchk = PlayerDataGet( dwCharID );
		SCHOOLWARS_REGISTER_DATA_QUEUE* pdataqueuechk = PlayerQueueGet( dwCharID );
		if ( !pdataregchk && !pdataqueuechk ){
			CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "RequestPlayerRegister failed to register [%d]'%s'", 
				pchar->m_dwCharID, pchar->m_szName );
		}
	}
	else
	{
		if ( pchar->m_wSchool == SCHOOLWARS_SCHOOL_SG )
			CancelRegister( pchar->m_dwCharID, m_mapPlayerDataSG, m_vecPlayerDataQueueSG );
		else if ( pchar->m_wSchool == SCHOOLWARS_SCHOOL_MP )
			CancelRegister( pchar->m_dwCharID, m_mapPlayerDataMP, m_vecPlayerDataQueueMP );
		else if ( pchar->m_wSchool == SCHOOLWARS_SCHOOL_PHX )
			CancelRegister( pchar->m_dwCharID, m_mapPlayerDataPHX, m_vecPlayerDataQueuePHX );
		else
		{
			netmsgfb.emFB = SCHOOLWARS_REGISTER_FB_INVALID_SCHOOL;
			GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
			return;
		}

		SCHOOLWARS_REGISTER_DATA* pdataregchk = PlayerDataGet( dwCharID );
		SCHOOLWARS_REGISTER_DATA_QUEUE* pdataqueuechk = PlayerQueueGet( dwCharID );
		if ( pdataregchk || pdataqueuechk ){
			CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "RequestPlayerRegister failed to unregister [%d]'%s'", 
				pchar->m_dwCharID, pchar->m_szName );
		}
	}
}

void GLPVPSchoolWarsAgent::RequestPlayerCancelRegister( DWORD dwCharID )
{	
	if ( dwCharID == SCHOOLWARS_PLAYER_NULL )	return;
	if ( m_emState != SCHOOLWARS_STATE_REGISTER )	return;

	SCHOOLWARS_REGISTER_DATA* pdatareg = PlayerDataGet( dwCharID );
	if ( pdatareg )
	{
		WORD wplayerschool = pdatareg->wInfoSchool;

		if ( wplayerschool == SCHOOLWARS_SCHOOL_SG && DeleteRegister( dwCharID, m_mapPlayerDataSG ) )
		{
			ProcessQueueData( m_mapPlayerDataSG, m_vecPlayerDataQueueSG );
			UpdateQueuePosition( m_vecPlayerDataQueueSG );
		}
		else if ( wplayerschool == SCHOOLWARS_SCHOOL_MP && DeleteRegister( dwCharID, m_mapPlayerDataMP ) )
		{
			ProcessQueueData( m_mapPlayerDataMP, m_vecPlayerDataQueueMP );
			UpdateQueuePosition( m_vecPlayerDataQueueMP );
		}
		else if ( wplayerschool == SCHOOLWARS_SCHOOL_PHX && DeleteRegister( dwCharID, m_mapPlayerDataPHX ) )
		{
			ProcessQueueData( m_mapPlayerDataPHX, m_vecPlayerDataQueuePHX );
			UpdateQueuePosition( m_vecPlayerDataQueuePHX );
		}
	}

	SCHOOLWARS_REGISTER_DATA_QUEUE* pdataqueue = PlayerQueueGet( dwCharID );
	if ( pdataqueue )
	{
		WORD wplayerschool = pdataqueue->sPlayerData.wInfoSchool;

		if ( wplayerschool == SCHOOLWARS_SCHOOL_SG && DeleteQueue( dwCharID, m_vecPlayerDataQueueSG ) )
		{
			ProcessQueueData( m_mapPlayerDataSG, m_vecPlayerDataQueueSG );
			UpdateQueuePosition( m_vecPlayerDataQueueSG );
		}
		else if ( wplayerschool == SCHOOLWARS_SCHOOL_MP && DeleteQueue( dwCharID, m_vecPlayerDataQueueMP ) )
		{
			ProcessQueueData( m_mapPlayerDataMP, m_vecPlayerDataQueueMP );
			UpdateQueuePosition( m_vecPlayerDataQueueMP );
		}
		else if ( wplayerschool == SCHOOLWARS_SCHOOL_PHX && DeleteQueue( dwCharID, m_vecPlayerDataQueuePHX ) )
		{
			ProcessQueueData( m_mapPlayerDataPHX, m_vecPlayerDataQueuePHX );
			UpdateQueuePosition( m_vecPlayerDataQueuePHX );
		}
	}


	SCHOOLWARS_REGISTER_DATA* pdataregchk = PlayerDataGet( dwCharID );
	if ( pdataregchk ){
		CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "RequestPlayerCancelRegister failed to delete registered [%d]'%s'", 
			pdataregchk->dwInfoCharID, pdataregchk->szInfoCharName );
	}

	SCHOOLWARS_REGISTER_DATA_QUEUE* pdataqueuechk = PlayerQueueGet( dwCharID );
	if ( pdataqueuechk ){
		CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "RequestPlayerCancelRegister failed to delete queued [%d]'%s'", 
			pdataqueuechk->sPlayerData.dwInfoCharID, pdataqueuechk->sPlayerData.szInfoCharName );
	}

	return;
}

void GLPVPSchoolWarsAgent::PlayerRegister( DWORD dwCharID, SCHOOLWARS_REGISTER_DATA_MAP& mapData, SCHOOLWARS_REGISTER_DATA_QUEUE_VEC& vecData )
{
	PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( dwCharID );
	if ( !pchar )	return;

	GLMSG::SNETPC_SCHOOLWARS_A2C_REGISTER_FB netmsgfb;

	//player already registered
	if ( IsPlayerReg( mapData, pchar->m_dwCharID ) ){
		netmsgfb.emFB = SCHOOLWARS_REGISTER_FB_ALREADY_REG;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}

	//player already queued
	if ( IsPlayerQueue( vecData, pchar->m_dwCharID ) ){
		netmsgfb.emFB = SCHOOLWARS_REGISTER_FB_ALREADY_QUEUE;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}

	if ( IsPlayerFull( mapData ) ){
		//player map is full insert to queue instead
		BOOL binsert = InsertQueue( vecData, pchar->m_dwCharID, pchar->m_szName, CharClassToIndex( pchar->m_emClass ), pchar->m_wSchool );
		if ( binsert ){
			netmsgfb.emFB = SCHOOLWARS_REGISTER_FB_QUEUED;
			netmsgfb.wQueueNum = (WORD)vecData.size();
			GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
			return;
		}
	}
	else
	{
		//insert player data
		BOOL binsert = InsertRegister( mapData, pchar->m_dwCharID, pchar->m_szName, CharClassToIndex( pchar->m_emClass ), pchar->m_wSchool );
		if ( binsert ){
			netmsgfb.emFB = SCHOOLWARS_REGISTER_FB_REGISTERED;
			GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
			return;
		}
	}

	netmsgfb.emFB = SCHOOLWARS_REGISTER_FB_FAILED;
	GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
}

void GLPVPSchoolWarsAgent::CancelRegister( DWORD dwCharID, SCHOOLWARS_REGISTER_DATA_MAP& mapData, SCHOOLWARS_REGISTER_DATA_QUEUE_VEC& vecData )
{
	PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( dwCharID );
	if ( !pchar )	return;

	GLMSG::SNETPC_SCHOOLWARS_A2C_REGISTER_FB netmsgfb;

	//if exist in registered, remove
	BOOL bdelete = DeleteRegister( pchar->m_dwCharID, mapData );
	if ( bdelete ){
		netmsgfb.emFB = SCHOOLWARS_REGISTER_FB_CANCEL_REG;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		ProcessQueueData( mapData, vecData );
		UpdateQueuePosition( vecData );
		return;
	}

	//not exist in registered, must be in queue vector
	bdelete = DeleteQueue( pchar->m_dwCharID, vecData );
	if ( bdelete ){
		netmsgfb.emFB = SCHOOLWARS_REGISTER_FB_CANCEL_REG;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		ProcessQueueData( mapData, vecData );
		UpdateQueuePosition( vecData );
		return;
	}

	//fail to delete
	if ( !bdelete ){
		netmsgfb.emFB = SCHOOLWARS_REGISTER_FB_CANCEL_FAIL;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}
}

void GLPVPSchoolWarsAgent::SendPlayerData()
{
	SCHOOLWARS_REGISTER_DATA_VEC vecdata;
	vecdata.reserve( m_mapPlayerDataSG.size() + m_mapPlayerDataMP.size() + m_mapPlayerDataPHX.size() );

	for( SCHOOLWARS_REGISTER_DATA_MAP_ITER iterplayer = m_mapPlayerDataSG.begin(); iterplayer != m_mapPlayerDataSG.end(); ++ iterplayer ){
		const SCHOOLWARS_REGISTER_DATA& splayerdata = (*iterplayer).second;
		vecdata.push_back( splayerdata );
	}

	for( SCHOOLWARS_REGISTER_DATA_MAP_ITER iterplayer = m_mapPlayerDataMP.begin(); iterplayer != m_mapPlayerDataMP.end(); ++ iterplayer ){
		const SCHOOLWARS_REGISTER_DATA& splayerdata = (*iterplayer).second;
		vecdata.push_back( splayerdata );
	}

	for( SCHOOLWARS_REGISTER_DATA_MAP_ITER iterplayer = m_mapPlayerDataPHX.begin(); iterplayer != m_mapPlayerDataPHX.end(); ++ iterplayer ){
		const SCHOOLWARS_REGISTER_DATA& splayerdata = (*iterplayer).second;
		vecdata.push_back( splayerdata );
	}

	WORD wNum = 0;
	GLMSG::SNETPC_SCHOOLWARS_A2F_PLAYER_DATA	net_msg_data;
	for ( size_t i=0; i<vecdata.size(); ++i )
	{
		const SCHOOLWARS_REGISTER_DATA& splayer_data = vecdata[i];
		net_msg_data.ADDRANK( splayer_data );
		if ( net_msg_data.wRankNum == SCHOOLWARS_PACKET_PLAYER_DATA_NUM )
		{
			GLAgentServer::GetInstance().SENDTOCHANNEL( &net_msg_data, 0 );
			wNum += net_msg_data.wRankNum;
			net_msg_data.Reset();
		}
	}

	if ( net_msg_data.wRankNum != 0 )
	{
		GLAgentServer::GetInstance().SENDTOCHANNEL( &net_msg_data, 0 );
		wNum += net_msg_data.wRankNum;
		net_msg_data.Reset();
	}

	CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "GLPVPSchoolWarsAgent Send PlayerData num: %u", wNum );
	GLAgentServer::GetInstance().CONSOLEMSG_WRITE( "GLPVPSchoolWarsAgent Send PlayerData num: %u", wNum );
}
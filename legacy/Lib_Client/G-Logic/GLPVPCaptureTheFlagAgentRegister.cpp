#include "StdAfx.h"
#include "GLPVPCaptureTheFlagAgent.h"
#include "GLAgentServer.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAPTURE_THE_FLAG_REGISTER_DATA* GLPVPCaptureTheFlagAgent::PlayerDataGet( DWORD dwCharID )
{
	if ( dwCharID == CAPTURE_THE_FLAG_PLAYER_NULL ) return NULL;

	{
		CAPTURE_THE_FLAG_REGISTER_DATA_MAP_ITER iterdata = m_mapPlayerDataA.find( dwCharID );
		if( iterdata != m_mapPlayerDataA.end() ){
			return &(*iterdata).second;
		}
	}

	{
		CAPTURE_THE_FLAG_REGISTER_DATA_MAP_ITER iterdata = m_mapPlayerDataB.find( dwCharID );
		if( iterdata != m_mapPlayerDataB.end() ){
			return &(*iterdata).second;
		}
	}

	return NULL;
};

BOOL GLPVPCaptureTheFlagAgent::IsPlayerFull( CAPTURE_THE_FLAG_REGISTER_DATA_MAP& mapData  )
{
	//unlimited team members
	if ( m_wPlayerLimit == 0 )	return FALSE;

	if ( mapData.size() >= m_wPlayerLimit )
		return TRUE;

	return FALSE;
}

BOOL GLPVPCaptureTheFlagAgent::InsertRegister( CAPTURE_THE_FLAG_REGISTER_DATA_MAP& mapData, DWORD dwCharID, const char* szName, WORD wClass, WORD wSchool, WORD wTeam )
{
	if ( !szName )										return FALSE;
	if ( dwCharID == CAPTURE_THE_FLAG_PLAYER_NULL )		return FALSE;
	if ( wClass >= GLCI_NUM_8CLASS )					return FALSE;
	if ( wSchool >= CAPTURE_THE_FLAG_INFO_SCHOOL_SIZE )	return FALSE;
	if ( wTeam >= CAPTURE_THE_FLAG_TEAM_SIZE )			return FALSE;

	CAPTURE_THE_FLAG_REGISTER_DATA sdata;
	sdata.dwInfoCharID = dwCharID;
	sdata.wInfoClass = wClass;
	sdata.wInfoSchool = wSchool;
	sdata.wInfoTeam = wTeam;
	StringCchCopy( sdata.szInfoCharName, CAPTURE_THE_FLAG_CHARNAME_STRING_SIZE, szName );
	mapData.insert( std::make_pair( sdata.dwInfoCharID, sdata ) );

	return TRUE;
}

BOOL GLPVPCaptureTheFlagAgent::DeleteRegister( DWORD dwCharID, CAPTURE_THE_FLAG_REGISTER_DATA_MAP& mapData  )
{
	if ( dwCharID == CAPTURE_THE_FLAG_PLAYER_NULL )	return FALSE;

	CAPTURE_THE_FLAG_REGISTER_DATA_MAP_ITER iterdata = mapData.find( dwCharID );
	if( iterdata != mapData.end() ){
		mapData.erase( iterdata );
		return TRUE;
	}

	return FALSE;
}

void GLPVPCaptureTheFlagAgent::RequestPlayerRegister( DWORD dwCharID, WORD wLevel, LONGLONG llContri, BOOL bRegister, WORD wTeam )
{	
	PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( dwCharID );
	if ( !pchar )	return;

	GLMSG::SNETPC_CAPTURE_THE_FLAG_A2C_REGISTER_FB netmsgfb;
	netmsgfb.wPlayerNum[CAPTURE_THE_FLAG_TEAM_A] = (WORD)m_mapPlayerDataA.size();
	netmsgfb.wPlayerNum[CAPTURE_THE_FLAG_TEAM_B] = (WORD)m_mapPlayerDataB.size();
	netmsgfb.wTeam = wTeam;

	//invalid charid
	if ( pchar->m_dwCharID != dwCharID ){
		netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_INVALID_CHARID;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}

	//registration not running
	if ( m_emState != CAPTURE_THE_FLAG_STATE_REGISTER ){
		netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_NOTPOSSIBLE;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}

	//register only available in channel 0 
	if ( pchar->m_nChannel != 0 ){
		netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_INVALID_CHANNEL;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}

	//player level too low
	if ( wLevel < m_wLevelReq ){
		netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_LEVEL_LOW;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}

	//contri low
	if ( llContri < m_llContributionReq ){
		netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_CONTRI_LOW;
		GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
		return;
	}

	if ( bRegister )
	{
		//already registered
		CAPTURE_THE_FLAG_REGISTER_DATA*	pPlayerData = PlayerDataGet( dwCharID );
		if ( pPlayerData )
		{
			if ( pPlayerData->wInfoTeam != wTeam )
			{
				netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_OTHER_TEAM;
				GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
				return;
			}else{
				netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_ALREADY_REG;
				GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
				return;
			}
		}

		if ( wTeam == CAPTURE_THE_FLAG_TEAM_A )
		{
			if ( m_wBalancer != 0 && m_mapPlayerDataA.size() > m_mapPlayerDataB.size() )
			{
				if ( std::abs( (int)m_mapPlayerDataA.size() - (int)m_mapPlayerDataB.size() ) >= m_wBalancer )
				{
					netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_TEAM_A_BALANCE;
					GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
					return;
				}
			}

			if ( IsPlayerFull( m_mapPlayerDataA ) ){
				netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_TEAM_A_FULL;
				GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
				return;
			}else{
				//insert player data
				BOOL binsert = InsertRegister( m_mapPlayerDataA, pchar->m_dwCharID, pchar->m_szName, CharClassToIndex( pchar->m_emClass ), pchar->m_wSchool, wTeam );
				if ( binsert ){
					netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_REGISTERED_A;
					netmsgfb.wPlayerNum[CAPTURE_THE_FLAG_TEAM_A] = (WORD)m_mapPlayerDataA.size();
					netmsgfb.wPlayerNum[CAPTURE_THE_FLAG_TEAM_B] = (WORD)m_mapPlayerDataB.size();
					GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
					CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "Player [%d]'%s' register team red", 
					pchar->m_dwCharID, pchar->m_szName );
					return;
				}
			}
		}
		else if ( wTeam == CAPTURE_THE_FLAG_TEAM_B )
		{
			if ( m_wBalancer != 0 && m_mapPlayerDataB.size() > m_mapPlayerDataA.size() )
			{
				if ( std::abs( (int)m_mapPlayerDataB.size() - (int)m_mapPlayerDataA.size() ) >= m_wBalancer )
				{
					netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_TEAM_B_BALANCE;
					GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
					return;
				}
			}

			if ( IsPlayerFull( m_mapPlayerDataB ) ){
				netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_TEAM_B_FULL;
				GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
				return;
			}else{
				//insert player data
				BOOL binsert = InsertRegister( m_mapPlayerDataB, pchar->m_dwCharID, pchar->m_szName, CharClassToIndex( pchar->m_emClass ), pchar->m_wSchool, wTeam );
				if ( binsert ){
					netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_REGISTERED_B;
					netmsgfb.wPlayerNum[CAPTURE_THE_FLAG_TEAM_A] = (WORD)m_mapPlayerDataA.size();
					netmsgfb.wPlayerNum[CAPTURE_THE_FLAG_TEAM_B] = (WORD)m_mapPlayerDataB.size();
					GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
					CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "Player [%d]'%s' register team blue", 
					pchar->m_dwCharID, pchar->m_szName );
					return;
				}
			}
		}
		else
		{
			netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_INVALID_TEAM;
			GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
			return;
		}

		CAPTURE_THE_FLAG_REGISTER_DATA* pdataregchk = PlayerDataGet( dwCharID );
		if ( !pdataregchk ){
			CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "RequestPlayerRegister failed to register [%d]'%s'", 
				pchar->m_dwCharID, pchar->m_szName );
		}
	}
	else
	{
		CAPTURE_THE_FLAG_REGISTER_DATA*	pPlayerData = PlayerDataGet( dwCharID );
		if ( pPlayerData )
		{
			if ( pPlayerData->wInfoTeam != wTeam )
			{
				netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_OTHER_TEAM;
				GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
				return;
			}
		}

		if ( wTeam == CAPTURE_THE_FLAG_TEAM_A )
		{
			//if exist in registered, remove
			BOOL bdelete = DeleteRegister( pchar->m_dwCharID, m_mapPlayerDataA );
			if ( bdelete ){
				netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_CANCEL_REG;
				netmsgfb.wPlayerNum[CAPTURE_THE_FLAG_TEAM_A] = (WORD)m_mapPlayerDataA.size();
				netmsgfb.wPlayerNum[CAPTURE_THE_FLAG_TEAM_B] = (WORD)m_mapPlayerDataB.size();
				GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
				return;
			}

			//fail to delete
			if ( !bdelete ){
				netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_CANCEL_FAIL;
				GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
				return;
			}
		}
		else if ( wTeam == CAPTURE_THE_FLAG_TEAM_B )
		{
			//if exist in registered, remove
			BOOL bdelete = DeleteRegister( pchar->m_dwCharID, m_mapPlayerDataB );
			if ( bdelete ){
				netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_CANCEL_REG;
				netmsgfb.wPlayerNum[CAPTURE_THE_FLAG_TEAM_A] = (WORD)m_mapPlayerDataA.size();
				netmsgfb.wPlayerNum[CAPTURE_THE_FLAG_TEAM_B] = (WORD)m_mapPlayerDataB.size();
				GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
				return;
			}

			//fail to delete
			if ( !bdelete ){
				netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_CANCEL_FAIL;
				GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
				return;
			}
		}
		else
		{
			netmsgfb.emFB = CAPTURE_THE_FLAG_REGISTER_FB_INVALID_TEAM;
			GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgfb );
			return;
		}

		CAPTURE_THE_FLAG_REGISTER_DATA* pdataregchk = PlayerDataGet( dwCharID );
		if ( pdataregchk ){
			CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "RequestPlayerRegister failed to unregister [%d]'%s'", 
				pchar->m_dwCharID, pchar->m_szName );
		}
	}
}

void GLPVPCaptureTheFlagAgent::RequestPlayerCancelRegister( DWORD dwCharID )
{	
	if ( dwCharID == CAPTURE_THE_FLAG_PLAYER_NULL )	return;
	if ( m_emState != CAPTURE_THE_FLAG_STATE_REGISTER )	return;

	CAPTURE_THE_FLAG_REGISTER_DATA* pdatareg = PlayerDataGet( dwCharID );
	if ( pdatareg )
	{
		WORD wteam = pdatareg->wInfoTeam;

		if ( wteam == CAPTURE_THE_FLAG_TEAM_A )
			DeleteRegister( dwCharID, m_mapPlayerDataA );
		else if ( wteam == CAPTURE_THE_FLAG_TEAM_B )
			DeleteRegister( dwCharID, m_mapPlayerDataB );
	}

	CAPTURE_THE_FLAG_REGISTER_DATA* pdataregchk = PlayerDataGet( dwCharID );
	if ( pdataregchk ){
		CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "RequestPlayerCancelRegister failed to delete registered [%d]'%s'", 
			pdataregchk->dwInfoCharID, pdataregchk->szInfoCharName );
	}
}

void GLPVPCaptureTheFlagAgent::SendPlayerData()
{
	CAPTURE_THE_FLAG_REGISTER_DATA_VEC vecdata;
	vecdata.reserve( m_mapPlayerDataA.size() + m_mapPlayerDataB.size() );

	for( CAPTURE_THE_FLAG_REGISTER_DATA_MAP_ITER iterplayer = m_mapPlayerDataA.begin(); iterplayer != m_mapPlayerDataA.end(); ++ iterplayer ){
		const CAPTURE_THE_FLAG_REGISTER_DATA& splayerdata = (*iterplayer).second;
		vecdata.push_back( splayerdata );
	}

	for( CAPTURE_THE_FLAG_REGISTER_DATA_MAP_ITER iterplayer = m_mapPlayerDataB.begin(); iterplayer != m_mapPlayerDataB.end(); ++ iterplayer ){
		const CAPTURE_THE_FLAG_REGISTER_DATA& splayerdata = (*iterplayer).second;
		vecdata.push_back( splayerdata );
	}

	WORD wNum = 0;
	GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_PLAYER_DATA	net_msg_data;
	for ( size_t i=0; i<vecdata.size(); ++i )
	{
		const CAPTURE_THE_FLAG_REGISTER_DATA& splayer_data = vecdata[i];
		net_msg_data.ADDRANK( splayer_data );
		if ( net_msg_data.wRankNum == CAPTURE_THE_FLAG_PACKET_PLAYER_DATA_NUM )
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

	CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagAgent Send PlayerData num: %u", wNum );
	GLAgentServer::GetInstance().CONSOLEMSG_WRITE( "GLPVPCaptureTheFlagAgent Send PlayerData num: %u", wNum );
}
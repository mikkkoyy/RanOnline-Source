#include "StdAfx.h"
#include "GLPVPCaptureTheFlagField.h"
#include "GLGaeaServer.h"
#include "GLLandMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GLPVPCaptureTheFlagField& GLPVPCaptureTheFlagField::GetInstance()
{
	static GLPVPCaptureTheFlagField cInstance;
	return cInstance;
}

void GLPVPCaptureTheFlagField::LoadEvent( std::string strFile )
{
	ResetData();

	bool bok = Load( strFile );
	if ( bok )
	{
		FindScheduleFirst();
	}
}

bool GLPVPCaptureTheFlagField::SetMapState ()
{
	if( m_sMap.IsValidNativeID() )
	{
		SMAPNODE *pmapnode = GLGaeaServer::GetInstance().FindMapNode ( m_sMap );
		if ( pmapnode )	
			pmapnode->bPVPCaptureTheFlagZone = true;

		GLLandMan* plandman = GLGaeaServer::GetInstance().GetByMapID ( m_sMap );
		if ( plandman )
		{
			plandman->m_bPVPCaptureTheFlagMap = true;	
			m_bValidInstance = TRUE;
		}
	}

	return true;
}

bool GLPVPCaptureTheFlagField::FrameMove ( float fElaps )
{
	if ( IsBattle() )
	{
		for ( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
		{
			if ( m_bFlagHold[i] && m_fHoldTimer[i] > 0.0f )
			{
				m_fHoldTimer[i] -= fElaps;

				if ( m_fHoldTimer[i] <= 0.0f )
				{
					FlagTimeOut(i);
				}
			}
		}

		m_fFlagLocationUpdate += fElaps;
		if ( m_fFlagLocationUpdate >= CAPTURE_THE_FLAG_FLAG_POS_BRD_TIME )
		{
			FlagLocationUpdate();
			m_fFlagLocationUpdate = 0.0f;
		}
	}

	return true;
}

void GLPVPCaptureTheFlagField::ResetData()
{
	m_emState = CAPTURE_THE_FLAG_STATE_ENDED;
	m_sScheduleNext = CAPTURE_THE_FLAG_SCHED_NEXT();
	m_vecSchedule.clear();

	m_mapPlayerDataA.clear();
	m_mapPlayerDataB.clear();
	m_mapRewarded.clear();

	m_bValidInstance = FALSE;

	for ( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
	{
		m_bFlagHold[i] = FALSE;
		m_fHoldTimer[i] = 0.0f;
		m_sFlagTarget[i].RESET();
	}

	m_fFlagLocationUpdate = 0.0f;
}

void GLPVPCaptureTheFlagField::DoStateRegister()
{
	m_mapPlayerDataA.clear();
	m_mapPlayerDataB.clear();
	m_mapRewarded.clear();

	m_emState = CAPTURE_THE_FLAG_STATE_REGISTER;

	for ( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
	{
		m_bFlagHold[i] = FALSE;
		m_fHoldTimer[i] = 0.0f;
		m_sFlagTarget[i].RESET();
	}

	GLGaeaServer::GetInstance().GetConsoleMsg()->Write( "GLPVPCaptureTheFlagField Change State to Register Start" );
	CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagField Change State to Register Start" );
}

void GLPVPCaptureTheFlagField::DoStateBattle()
{
	FlagCrowSetHide( CAPTURE_THE_FLAG_TEAM_SIZE, false );
	
	m_fFlagLocationUpdate = 0.0f;

	m_emState = CAPTURE_THE_FLAG_STATE_BATTLE;

	GLGaeaServer::GetInstance().GetConsoleMsg()->Write( "GLPVPCaptureTheFlagField Change State to Battle Start" );
	CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagField Change State to Battle Start" );
}

void GLPVPCaptureTheFlagField::DoStateReward( WORD wTeamWinner )
{
	if ( m_bValidInstance )
	{
		//calculate final scores
		ScoreCalculate( m_mapPlayerDataA, wTeamWinner );
		ScoreCalculate( m_mapPlayerDataB, wTeamWinner );

		//sort rankings per team
		CalculateRankingTeam();

		//sort overall rank
		CalculateRankingAll();

		//send rankings to all players on map
		RankingSend();

		//send contribution points
		PointSend();

		//check reward
		GetRewards( wTeamWinner );

		//send rewards
		RewardSend();

		//check if rewards were sent
		CheckRewards();
	}
	
	m_emState = CAPTURE_THE_FLAG_STATE_REWARD;

	GLGaeaServer::GetInstance().GetConsoleMsg()->Write( "GLPVPCaptureTheFlagField Change State to Award Start" );
	CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagField Change State to Award Start" );
}

void GLPVPCaptureTheFlagField::DoStateEnd()
{
	m_mapPlayerDataA.clear();
	m_mapPlayerDataB.clear();
	m_mapRewarded.clear();

	m_emState = CAPTURE_THE_FLAG_STATE_ENDED;

	for ( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
	{
		m_bFlagHold[i] = FALSE;
		m_fHoldTimer[i] = 0.0f;
		m_sFlagTarget[i].RESET();
	}

	m_fFlagLocationUpdate = 0.0f;

	GLGaeaServer::GetInstance().GetConsoleMsg()->Write( "GLPVPCaptureTheFlagField Change State to Event End" );
	CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagField Change State to Event End" );
}

void GLPVPCaptureTheFlagField::RequestInfoPC( DWORD dwCharID, BOOL bEventMap )
{
	PGLCHAR pchar = GLGaeaServer::GetInstance().GetCharID( dwCharID );
	if ( !pchar )	return;

	GLMSG::SNET_CAPTURE_THE_FLAG_F2A_BATTLEINFO_PC netmsg2agent;
	netmsg2agent.dwCharID = dwCharID;
	netmsg2agent.bEventMap = bEventMap;
	GLGaeaServer::GetInstance().SENDTOAGENT( &netmsg2agent );
}

CAPTURE_THE_FLAG_PLAYER_DATA* GLPVPCaptureTheFlagField::PlayerDataGet( DWORD dwCharID )
{
	if ( dwCharID == CAPTURE_THE_FLAG_PLAYER_NULL ) return NULL;

	{
		CAPTURE_THE_FLAG_PLAYER_DATA_MAP_ITER iterdata = m_mapPlayerDataA.find( dwCharID );
		if( iterdata != m_mapPlayerDataA.end() ){
			return &(*iterdata).second;
		}
	}

	{
		CAPTURE_THE_FLAG_PLAYER_DATA_MAP_ITER iterdata = m_mapPlayerDataB.find( dwCharID );
		if( iterdata != m_mapPlayerDataB.end() ){
			return &(*iterdata).second;
		}
	}

	return NULL;
};

void GLPVPCaptureTheFlagField::PlayerRevive( DWORD dwGaeaID, DWORD dwCharID )
{
	PGLCHAR pchar = GLGaeaServer::GetInstance().GetCharID( dwCharID );
	if ( !pchar )	return;

	GLMSG::SNETPC_REQ_REVIVE_FB netmsgclient;

	if ( m_emState != CAPTURE_THE_FLAG_STATE_BATTLE &&
		m_emState != CAPTURE_THE_FLAG_STATE_REWARD )
	{
		netmsgclient.emFB = EMREQ_REVIVE_FB_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT( pchar->m_dwClientID, &netmsgclient );
		return;
	}

	BOOL brevive = pchar->PVPCaptureTheFlagRevive( m_wReviveGate[pchar->m_wCaptureTheFlagTeam] );
	if ( !brevive )
	{
		netmsgclient.emFB = EMREQ_REVIVE_FB_FAIL;
		GLGaeaServer::GetInstance().SENDTOCLIENT( pchar->m_dwClientID, &netmsgclient );
	}
}

void GLPVPCaptureTheFlagField::GetRewards( WORD wTeamWinner )
{
	m_mapRewarded.clear();

	if ( wTeamWinner >= CAPTURE_THE_FLAG_TEAM_SIZE )	return;

	for( CAPTURE_THE_FLAG_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataA.begin(); 
		iterplayer != m_mapPlayerDataA.end(); ++ iterplayer )
	{
		const CAPTURE_THE_FLAG_PLAYER_DATA& splayerdata = (*iterplayer).second;
		if ( splayerdata.wRankAll == 0 )						continue;
		if ( splayerdata.wInfoTeam == wTeamWinner && splayerdata.dwScoreTotal < m_wRewardLimitScoreWin )			continue;
		if ( splayerdata.wInfoTeam != wTeamWinner && splayerdata.dwScoreTotal < m_wRewardLimitScoreLost )			continue;

		CAPTURE_THE_FLAG_REWARD_DATA sdata;
		sdata.dwCharID = splayerdata.dwInfoCharID;
		sdata.sItemReward = (splayerdata.wInfoTeam == wTeamWinner) ? m_sItemRewardWin:m_sItemRewardLost;
		sdata.bSent = false;
		m_mapRewarded.insert( std::make_pair( sdata.dwCharID, sdata ) );
	}

	for( CAPTURE_THE_FLAG_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataB.begin(); 
		iterplayer != m_mapPlayerDataB.end(); ++ iterplayer )
	{
		const CAPTURE_THE_FLAG_PLAYER_DATA& splayerdata = (*iterplayer).second;
		if ( splayerdata.wRankAll == 0 )						continue;
		if ( splayerdata.wInfoTeam == wTeamWinner && splayerdata.dwScoreTotal < m_wRewardLimitScoreWin )			continue;
		if ( splayerdata.wInfoTeam != wTeamWinner && splayerdata.dwScoreTotal < m_wRewardLimitScoreLost )			continue;

		CAPTURE_THE_FLAG_REWARD_DATA sdata;
		sdata.dwCharID = splayerdata.dwInfoCharID;
		sdata.sItemReward = (splayerdata.wInfoTeam == wTeamWinner) ? m_sItemRewardWin:m_sItemRewardLost;
		sdata.bSent = false;
		m_mapRewarded.insert( std::make_pair( sdata.dwCharID, sdata ) );
	}
}

void GLPVPCaptureTheFlagField::RewardSend()
{
	if ( m_mapRewarded.empty() )				return;

	for( CAPTURE_THE_FLAG_REWARD_MAP_ITER iterreward = m_mapRewarded.begin(); iterreward != m_mapRewarded.end(); ++ iterreward )
	{
		CAPTURE_THE_FLAG_REWARD_DATA &sReward = (*iterreward).second; 
		PGLCHAR pChar = GLGaeaServer::GetInstance().GetCharID( sReward.dwCharID );
		if ( !pChar )	continue;

		SITEM* pitem_reward = GLItemMan::GetInstance().GetItem( sReward.sItemReward );
		if ( !pitem_reward )	continue;

		SITEMCUSTOM sITEM_NEW;
		sITEM_NEW.sNativeID = sReward.sItemReward;
		sITEM_NEW.tBORNTIME = CTime::GetCurrentTime().GetTime();
		sITEM_NEW.cGenType = EMGEN_SYSTEM;
		sITEM_NEW.cChnID = (BYTE)GLGaeaServer::GetInstance().GetServerChannel();
		sITEM_NEW.cFieldID = (BYTE)GLGaeaServer::GetInstance().GetFieldSvrID();
		sITEM_NEW.lnGenNum = GLITEMLMT::GetInstance().RegItemGen ( sITEM_NEW.sNativeID, (EMITEMGEN)sITEM_NEW.cGenType );

		/*item color, Juver, 2018/01/08 */
		SITEM* pitem_data_check = GLItemMan::GetInstance().GetItem( sITEM_NEW.sNativeID );
		if ( pitem_data_check && pitem_data_check->sBasicOp.bItemColor )
		{
			sITEM_NEW.wColor1 = pitem_data_check->sBasicOp.wItemColor1;
			sITEM_NEW.wColor2 = pitem_data_check->sBasicOp.wItemColor2;
		}

		CItemDrop cDropItem;
		cDropItem.sItemCustom = sITEM_NEW;
		if ( pChar->IsInsertToInvenEx ( &cDropItem ) )
		{
			pChar->InsertToInvenEx ( &cDropItem );
			GLITEMLMT::GetInstance().ReqItemRoute ( sITEM_NEW, ID_CLUB, pChar->m_dwGuild, ID_CHAR, pChar->m_dwCharID, EMITEM_ROUTE_SYSTEM, sITEM_NEW.wTurnNum );
			sReward.bSent = TRUE;
		}
	}
}

void GLPVPCaptureTheFlagField::PointSend()
{
	CAPTURE_THE_FLAG_PLAYER_DATA_VEC vecdata;
	vecdata.reserve( m_mapPlayerDataA.size() + m_mapPlayerDataB.size() );

	for( CAPTURE_THE_FLAG_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataA.begin(); iterplayer != m_mapPlayerDataA.end(); ++ iterplayer ){
		const CAPTURE_THE_FLAG_PLAYER_DATA& splayerdata = (*iterplayer).second;
		if ( splayerdata.wRewardPoint == 0 )	continue;
		vecdata.push_back( splayerdata );
	}

	for( CAPTURE_THE_FLAG_PLAYER_DATA_MAP_ITER iterplayer = m_mapPlayerDataB.begin(); iterplayer != m_mapPlayerDataB.end(); ++ iterplayer ){
		const CAPTURE_THE_FLAG_PLAYER_DATA& splayerdata = (*iterplayer).second;
		if ( splayerdata.wRewardPoint == 0 )	continue;
		vecdata.push_back( splayerdata );
	}

	for ( size_t i=0; i<vecdata.size(); ++i )
	{
		const CAPTURE_THE_FLAG_PLAYER_DATA& splayer_data = vecdata[i];

		PGLCHAR pchar = GLGaeaServer::GetInstance().GetCharID( splayer_data.dwInfoCharID );
		if ( !pchar ) continue;

		pchar->m_llContributionPoint += splayer_data.wRewardPoint;

		GLMSG::SNETPC_UPDATE_CONTRIBUTION_POINT netmsgclient;
		netmsgclient.bNotice = true;
		netmsgclient.llPoint = pchar->m_llContributionPoint;
		GLGaeaServer::GetInstance().SENDTOCLIENT( pchar->m_dwClientID, &netmsgclient );
	}
}

void GLPVPCaptureTheFlagField::CheckRewards()
{
	for( CAPTURE_THE_FLAG_REWARD_MAP_ITER iterreward = m_mapRewarded.begin(); iterreward != m_mapRewarded.end(); ++ iterreward )
	{
		CAPTURE_THE_FLAG_REWARD_DATA &sReward = (*iterreward).second; 

		if( !sReward.bSent )
		{
			CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagField Failed to insert reward to :%d item[%d~%d]", sReward.dwCharID, sReward.sItemReward.wMainID, sReward.sItemReward.wSubID );
		}
	}
}

#include "StdAfx.h"
#include "GLPVPCaptureTheFlagAgent.h"
#include "GLAgentServer.h"
#include "GLAGLandMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GLPVPCaptureTheFlagAgent& GLPVPCaptureTheFlagAgent::GetInstance()
{
	static GLPVPCaptureTheFlagAgent cInstance;
	return cInstance;
}

void GLPVPCaptureTheFlagAgent::LoadEvent( std::string strFile )
{
	ResetData();

	bool bok = Load( strFile );
	if ( bok )
	{
		FindScheduleFirst();
	}
}

bool GLPVPCaptureTheFlagAgent::SetMapState ()
{
	if( m_sMap.IsValidNativeID() )
	{
		GLAGLandMan *plandman = GLAgentServer::GetInstance().GetByMapID ( m_sMap );
		if ( plandman )	plandman->m_bPVPCaptureTheFlagMap = true;
	}
	
	return true;
}

bool GLPVPCaptureTheFlagAgent::FrameMove ( float fElaps )
{
	if ( GLCONST_CHAR::bPKLESS )	return false;
	if ( m_vecSchedule.empty() )	return false;

	if ( IsEnded() )
	{
		if ( m_sScheduleNext.dwIndex != CAPTURE_THE_FLAG_SCHEDULE_NONE )
		{
			CTime ctimecur = CTime::GetCurrentTime();
			if ( m_sScheduleNext.sStartSched.wStartDay == (WORD)ctimecur.GetDayOfWeek() &&
				m_sScheduleNext.sStartSched.wStartHour == (WORD)ctimecur.GetHour() &&
				m_sScheduleNext.sStartSched.wStartMinute == (WORD)ctimecur.GetMinute() )
			{
				SetEventState( CAPTURE_THE_FLAG_STATE_REGISTER );
			}
		}
	}
	

	if ( IsRegister() )
	{
		m_fTimer += fElaps;
		m_fRemain -= fElaps;

		NotifyTimeToStart( m_fRemain );	

		m_fBrdTimer += fElaps;
		if ( m_fBrdTimer >= CAPTURE_THE_FLAG_REG_NUM_BRD_TIME )
		{
			m_fBrdTimer = 0.0f;
			BroadCastPlayerNum();
		}

		if ( m_fTimer >= (float)m_wRegisterTime ){
			SetEventState( CAPTURE_THE_FLAG_STATE_BATTLE );
		}
	}

	if ( IsBattle() )
	{
		m_fTimer += fElaps;
		m_fRemain -= fElaps;

		if ( m_fTimer >= (float)m_wBattleTime ){
			SetEventState( CAPTURE_THE_FLAG_STATE_REWARD );
		}
	}

	if ( IsReward() )
	{
		m_fTimer += fElaps;
		m_fRemain -= fElaps;

		if ( m_fTimer >= (float)m_wRewardTime ){
			SetEventState( CAPTURE_THE_FLAG_STATE_ENDED );
		}
	}

	if ( m_bMoveIn && IsBattle() )
	{
		m_fMoveTimer += fElaps;
		m_fMoveTimerTotal += fElaps;

		if( m_fMoveTimer >= 0.2f )
		{
			MapMoveInPlayer();
			m_fMoveTimer = 0.0f;
		}

		if ( m_fMoveTimerTotal >= 300.0f )
		{
			m_bMoveIn = false;
			m_fMoveTimer = 0.0f;
			m_fMoveTimerTotal = 0.0f;
		}

		if ( m_vecPlayerMove.empty() )
		{
			m_bMoveIn = false;
			m_fMoveTimer = 0.0f;
			m_fMoveTimerTotal = 0.0f;
		}

	}

	if ( m_bMoveOut && IsEnded() )
	{
		m_fMoveTimer += fElaps;
		m_fMoveTimerTotal += fElaps;

		if( m_fMoveTimer >= 0.2f )
		{
			MapMoveOutPlayer();
			m_fMoveTimer = 0.0f;
		}

		if ( m_fMoveTimerTotal >= 300.0f )
		{
			m_bMoveOut = false;
			m_fMoveTimer = 0.0f;
			m_fMoveTimerTotal = 0.0f;
		}

		if ( m_vecPlayerMove.empty() )
		{
			m_bMoveOut = false;
			m_fMoveTimer = 0.0f;
			m_fMoveTimerTotal = 0.0f;
		}
	}
	
	return true;
}

void GLPVPCaptureTheFlagAgent::SetEventState( CAPTURE_THE_FLAG_STATE emState )
{
	std::string streventstate[CAPTURE_THE_FLAG_STATE_SIZE] = 
	{
		"Event End",
		"Register Start",
		"Battle Start",
		"Award Start"
	};

	GLAgentServer::GetInstance().CONSOLEMSG_WRITE( "GLPVPCaptureTheFlagAgent Change State to '%s'", streventstate[emState].c_str() );
	CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagAgent Change State to '%s'", streventstate[emState].c_str() );

	switch( emState )
	{
	case CAPTURE_THE_FLAG_STATE_ENDED:
		{
			DoStateEnd();
		}break;

	case CAPTURE_THE_FLAG_STATE_REGISTER:
		{
			DoStateRegister();
		}break;

	case CAPTURE_THE_FLAG_STATE_BATTLE:
		{
			DoStateBattle();
		}break;

	case CAPTURE_THE_FLAG_STATE_REWARD:
		{
			DoStateReward();
		}break;
	};

	m_emState = emState;
}

void GLPVPCaptureTheFlagAgent::ResetData()
{
	m_emState = CAPTURE_THE_FLAG_STATE_ENDED;
	m_sScheduleNext = CAPTURE_THE_FLAG_SCHED_NEXT();
	m_vecSchedule.clear();
	
	for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
	{
		m_sScore[i].Reset();
		m_sScore[i].wTeam = i;

		m_bFlagHold[i] = FALSE;
	}

	m_wLastWinner = CAPTURE_THE_FLAG_TEAM_SIZE;

	for( int i=0; i<CAPTURE_THE_FLAG_MINI_RANKING_NUM; ++i )
		m_sLastTop[i] = CAPTURE_THE_FLAG_PLAYER_DATA();

	m_vecPlayerMove.clear();
	m_mapPlayerDataA.clear();
	m_mapPlayerDataB.clear();

	m_bMoveIn = false;
	m_bMoveOut = false;

	m_fMoveTimer = 0.0f;
	m_fMoveTimerTotal = 0.0f;
}

void GLPVPCaptureTheFlagAgent::DoStateRegister()
{
	m_wLastNotifyTime = 0;
	m_fTimer	= 0.0f;
	m_fRemain	= (float)m_wRegisterTime;
	m_fBrdTimer = 0.0f;

	m_vecPlayerMove.clear();
	m_mapPlayerDataA.clear();
	m_mapPlayerDataB.clear();

	for ( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
		m_bFlagHold[i] = FALSE;
	
	GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_REGISTER netmsgstate;
	netmsgstate.fTime = m_fRemain;
	GLAgentServer::GetInstance().SENDTOCHANNEL ( &netmsgstate, 0 );
	GLAgentServer::GetInstance().SENDTOALLCLIENT ( &netmsgstate );
}

void GLPVPCaptureTheFlagAgent::DoStateBattle()
{
	m_fTimer	= 0.0f;
	m_fRemain	= (float)m_wBattleTime;
	m_fBrdTimer = 0.0f;
	
	for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
	{
		m_sScore[i].Reset();
		m_sScore[i].wTeam = i;
	}

	//reset the last winner
	m_wLastWinner = CAPTURE_THE_FLAG_TEAM_SIZE;

	for( int i=0; i<CAPTURE_THE_FLAG_MINI_RANKING_NUM; ++i )
		m_sLastTop[i] = CAPTURE_THE_FLAG_PLAYER_DATA();

	SendPlayerData();
	QueueMoveIn();
	ClearPlayerParty();

	GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_BATTLE netmsgstate;
	netmsgstate.fTime = m_fRemain;
	netmsgstate.wPlayerNum[CAPTURE_THE_FLAG_TEAM_A] = (WORD)m_mapPlayerDataA.size();
	netmsgstate.wPlayerNum[CAPTURE_THE_FLAG_TEAM_B] = (WORD)m_mapPlayerDataB.size();
	GLAgentServer::GetInstance().SENDTOCHANNEL ( &netmsgstate, 0 );
	GLAgentServer::GetInstance().SENDTOALLCLIENT ( &netmsgstate );
}

void GLPVPCaptureTheFlagAgent::DoStateReward()
{
	m_fTimer	= 0.0f;
	m_fRemain	= (float)m_wRewardTime;
	m_fBrdTimer = 0.0f;
	
	CheckWinner();

	GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_REWARD netmsgstate;
	netmsgstate.fTime = m_fRemain;
	netmsgstate.wWinnerTeam = m_wLastWinner;
	GLAgentServer::GetInstance().SENDTOCHANNEL ( &netmsgstate, 0 );
	GLAgentServer::GetInstance().SENDTOALLCLIENT ( &netmsgstate );
}

void GLPVPCaptureTheFlagAgent::DoStateEnd()
{
	GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_ENDED netmsgstate;
	GLAgentServer::GetInstance().SENDTOCHANNEL ( &netmsgstate, 0 );
	GLAgentServer::GetInstance().SENDTOALLCLIENT ( &netmsgstate );

	if ( FindScheduleNext() )
	{
		GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_NEXTSCHED netmsgschedule;
		netmsgschedule.sScheduleNext = m_sScheduleNext;
		GLAgentServer::GetInstance().SENDTOCHANNEL( &netmsgschedule, 0 );
		GLAgentServer::GetInstance().SENDTOALLCLIENT( &netmsgschedule );
	}

	QueueMoveOut();
	ClearPlayerParty();

	m_mapPlayerDataA.clear();
	m_mapPlayerDataB.clear();

	for ( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
		m_bFlagHold[i] = FALSE;
}

void GLPVPCaptureTheFlagAgent::NotifyTimeToStart( float fTime )
{
	WORD wremain = (WORD)m_fRemain;
	if( m_wLastNotifyTime == wremain )	return;

	bool bnotify = false;
	if ( wremain > 300 && ( wremain%300 == 0 ) )	
		bnotify = true;
	else if ( wremain > 0 && wremain <= 300 && ( wremain%60 == 0 ) )
		bnotify = true;

	if ( !bnotify )	return;

	m_wLastNotifyTime = wremain;

	GLMSG::SNET_CAPTURE_THE_FLAG_A2C_TOBATTLE_TIME netmsgclient;
	netmsgclient.wTime = wremain / 60;
	GLAgentServer::GetInstance().SENDTOALLCLIENT ( &netmsgclient );
}

void GLPVPCaptureTheFlagAgent::RequestInfoPC( DWORD dwCharID, BOOL bEventMap )
{
	PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( dwCharID );
	if ( !pchar )	return;

	//event info
	GLMSG::SNET_CAPTURE_THE_FLAG_A2C_BATTLEINFO_PC netmsgclient;
	netmsgclient.emState = m_emState;
	netmsgclient.fRemain = m_fRemain;
	netmsgclient.sScheduleNext = m_sScheduleNext;
	netmsgclient.wLevelReq = m_wLevelReq;
	netmsgclient.llContributionReq = m_llContributionReq;
	netmsgclient.wPlayerLimit = m_wPlayerLimit;
	netmsgclient.wMaxScore = m_wMaxScore;
	netmsgclient.wBattleTime = m_wBattleTime;
	netmsgclient.wPlayerNum[CAPTURE_THE_FLAG_TEAM_A] = (WORD)m_mapPlayerDataA.size();
	netmsgclient.wPlayerNum[CAPTURE_THE_FLAG_TEAM_B] = (WORD)m_mapPlayerDataB.size();

	//register info
	CAPTURE_THE_FLAG_REGISTER_DATA* pplayerdata = PlayerDataGet( dwCharID );
	if ( pplayerdata ){
		netmsgclient.bRegistered = true;
		netmsgclient.wTeam = pplayerdata->wInfoTeam;
	}

	GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgclient );

	//char team info
	if( bEventMap && IsBattle() && pplayerdata )
		pchar->m_wCaptureTheFlagTeam = pplayerdata->wInfoTeam;
	else
		pchar->m_wCaptureTheFlagTeam = CAPTURE_THE_FLAG_TEAM_SIZE;

	GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_PLAYER_TEAM netmsgteam;
	netmsgteam.dwCharID = pchar->m_dwCharID;
	netmsgteam.wTeam = pchar->m_wCaptureTheFlagTeam;
	GLAgentServer::GetInstance().SENDTOFIELDSVR ( pchar->m_nChannel, pchar->m_dwCurFieldSvr, &netmsgteam );

	//last rankinfo
	GLMSG::SNETPC_CAPTURE_THE_FLAG_A2C_RANKINFO_PC netmsglastrank;
	netmsglastrank.wLastWinner = m_wLastWinner;
	for( int i=0; i<CAPTURE_THE_FLAG_MINI_RANKING_NUM; ++i )
		netmsglastrank.sPlayerData[i] = m_sLastTop[i];

	GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsglastrank );
}

void GLPVPCaptureTheFlagAgent::BroadCastPlayerNum()
{
	GLMSG::SNET_CAPTURE_THE_FLAG_A2C_PLAYER_NUM netmsgclient;
	netmsgclient.wPlayerNum[CAPTURE_THE_FLAG_TEAM_A] = (WORD)m_mapPlayerDataA.size();
	netmsgclient.wPlayerNum[CAPTURE_THE_FLAG_TEAM_B] = (WORD)m_mapPlayerDataB.size();
	GLAgentServer::GetInstance().SENDTOALLCLIENT ( &netmsgclient );
}

void GLPVPCaptureTheFlagAgent::ClearPlayerParty()
{
	for( CAPTURE_THE_FLAG_REGISTER_DATA_MAP_ITER iterplayer = m_mapPlayerDataA.begin(); iterplayer != m_mapPlayerDataA.end(); ++ iterplayer ){
		const CAPTURE_THE_FLAG_REGISTER_DATA& splayerdata = (*iterplayer).second;
		GLAgentServer::GetInstance().CaptureTheFlagRemoveParty( splayerdata.dwInfoCharID );
	}

	for( CAPTURE_THE_FLAG_REGISTER_DATA_MAP_ITER iterplayer = m_mapPlayerDataB.begin(); iterplayer != m_mapPlayerDataB.end(); ++ iterplayer ){
		const CAPTURE_THE_FLAG_REGISTER_DATA& splayerdata = (*iterplayer).second;

		GLAgentServer::GetInstance().CaptureTheFlagRemoveParty( splayerdata.dwInfoCharID );
	}
}

void GLPVPCaptureTheFlagAgent::RequestScoreInfo( DWORD dwCharID )
{
	PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( dwCharID );
	if ( !pchar )	return;

	GLMSG::SNETPC_CAPTURE_THE_FLAG_A2C_SCORE_INFO netmsgclient;
	netmsgclient.wLastWinner = m_wLastWinner;

	for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
	{
		netmsgclient.sScore[i] = m_sScore[i];
		netmsgclient.bFlagHold[i] = m_bFlagHold[i];
	}

	GLAgentServer::GetInstance().SENDTOCLIENT( pchar->m_dwClientID, &netmsgclient );
}

void GLPVPCaptureTheFlagAgent::CheckWinner()
{
	//already have winner from capturing all the towers
	if( m_wLastWinner != CAPTURE_THE_FLAG_TEAM_SIZE )
	{
		GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_WINNER netmsgwinner;
		netmsgwinner.wLastWinner = m_wLastWinner;
		netmsgwinner.bFullCapture = TRUE;
		GLAgentServer::GetInstance().SENDTOCHANNEL( &netmsgwinner, 0 );
		return;
	}

	CAPTURE_THE_FLAG_SCORE_DATA_VEC vecscore;
	for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
	{
		if( m_sScore[i].wCapture == 0 && m_sScore[i].wKills == 0 && m_sScore[i].wDeaths == 0 )
			continue;

		vecscore.push_back( m_sScore[i] );
	}

	if ( vecscore.empty() )
	{
		//no winner
		GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_WINNER netmsgwinner;
		netmsgwinner.wLastWinner = CAPTURE_THE_FLAG_TEAM_SIZE;
		netmsgwinner.bFullCapture = FALSE;
		GLAgentServer::GetInstance().SENDTOCHANNEL( &netmsgwinner, 0 );
		return;
	}

	std::sort( vecscore.begin(), vecscore.end() );

	m_wLastWinner = vecscore[0].wTeam;

	GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_WINNER netmsgwinner;
	netmsgwinner.wLastWinner = m_wLastWinner;
	netmsgwinner.bFullCapture = FALSE;
	GLAgentServer::GetInstance().SENDTOCHANNEL( &netmsgwinner, 0 );
}

void GLPVPCaptureTheFlagAgent::ScoreCheck()
{
	if ( m_emState != CAPTURE_THE_FLAG_STATE_BATTLE )		return;
	if ( m_wLastWinner != CAPTURE_THE_FLAG_TEAM_SIZE )		return;

	for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
	{
		if ( m_sScore[i].wCapture >= m_wMaxScore )
		{
			m_wLastWinner = i;

			//full capture, end
			SetEventState( CAPTURE_THE_FLAG_STATE_REWARD );
		}
	}
}
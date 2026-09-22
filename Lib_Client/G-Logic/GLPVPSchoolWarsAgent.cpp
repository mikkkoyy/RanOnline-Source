#include "StdAfx.h"
#include "GLPVPSchoolWarsAgent.h"
#include "GLAgentServer.h"
#include "GLAGLandMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GLPVPSchoolWarsAgent& GLPVPSchoolWarsAgent::GetInstance()
{
	static GLPVPSchoolWarsAgent cInstance;
	return cInstance;
}

void GLPVPSchoolWarsAgent::LoadEvent( std::string strFile )
{
	ResetData();

	bool bok = Load( strFile );
	if ( bok )
	{
		FindScheduleFirst();
	}
}

bool GLPVPSchoolWarsAgent::SetMapState ()
{
	if( m_sMap.IsValidNativeID() )
	{
		GLAGLandMan *plandman = GLAgentServer::GetInstance().GetByMapID ( m_sMap );
		if ( plandman )
			plandman->m_bPVPSchoolWarsMap = true;
	}
	
	return true;
}

bool GLPVPSchoolWarsAgent::FrameMove ( float fElaps )
{
	if ( GLCONST_CHAR::bPKLESS )	return false;
	if ( m_vecSchedule.empty() )	return false;

	if ( IsEnded() )
	{
		if ( m_sScheduleNext.dwIndex != SCHOOLWARS_SCHEDULE_NONE )
		{
			CTime ctimecur = CTime::GetCurrentTime();
			if ( m_sScheduleNext.sStartSched.wStartDay == (WORD)ctimecur.GetDayOfWeek() &&
				m_sScheduleNext.sStartSched.wStartHour == (WORD)ctimecur.GetHour() &&
				m_sScheduleNext.sStartSched.wStartMinute == (WORD)ctimecur.GetMinute() )
			{
				SetEventState( SCHOOLWARS_STATE_REGISTER );
			}
		}
	}
	

	if ( IsRegister() )
	{
		m_fTimer += fElaps;
		m_fRemain -= fElaps;

		NotifyTimeToStart( m_fRemain );	

		if ( m_fTimer >= (float)m_wRegisterTime ){
			SetEventState( SCHOOLWARS_STATE_BATTLE );
		}
	}

	if ( IsBattle() )
	{
		m_fTimer += fElaps;
		m_fRemain -= fElaps;

		if ( m_fTimer >= (float)m_wBattleTime ){
			SetEventState( SCHOOLWARS_STATE_REWARD );
		}

		m_fScoreUpdate += fElaps;
		if ( m_fScoreUpdate >= SCHOOLWARS_SCORE_UPDATE_TIME )
		{
			ScoreUpdate();
			m_fScoreUpdate = 0.0f;
		}

		if ( m_bDoublePoint )
		{
			m_fDoublePointInterval -= fElaps;
			if ( m_fDoublePointInterval <= 0.0f ){
				m_fDoublePoint = 0.0f;
				m_bDoublePoint = false;
			}
		}
		else
		{
			m_fDoublePoint += fElaps;
			if ( m_fDoublePoint >= SCHOOLWARS_DOUBLE_POINT_TIME_INTERVAL ){
				m_bDoublePoint = true;
				m_fDoublePointInterval = SCHOOLWARS_DOUBLE_POINT_TIME_DURATION;
			}
				
		}
	}

	if ( IsReward() )
	{
		m_fTimer += fElaps;
		m_fRemain -= fElaps;

		if ( m_fTimer >= (float)m_wRewardTime ){
			SetEventState( SCHOOLWARS_STATE_ENDED );
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

void GLPVPSchoolWarsAgent::SetEventState( SCHOOLWARS_STATE emState )
{
	std::string streventstate[SCHOOLWARS_STATE_SIZE] = 
	{
		"Event End",
		"Register Start",
		"Battle Start",
		"Award Start"
	};

	GLAgentServer::GetInstance().CONSOLEMSG_WRITE( "GLPVPSchoolWarsAgent Change State to '%s'", streventstate[emState].c_str() );
	CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "GLPVPSchoolWarsAgent Change State to '%s'", streventstate[emState].c_str() );

	switch( emState )
	{
	case SCHOOLWARS_STATE_ENDED:
		{
			DoStateEnd();
		}break;

	case SCHOOLWARS_STATE_REGISTER:
		{
			DoStateRegister();
		}break;

	case SCHOOLWARS_STATE_BATTLE:
		{
			DoStateBattle();
		}break;

	case SCHOOLWARS_STATE_REWARD:
		{
			DoStateReward();
		}break;
	};

	m_emState = emState;
}

void GLPVPSchoolWarsAgent::ResetData()
{
	m_emState = SCHOOLWARS_STATE_ENDED;
	m_sScheduleNext = SCHOOLWARS_SCHED_NEXT();
	m_vecSchedule.clear();
	
	for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++i )
	{
		m_sScore[i].Reset();
		m_sScore[i].wSchool = i;
	}

	m_wLastWinner = SCHOOLWARS_SCHOOL_SIZE;

	for( int i=0; i<SCHOOLWARS_MINI_RANKING_NUM; ++i )
		m_sLastTop[i] = SCHOOLWARS_PLAYER_DATA();

	m_vecPlayerMove.clear();
	m_mapPlayerDataSG.clear();
	m_mapPlayerDataMP.clear();
	m_mapPlayerDataPHX.clear();
	m_vecPlayerDataQueueSG.clear();
	m_vecPlayerDataQueueMP.clear();
	m_vecPlayerDataQueuePHX.clear();

	m_bMoveIn = false;
	m_bMoveOut = false;

	m_fMoveTimer = 0.0f;
	m_fMoveTimerTotal = 0.0f;

	m_fScoreUpdate = 0.0f;
	m_fDoublePoint = 0.0f;
	m_fDoublePointInterval = 0.0f;
	m_bDoublePoint = false;
}

void GLPVPSchoolWarsAgent::DoStateRegister()
{
	m_wLastNotifyTime = 0;
	m_fTimer	= 0.0f;
	m_fRemain	= (float)m_wRegisterTime;

	m_vecPlayerMove.clear();
	m_mapPlayerDataSG.clear();
	m_mapPlayerDataMP.clear();
	m_mapPlayerDataPHX.clear();
	m_vecPlayerDataQueueSG.clear();
	m_vecPlayerDataQueueMP.clear();
	m_vecPlayerDataQueuePHX.clear();

	GLMSG::SNET_SCHOOLWARS_A2FC_STATE_REGISTER netmsgstate;
	netmsgstate.fTime = m_fRemain;
	GLAgentServer::GetInstance().SENDTOCHANNEL ( &netmsgstate, 0 );
	GLAgentServer::GetInstance().SENDTOALLCLIENT ( &netmsgstate );
}

void GLPVPSchoolWarsAgent::DoStateBattle()
{
	m_fTimer	= 0.0f;
	m_fRemain	= (float)m_wBattleTime;

	for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++i )
	{
		m_sScore[i].Reset();
		m_sScore[i].wSchool = i;
	}

	//reset the last winner
	m_wLastWinner = SCHOOLWARS_SCHOOL_SIZE;

	for( int i=0; i<SCHOOLWARS_MINI_RANKING_NUM; ++i )
		m_sLastTop[i] = SCHOOLWARS_PLAYER_DATA();

	m_fScoreUpdate = 0.0f;
	m_fDoublePoint = 0.0f;
	m_fDoublePointInterval = 0.0f;
	m_bDoublePoint = false;

	ProcessQueueDataAll( m_mapPlayerDataSG, m_vecPlayerDataQueueSG );
	ProcessQueueDataAll( m_mapPlayerDataMP, m_vecPlayerDataQueueMP );
	ProcessQueueDataAll( m_mapPlayerDataPHX, m_vecPlayerDataQueuePHX );

	SendPlayerData();
	QueueMoveIn();

	GLMSG::SNET_SCHOOLWARS_A2FC_STATE_BATTLE netmsgstate;
	netmsgstate.fTime = m_fRemain;
	netmsgstate.wPlayerNum[SCHOOLWARS_SCHOOL_SG] = (WORD)m_mapPlayerDataSG.size();
	netmsgstate.wPlayerNum[SCHOOLWARS_SCHOOL_MP] = (WORD)m_mapPlayerDataMP.size();
	netmsgstate.wPlayerNum[SCHOOLWARS_SCHOOL_PHX] = (WORD)m_mapPlayerDataPHX.size();
	GLAgentServer::GetInstance().SENDTOCHANNEL ( &netmsgstate, 0 );
	GLAgentServer::GetInstance().SENDTOALLCLIENT ( &netmsgstate );
}

void GLPVPSchoolWarsAgent::DoStateReward()
{
	m_fTimer	= 0.0f;
	m_fRemain	= (float)m_wRewardTime;

	//get the winner
	ScoreUpdate();
	CheckWinner();

	GLMSG::SNET_SCHOOLWARS_A2FC_STATE_REWARD netmsgstate;
	netmsgstate.fTime = m_fRemain;
	netmsgstate.wWinnerSchool = m_wLastWinner;
	GLAgentServer::GetInstance().SENDTOCHANNEL ( &netmsgstate, 0 );
	GLAgentServer::GetInstance().SENDTOALLCLIENT ( &netmsgstate );
}

void GLPVPSchoolWarsAgent::DoStateEnd()
{
	GLMSG::SNET_SCHOOLWARS_A2FC_STATE_ENDED netmsgstate;
	GLAgentServer::GetInstance().SENDTOCHANNEL ( &netmsgstate, 0 );
	GLAgentServer::GetInstance().SENDTOALLCLIENT ( &netmsgstate );

	if ( FindScheduleNext() )
	{
		GLMSG::SNET_SCHOOLWARS_A2FC_NEXTSCHED netmsgschedule;
		netmsgschedule.sScheduleNext = m_sScheduleNext;
		GLAgentServer::GetInstance().SENDTOCHANNEL( &netmsgschedule, 0 );
		GLAgentServer::GetInstance().SENDTOALLCLIENT( &netmsgschedule );
	}

	QueueMoveOut();

	m_mapPlayerDataSG.clear();
	m_mapPlayerDataMP.clear();
	m_mapPlayerDataPHX.clear();
	m_vecPlayerDataQueueSG.clear();
	m_vecPlayerDataQueueMP.clear();
	m_vecPlayerDataQueuePHX.clear();
}

void GLPVPSchoolWarsAgent::NotifyTimeToStart( float fTime )
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

	GLMSG::SNET_SCHOOLWARS_A2C_TOBATTLE_TIME netmsgclient;
	netmsgclient.wTime = wremain / 60;
	GLAgentServer::GetInstance().SENDTOALLCLIENT ( &netmsgclient );
}

void GLPVPSchoolWarsAgent::RequestInfoPC( DWORD dwCharID )
{
	PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( dwCharID );
	if ( !pchar )	return;

	//event info
	GLMSG::SNET_SCHOOLWARS_A2C_BATTLEINFO_PC netmsgclient;
	netmsgclient.emState = m_emState;
	netmsgclient.fRemain = m_fRemain;
	netmsgclient.sScheduleNext = m_sScheduleNext;
	netmsgclient.wLevelReq = m_wLevelReq;
	netmsgclient.llContriReq = m_llContriReq;
	netmsgclient.wPlayerLimit = m_wPlayerLimit;
	netmsgclient.wMaxScore = m_wMaxScore;
	netmsgclient.wBattleTime = m_wBattleTime;

	//register info
	SCHOOLWARS_REGISTER_DATA* pplayerdata = PlayerDataGet( dwCharID );
	if ( pplayerdata ){
		netmsgclient.bRegistered = true;
	}

	SCHOOLWARS_REGISTER_DATA_QUEUE* pplayerqueue = PlayerQueueGet( dwCharID );
	if ( pplayerqueue ){
		netmsgclient.bRegistered = true;
		netmsgclient.bQueued = true;
		netmsgclient.wQueueNum = pplayerqueue->wQueueNum;
	}

	GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgclient );

	//last rankinfo
	GLMSG::SNETPC_SCHOOLWARS_A2C_RANKINFO_PC netmsglastrank;
	netmsglastrank.wLastWinner = m_wLastWinner;
	for( int i=0; i<SCHOOLWARS_MINI_RANKING_NUM; ++i )
		netmsglastrank.sPlayerData[i] = m_sLastTop[i];

	GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsglastrank );
}

void GLPVPSchoolWarsAgent::RequestScoreInfo( DWORD dwCharID )
{
	PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( dwCharID );
	if ( !pchar )	return;


	GLMSG::SNETPC_SCHOOLWARS_A2C_SCORE_INFO netmsgclient;
	netmsgclient.wLastWinner = m_wLastWinner;
	for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++i )
		netmsgclient.sScore[i] = m_sScore[i];

	netmsgclient.fDoublePoint = m_fDoublePoint;
	netmsgclient.bDoublePoint = m_bDoublePoint;

	GLAgentServer::GetInstance().SENDTOCLIENT( pchar->m_dwClientID, &netmsgclient );
}

void GLPVPSchoolWarsAgent::CheckWinner()
{
	//already have winner from capturing all the towers
	if( m_wLastWinner != SCHOOLWARS_SCHOOL_SIZE )
	{
		GLMSG::SNETPC_SCHOOLWARS_A2F_SCORE_WINNER netmsgwinner;
		netmsgwinner.wLastWinner = m_wLastWinner;
		netmsgwinner.bFullScore = TRUE;
		GLAgentServer::GetInstance().SENDTOCHANNEL( &netmsgwinner, 0 );
		return;
	}

	SCHOOLWARS_SCORE_DATA_VEC vecscore;
	for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++i )
	{
		if( m_sScore[i].wKills == 0 && m_sScore[i].wDeaths == 0 )
			continue;

		vecscore.push_back( m_sScore[i] );
	}

	if ( vecscore.empty() )
	{
		//no winner
		GLMSG::SNETPC_SCHOOLWARS_A2F_SCORE_WINNER netmsgwinner;
		netmsgwinner.wLastWinner = SCHOOLWARS_SCHOOL_SIZE;
		netmsgwinner.bFullScore = FALSE;
		GLAgentServer::GetInstance().SENDTOCHANNEL( &netmsgwinner, 0 );
		return;
	}

	std::sort( vecscore.begin(), vecscore.end() );

	m_wLastWinner = vecscore[0].wSchool;

	GLMSG::SNETPC_SCHOOLWARS_A2F_SCORE_WINNER netmsgwinner;
	netmsgwinner.wLastWinner = m_wLastWinner;
	netmsgwinner.bFullScore = FALSE;
	GLAgentServer::GetInstance().SENDTOCHANNEL( &netmsgwinner, 0 );
}

void GLPVPSchoolWarsAgent::ScoreCheck()
{
	if ( m_emState != SCHOOLWARS_STATE_BATTLE )		return;
	if ( m_wLastWinner != SCHOOLWARS_SCHOOL_SIZE )	return;

	for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++i )
	{
		if ( m_sScore[i].wKills >= m_wMaxScore )
		{
			m_wLastWinner = i;

			//full capture, end
			SetEventState( SCHOOLWARS_STATE_REWARD );
		}
	}
}

void GLPVPSchoolWarsAgent::ScoreUpdate()
{
	GLMSG::SNETPC_SCHOOLWARS_A2F_SCORE_UPDATE netmsgfield;
	for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++i )
		netmsgfield.sScore[i] = m_sScore[i];
	GLAgentServer::GetInstance().SENDTOCHANNEL( &netmsgfield, 0 );
}
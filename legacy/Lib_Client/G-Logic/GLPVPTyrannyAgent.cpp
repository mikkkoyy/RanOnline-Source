#include "StdAfx.h"
#include "GLPVPTyrannyAgent.h"
#include "GLAgentServer.h"
#include "GLAGLandMan.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GLPVPTyrannyAgent& GLPVPTyrannyAgent::GetInstance()
{
	static GLPVPTyrannyAgent cInstance;
	return cInstance;
}

void GLPVPTyrannyAgent::LoadEvent( std::string strFile )
{
	ResetData();

	bool bok = Load( strFile );
	if ( bok )
	{
		FindScheduleFirst();
	}
}

bool GLPVPTyrannyAgent::SetMapState ()
{
	if( m_sMap.IsValidNativeID() )
	{
		GLAGLandMan *plandman = GLAgentServer::GetInstance().GetByMapID ( m_sMap );
		if ( plandman )
			plandman->m_bPVPTyrannyMap = true;
	}
	
	return true;
}

bool GLPVPTyrannyAgent::FrameMove ( float fElaps )
{
	if ( GLCONST_CHAR::bPKLESS )	return false;
	if ( m_vecSchedule.empty() )	return false;

	if ( IsEnded() )
	{
		if ( m_sScheduleNext.dwIndex != TYRANNY_SCHEDULE_NONE )
		{
			CTime ctimecur = CTime::GetCurrentTime();
			if ( m_sScheduleNext.sStartSched.wStartDay == (WORD)ctimecur.GetDayOfWeek() &&
				m_sScheduleNext.sStartSched.wStartHour == (WORD)ctimecur.GetHour() &&
				m_sScheduleNext.sStartSched.wStartMinute == (WORD)ctimecur.GetMinute() )
			{
				SetEventState( TYRANNY_STATE_REGISTER );
			}
		}
	}
	

	if ( IsRegister() )
	{
		m_fTimer += fElaps;
		m_fRemain -= fElaps;

		NotifyTimeToStart( m_fRemain );	

		if ( m_fTimer >= (float)m_wRegisterTime ){
			SetEventState( TYRANNY_STATE_BATTLE );
		}
	}

	if ( IsBattle() )
	{
		m_fTimer += fElaps;
		m_fRemain -= fElaps;

		if ( m_fTimer >= (float)m_wBattleTime ){
			SetEventState( TYRANNY_STATE_REWARD );
		}
	}

	if ( IsReward() )
	{
		m_fTimer += fElaps;
		m_fRemain -= fElaps;

		if ( m_fTimer >= (float)m_wRewardTime ){
			SetEventState( TYRANNY_STATE_ENDED );
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

void GLPVPTyrannyAgent::SetEventState( TYRANNY_STATE emState )
{
	std::string streventstate[TYRANNY_STATE_SIZE] = 
	{
		"Event End",
		"Register Start",
		"Battle Start",
		"Award Start"
	};

	GLAgentServer::GetInstance().CONSOLEMSG_WRITE( "GLPVPTyrannyAgent Change State to '%s'", streventstate[emState].c_str() );
	CDebugSet::ToFileWithTime( "_pvptyranny.txt", "GLPVPTyrannyAgent Change State to '%s'", streventstate[emState].c_str() );

	switch( emState )
	{
	case TYRANNY_STATE_ENDED:
		{
			DoStateEnd();
		}break;

	case TYRANNY_STATE_REGISTER:
		{
			DoStateRegister();
		}break;

	case TYRANNY_STATE_BATTLE:
		{
			DoStateBattle();
		}break;

	case TYRANNY_STATE_REWARD:
		{
			DoStateReward();
		}break;
	};

	m_emState = emState;
}

void GLPVPTyrannyAgent::ResetData()
{
	m_emState = TYRANNY_STATE_ENDED;
	m_sScheduleNext = TYRANNY_SCHED_NEXT();
	m_vecSchedule.clear();
	
	for( int i=0; i<TYRANNY_TOWER_SIZE; ++i )
		m_sTowerData[i].wOwner = TYRANNY_SCHOOL_SIZE;

	for( int i=0; i<TYRANNY_SCHOOL_SIZE; ++i )
	{
		m_sTowerScore[i].Reset();
		m_sTowerScore[i].wSchool = i;
	}

	m_wLastWinner = TYRANNY_SCHOOL_SIZE;

	for( int i=0; i<TYRANNY_MINI_RANKING_NUM; ++i )
		m_sLastTop[i] = TYRANNY_PLAYER_DATA();

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
}

void GLPVPTyrannyAgent::DoStateRegister()
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

	GLMSG::SNET_TYRANNY_A2FC_STATE_REGISTER netmsgstate;
	netmsgstate.fTime = m_fRemain;
	GLAgentServer::GetInstance().SENDTOCHANNEL ( &netmsgstate, 0 );
	GLAgentServer::GetInstance().SENDTOALLCLIENT ( &netmsgstate );

	GLAgentServer::GetInstance().TyrannyResetBuffs();
}

void GLPVPTyrannyAgent::DoStateBattle()
{
	m_fTimer	= 0.0f;
	m_fRemain	= (float)m_wBattleTime;

	//reset tower owner
	for( int i=0; i<TYRANNY_TOWER_SIZE; ++i )
		m_sTowerData[i].wOwner = TYRANNY_SCHOOL_SIZE;

	//reset tower scores
	for( int i=0; i<TYRANNY_SCHOOL_SIZE; ++i )
	{
		m_sTowerScore[i].Reset();
		m_sTowerScore[i].wSchool = i;
	}

	//reset the last winner
	m_wLastWinner = TYRANNY_SCHOOL_SIZE;

	for( int i=0; i<TYRANNY_MINI_RANKING_NUM; ++i )
		m_sLastTop[i] = TYRANNY_PLAYER_DATA();

	ProcessQueueDataAll( m_mapPlayerDataSG, m_vecPlayerDataQueueSG );
	ProcessQueueDataAll( m_mapPlayerDataMP, m_vecPlayerDataQueueMP );
	ProcessQueueDataAll( m_mapPlayerDataPHX, m_vecPlayerDataQueuePHX );

	CalculateBattleBuff();
	SendPlayerData();
	QueueMoveIn();

	GLMSG::SNET_TYRANNY_A2FC_STATE_BATTLE netmsgstate;
	netmsgstate.fTime = m_fRemain;
	netmsgstate.wPlayerNum[TYRANNY_SCHOOL_SG] = (WORD)m_mapPlayerDataSG.size();
	netmsgstate.wPlayerNum[TYRANNY_SCHOOL_MP] = (WORD)m_mapPlayerDataMP.size();
	netmsgstate.wPlayerNum[TYRANNY_SCHOOL_PHX] = (WORD)m_mapPlayerDataPHX.size();
	GLAgentServer::GetInstance().SENDTOCHANNEL ( &netmsgstate, 0 );
	GLAgentServer::GetInstance().SENDTOALLCLIENT ( &netmsgstate );
}

void GLPVPTyrannyAgent::DoStateReward()
{
	m_fTimer	= 0.0f;
	m_fRemain	= (float)m_wRewardTime;

	//get the winner
	CheckWinner();

	GLMSG::SNET_TYRANNY_A2FC_STATE_REWARD netmsgstate;
	netmsgstate.fTime = m_fRemain;
	netmsgstate.wWinnerSchool = m_wLastWinner;
	GLAgentServer::GetInstance().SENDTOCHANNEL ( &netmsgstate, 0 );
	GLAgentServer::GetInstance().SENDTOALLCLIENT ( &netmsgstate );
}

void GLPVPTyrannyAgent::DoStateEnd()
{
	GLMSG::SNET_TYRANNY_A2FC_STATE_ENDED netmsgstate;
	GLAgentServer::GetInstance().SENDTOCHANNEL ( &netmsgstate, 0 );
	GLAgentServer::GetInstance().SENDTOALLCLIENT ( &netmsgstate );

	if ( FindScheduleNext() )
	{
		GLMSG::SNET_TYRANNY_A2FC_NEXTSCHED netmsgschedule;
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

	/*dmk14 randomized buff*/
	//RandomizeSystemBuff();

	if ( m_bRewardBuff )
		GLAgentServer::GetInstance().TyrannySetRewardBuffs();
}

void GLPVPTyrannyAgent::NotifyTimeToStart( float fTime )
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

	GLMSG::SNET_TYRANNY_A2C_TOBATTLE_TIME netmsgclient;
	netmsgclient.wTime = wremain / 60;
	GLAgentServer::GetInstance().SENDTOALLCLIENT ( &netmsgclient );
}

/*global buffs map setting, Juver, 2018/01/23 */
void GLPVPTyrannyAgent::RequestInfoPC( DWORD dwCharID, BOOL bTyrannyMap, BOOL bDisableSystemBuffs )
{
	PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( dwCharID );
	if ( !pchar )	return;

	//event info
	GLMSG::SNET_TYRANNY_A2C_BATTLEINFO_PC netmsgclient;
	netmsgclient.emState = m_emState;
	netmsgclient.fRemain = m_fRemain;
	netmsgclient.sScheduleNext = m_sScheduleNext;
	netmsgclient.wLevelReq = m_wLevelReq;
	netmsgclient.wPlayerLimit = m_wPlayerLimit;
	
	//register info
	TYRANNY_REGISTER_DATA* pplayerdata = PlayerDataGet( dwCharID );
	if ( pplayerdata ){
		netmsgclient.bRegistered = true;
	}

	TYRANNY_REGISTER_DATA_QUEUE* pplayerqueue = PlayerQueueGet( dwCharID );
	if ( pplayerqueue ){
		netmsgclient.bRegistered = true;
		netmsgclient.bQueued = true;
		netmsgclient.wQueueNum = pplayerqueue->wQueueNum;
	}

	GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsgclient );

	//last rankinfo
	GLMSG::SNETPC_TYRANNY_A2C_RANKINFO_PC netmsglastrank;
	netmsglastrank.wLastWinner = m_wLastWinner;
	for( int i=0; i<TYRANNY_MINI_RANKING_NUM; ++i )
		netmsglastrank.sPlayerData[i] = m_sLastTop[i];

	for( int i=0; i<TYRANNY_REWARD_BUFF_NUM; ++i )
		netmsglastrank.sRewardBuff[i] = m_sRewardBuff[i].sidSkill;

	GLAgentServer::GetInstance().SENDTOCLIENT ( pchar->m_dwClientID, &netmsglastrank );

	

	if ( bTyrannyMap )
	{
		if ( m_bBattleBuff )
		{
			GLMSG::SNETPC_TYRANNY_A2F_BUFF_BATTLE netmsgbuff;
			netmsgbuff.dwCharID = pchar->m_dwCharID;

			for ( int i=0; i<TYRANNY_BATTLE_BUFF_NUM; ++i )
			{
				WORD _wLevel = m_wBattleBuffLevelIndex[pchar->m_wSchool];
				netmsgbuff.sBattleBuff[i].sidSkill = m_sBattleBuff[i].sidSkill;
				netmsgbuff.sBattleBuff[i].wLevel = m_sBattleBuff[i].wLevel[_wLevel];
			}

			GLAgentServer::GetInstance().SENDTOFIELDSVR ( pchar->m_nChannel, pchar->m_dwCurFieldSvr, &netmsgbuff );
		}
	}
	else
	{
		/*global buffs map setting, Juver, 2018/01/23 */
		if ( m_bRewardBuff && !bDisableSystemBuffs )
		{
			BOOL bEnableBuff = TRUE;
			if ( pchar->m_wSchool != m_wLastWinner )	bEnableBuff = FALSE;
			if ( m_emState != TYRANNY_STATE_ENDED )		bEnableBuff = FALSE;
			if ( m_wLastWinner == TYRANNY_SCHOOL_SIZE ) bEnableBuff = FALSE;

			if ( bEnableBuff )
			{
				GLMSG::SNETPC_TYRANNY_A2F_BUFF_REWARD netmsgbuff;
				netmsgbuff.dwCharID = pchar->m_dwCharID;
				netmsgbuff.sRewardBuffMain = m_sRewardBuffMain;

				for ( int i=0; i<TYRANNY_REWARD_BUFF_NUM; ++i )
					netmsgbuff.sRewardBuff[i] = m_sRewardBuff[i];

				GLAgentServer::GetInstance().SENDTOFIELDSVR ( pchar->m_nChannel, pchar->m_dwCurFieldSvr, &netmsgbuff );
			}
		}
	}
}

void GLPVPTyrannyAgent::CalculateBattleBuff()
{
	//reset buff level
	for( int i=0; i<TYRANNY_SCHOOL_SIZE; ++i )
		m_wBattleBuffLevelIndex[i] = 0;

	if ( m_wPlayerLimit > 0 )
	{
		{	//sg
			int nPlayerCnt = (int)m_mapPlayerDataSG.size();
			float fPercent = ((float)nPlayerCnt/(float)m_wPlayerLimit) * 10.0f; //1~10 %
			m_wBattleBuffLevelIndex[0] = static_cast <WORD> (std::floor(fPercent) );
		}

		{	//mp
			int nPlayerCnt = (int)m_mapPlayerDataMP.size();
			float fPercent = ((float)nPlayerCnt/(float)m_wPlayerLimit) * 10.0f; //1~10 %
			m_wBattleBuffLevelIndex[1] = static_cast <WORD> (std::floor(fPercent) );
		}

		{	//phx
			int nPlayerCnt = (int)m_mapPlayerDataPHX.size();
			float fPercent = ((float)nPlayerCnt/(float)m_wPlayerLimit) * 10.0f; //1~10 %
			m_wBattleBuffLevelIndex[2] = static_cast <WORD> (std::floor(fPercent) );
		}
	}

	for( int i=0; i<TYRANNY_SCHOOL_SIZE; ++i )
	{
		if( m_wBattleBuffLevelIndex[i] >= 0 && m_wBattleBuffLevelIndex[i] < TYRANNY_BATTLE_BUFF_LEVEL_NUM )
		{
			CDebugSet::ToFileWithTime( "_pvptyranny.txt", "GLPVPTyrannyAgent player buff levels index:%d level:%d", i, m_wBattleBuffLevelIndex[i] );
		}
		else
		{
			WORD wLast = m_wBattleBuffLevelIndex[i];
			m_wBattleBuffLevelIndex[i] = TYRANNY_BATTLE_BUFF_LEVEL_NUM -1 ;
			CDebugSet::ToFileWithTime( "_pvptyranny.txt", "GLPVPTyrannyAgent player wrong buff levels index:%d level:%d corrected to level:%d", i, wLast, m_wBattleBuffLevelIndex[i] );
		}
	}
}

/*dmk14 randomized buff*/
void GLPVPTyrannyAgent::RandomizeSystemBuff()
{

	m_sRewardBuffMain = TYRANNY_BUFF_DATA();

	for( int i=0; i<TYRANNY_REWARD_BUFF_NUM; ++i )
		m_sRewardBuff[i] = TYRANNY_BUFF_DATA();

	VEC_SYSTEM vecSystemID;
	vecSystemID.clear();

	//buff main
	SNATIVEID sMain = SNATIVEID( 51,13 );
	m_sRewardBuffMain.sidSkill = sMain;
	m_sRewardBuffMain.wLevel = 0;

	int nCount = 0;
	for( int j=0;j<SYSTEM_BUFF_SETTING_SIZE;++j)
	{
		if( nCount >= TYRANNY_REWARD_BUFF_NUM )									break;
		VEC_ITER_SYSTEM iter = vecSystemID.find( GLCONST_CHAR::sRandomSystemBuff[j].sSystemID.dwID );

		if( iter != vecSystemID.end() )											continue;
		if( GLCONST_CHAR::sRandomSystemBuff[j].sSystemID == NATIVEID_NULL() )	continue;

		float fRate = GLCONST_CHAR::sRandomSystemBuff[j].fRATE;

		if ( RANDOM_GEN(fRate) )
		{
			m_sRewardBuff[nCount].sidSkill = GLCONST_CHAR::sRandomSystemBuff[j].sSystemID;
			m_sRewardBuff[nCount].wLevel = 0;

			nCount++;
		}
	}
}
#include "StdAfx.h"
#include "GLPVPSchoolWars.h"

#include "gltexfile.h"
#include "IniLoader.h"
#include "GLOGIC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool GLPVPSchoolWars::Load( std::string strFile )
{
	if( strFile.empty() )	return FALSE;

	std::string strpath;
	strpath = GLOGIC::GetServerPath ();
	strpath += strFile;

	CIniLoader cFILE;

	if( GLOGIC::bGLOGIC_ZIPFILE )
		cFILE.SetZipFile( GLOGIC::strGLOGIC_SERVER_ZIPFILE );

	if( !cFILE.open ( strpath, true ) )
	{
		CDebugSet::ToLogFile ( "ERROR : GLPVPSchoolWars::Load(), File Open %s", strFile.c_str() );
		return false;
	}

	//event map
	cFILE.getflag( "EVENT", "EVENT_MAP", 0, 2, m_sMap.wMainID );
	cFILE.getflag( "EVENT", "EVENT_MAP", 1, 2, m_sMap.wSubID );

	//base gate
	for( DWORD i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++i ){
		cFILE.getflag( "EVENT", "EVENT_GATE", i, SCHOOLWARS_SCHOOL_SIZE, m_wSchoolGate[i] );
	}

	//out map/gate
	cFILE.getflag( "EVENT", "EXIT_MAP", 0, 2, m_sOutMap.wMainID );
	cFILE.getflag( "EVENT", "EXIT_MAP", 1, 2, m_sOutMap.wSubID );
	cFILE.getflag( "EVENT", "EXIT_GATE", 0, 1, m_wOutGate );

	//reg settings
	cFILE.getflag( "EVENT", "PLAYER_LIMIT", 0, 1, m_wPlayerLimit );
	cFILE.getflag( "EVENT", "LEVEL_REQ", 0, 1, m_wLevelReq );
	cFILE.getflag( "EVENT", "CONTRI_REQ", 0, 1, m_llContriReq );
	cFILE.getflag( "EVENT", "MAX_SCORE", 0, 1, m_wMaxScore );

	//durations
	cFILE.getflag( "EVENT", "DURATION_REGISTER", 0, 1, m_wRegisterTime );
	cFILE.getflag( "EVENT", "DURATION_BATTLE", 0, 1, m_wBattleTime );
	cFILE.getflag( "EVENT", "DURATION_REWARD", 0, 1, m_wRewardTime );

	//score logic
	cFILE.getflag( "EVENT", "VAR_KILL", 0, 1, m_fKillVar );
	cFILE.getflag( "EVENT", "VAR_HEAL", 0, 1, m_fHealVar );
	cFILE.getflag( "EVENT", "VAR_RESU", 0, 1, m_fResuVar );

	//point logic
	cFILE.getflag( "EVENT", "VAR_POINT", 0, 1, m_fPointVar );
	cFILE.getflag( "EVENT", "POINT_BASE_WIN", 0, 1, m_wBasePointWin );
	cFILE.getflag( "EVENT", "POINT_BASE_LOSS", 0, 1, m_wBasePointLoss );
	cFILE.getflag( "EVENT", "POINT_MAX", 0, 1, m_wMaxRewardPoint );

	//reward item
	cFILE.getflag( "EVENT", "REWARD_ITEM_WIN", 0, 2, m_sItemRewardWin.wMainID );
	cFILE.getflag( "EVENT", "REWARD_ITEM_WIN", 1, 2, m_sItemRewardWin.wSubID );
	cFILE.getflag( "EVENT", "REWARD_SCORE_WIN_LIMIT", 0, 1, m_wRewardLimitScoreWin );

	cFILE.getflag( "EVENT", "REWARD_ITEM_LOST", 0, 2, m_sItemRewardLost.wMainID );
	cFILE.getflag( "EVENT", "REWARD_ITEM_LOST", 1, 2, m_sItemRewardLost.wSubID );
	cFILE.getflag( "EVENT", "REWARD_SCORE_LOST_LIMIT", 0, 1, m_wRewardLimitScoreLost );

	
	if ( m_wPlayerLimit == 0 )
	{
		CDebugSet::MsgBoxAfx( "GLPVPSchoolWars::Load invalid PlayerLimit:%u limit must be atleast 1", m_wPlayerLimit );
		m_wPlayerLimit = 1;
	}

	if ( m_wLevelReq == 0 )
	{
		CDebugSet::MsgBoxAfx( "GLPVPSchoolWars::Load invalid LevelReq:%u req must be atleast 1", m_wLevelReq );
		m_wLevelReq = 1;
	}

	if ( m_wMaxScore == 0 )
	{
		CDebugSet::MsgBoxAfx( "GLPVPSchoolWars::Load invalid MaxScore:%u limit must be atleast 1", m_wMaxScore );
		m_wMaxScore = 1;
	}

	if ( m_wRegisterTime < 300 )
	{
		CDebugSet::MsgBoxAfx( "GLPVPSchoolWars::Load invalid BattleTime time must be more than 300 sec:%u", m_wRegisterTime );
		m_wRegisterTime = 300;
	}

	if ( m_wBattleTime < 300 )
	{
		CDebugSet::MsgBoxAfx( "GLPVPSchoolWars::Load invalid BattleTime time must be more than 300 sec:%u", m_wBattleTime );
		m_wBattleTime = 300;
	}

	if ( m_wRewardTime < 180 )
	{
		CDebugSet::MsgBoxAfx( "GLPVPSchoolWars::Load invalid RewardTime time must be more than 300 sec:%u", m_wRewardTime );
		m_wRewardTime = 180;
	}

	//schedule day-hour-minute
	m_vecSchedule.clear();
	DWORD dwnum = cFILE.GetKeySize( "EVENT", "BATTLE_TIME" );
	for( DWORD i=0; i<dwnum; ++i )
	{
		SCHOOLWARS_SCHED sched;
		cFILE.getflag( i, "EVENT", "BATTLE_TIME", 0, 3, sched.wStartDay );
		cFILE.getflag( i, "EVENT", "BATTLE_TIME", 1, 3, sched.wStartHour );
		cFILE.getflag( i, "EVENT", "BATTLE_TIME", 2, 3, sched.wStartMinute );

		if ( sched.wStartDay < 1 || sched.wStartDay > 7 )
		{
			CDebugSet::MsgBoxAfx( "GLPVPSchoolWars::Load invalid startday:%u index:%d", sched.wStartDay, i );
			continue;
		}

		if ( sched.wStartHour >= 24 )
		{
			CDebugSet::MsgBoxAfx( "GLPVPSchoolWars::Load invalid StartHour:%u index:%d", sched.wStartHour, i );
			continue;
		}


		if ( sched.wStartMinute >= 60 )
		{
			CDebugSet::MsgBoxAfx( "GLPVPSchoolWars::Load invalid StartMinute:%u index:%d", sched.wStartMinute, i );
			continue;
		}

		m_vecSchedule.push_back( sched );
	}

	std::sort( m_vecSchedule.begin(), m_vecSchedule.end() );

	if ( m_vecSchedule.empty() )
	{
		CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "GLPVPSchoolWars::Load no schedules loaded" );
	}

	return true;
}

void GLPVPSchoolWars::FindScheduleFirst()
{
	if ( m_vecSchedule.empty() )	return;

	m_sScheduleNext.dwIndex = SCHOOLWARS_SCHEDULE_NONE;

	if ( m_vecSchedule.size() == 1 )
	{
		m_sScheduleNext.dwIndex = 0;
		m_sScheduleNext.sStartSched = m_vecSchedule[0];
		m_sScheduleNext.CalculateEndTime( m_wRegisterTime, m_wBattleTime, m_wRewardTime );
	}
	else
	{
		CTime ctimecur = CTime::GetCurrentTime();

		//expensive loop
		int nminutes = 0;
		while(1)
		{
			for ( size_t i=0; i<m_vecSchedule.size(); ++i )
			{
				if ( m_vecSchedule[i].wStartDay != (WORD)ctimecur.GetDayOfWeek() )	continue;
				if ( m_vecSchedule[i].wStartHour != (WORD)ctimecur.GetHour() )		continue;
				if ( m_vecSchedule[i].wStartMinute != (WORD)ctimecur.GetMinute() )	continue;

				m_sScheduleNext.dwIndex = i;
				m_sScheduleNext.sStartSched = m_vecSchedule[i];
				m_sScheduleNext.CalculateEndTime( m_wRegisterTime, m_wBattleTime, m_wRewardTime );
			}

			if ( m_sScheduleNext.dwIndex != SCHOOLWARS_SCHEDULE_NONE )
				break;

			//week limit
			if ( nminutes >= 11000/*10080*/ )
				break;

			CTimeSpan span( 0, 0, 1, 0 );
			ctimecur += span;
			nminutes ++;
			Sleep(0);
		}

		if ( m_sScheduleNext.dwIndex == SCHOOLWARS_SCHEDULE_NONE )
		{
			CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "FindScheduleFirst failed! no events will start" );
		}
		else
		{
			CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "FindScheduleFirst Index:%u Start(D:%u/H:%u/M:%u) BattleStart(H:%u/M:%u) BattleEnd(H:%u/M:%u)", m_sScheduleNext.dwIndex, 
				m_sScheduleNext.sStartSched.wStartDay, m_sScheduleNext.sStartSched.wStartHour, m_sScheduleNext.sStartSched.wStartMinute,
				m_sScheduleNext.wBattleStartHour, m_sScheduleNext.wBattleStartMinute, m_sScheduleNext.wBattleEndHour, m_sScheduleNext.wBattleEndMinute );
		}
	}
};

bool GLPVPSchoolWars::FindScheduleNext()
{
	if ( m_sScheduleNext.dwIndex == SCHOOLWARS_SCHEDULE_NONE )	return false;

	DWORD dwnewidx = m_sScheduleNext.dwIndex + 1;
	if ( dwnewidx >= (DWORD)m_vecSchedule.size() )	dwnewidx = 0;

	m_sScheduleNext.dwIndex = dwnewidx;
	m_sScheduleNext.sStartSched = m_vecSchedule[dwnewidx];
	m_sScheduleNext.CalculateEndTime( m_wRegisterTime, m_wBattleTime, m_wRewardTime );

	CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "FindScheduleNext Index:%u Start(D:%u/H:%u/M:%u) BattleStart(H:%u/M:%u) BattleEnd(H:%u/M:%u)", m_sScheduleNext.dwIndex, 
		m_sScheduleNext.sStartSched.wStartDay, m_sScheduleNext.sStartSched.wStartHour, m_sScheduleNext.sStartSched.wStartMinute,
		m_sScheduleNext.wBattleStartHour, m_sScheduleNext.wBattleStartMinute, m_sScheduleNext.wBattleEndHour, m_sScheduleNext.wBattleEndMinute );

	return true;
};


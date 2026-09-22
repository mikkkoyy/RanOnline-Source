#include "StdAfx.h"
#include "GLPVPTyranny.h"

#include "gltexfile.h"
#include "IniLoader.h"
#include "GLOGIC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool GLPVPTyranny::Load( std::string strFile )
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
		CDebugSet::ToLogFile ( "ERROR : GLPVPTyranny::Load(), File Open %s", strFile.c_str() );
		return false;
	}

	//event map
	cFILE.getflag( "EVENT", "EVENT_MAP", 0, 2, m_sMap.wMainID );
	cFILE.getflag( "EVENT", "EVENT_MAP", 1, 2, m_sMap.wSubID );

	//base gate
	for( DWORD i=0; i<TYRANNY_SCHOOL_SIZE; ++i ){
		cFILE.getflag( "EVENT", "EVENT_GATE", i, TYRANNY_SCHOOL_SIZE, m_wSchoolGate[i] );
	}

	//out map/gate
	cFILE.getflag( "EVENT", "EXIT_MAP", 0, 2, m_sOutMap.wMainID );
	cFILE.getflag( "EVENT", "EXIT_MAP", 1, 2, m_sOutMap.wSubID );
	cFILE.getflag( "EVENT", "EXIT_GATE", 0, 1, m_wOutGate );

	//reg settings
	cFILE.getflag( "EVENT", "PLAYER_LIMIT", 0, 1, m_wPlayerLimit );
	cFILE.getflag( "EVENT", "LEVEL_REQ", 0, 1, m_wLevelReq );

	//durations
	cFILE.getflag( "EVENT", "DURATION_REGISTER", 0, 1, m_wRegisterTime );
	cFILE.getflag( "EVENT", "DURATION_BATTLE", 0, 1, m_wBattleTime );
	cFILE.getflag( "EVENT", "DURATION_REWARD", 0, 1, m_wRewardTime );

	//tower
	cFILE.getflag( "EVENT", "TOWER_DAMAGE_CAPTURE", 0, 1, m_dwDamageCapture );
	cFILE.getflag( "EVENT", "TOWER_DAMAGE_BASE", 0, 1, m_dwDamageBase );

	//score logic
	cFILE.getflag( "EVENT", "VAR_DAMAGE", 0, 1, m_fDamageVar );
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

	for ( int i=0; i<TYRANNY_TOWER_SIZE; ++i )
	{
		CString strtowerdata;
		strtowerdata.Format ( "TOWER_%02d", i );
		cFILE.getflag( "EVENT", strtowerdata.GetString(), 0, 2, m_sTowerData[i].sidCrow.wMainID );
		cFILE.getflag( "EVENT", strtowerdata.GetString(), 1, 2, m_sTowerData[i].sidCrow.wSubID );
	}

	//buffs
	cFILE.getflag( "EVENT", "BUFF_REWARD_USE", 0, 1, m_bRewardBuff );
	cFILE.getflag( "EVENT", "BUFF_BATTLE_USE", 0, 1, m_bBattleBuff );

	cFILE.getflag( "EVENT", "BUFF_REWARD_MAIN", 0, 3, m_sRewardBuffMain.sidSkill.wMainID );
	cFILE.getflag( "EVENT", "BUFF_REWARD_MAIN", 1, 3, m_sRewardBuffMain.sidSkill.wSubID );
	cFILE.getflag( "EVENT", "BUFF_REWARD_MAIN", 2, 3, m_sRewardBuffMain.wLevel );

	for ( int i=0; i<TYRANNY_REWARD_BUFF_NUM; ++i )
	{
		CString strbuff;
		strbuff.Format ( "BUFF_REWARD_%02d", i );
		cFILE.getflag( "EVENT", strbuff.GetString(), 0, 3, m_sRewardBuff[i].sidSkill.wMainID );
		cFILE.getflag( "EVENT", strbuff.GetString(), 1, 3, m_sRewardBuff[i].sidSkill.wSubID );
		cFILE.getflag( "EVENT", strbuff.GetString(), 2, 3, m_sRewardBuff[i].wLevel );
	}

	for ( int i=0; i<TYRANNY_BATTLE_BUFF_NUM; ++i )
	{
		CString strbuff;
		strbuff.Format ( "BUFF_BATTLE_%02d", i );
		cFILE.getflag( "EVENT", strbuff.GetString(), 0, 12, m_sBattleBuff[i].sidSkill.wMainID );
		cFILE.getflag( "EVENT", strbuff.GetString(), 1, 12, m_sBattleBuff[i].sidSkill.wSubID );
		cFILE.getflag( "EVENT", strbuff.GetString(), 2, 12, m_sBattleBuff[i].wLevel[0] );
		cFILE.getflag( "EVENT", strbuff.GetString(), 3, 12, m_sBattleBuff[i].wLevel[1] );
		cFILE.getflag( "EVENT", strbuff.GetString(), 4, 12, m_sBattleBuff[i].wLevel[2] );
		cFILE.getflag( "EVENT", strbuff.GetString(), 5, 12, m_sBattleBuff[i].wLevel[3] );
		cFILE.getflag( "EVENT", strbuff.GetString(), 6, 12, m_sBattleBuff[i].wLevel[4] );
		cFILE.getflag( "EVENT", strbuff.GetString(), 7, 12, m_sBattleBuff[i].wLevel[5] );
		cFILE.getflag( "EVENT", strbuff.GetString(), 8, 12, m_sBattleBuff[i].wLevel[6] );
		cFILE.getflag( "EVENT", strbuff.GetString(), 9, 12, m_sBattleBuff[i].wLevel[7] );
		cFILE.getflag( "EVENT", strbuff.GetString(), 10, 12, m_sBattleBuff[i].wLevel[8] );
		cFILE.getflag( "EVENT", strbuff.GetString(), 11, 12, m_sBattleBuff[i].wLevel[9] );
	}

	for ( int i=0; i<TYRANNY_TOWER_SIZE; ++i )
	{
		for ( int ii=0; ii<TYRANNY_TOWER_SIZE; ++ii )
		{
			if ( i == ii )	continue;

			if ( m_sTowerData[i].sidCrow == m_sTowerData[ii].sidCrow )
			{
				CDebugSet::MsgBoxAfx( "GLPVPTyranny::Load duplicate id in index %d~%d ID[%d~%d]", i, ii, m_sTowerData[i].sidCrow.wMainID, m_sTowerData[i].sidCrow.wSubID );
			}
		}
	}

	if ( m_wPlayerLimit == 0 )
	{
		CDebugSet::MsgBoxAfx( "GLPVPTyranny::Load invalid PlayerLimit:%u limit must be atleast 1", m_wPlayerLimit );
		m_wPlayerLimit = 1;
	}

	if ( m_wLevelReq == 0 )
	{
		CDebugSet::MsgBoxAfx( "GLPVPTyranny::Load invalid LevelReq:%u req must be atleast 1", m_wLevelReq );
		m_wLevelReq = 1;
	}

	if ( m_wRegisterTime < 1 )
	{
		CDebugSet::MsgBoxAfx( "GLPVPTyranny::Load invalid BattleTime time must be more than 300 sec:%u", m_wRegisterTime );
		m_wRegisterTime = 1;
	}

	if ( m_wBattleTime < 1 )
	{
		CDebugSet::MsgBoxAfx( "GLPVPTyranny::Load invalid BattleTime time must be more than 300 sec:%u", m_wBattleTime );
		m_wBattleTime = 1;
	}

	if ( m_wRewardTime < 1 )
	{
		CDebugSet::MsgBoxAfx( "GLPVPTyranny::Load invalid RewardTime time must be more than 300 sec:%u", m_wRewardTime );
		m_wRewardTime = 1;
	}

	if ( m_dwDamageCapture <= 0 )
	{
		CDebugSet::MsgBoxAfx( "GLPVPTyranny::Load invalid m_dwDamageCapture must be more than 0 val:%u", m_dwDamageCapture );
		m_dwDamageCapture = 1;
	}

	if ( m_dwDamageBase <= 0 )
	{
		CDebugSet::MsgBoxAfx( "GLPVPTyranny::Load invalid m_dwDamageBase must be more than 0 val:%u", m_dwDamageBase );
		m_dwDamageBase = 1;
	}

	//schedule day-hour-minute
	m_vecSchedule.clear();
	DWORD dwnum = cFILE.GetKeySize( "EVENT", "BATTLE_TIME" );
	for( DWORD i=0; i<dwnum; ++i )
	{
		TYRANNY_SCHED sched;
		cFILE.getflag( i, "EVENT", "BATTLE_TIME", 0, 3, sched.wStartDay );
		cFILE.getflag( i, "EVENT", "BATTLE_TIME", 1, 3, sched.wStartHour );
		cFILE.getflag( i, "EVENT", "BATTLE_TIME", 2, 3, sched.wStartMinute );

		if ( sched.wStartDay < 1 || sched.wStartDay > 7 )
		{
			CDebugSet::MsgBoxAfx( "GLPVPTyranny::Load invalid startday:%u index:%d", sched.wStartDay, i );
			continue;
		}

		if ( sched.wStartHour >= 24 )
		{
			CDebugSet::MsgBoxAfx( "GLPVPTyranny::Load invalid StartHour:%u index:%d", sched.wStartHour, i );
			continue;
		}


		if ( sched.wStartMinute >= 60 )
		{
			CDebugSet::MsgBoxAfx( "GLPVPTyranny::Load invalid StartMinute:%u index:%d", sched.wStartMinute, i );
			continue;
		}

		m_vecSchedule.push_back( sched );
	}

	std::sort( m_vecSchedule.begin(), m_vecSchedule.end() );

	if ( m_vecSchedule.empty() )
	{
		CDebugSet::ToFileWithTime( "_pvptyranny.txt", "GLPVPTyranny::Load no schedules loaded" );
	}

	

	return true;
}

void GLPVPTyranny::FindScheduleFirst()
{
	if ( m_vecSchedule.empty() )	return;

	m_sScheduleNext.dwIndex = TYRANNY_SCHEDULE_NONE;

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

			if ( m_sScheduleNext.dwIndex != TYRANNY_SCHEDULE_NONE )
				break;

			//week limit
			if ( nminutes >= 11000/*10080*/ )
				break;

			CTimeSpan span( 0, 0, 1, 0 );
			ctimecur += span;
			nminutes ++;
			Sleep(0);
		}

		if ( m_sScheduleNext.dwIndex == TYRANNY_SCHEDULE_NONE )
		{
			CDebugSet::ToFileWithTime( "_pvptyranny.txt", "FindScheduleFirst failed! no events will start" );
		}
		else
		{
			CDebugSet::ToFileWithTime( "_pvptyranny.txt", "FindScheduleFirst Index:%u Start(D:%u/H:%u/M:%u) BattleStart(H:%u/M:%u) BattleEnd(H:%u/M:%u)", m_sScheduleNext.dwIndex, 
				m_sScheduleNext.sStartSched.wStartDay, m_sScheduleNext.sStartSched.wStartHour, m_sScheduleNext.sStartSched.wStartMinute,
				m_sScheduleNext.wBattleStartHour, m_sScheduleNext.wBattleStartMinute, m_sScheduleNext.wBattleEndHour, m_sScheduleNext.wBattleEndMinute );
		}
	}
};

bool GLPVPTyranny::FindScheduleNext()
{
	if ( m_sScheduleNext.dwIndex == TYRANNY_SCHEDULE_NONE )	return false;

	DWORD dwnewidx = m_sScheduleNext.dwIndex + 1;
	if ( dwnewidx >= (DWORD)m_vecSchedule.size() )	dwnewidx = 0;

	m_sScheduleNext.dwIndex = dwnewidx;
	m_sScheduleNext.sStartSched = m_vecSchedule[dwnewidx];
	m_sScheduleNext.CalculateEndTime( m_wRegisterTime, m_wBattleTime, m_wRewardTime );

	CDebugSet::ToFileWithTime( "_pvptyranny.txt", "FindScheduleNext Index:%u Start(D:%u/H:%u/M:%u) BattleStart(H:%u/M:%u) BattleEnd(H:%u/M:%u)", m_sScheduleNext.dwIndex, 
		m_sScheduleNext.sStartSched.wStartDay, m_sScheduleNext.sStartSched.wStartHour, m_sScheduleNext.sStartSched.wStartMinute,
		m_sScheduleNext.wBattleStartHour, m_sScheduleNext.wBattleStartMinute, m_sScheduleNext.wBattleEndHour, m_sScheduleNext.wBattleEndMinute );

	return true;
};


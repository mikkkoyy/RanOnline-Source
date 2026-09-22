#pragma once

#include "GLCDMDefine.h"

#define RANKING_NUM		10
#define CLUB_NAME		10

class GLClubDeathMatch
{
public:
	enum 
	{		
		MAX_TIME = 4,
	};

	enum EM_BATTLE_STATE 
	{
		BATTLE_NOSTART		= 0,
		BATTLE_READY		= 1,
		BATTLE_START		= 2,
		BATTLE_END_AWARD	= 3,
		BATTLE_END			= 4,
	};

public:
	DWORD				m_dwID;
	std::string			m_strName;
	DWORD				m_dwClubMap;
	DWORD				m_dwCLubMapGate;
	DWORD				m_dwClubHallMap;
	DWORD				m_dwClubHallGate;
	DWORD				m_dwWinClub;
	DWORD				m_dwBattleTime;
	SCDM_TIME			m_sCdmTIME[MAX_TIME];
	SCDM_AWARD_ITEM		m_sCdmAwardItem;		
	CDM_AWARD_CHAR		m_vecCdmAwardChar;
	CDM_RANK_INFO_MAP	m_mapCdmScore;
	bool				m_bNotifyOneHour;
	bool				m_bNotifyHalfHour;
	bool				m_bNotify10MinHour;
	EM_BATTLE_STATE		m_emBattleState;
	DWORD				m_dwBattleOrder;
	DWORD				m_dwLastBattleDay;
	float				m_fBattleTimer;
	float				m_fCHECK_TIMER;
	float				m_fCHECK_TIME;
	bool				m_bBattleEndCheck;
	float				m_fRankingUpdate;

public:
	GLClubDeathMatch ();

	GLClubDeathMatch ( const GLClubDeathMatch &value )
	{
		operator= ( value );
	}

	GLClubDeathMatch& operator= ( const GLClubDeathMatch& value );

	bool operator < ( const GLClubDeathMatch& sClubDM )
	{
		return m_dwID < sClubDM.m_dwID;
	}

public:
	bool Load ( std::string strFile );

public:
	void AddCDMScore( DWORD dwKillClub, DWORD dwDeathClub );
	SCDM_RANK_INFO* GetCDMRankInof( DWORD dwClubID );

public:
	bool IsBattle () { return ( m_emBattleState == BATTLE_START ); }
	bool IsBattleReady() { return ( m_emBattleState == BATTLE_READY ); }
	bool IsBattleEndAward() { return ( m_emBattleState == BATTLE_END_AWARD); }
	bool IsBattleEnd() { return ( m_emBattleState == BATTLE_END ); }
	bool IsEnterMap( DWORD dwClubID );
	DWORD IsBattleWeekDay ( int nDayOfWeek, int nDay );
	void SetBattle( EM_BATTLE_STATE emBattleState )	{ m_emBattleState = emBattleState; }
	bool IsBattleHour ( DWORD dwORDER, int nHour );

public:
	void UpdateBattleDay ( int nDay );
	void UpdateNotifyBattle ( DWORD dwORDER, int nHour, int nMinute );
	void CheckAwardClub( float fElaps );
	void CheckExtraClub( float fElaps );

public:
	void DoBattleStart ( DWORD dwORDER, int nDay );
	void DoBattleEnd();
};

class GLClubDeathMatchMan
{
public:
	typedef std::vector<GLClubDeathMatch>			CDM_VEC;
	typedef CDM_VEC::iterator						CDM_VEC_ITER;
	typedef std::pair<CDM_VEC_ITER,CDM_VEC_ITER>	CDM_VEC_PAIR;

protected:
	CDM_VEC											m_vecClubDM;

public:
	bool Load ( std::vector<std::string> &vecFiles );

public:
	GLClubDeathMatch* Find ( DWORD dwID );
	bool IsBattle ();

public:
	GLClubDeathMatchMan ()
	{
	}
};

class GLClubDeathMatchAgentMan : public GLClubDeathMatchMan
{
protected:
	float m_fRemainTimer, m_fTimer;

public:
	bool SetMapState ();

public:
	DWORD GetRemainTime () { return (DWORD)m_fRemainTimer; }

public:
	bool FrameMove ( float fElapsedAppTime );

public:
	GLClubDeathMatchAgentMan ()
		: m_fRemainTimer(0)
		, m_fTimer(0)
	{
	}

public:
	static GLClubDeathMatchAgentMan& GetInstance();
};

class GLClubDeathMatchFieldMan : public GLClubDeathMatchMan
{
public:
	bool SetMapState ();

public:
	bool ReadyBattle( DWORD dwID );
	bool BeginBattle ( DWORD dwID );
	bool EndBattle ( DWORD dwID );

public:
	DWORD GetWinClubID ( DWORD dwClubID );
	const std::string GetName ( DWORD dwID );

	void UpdateCDMRanking( DWORD dwID );
	void CDMAwardItem( DWORD dwID );

public:
	bool FrameMove ( float fElaps );

protected:
	GLClubDeathMatchFieldMan ()
	{
	}

public:
	static GLClubDeathMatchFieldMan& GetInstance();
};
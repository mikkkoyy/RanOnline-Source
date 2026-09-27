/*!
 * \file GLPVPCaptureTheFlagClient.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */
#pragma once

#include "GLPVPCaptureTheFlagData.h"


class	GLPVPCaptureTheFlagClient
{
public:
	CAPTURE_THE_FLAG_STATE		m_emState;
	CAPTURE_THE_FLAG_SCHED_NEXT	m_sScheduleNext;
	
	WORD	m_wLevelReq;
	WORD	m_wPlayerLimit;
	LONGLONG m_llContributionReq;

	WORD	m_wMaxScore;
	WORD	m_wBattleTime;

	float	m_fRemain;
	bool	m_bRegistered;
	WORD	m_wTeam;
	
	WORD	m_wPlayerNum[CAPTURE_THE_FLAG_TEAM_SIZE];

	WORD	m_wLastWinner;
	CAPTURE_THE_FLAG_SCORE_DATA	m_sScore[CAPTURE_THE_FLAG_TEAM_SIZE];

	CAPTURE_THE_FLAG_PLAYER_DATA_VEC	m_vecRankingAll;
	CAPTURE_THE_FLAG_PLAYER_DATA_VEC	m_vecRankingA;
	CAPTURE_THE_FLAG_PLAYER_DATA_VEC	m_vecRankingB;

	CAPTURE_THE_FLAG_PLAYER_DATA		m_sLastTop[CAPTURE_THE_FLAG_MINI_RANKING_NUM];

	BOOL		m_bFlagHold[CAPTURE_THE_FLAG_TEAM_SIZE];
	STARGETID	m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_SIZE];
	BOOL		m_bShowStartNotice;

public:
	HRESULT MsgProcess ( NET_MSG_GENERIC* nmg );

public:
	bool	FrameMove ( float fTime, float fElapsedTime );

public:
	bool	IsRegister() { return bool(m_emState == CAPTURE_THE_FLAG_STATE_REGISTER); }
	bool	IsEnded()	{ return bool(m_emState == CAPTURE_THE_FLAG_STATE_ENDED); }
	bool	IsBattle()	{ return bool(m_emState == CAPTURE_THE_FLAG_STATE_BATTLE); }
	bool	IsReward()	{ return bool(m_emState == CAPTURE_THE_FLAG_STATE_REWARD); }
	void	SetEventState( CAPTURE_THE_FLAG_STATE emState );
	void	DoStateRegister();
	void	DoStateBattle();
	void	DoStateReward();
	void	DoStateEnd();
	void	NotifyTimeToStart( WORD wTime );
	void	DoRegister( WORD wTeam );
	void	DoRevive();
	void	RequestScoreInfo();
	void	ResetScore();

	CAPTURE_THE_FLAG_PLAYER_DATA*	GetOwnRanking();
	int		GetRankingSize( int nType );
	CAPTURE_THE_FLAG_PLAYER_DATA_VEC&	GetRankingVector( int nType );
	void	RequestMapExit();

public:
	void	FlagActionCapture( DWORD dwCharID, EMNPC_CTF_FLAG_TYPE	emCTFType, DWORD dwGlobID );
	
	
public:
	GLPVPCaptureTheFlagClient()
		: m_emState(CAPTURE_THE_FLAG_STATE_ENDED)
		, m_fRemain(0.0f)
		, m_wLevelReq(1)
		, m_wPlayerLimit(1)
		, m_llContributionReq(0)
		, m_wMaxScore(1)
		, m_wBattleTime(0)
		, m_bRegistered(FALSE)
		, m_wTeam(CAPTURE_THE_FLAG_TEAM_SIZE)
		, m_wLastWinner(CAPTURE_THE_FLAG_TEAM_SIZE)
		, m_bShowStartNotice(FALSE)
	{
		for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
		{
			m_sScore[i].wTeam = i;
			m_sScore[i].Reset();
			m_bFlagHold[i] = FALSE;
			m_sFlagTarget[i].RESET();
		}
	}

	~GLPVPCaptureTheFlagClient()
	{

	}

public:
	static GLPVPCaptureTheFlagClient& GetInstance();

};
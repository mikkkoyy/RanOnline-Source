/*!
 * \file GLPVPCaptureTheFlagAgent.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */
#pragma once

#include "GLPVPCaptureTheFlag.h"

class	GLPVPCaptureTheFlagAgent : public GLPVPCaptureTheFlag
{

public:
	void	LoadEvent( std::string strFile );
	bool	FrameMove ( float fElaps );
	bool	SetMapState ();
	void	SetEventState( CAPTURE_THE_FLAG_STATE emState );
	void	ResetData();
	void	DoStateRegister();
	void	DoStateBattle();
	void	DoStateReward();
	void	DoStateEnd();
	void	NotifyTimeToStart( float fTime );
	void	RequestInfoPC( DWORD dwCharID, BOOL bEventMap );
	void	BroadCastPlayerNum();

public:
	CAPTURE_THE_FLAG_REGISTER_DATA*			PlayerDataGet( DWORD dwCharID );

private:
	BOOL	IsPlayerFull( CAPTURE_THE_FLAG_REGISTER_DATA_MAP& mapData );
	BOOL	InsertRegister( CAPTURE_THE_FLAG_REGISTER_DATA_MAP& mapData, DWORD dwCharID, const char* szName, WORD wClass, WORD wSchool, WORD wTeam );
	BOOL	DeleteRegister( DWORD dwCharID, CAPTURE_THE_FLAG_REGISTER_DATA_MAP& mapData );
	
public:
	void	RequestPlayerRegister( DWORD dwCharID, WORD wLevel, LONGLONG llContri, BOOL bRegister, WORD wTeam );
	void	RequestPlayerCancelRegister( DWORD dwCharID );
	void	PlayerRejoinBattle( DWORD dwCharID );
	void	MapExit( DWORD dwCharID );

private:
	void	ClearPlayerParty();
	void	MapMoveInPlayer();
	void	MapMoveOutPlayer();
	void	QueueMoveIn();
	void	QueueMoveOut();
	void	SendPlayerData();
	void	CheckWinner();
	void	ScoreCheck();
	void	RequestScoreInfo( DWORD dwCharID );

public:
	float	m_fTimer;
	float	m_fRemain;
	float	m_fBrdTimer;
	WORD	m_wLastNotifyTime;
	WORD	m_wLastWinner;

	CAPTURE_THE_FLAG_SCORE_DATA		m_sScore[CAPTURE_THE_FLAG_TEAM_SIZE];
	CAPTURE_THE_FLAG_PLAYER_DATA	m_sLastTop[CAPTURE_THE_FLAG_MINI_RANKING_NUM];

	CAPTURE_THE_FLAG_REGISTER_DATA_VEC		m_vecPlayerMove;
	CAPTURE_THE_FLAG_REGISTER_DATA_MAP		m_mapPlayerDataA;
	CAPTURE_THE_FLAG_REGISTER_DATA_MAP		m_mapPlayerDataB;

	bool		m_bMoveIn;
	bool		m_bMoveOut;
	float		m_fMoveTimer;
	float		m_fMoveTimerTotal;

	BOOL		m_bFlagHold[CAPTURE_THE_FLAG_TEAM_SIZE];

public:
	HRESULT MsgProcess ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

public:
	GLPVPCaptureTheFlagAgent()
		: m_fTimer(0.0f)
		, m_fRemain(0.0f)
		, m_fBrdTimer(0.0f)
		, m_wLastNotifyTime(0)
		, m_wLastWinner( CAPTURE_THE_FLAG_TEAM_SIZE )
		, m_bMoveIn(false)
		, m_bMoveOut(false)
		, m_fMoveTimer(0.0f)
		, m_fMoveTimerTotal(0.0f)
	{
		for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
		{
			m_sScore[i].wTeam = i;
			m_sScore[i].Reset();
			m_bFlagHold[i] = FALSE;
		}
	}

	~GLPVPCaptureTheFlagAgent()
	{

	}


public:
	static GLPVPCaptureTheFlagAgent& GetInstance();

};
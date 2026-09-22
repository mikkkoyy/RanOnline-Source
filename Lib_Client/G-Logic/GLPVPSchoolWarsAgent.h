/*!
 * \file GLPVPSchoolWarsAgent.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */
#pragma once

#include "GLPVPSchoolWars.h"

class	GLPVPSchoolWarsAgent : public GLPVPSchoolWars
{

public:
	void	LoadEvent( std::string strFile );
	bool	FrameMove ( float fElaps );
	bool	SetMapState ();
	void	SetEventState( SCHOOLWARS_STATE emState );
	void	ResetData();
	void	DoStateRegister();
	void	DoStateBattle();
	void	DoStateReward();
	void	DoStateEnd();
	void	NotifyTimeToStart( float fTime );
	void	RequestInfoPC( DWORD dwCharID );
	
public:
	SCHOOLWARS_REGISTER_DATA*			PlayerDataGet( DWORD dwCharID );
	SCHOOLWARS_REGISTER_DATA_QUEUE*		PlayerQueueGet( DWORD dwCharID );

private:
	BOOL	IsPlayerFull( SCHOOLWARS_REGISTER_DATA_MAP& mapData );
	BOOL	IsPlayerReg( SCHOOLWARS_REGISTER_DATA_MAP& mapData, DWORD dwCharID );
	BOOL	IsPlayerQueue( SCHOOLWARS_REGISTER_DATA_QUEUE_VEC& vecData, DWORD dwCharID );
	BOOL	InsertQueue( SCHOOLWARS_REGISTER_DATA_QUEUE_VEC& vecData, DWORD dwCharID, const char* szName, WORD wClass, WORD wSchool );
	BOOL	InsertRegister( SCHOOLWARS_REGISTER_DATA_MAP& mapData, DWORD dwCharID, const char* szName, WORD wClass, WORD wSchool );
	BOOL	DeleteRegister( DWORD dwCharID, SCHOOLWARS_REGISTER_DATA_MAP& mapData );
	BOOL	DeleteQueue( DWORD dwCharID, SCHOOLWARS_REGISTER_DATA_QUEUE_VEC& vecData );
	void	ProcessQueueData( SCHOOLWARS_REGISTER_DATA_MAP& mapData, SCHOOLWARS_REGISTER_DATA_QUEUE_VEC& vecData );
	void	UpdateQueuePosition( SCHOOLWARS_REGISTER_DATA_QUEUE_VEC& vecData );
	void	ProcessQueueDataAll( SCHOOLWARS_REGISTER_DATA_MAP& mapData, SCHOOLWARS_REGISTER_DATA_QUEUE_VEC& vecData );

public:
	void	RequestPlayerRegister( DWORD dwCharID, WORD wLevel, LONGLONG llContri, BOOL bRegister );
	void	RequestPlayerCancelRegister( DWORD dwCharID );
	void	PlayerRegister( DWORD dwCharID, SCHOOLWARS_REGISTER_DATA_MAP& mapData, SCHOOLWARS_REGISTER_DATA_QUEUE_VEC& vecData );
	void	CancelRegister( DWORD dwCharID, SCHOOLWARS_REGISTER_DATA_MAP& mapData, SCHOOLWARS_REGISTER_DATA_QUEUE_VEC& vecData );
	void	PlayerRejoinBattle( DWORD dwCharID );
	void	MapExit( DWORD dwCharID );

private:
	void	MapMoveInPlayer();
	void	MapMoveOutPlayer();
	void	QueueMoveIn();
	void	QueueMoveOut();
	void	SendPlayerData();
	void	CheckWinner();

private:
	void	ScoreCheck();
	void	RequestScoreInfo( DWORD dwCharID );
	void	ScoreUpdate();

public:
	float	m_fTimer;
	float	m_fRemain;
	WORD	m_wLastNotifyTime;
	WORD	m_wLastWinner;

	SCHOOLWARS_PLAYER_DATA m_sLastTop[SCHOOLWARS_MINI_RANKING_NUM];

	SCHOOLWARS_REGISTER_DATA_VEC		m_vecPlayerMove;
	SCHOOLWARS_REGISTER_DATA_MAP		m_mapPlayerDataSG;
	SCHOOLWARS_REGISTER_DATA_MAP		m_mapPlayerDataMP;
	SCHOOLWARS_REGISTER_DATA_MAP		m_mapPlayerDataPHX;

	SCHOOLWARS_REGISTER_DATA_QUEUE_VEC	m_vecPlayerDataQueueSG;
	SCHOOLWARS_REGISTER_DATA_QUEUE_VEC	m_vecPlayerDataQueueMP;
	SCHOOLWARS_REGISTER_DATA_QUEUE_VEC	m_vecPlayerDataQueuePHX;

	SCHOOLWARS_SCORE_DATA		m_sScore[SCHOOLWARS_SCHOOL_SIZE];

	bool		m_bMoveIn;
	bool		m_bMoveOut;
	float		m_fMoveTimer;
	float		m_fMoveTimerTotal;

	float		m_fScoreUpdate;
	float		m_fDoublePoint;
	float		m_fDoublePointInterval;
	bool		m_bDoublePoint;

public:
	HRESULT MsgProcess ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

public:
	GLPVPSchoolWarsAgent()
		: m_fTimer(0.0f)
		, m_fRemain(0.0f)
		, m_wLastNotifyTime(0)
		, m_wLastWinner( SCHOOLWARS_SCHOOL_SIZE )
		, m_bMoveIn(false)
		, m_bMoveOut(false)
		, m_fMoveTimer(0.0f)
		, m_fMoveTimerTotal(0.0f)
		, m_fScoreUpdate(0.0f)
		, m_fDoublePoint(0.0f)
		, m_fDoublePointInterval(0.0f)
		, m_bDoublePoint(false)
	{
		for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++i )
		{
			m_sScore[i].Reset();
			m_sScore[i].wSchool = i;
		}
	}

	~GLPVPSchoolWarsAgent()
	{

	}


public:
	static GLPVPSchoolWarsAgent& GetInstance();

};
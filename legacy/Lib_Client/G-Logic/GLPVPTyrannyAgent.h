/*!
 * \file GLPVPTyrannyAgent.h
 *
 * \author Juver
 * \date June 2017
 *
 * 
 */

#pragma once

#include "GLPVPTyranny.h"

class	GLPVPTyrannyAgent : public GLPVPTyranny
{

public:
	void	LoadEvent( std::string strFile );
	bool	FrameMove ( float fElaps );
	bool	SetMapState ();
	void	SetEventState( TYRANNY_STATE emState );
	void	ResetData();
	void	DoStateRegister();
	void	DoStateBattle();
	void	DoStateReward();
	void	DoStateEnd();
	void	NotifyTimeToStart( float fTime );

	/*global buffs map setting, Juver, 2018/01/23 */
	void	RequestInfoPC( DWORD dwCharID, BOOL bTyrannyMap, BOOL bDisableSystemBuffs );

	/*dmk14 randomized buff*/
	void	RandomizeSystemBuff();
public:
	TYRANNY_REGISTER_DATA*			PlayerDataGet( DWORD dwCharID );
	TYRANNY_REGISTER_DATA_QUEUE*	PlayerQueueGet( DWORD dwCharID );

private:
	BOOL	IsPlayerFull( TYRANNY_REGISTER_DATA_MAP& mapData );
	BOOL	IsPlayerReg( TYRANNY_REGISTER_DATA_MAP& mapData, DWORD dwCharID );
	BOOL	IsPlayerQueue( TYRANNY_REGISTER_DATA_QUEUE_VEC& vecData, DWORD dwCharID );
	BOOL	InsertQueue( TYRANNY_REGISTER_DATA_QUEUE_VEC& vecData, DWORD dwCharID, const char* szName, WORD wClass, WORD wSchool );
	BOOL	InsertRegister( TYRANNY_REGISTER_DATA_MAP& mapData, DWORD dwCharID, const char* szName, WORD wClass, WORD wSchool );
	BOOL	DeleteRegister( DWORD dwCharID, TYRANNY_REGISTER_DATA_MAP& mapData );
	BOOL	DeleteQueue( DWORD dwCharID, TYRANNY_REGISTER_DATA_QUEUE_VEC& vecData );
	void	ProcessQueueData( TYRANNY_REGISTER_DATA_MAP& mapData, TYRANNY_REGISTER_DATA_QUEUE_VEC& vecData );
	void	UpdateQueuePosition( TYRANNY_REGISTER_DATA_QUEUE_VEC& vecData );
	void	ProcessQueueDataAll( TYRANNY_REGISTER_DATA_MAP& mapData, TYRANNY_REGISTER_DATA_QUEUE_VEC& vecData );

public:
	void	RequestPlayerRegister( DWORD dwCharID, WORD wLevel, BOOL bRegister );
	void	RequestPlayerCancelRegister( DWORD dwCharID );
	void	PlayerRegister( DWORD dwCharID, TYRANNY_REGISTER_DATA_MAP& mapData, TYRANNY_REGISTER_DATA_QUEUE_VEC& vecData );
	void	CancelRegister( DWORD dwCharID, TYRANNY_REGISTER_DATA_MAP& mapData, TYRANNY_REGISTER_DATA_QUEUE_VEC& vecData );
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
	void	TowerCapture( SNATIVEID sidMob, WORD wSchool );
	void	RequestTowerInfo( DWORD dwCharID );
	void	CalculateBattleBuff();

public:
	float	m_fTimer;
	float	m_fRemain;
	WORD	m_wLastNotifyTime;
	WORD	m_wLastWinner;

	TYRANNY_PLAYER_DATA m_sLastTop[TYRANNY_MINI_RANKING_NUM];

	TYRANNY_REGISTER_DATA_VEC		m_vecPlayerMove;
	TYRANNY_REGISTER_DATA_MAP		m_mapPlayerDataSG;
	TYRANNY_REGISTER_DATA_MAP		m_mapPlayerDataMP;
	TYRANNY_REGISTER_DATA_MAP		m_mapPlayerDataPHX;

	TYRANNY_REGISTER_DATA_QUEUE_VEC	m_vecPlayerDataQueueSG;
	TYRANNY_REGISTER_DATA_QUEUE_VEC	m_vecPlayerDataQueueMP;
	TYRANNY_REGISTER_DATA_QUEUE_VEC	m_vecPlayerDataQueuePHX;

	TYRANNY_SCORE_DATA		m_sTowerScore[TYRANNY_SCHOOL_SIZE];

	bool		m_bMoveIn;
	bool		m_bMoveOut;
	float		m_fMoveTimer;
	float		m_fMoveTimerTotal;

	WORD		m_wBattleBuffLevelIndex[TYRANNY_SCHOOL_SIZE]; 

public:
	HRESULT MsgProcess ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

public:
	GLPVPTyrannyAgent()
		: m_fTimer(0.0f)
		, m_fRemain(0.0f)
		, m_wLastNotifyTime(0)
		, m_wLastWinner( TYRANNY_SCHOOL_SIZE )
		, m_bMoveIn(false)
		, m_bMoveOut(false)
		, m_fMoveTimer(0.0f)
		, m_fMoveTimerTotal(0.0f)
	{
		for( int i=0; i<TYRANNY_SCHOOL_SIZE; ++i )
		{
			m_sTowerScore[i].Reset();
			m_sTowerScore[i].wSchool = i;
			m_wBattleBuffLevelIndex[i] = 0;
		}
	}

	~GLPVPTyrannyAgent()
	{

	}


public:
	static GLPVPTyrannyAgent& GetInstance();

};
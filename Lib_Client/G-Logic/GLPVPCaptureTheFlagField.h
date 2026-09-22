/*!
 * \file GLPVPCaptureTheFlagField.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */

#pragma once

#include "GLPVPCaptureTheFlag.h"

class	GLPVPCaptureTheFlagField : public GLPVPCaptureTheFlag
{

public:
	void	LoadEvent( std::string strFile );
	bool	FrameMove ( float fElaps );
	bool	SetMapState ();

	void	ResetData();
	void	DoStateRegister();
	void	DoStateBattle();
	void	DoStateReward( WORD wTeamWinner );
	void	DoStateEnd();
	void	RequestInfoPC( DWORD dwCharID, BOOL bEventMap );

	CAPTURE_THE_FLAG_PLAYER_DATA*		PlayerDataGet( DWORD dwCharID );

private:
	void	InsertPlayerData( CAPTURE_THE_FLAG_REGISTER_DATA* pPlayerData );
	void	PlayerRevive( DWORD dwGaeaID, DWORD dwCharID );

public:
	void	ScoreKill( DWORD dwCharKilled, DWORD dwCharKiller );
	void	ScoreHeal( DWORD dwChar, DWORD dwHeal );
	void	ScoreResu( DWORD dwChar );
	void	ScoreCalculate( CAPTURE_THE_FLAG_PLAYER_DATA_MAP& mapData, WORD wTeamWinner );

	void	CalculateRankingTeam();
	void	CalculateRankingAll();
	void	RankingSend();
	void	GetRewards( WORD wTeamWinner );
	void	RewardSend();
	void	PointSend();
	void	CheckRewards();

public:
	void	FlagCrowSetHide( WORD wTeam, bool bHide );
	void	FlagToHold( DWORD dwCharID, DWORD dwGlobID );
	void	FlagToBase( DWORD dwCharID, DWORD dwGateID );
	void	FlagTimeOut( WORD wTeam );
	void	FlagResetBase( DWORD dwCharID, CAPTURE_THE_FLAG_FLAG_HOLD_STATUS emStatus );

	void	FlagLocationUpdate();

public:
	HRESULT MsgProcess ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

private:
	CAPTURE_THE_FLAG_PLAYER_DATA_MAP	m_mapPlayerDataA;
	CAPTURE_THE_FLAG_PLAYER_DATA_MAP	m_mapPlayerDataB;
	CAPTURE_THE_FLAG_REWARD_MAP			m_mapRewarded;
	BOOL								m_bValidInstance;

	BOOL		m_bFlagHold[CAPTURE_THE_FLAG_TEAM_SIZE];
	float		m_fHoldTimer[CAPTURE_THE_FLAG_TEAM_SIZE];
	STARGETID	m_sFlagTarget[CAPTURE_THE_FLAG_TEAM_SIZE];

	float		m_fHolderTimeCheck;
	float		m_fFlagLocationUpdate;

public:
	GLPVPCaptureTheFlagField()
		: m_bValidInstance(FALSE)
		, m_fHolderTimeCheck(0.0f)
		, m_fFlagLocationUpdate(0.0f)
	{
		for ( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
		{
			m_bFlagHold[i] = FALSE;
			m_fHoldTimer[i] = 0.0f;
			m_sFlagTarget[i].RESET();
		}
	}

	~GLPVPCaptureTheFlagField()
	{

	}


public:
	static GLPVPCaptureTheFlagField& GetInstance();
};

/*!
 * \file GLPVPTyrannyField.h
 *
 * \author Juver
 * \date June 2017
 *
 * 
 */

#pragma once

#include "GLPVPTyranny.h"

class	GLPVPTyrannyField : public GLPVPTyranny
{

public:
	void	LoadEvent( std::string strFile );
	bool	FrameMove ( float fElaps );
	bool	SetMapState ();

	void	ResetData();
	void	DoStateRegister();
	void	DoStateBattle();
	void	DoStateReward( WORD wSchoolWinner );
	void	DoStateEnd();
	/*global buffs map setting, Juver, 2018/01/23 */
	void	RequestInfoPC( DWORD dwCharID, BOOL bTyrannyMap, BOOL bDisableSystemBuffs );

	TYRANNY_PLAYER_DATA*		PlayerDataGet( DWORD dwCharID );

private:
	void	InsertPlayerData( TYRANNY_REGISTER_DATA* pPlayerData );
	void	PlayerRevive( DWORD dwGaeaID, DWORD dwCharID );

public:
	void	TowerCapture( SNATIVEID sidMob, WORD wSchool );

public:
	void	ScoreKill( DWORD dwCharKilled, DWORD dwCharKiller );
	void	ScoreDamage( DWORD dwChar, DWORD dwDamage );
	void	ScoreHeal( DWORD dwChar, DWORD dwHeal );
	void	ScoreResu( DWORD dwChar );
	void	ScoreCalculate( TYRANNY_PLAYER_DATA_MAP& mapData, WORD wSchoolWinner );

	void	CalculateRankingSchool();
	void	CalculateRankingAll();
	void	RankingSend();
	void	GetRewards( WORD wSchoolWinner );
	void	RewardSend();
	void	PointSend();
	void	CheckRewards();

public:
	HRESULT MsgProcess ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID );

private:
	TYRANNY_PLAYER_DATA_MAP			m_mapPlayerDataSG;
	TYRANNY_PLAYER_DATA_MAP			m_mapPlayerDataMP;
	TYRANNY_PLAYER_DATA_MAP			m_mapPlayerDataPHX;
	TYRANNY_REWARD_MAP				m_mapRewarded;
	BOOL							m_bValidInstance;

public:
	GLPVPTyrannyField()
		: m_bValidInstance(FALSE)
	{
		
	}

	~GLPVPTyrannyField()
	{

	}


public:
	static GLPVPTyrannyField& GetInstance();
};

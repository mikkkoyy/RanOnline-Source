/*!
 * \file GLPVPCaptureTheFlag.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */

#pragma once

#include "GLPVPCaptureTheFlagData.h"

class DxLandGate;

class	GLPVPCaptureTheFlag
{
public:
	CAPTURE_THE_FLAG_STATE	m_emState;
	SNATIVEID			m_sMap;
	
	WORD	m_wRegisterTime;
	WORD	m_wBattleTime;
	WORD	m_wRewardTime;

	WORD	m_wLevelReq;
	WORD	m_wPlayerLimit;
	LONGLONG m_llContributionReq;

	WORD	m_wMaxScore;
	WORD	m_wBalancer;
	WORD	m_wTeamGate[CAPTURE_THE_FLAG_TEAM_SIZE];
	WORD	m_wReviveGate[CAPTURE_THE_FLAG_TEAM_SIZE];

	SNATIVEID	m_sOutMap;
	WORD		m_wOutGate;

	float		m_fFlagVar;
	float		m_fKillVar;
	float		m_fHealVar;
	float		m_fResuVar;

	float		m_fPointVar;
	WORD		m_wBasePointWin;
	WORD		m_wBasePointLoss;
	WORD		m_wMaxRewardPoint;

	SNATIVEID	m_sItemRewardWin;
	WORD		m_wRewardLimitScoreWin;

	SNATIVEID	m_sItemRewardLost;
	WORD		m_wRewardLimitScoreLost;

public:
	CAPTURE_THE_FLAG_SCHED_NEXT	m_sScheduleNext;	
	CAPTURE_THE_FLAG_SCHED_VEC	m_vecSchedule;

	SNATIVEID	m_sFlagCrow[CAPTURE_THE_FLAG_TEAM_SIZE];

public:
	bool Load( std::string strFile );
	bool IsRegister() { return bool(m_emState == CAPTURE_THE_FLAG_STATE_REGISTER); }
	bool IsEnded()	{ return bool(m_emState == CAPTURE_THE_FLAG_STATE_ENDED); }
	bool IsBattle()	{ return bool(m_emState == CAPTURE_THE_FLAG_STATE_BATTLE); }
	bool IsReward()	{ return bool(m_emState == CAPTURE_THE_FLAG_STATE_REWARD); }
	
public:
	//scheduler function
	void	FindScheduleFirst();
	bool	FindScheduleNext();

	

public:
	GLPVPCaptureTheFlag()
		: m_emState(CAPTURE_THE_FLAG_STATE_ENDED)
		, m_sMap(false)
		, m_wRegisterTime(0)
		, m_wBattleTime(0)
		, m_wRewardTime(0)
		, m_wLevelReq(1)
		, m_wPlayerLimit(1)
		, m_llContributionReq(0)
		, m_wMaxScore(0)
		, m_wBalancer(0)
		, m_sOutMap(false)
		, m_wOutGate(0)
		, m_fFlagVar(1.0f)
		, m_fKillVar(1.0f)
		, m_fHealVar(0.1f)
		, m_fResuVar(1.0f)
		, m_sItemRewardWin(false)
		, m_wRewardLimitScoreWin(10)
		, m_sItemRewardLost(false)
		, m_wRewardLimitScoreLost(10)
		, m_fPointVar(0.0f)
		, m_wBasePointWin(0)
		, m_wBasePointLoss(0)
		, m_wMaxRewardPoint(0)
	{
		for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
		{
			m_wTeamGate[i] = 0;
			m_sFlagCrow[i] = NATIVEID_NULL();

			m_wReviveGate[i] = 0;
		}
	}

	virtual ~GLPVPCaptureTheFlag()
	{

	}
};
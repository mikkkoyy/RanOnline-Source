/*!
 * \file GLPVPTyranny.h
 *
 * \author Juver
 * \date June 2017
 *
 * 
 */

#pragma once

#include "GLPVPTyrannyData.h"

class DxLandGate;

class	GLPVPTyranny
{
public:
	TYRANNY_STATE		m_emState;
	SNATIVEID			m_sMap;
	
	WORD	m_wRegisterTime;
	WORD	m_wBattleTime;
	WORD	m_wRewardTime;

	WORD	m_wLevelReq;
	WORD	m_wPlayerLimit;
	WORD	m_wSchoolGate[TYRANNY_SCHOOL_SIZE];

	DWORD	m_dwDamageCapture;
	DWORD	m_dwDamageBase;

	SNATIVEID	m_sOutMap;
	WORD		m_wOutGate;

	float		m_fDamageVar;
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

	TYRANNY_TOWER_DATA	m_sTowerData[TYRANNY_TOWER_SIZE];

	BOOL					m_bRewardBuff;
	BOOL					m_bBattleBuff;
	TYRANNY_BUFF_DATA		m_sRewardBuffMain;
	TYRANNY_BUFF_DATA		m_sRewardBuff[TYRANNY_REWARD_BUFF_NUM];
	TYRANNY_BUFF_DATA2		m_sBattleBuff[TYRANNY_BATTLE_BUFF_NUM];

public:
	TYRANNY_SCHED_NEXT	m_sScheduleNext;	
	TYRANNY_SCHED_VEC	m_vecSchedule;


public:
	bool Load( std::string strFile );
	bool IsRegister() { return bool(m_emState == TYRANNY_STATE_REGISTER); }
	bool IsEnded()	{ return bool(m_emState == TYRANNY_STATE_ENDED); }
	bool IsBattle()	{ return bool(m_emState == TYRANNY_STATE_BATTLE); }
	bool IsReward()	{ return bool(m_emState == TYRANNY_STATE_REWARD); }
	
public:
	//scheduler function
	void	FindScheduleFirst();
	bool	FindScheduleNext();

	

public:
	GLPVPTyranny()
		: m_emState(TYRANNY_STATE_ENDED)
		, m_sMap(false)
		, m_wRegisterTime(0)
		, m_wBattleTime(0)
		, m_wRewardTime(0)
		, m_wLevelReq(1)
		, m_wPlayerLimit(1)
		, m_sOutMap(false)
		, m_wOutGate(0)
		, m_dwDamageCapture(2000)
		, m_dwDamageBase(1000)
		, m_fDamageVar(0.1f)
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
		, m_bRewardBuff(FALSE)
		, m_bBattleBuff(FALSE)
	{
		for( int i=0; i<TYRANNY_SCHOOL_SIZE; ++i )
			m_wSchoolGate[i] = 0;

		for( int i=0; i<TYRANNY_TOWER_SIZE; ++i )
			m_sTowerData[i] = TYRANNY_TOWER_DATA();

		m_sRewardBuffMain = TYRANNY_BUFF_DATA();

		for( int i=0; i<TYRANNY_REWARD_BUFF_NUM; ++i )
			m_sRewardBuff[i] = TYRANNY_BUFF_DATA();

		for( int i=0; i<TYRANNY_BATTLE_BUFF_NUM; ++i )
			m_sBattleBuff[i] = TYRANNY_BUFF_DATA2();
	}

	virtual ~GLPVPTyranny()
	{

	}
};
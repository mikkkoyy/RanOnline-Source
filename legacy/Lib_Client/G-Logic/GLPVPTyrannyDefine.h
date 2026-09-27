/*!
 * \file GLPVPTyrannyDefine.h
 *
 * \author Juver
 * \date July 2017
 *
 * 
 */

#pragma once

#define TYRANNY_SCHEDULE_NONE			UINT_MAX
#define TYRANNY_CHARNAME_STRING_SIZE	33			//similar to CHAR_SZNAME
#define TYRANNY_PLAYER_NULL				0xFFFFFFFF	//similar to GAEAID_NULL
#define TYRANNY_MINI_RANKING_NUM		10			//top rankings size
#define TYRANNY_REVIVE_TIME				15.0f		//revive time
#define TYRANNY_PACKET_PLAYER_DATA_NUM	32			//agent to field player data
#define TYRANNY_PACKET_RANK_NUM			26			//field to client ranking data
#define TYRANNY_DRAIN_FACTOR			0.5f

#define TYRANNY_REWARD_BUFF_NUM	4
#define TYRANNY_BATTLE_BUFF_NUM 2
#define TYRANNY_BATTLE_BUFF_LEVEL_NUM 10

/*dmk14 randomized buff*/
typedef std::set<DWORD>					VEC_SYSTEM;
typedef VEC_SYSTEM::iterator			VEC_ITER_SYSTEM;

enum TYRANNY_STATE 
{
	TYRANNY_STATE_ENDED		= 0,
	TYRANNY_STATE_REGISTER	= 1,
	TYRANNY_STATE_BATTLE	= 2,
	TYRANNY_STATE_REWARD	= 3,
	TYRANNY_STATE_SIZE		= 4,
};

enum 
{
	TYRANNY_SCHOOL_SG	= 0,
	TYRANNY_SCHOOL_MP	= 1,
	TYRANNY_SCHOOL_PHX	= 2,
	TYRANNY_SCHOOL_SIZE	= 3,
};

enum 
{
	TYRANNY_TOWER_0	= 0,
	TYRANNY_TOWER_1	= 1,
	TYRANNY_TOWER_2	= 2,
	TYRANNY_TOWER_SIZE	= 3,
};
/*!
 * \file GLPVPSchoolWarsDefine.h
 *
 * \author Juver
 * \date January 2018
 *
 * 
 */

#pragma once

#define SCHOOLWARS_SCHEDULE_NONE			UINT_MAX
#define SCHOOLWARS_CHARNAME_STRING_SIZE		33			//similar to CHAR_SZNAME
#define SCHOOLWARS_PLAYER_NULL				0xFFFFFFFF	//similar to GAEAID_NULL
#define SCHOOLWARS_MINI_RANKING_NUM			10			//top rankings size
#define SCHOOLWARS_REVIVE_TIME				10.0f		//revive time
#define SCHOOLWARS_PACKET_PLAYER_DATA_NUM	32			//agent to field player data
#define SCHOOLWARS_PACKET_RANK_NUM			26			//field to client ranking data
#define SCHOOLWARS_SCORE_UPDATE_TIME		5.0f		//time to send score
#define SCHOOLWARS_DOUBLE_POINT_TIME_DURATION		10.0f		//double point time
#define SCHOOLWARS_DOUBLE_POINT_TIME_INTERVAL		600.0f		//double point time

enum SCHOOLWARS_STATE 
{
	SCHOOLWARS_STATE_ENDED		= 0,
	SCHOOLWARS_STATE_REGISTER	= 1,
	SCHOOLWARS_STATE_BATTLE		= 2,
	SCHOOLWARS_STATE_REWARD		= 3,
	SCHOOLWARS_STATE_SIZE		= 4,
};

enum 
{
	SCHOOLWARS_SCHOOL_SG	= 0,
	SCHOOLWARS_SCHOOL_MP	= 1,
	SCHOOLWARS_SCHOOL_PHX	= 2,
	SCHOOLWARS_SCHOOL_SIZE	= 3,
};


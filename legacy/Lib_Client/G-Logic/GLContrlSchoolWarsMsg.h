#pragma once

#include "GLContrlBaseMsg.h"
#include "GLContrlSchoolWarsMsgEnum.h"
#include "GLPVPSchoolWarsData.h"

namespace GLMSG
{
	#pragma pack(1)

	struct SNET_SCHOOLWARS_A2FC_STATE_REGISTER	
	{
		NET_MSG_GENERIC			nmg;
		float		fTime;

		SNET_SCHOOLWARS_A2FC_STATE_REGISTER () 
			: fTime(0.0f)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_A2FC_STATE_REGISTER;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};
	
	struct SNET_SCHOOLWARS_A2FC_STATE_BATTLE
	{
		NET_MSG_GENERIC			nmg;
		float		fTime;
		WORD		wPlayerNum[SCHOOLWARS_SCHOOL_SIZE];

		SNET_SCHOOLWARS_A2FC_STATE_BATTLE () 
			: fTime(0.0f)
		{
			for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++i )
				wPlayerNum[i] = 0;

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_A2FC_STATE_BATTLE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_SCHOOLWARS_A2FC_STATE_REWARD
	{
		NET_MSG_GENERIC			nmg;
		float		fTime;
		WORD		wWinnerSchool;

		SNET_SCHOOLWARS_A2FC_STATE_REWARD () 
			: fTime(0.0f)
			, wWinnerSchool(SCHOOLWARS_SCHOOL_SIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_A2FC_STATE_REWARD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_SCHOOLWARS_A2FC_STATE_ENDED
	{
		NET_MSG_GENERIC			nmg;

		SNET_SCHOOLWARS_A2FC_STATE_ENDED () 
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_A2FC_STATE_ENDED;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_SCHOOLWARS_A2C_TOBATTLE_TIME
	{
		NET_MSG_GENERIC			nmg;
		WORD					wTime;

		SNET_SCHOOLWARS_A2C_TOBATTLE_TIME () 
			: wTime(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_A2C_TOBATTLE_TIME;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_SCHOOLWARS_A2FC_NEXTSCHED
	{
		NET_MSG_GENERIC			nmg;
		SCHOOLWARS_SCHED_NEXT		sScheduleNext;

		SNET_SCHOOLWARS_A2FC_NEXTSCHED () 
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_A2FC_NEXTSCHED;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_SCHOOLWARS_F2A_BATTLEINFO_PC
	{
		NET_MSG_GENERIC			nmg;
		DWORD		dwCharID;
	
		SNET_SCHOOLWARS_F2A_BATTLEINFO_PC () 
			: dwCharID(GAEAID_NULL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_F2A_BATTLEINFO_PC;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_SCHOOLWARS_A2C_BATTLEINFO_PC
	{
		NET_MSG_GENERIC			nmg;
		SCHOOLWARS_SCHED_NEXT	sScheduleNext;
		SCHOOLWARS_STATE		emState;
		float					fRemain;
		WORD					wLevelReq;
		LONGLONG				llContriReq;
		WORD					wPlayerLimit;
		WORD					wMaxScore;
		WORD					wBattleTime;
		bool					bRegistered;
		bool					bQueued;
		WORD					wQueueNum;

		SNET_SCHOOLWARS_A2C_BATTLEINFO_PC () 
			: emState(SCHOOLWARS_STATE_ENDED)
			, fRemain(0.0f)
			, wLevelReq(0)
			, llContriReq(0)
			, wPlayerLimit(0)
			, wMaxScore(0)
			, wBattleTime(0)
			, bRegistered(false)
			, bQueued(false)
			, wQueueNum(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_A2C_BATTLEINFO_PC;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_SCHOOLWARS_C2A_REGISTER_REQ
	{
		NET_MSG_GENERIC		nmg;
		DWORD	dwCharID;
		WORD	wLevel;
		LONGLONG llContri;
		BOOL	bRegister;

		SNETPC_SCHOOLWARS_C2A_REGISTER_REQ () 
			: dwCharID(SCHOOLWARS_PLAYER_NULL)
			, wLevel(1)
			, llContri(0)
			, bRegister(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_C2A_REGISTER_REQ;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_SCHOOLWARS_A2C_REGISTER_FB
	{
		NET_MSG_GENERIC		nmg;
		SCHOOLWARS_REGISTER_FB emFB;
		WORD				wQueueNum;

		SNETPC_SCHOOLWARS_A2C_REGISTER_FB()
			: emFB(SCHOOLWARS_REGISTER_FB_FAILED)
			, wQueueNum(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_A2C_REGISTER_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	struct SNETPC_SCHOOLWARS_A2C_QUEUE_MOVED
	{
		NET_MSG_GENERIC		nmg;

		SNETPC_SCHOOLWARS_A2C_QUEUE_MOVED ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_A2C_QUEUE_MOVED;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_SCHOOLWARS_A2C_QUEUE_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		WORD				wQueueNum;

		SNETPC_SCHOOLWARS_A2C_QUEUE_UPDATE ()
			: wQueueNum(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_A2C_QUEUE_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	struct SNETPC_SCHOOLWARS_C2A_REJOIN_REQ
	{
		NET_MSG_GENERIC		nmg;
		DWORD	dwCharID;

		SNETPC_SCHOOLWARS_C2A_REJOIN_REQ ()
			: dwCharID(SCHOOLWARS_PLAYER_NULL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_C2A_REJOIN_REQ;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_SCHOOLWARS_A2C_REJOIN_FB
	{
		NET_MSG_GENERIC		nmg;
		SCHOOLWARS_REJOIN_FB	emFB;

		SNETPC_SCHOOLWARS_A2C_REJOIN_FB()
			: emFB(SCHOOLWARS_REJOIN_FB_FAILED)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_A2C_REJOIN_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	struct SNETPC_SCHOOLWARS_A2F_MAP_MOVE
	{
		NET_MSG_GENERIC		nmg;
		SNATIVEID	nidMAP;
		DWORD		dwCharID;
		D3DXVECTOR3	vPos;
		BOOL		bCurMapCheck;

		SNETPC_SCHOOLWARS_A2F_MAP_MOVE () :
			nidMAP(false),
			dwCharID(GAEAID_NULL),
			vPos(0.0f,0.0f,0.0f),
			bCurMapCheck(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_A2F_MAP_MOVE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_SCHOOLWARS_C2A_MAPEXIT_REQ
	{
		NET_MSG_GENERIC		nmg;
		DWORD	dwCharID;

		SNETPC_SCHOOLWARS_C2A_MAPEXIT_REQ ()
			: dwCharID(SCHOOLWARS_PLAYER_NULL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_C2A_MAPEXIT_REQ;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_SCHOOLWARS_A2F_PLAYER_DATA
	{
		NET_MSG_GENERIC		nmg;
		WORD				wRankNum;
		SCHOOLWARS_REGISTER_DATA	sPlayerData[SCHOOLWARS_PACKET_PLAYER_DATA_NUM];

		SNETPC_SCHOOLWARS_A2F_PLAYER_DATA () 
			: wRankNum(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_A2F_PLAYER_DATA;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}

		BOOL ADDRANK ( const SCHOOLWARS_REGISTER_DATA& sRANK )
		{
			if ( wRankNum == SCHOOLWARS_PACKET_PLAYER_DATA_NUM )	return FALSE;
			sPlayerData[wRankNum] = sRANK;
			wRankNum++;

			nmg.dwSize = sizeof(*this) - sizeof(SCHOOLWARS_REGISTER_DATA) * ( SCHOOLWARS_PACKET_PLAYER_DATA_NUM - wRankNum );
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
			return TRUE;
		}

		void Reset()
		{
			wRankNum = 0;
			for( int i=0; i<SCHOOLWARS_PACKET_PLAYER_DATA_NUM; ++i ){
				sPlayerData[i] = SCHOOLWARS_REGISTER_DATA();
			}
		}
	};

	struct SNETPC_SCHOOLWARS_C2F_REVIVE_REQ
	{
		NET_MSG_GENERIC		nmg;
		DWORD	dwCharID;

		SNETPC_SCHOOLWARS_C2F_REVIVE_REQ ()
			: dwCharID(SCHOOLWARS_PLAYER_NULL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_C2F_REVIVE_REQ;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_SCHOOLWARS_C2A_SCORE_INFO_REQ
	{
		NET_MSG_GENERIC		nmg;
		DWORD	dwCharID;

		SNETPC_SCHOOLWARS_C2A_SCORE_INFO_REQ ()
			: dwCharID(SCHOOLWARS_PLAYER_NULL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_C2A_SCORE_INFO_REQ;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_SCHOOLWARS_A2C_SCORE_INFO
	{
		NET_MSG_GENERIC		nmg;
		SCHOOLWARS_SCORE_DATA	sScore[SCHOOLWARS_SCHOOL_SIZE];
		WORD	wLastWinner;
		float	fDoublePoint;
		bool	bDoublePoint;

		SNETPC_SCHOOLWARS_A2C_SCORE_INFO ()
			: wLastWinner(SCHOOLWARS_SCHOOL_SIZE)
			, fDoublePoint(0.0f)
			, bDoublePoint(false)
		{
			for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++ i )
				sScore[i].Reset();

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_A2C_SCORE_INFO;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	struct SNETPC_SCHOOLWARS_F2A_SCORE_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		WORD		wSchoolKilled;
		WORD		wSchoolKiller;
		DWORD		dwCharID;

		SNETPC_SCHOOLWARS_F2A_SCORE_UPDATE ()
			: wSchoolKilled(SCHOOLWARS_SCHOOL_SIZE)
			, wSchoolKiller(SCHOOLWARS_SCHOOL_SIZE)
			, dwCharID(SCHOOLWARS_PLAYER_NULL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_F2A_SCORE_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_SCHOOLWARS_A2F_SCORE_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		SCHOOLWARS_SCORE_DATA	sScore[SCHOOLWARS_SCHOOL_SIZE];

		SNETPC_SCHOOLWARS_A2F_SCORE_UPDATE ()
		{
			for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++ i )
				sScore[i].Reset();

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_A2F_SCORE_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_SCHOOLWARS_F2C_SCORE_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		SCHOOLWARS_SCORE_DATA	sScore[SCHOOLWARS_SCHOOL_SIZE];

		SNETPC_SCHOOLWARS_F2C_SCORE_UPDATE ()
		{
			for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++ i )
				sScore[i].Reset();

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_F2C_SCORE_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};
	

	struct SNETPC_SCHOOLWARS_A2C_RANKINFO_PC
	{
		NET_MSG_GENERIC		nmg;
		WORD					wLastWinner;
		SCHOOLWARS_PLAYER_DATA	sPlayerData[SCHOOLWARS_MINI_RANKING_NUM];

		SNETPC_SCHOOLWARS_A2C_RANKINFO_PC () 
			: wLastWinner(SCHOOLWARS_SCHOOL_SIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_A2C_RANKINFO_PC;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	struct SNETPC_SCHOOLWARS_A2F_SCORE_WINNER
	{
		NET_MSG_GENERIC		nmg;
		WORD	wLastWinner;
		BOOL	bFullScore;

		SNETPC_SCHOOLWARS_A2F_SCORE_WINNER ()
			: wLastWinner(SCHOOLWARS_SCHOOL_SIZE)
			, bFullScore(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_A2F_SCORE_WINNER;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_SCHOOLWARS_F2C_SCORE_WINNER
	{
		NET_MSG_GENERIC		nmg;
		WORD	wLastWinner;
		BOOL	bFullScore;

		SNETPC_SCHOOLWARS_F2C_SCORE_WINNER ()
			: wLastWinner(SCHOOLWARS_SCHOOL_SIZE)
			, bFullScore(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_F2C_SCORE_WINNER;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	struct SNETPC_SCHOOLWARS_F2C_RANKING_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		WORD				wRankNum;
		SCHOOLWARS_PLAYER_DATA	sPlayerData[SCHOOLWARS_PACKET_RANK_NUM];

		SNETPC_SCHOOLWARS_F2C_RANKING_UPDATE () 
			: wRankNum(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_F2C_RANKING_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}

		BOOL ADDRANK ( const SCHOOLWARS_PLAYER_DATA& sRANK )
		{
			if ( wRankNum == SCHOOLWARS_PACKET_RANK_NUM )	return FALSE;

			sPlayerData[wRankNum] = sRANK;
			wRankNum++;

			nmg.dwSize = sizeof(*this) - sizeof(SCHOOLWARS_PLAYER_DATA) * ( SCHOOLWARS_PACKET_RANK_NUM - wRankNum );
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
			return TRUE;
		}

		void Reset()
		{
			wRankNum = 0;
			for( int i=0; i<SCHOOLWARS_PACKET_RANK_NUM; ++i ){
				sPlayerData[i] = SCHOOLWARS_PLAYER_DATA();
			}
		}
	};

	struct SNETPC_SCHOOLWARS_F2C_RANKING_END
	{
		NET_MSG_GENERIC		nmg;
		WORD		wPlayerNum[SCHOOLWARS_SCHOOL_SIZE];

		SNETPC_SCHOOLWARS_F2C_RANKING_END ()
		{
			for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++i ){
				wPlayerNum[i] = 0;
			}

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_F2C_RANKING_END;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_SCHOOLWARS_F2A_RANKINFO_PC
	{
		NET_MSG_GENERIC		nmg;
		WORD				wRankNum;
		SCHOOLWARS_PLAYER_DATA	sPlayerData[SCHOOLWARS_MINI_RANKING_NUM];

		SNETPC_SCHOOLWARS_F2A_RANKINFO_PC () 
			: wRankNum(0)
		{
			nmg.dwSize = sizeof(NET_MSG_GENERIC) + sizeof(WORD);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_F2A_RANKINFO_PC;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}

		bool ADDRANK ( const SCHOOLWARS_PLAYER_DATA& sRANK )
		{
			if ( SCHOOLWARS_MINI_RANKING_NUM==wRankNum )		return false;

			sPlayerData[wRankNum] = sRANK;

			++wRankNum;

			nmg.dwSize = sizeof(NET_MSG_GENERIC) + sizeof(WORD) + sizeof(SCHOOLWARS_PLAYER_DATA)*wRankNum;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);

			return true;
		}
	};

	struct SNETPC_SCHOOLWARS_A2C_SCORE_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		SCHOOLWARS_SCORE_DATA	sScore[SCHOOLWARS_SCHOOL_SIZE];

		SNETPC_SCHOOLWARS_A2C_SCORE_UPDATE()
		{
			for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++ i )
				sScore[i].Reset();

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_SCHOOLWARS_A2C_SCORE_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	#pragma pack()
};


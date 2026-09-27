#pragma once

#include "GLContrlBaseMsg.h"
#include "GLContrlTyrannyMsgEnum.h"
#include "GLPVPTyrannyData.h"

namespace GLMSG
{
	#pragma pack(1)

	struct SNET_TYRANNY_A2FC_STATE_REGISTER	
	{
		NET_MSG_GENERIC			nmg;
		float		fTime;

		SNET_TYRANNY_A2FC_STATE_REGISTER () 
			: fTime(0.0f)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2FC_STATE_REGISTER;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_TYRANNY_A2FC_STATE_BATTLE
	{
		NET_MSG_GENERIC			nmg;
		float		fTime;
		WORD		wPlayerNum[TYRANNY_SCHOOL_SIZE];

		SNET_TYRANNY_A2FC_STATE_BATTLE () 
			: fTime(0.0f)
		{
			for( int i=0; i<TYRANNY_SCHOOL_SIZE; ++i )
				wPlayerNum[i] = 0;

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2FC_STATE_BATTLE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_TYRANNY_A2FC_STATE_REWARD
	{
		NET_MSG_GENERIC			nmg;
		float		fTime;
		WORD		wWinnerSchool;

		SNET_TYRANNY_A2FC_STATE_REWARD () 
			: fTime(0.0f)
			, wWinnerSchool(TYRANNY_SCHOOL_SIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2FC_STATE_REWARD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_TYRANNY_A2FC_STATE_ENDED
	{
		NET_MSG_GENERIC			nmg;

		SNET_TYRANNY_A2FC_STATE_ENDED () 
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2FC_STATE_ENDED;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_TYRANNY_A2C_TOBATTLE_TIME
	{
		NET_MSG_GENERIC			nmg;
		WORD					wTime;

		SNET_TYRANNY_A2C_TOBATTLE_TIME () 
			: wTime(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2C_TOBATTLE_TIME;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_TYRANNY_A2FC_NEXTSCHED
	{
		NET_MSG_GENERIC			nmg;
		TYRANNY_SCHED_NEXT		sScheduleNext;

		SNET_TYRANNY_A2FC_NEXTSCHED () 
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2FC_NEXTSCHED;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};
	
	struct SNET_TYRANNY_F2A_BATTLEINFO_PC
	{
		NET_MSG_GENERIC			nmg;
		DWORD		dwCharID;
		BOOL		bTyrannyMap;
		BOOL		bDisableSystemBuffs;	/*global buffs map setting, Juver, 2018/01/23 */

		SNET_TYRANNY_F2A_BATTLEINFO_PC () 
			: dwCharID(GAEAID_NULL)
			, bTyrannyMap(FALSE)
			, bDisableSystemBuffs(FALSE)	/*global buffs map setting, Juver, 2018/01/23 */
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_F2A_BATTLEINFO_PC;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_TYRANNY_A2C_BATTLEINFO_PC
	{
		NET_MSG_GENERIC			nmg;
		TYRANNY_SCHED_NEXT		sScheduleNext;
		TYRANNY_STATE			emState;
		float					fRemain;
		WORD					wLevelReq;
		WORD					wPlayerLimit;
		bool					bRegistered;
		bool					bQueued;
		WORD					wQueueNum;

		SNET_TYRANNY_A2C_BATTLEINFO_PC () 
			: emState(TYRANNY_STATE_ENDED)
			, fRemain(0.0f)
			, wLevelReq(0)
			, wPlayerLimit(0)
			, bRegistered(false)
			, bQueued(false)
			, wQueueNum(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2C_BATTLEINFO_PC;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_C2A_REGISTER_REQ
	{
		NET_MSG_GENERIC		nmg;
		DWORD	dwCharID;
		WORD	wLevel;
		BOOL	bRegister;
		
		SNETPC_TYRANNY_C2A_REGISTER_REQ () 
			: dwCharID(TYRANNY_PLAYER_NULL)
			, wLevel(1)
			, bRegister(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_C2A_REGISTER_REQ;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_A2C_REGISTER_FB
	{
		NET_MSG_GENERIC		nmg;
		TYRANNY_REGISTER_FB emFB;
		WORD				wQueueNum;

		SNETPC_TYRANNY_A2C_REGISTER_FB()
			: emFB(TYRANNY_REGISTER_FB_FAILED)
			, wQueueNum(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2C_REGISTER_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_A2C_QUEUE_MOVED
	{
		NET_MSG_GENERIC		nmg;

		SNETPC_TYRANNY_A2C_QUEUE_MOVED ()
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2C_QUEUE_MOVED;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_A2C_QUEUE_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		WORD				wQueueNum;

		SNETPC_TYRANNY_A2C_QUEUE_UPDATE ()
			: wQueueNum(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2C_QUEUE_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_A2F_MAP_MOVE
	{
		NET_MSG_GENERIC		nmg;
		SNATIVEID	nidMAP;
		DWORD		dwCharID;
		D3DXVECTOR3	vPos;
		BOOL		bCurMapCheck;

		SNETPC_TYRANNY_A2F_MAP_MOVE () :
			nidMAP(false),
			dwCharID(GAEAID_NULL),
			vPos(0.0f,0.0f,0.0f),
			bCurMapCheck(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2F_MAP_MOVE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_C2A_REJOIN_REQ
	{
		NET_MSG_GENERIC		nmg;
		DWORD	dwCharID;

		SNETPC_TYRANNY_C2A_REJOIN_REQ ()
			: dwCharID(TYRANNY_PLAYER_NULL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_C2A_REJOIN_REQ;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_A2C_REJOIN_FB
	{
		NET_MSG_GENERIC		nmg;
		TYRANNY_REJOIN_FB	emFB;

		SNETPC_TYRANNY_A2C_REJOIN_FB()
			: emFB(TYRANNY_REJOIN_FB_FAILED)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2C_REJOIN_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_C2A_MAPEXIT_REQ
	{
		NET_MSG_GENERIC		nmg;
		DWORD	dwCharID;

		SNETPC_TYRANNY_C2A_MAPEXIT_REQ ()
			: dwCharID(TYRANNY_PLAYER_NULL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_C2A_MAPEXIT_REQ;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_A2F_PLAYER_DATA
	{
		NET_MSG_GENERIC		nmg;
		WORD				wRankNum;
		TYRANNY_REGISTER_DATA	sPlayerData[TYRANNY_PACKET_PLAYER_DATA_NUM];

		SNETPC_TYRANNY_A2F_PLAYER_DATA () 
			: wRankNum(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2F_PLAYER_DATA;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}

		BOOL ADDRANK ( const TYRANNY_REGISTER_DATA& sRANK )
		{
			if ( wRankNum == TYRANNY_PACKET_PLAYER_DATA_NUM )	return FALSE;
			sPlayerData[wRankNum] = sRANK;
			wRankNum++;

			nmg.dwSize = sizeof(*this) - sizeof(TYRANNY_REGISTER_DATA) * ( TYRANNY_PACKET_PLAYER_DATA_NUM - wRankNum );
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
			return TRUE;
		}

		void Reset()
		{
			wRankNum = 0;
			for( int i=0; i<TYRANNY_PACKET_PLAYER_DATA_NUM; ++i ){
				sPlayerData[i] = TYRANNY_REGISTER_DATA();
			}
		}
	};


	struct SNETPC_TYRANNY_C2F_REVIVE_REQ
	{
		NET_MSG_GENERIC		nmg;
		DWORD	dwCharID;

		SNETPC_TYRANNY_C2F_REVIVE_REQ ()
			: dwCharID(TYRANNY_PLAYER_NULL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_C2F_REVIVE_REQ;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_F2A_TOWER_CAPTURE
	{
		NET_MSG_GENERIC		nmg;
		SNATIVEID	sidMob;
		WORD		wSchool;

		SNETPC_TYRANNY_F2A_TOWER_CAPTURE ()
			: sidMob(NATIVEID_NULL())
			, wSchool(TYRANNY_SCHOOL_SIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_F2A_TOWER_CAPTURE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_A2F_TOWER_CAPTURE
	{
		NET_MSG_GENERIC		nmg;
		WORD				wTowerSchool;
		WORD				wTowerIndex;
		WORD				wTowerScore;

		SNETPC_TYRANNY_A2F_TOWER_CAPTURE ()
			: wTowerSchool(TYRANNY_SCHOOL_SIZE)
			, wTowerIndex(TYRANNY_TOWER_SIZE)
			, wTowerScore(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2F_TOWER_CAPTURE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_F2C_TOWER_CAPTURE
	{
		NET_MSG_GENERIC		nmg;
		WORD				wTowerSchool;
		WORD				wTowerIndex;
		WORD				wTowerScore;

		SNETPC_TYRANNY_F2C_TOWER_CAPTURE ()
			: wTowerSchool(TYRANNY_SCHOOL_SIZE)
			, wTowerIndex(TYRANNY_TOWER_SIZE)
			, wTowerScore(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_F2C_TOWER_CAPTURE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};
	
	struct SNETPC_TYRANNY_C2A_TOWER_OWNER_INFO_REQ
	{
		NET_MSG_GENERIC		nmg;
		DWORD	dwCharID;

		SNETPC_TYRANNY_C2A_TOWER_OWNER_INFO_REQ ()
			: dwCharID(TYRANNY_PLAYER_NULL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_C2A_TOWER_INFO_REQ;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_A2C_TOWER_OWNER_INFO
	{
		NET_MSG_GENERIC		nmg;
		WORD	wTowerSchool[TYRANNY_TOWER_SIZE];
		WORD	wLastWinner;

		SNETPC_TYRANNY_A2C_TOWER_OWNER_INFO ()
			: wLastWinner(TYRANNY_SCHOOL_SIZE)
		{
			for( int i=0; i<TYRANNY_TOWER_SIZE; ++ i )
				wTowerSchool[i] = TYRANNY_SCHOOL_SIZE;

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2C_TOWER_INFO;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_F2A_TOWER_SCORE
	{
		NET_MSG_GENERIC		nmg;
		WORD		wSchoolKilled;
		WORD		wSchoolKiller;

		SNETPC_TYRANNY_F2A_TOWER_SCORE ()
			: wSchoolKilled(TYRANNY_SCHOOL_SIZE)
			, wSchoolKiller(TYRANNY_SCHOOL_SIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_F2A_TOWER_SCORE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	struct SNETPC_TYRANNY_A2F_TOWER_WINNER
	{
		NET_MSG_GENERIC		nmg;
		WORD	wLastWinner;
		BOOL	bFullCapture;

		SNETPC_TYRANNY_A2F_TOWER_WINNER ()
			: wLastWinner(TYRANNY_SCHOOL_SIZE)
			, bFullCapture(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2F_TOWER_WINNER;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_F2C_TOWER_WINNER
	{
		NET_MSG_GENERIC		nmg;
		WORD	wLastWinner;
		BOOL	bFullCapture;

		SNETPC_TYRANNY_F2C_TOWER_WINNER ()
			: wLastWinner(TYRANNY_SCHOOL_SIZE)
			, bFullCapture(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_F2C_TOWER_WINNER;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_F2C_RANKING_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		WORD				wRankNum;
		TYRANNY_PLAYER_DATA	sPlayerData[TYRANNY_PACKET_RANK_NUM];

		SNETPC_TYRANNY_F2C_RANKING_UPDATE () 
			: wRankNum(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_F2C_RANKING_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}

		BOOL ADDRANK ( const TYRANNY_PLAYER_DATA& sRANK )
		{
			if ( wRankNum == TYRANNY_PACKET_RANK_NUM )	return FALSE;

			sPlayerData[wRankNum] = sRANK;
			wRankNum++;

			nmg.dwSize = sizeof(*this) - sizeof(TYRANNY_PLAYER_DATA) * ( TYRANNY_PACKET_RANK_NUM - wRankNum );
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
			return TRUE;
		}

		void Reset()
		{
			wRankNum = 0;
			for( int i=0; i<TYRANNY_PACKET_RANK_NUM; ++i ){
				sPlayerData[i] = TYRANNY_PLAYER_DATA();
			}
		}
	};

	struct SNETPC_TYRANNY_F2C_RANKING_END
	{
		NET_MSG_GENERIC		nmg;
		WORD		wPlayerNum[TYRANNY_SCHOOL_SIZE];

		SNETPC_TYRANNY_F2C_RANKING_END ()
		{
			for( int i=0; i<TYRANNY_SCHOOL_SIZE; ++i ){
				wPlayerNum[i] = 0;
			}
			
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_F2C_RANKING_END;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_F2A_RANKINFO_PC
	{
		NET_MSG_GENERIC		nmg;
		WORD				wRankNum;
		TYRANNY_PLAYER_DATA	sPlayerData[TYRANNY_MINI_RANKING_NUM];

		SNETPC_TYRANNY_F2A_RANKINFO_PC () 
			: wRankNum(0)
		{
			nmg.dwSize = sizeof(NET_MSG_GENERIC) + sizeof(WORD);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_F2A_RANKINFO_PC;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}

		bool ADDRANK ( const TYRANNY_PLAYER_DATA& sRANK )
		{
			if ( TYRANNY_MINI_RANKING_NUM==wRankNum )		return false;

			sPlayerData[wRankNum] = sRANK;

			++wRankNum;

			nmg.dwSize = sizeof(NET_MSG_GENERIC) + sizeof(WORD) + sizeof(TYRANNY_PLAYER_DATA)*wRankNum;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);

			return true;
		}
	};

	struct SNETPC_TYRANNY_A2C_RANKINFO_PC
	{
		NET_MSG_GENERIC		nmg;
		WORD				wLastWinner;
		TYRANNY_PLAYER_DATA	sPlayerData[TYRANNY_MINI_RANKING_NUM];
		SNATIVEID			sRewardBuff[TYRANNY_REWARD_BUFF_NUM];

		SNETPC_TYRANNY_A2C_RANKINFO_PC () 
			: wLastWinner(TYRANNY_SCHOOL_SIZE)
		{
			for( int i=0; i<TYRANNY_REWARD_BUFF_NUM; ++i )
				sRewardBuff[i] = NATIVEID_NULL();

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2C_RANKINFO_PC;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_A2F_BUFF_RESET
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwCharID;

		SNETPC_TYRANNY_A2F_BUFF_RESET ()
			: dwCharID(GAEAID_NULL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2F_BUFF_RESET;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_A2F_BUFF_REWARD
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwCharID;

		TYRANNY_BUFF_DATA	sRewardBuffMain;
		TYRANNY_BUFF_DATA	sRewardBuff[TYRANNY_REWARD_BUFF_NUM];

		SNETPC_TYRANNY_A2F_BUFF_REWARD ()
			: dwCharID(GAEAID_NULL)
		{
			sRewardBuffMain = TYRANNY_BUFF_DATA();

			for( int i=0; i<TYRANNY_REWARD_BUFF_NUM; ++i )
				sRewardBuff[i] = TYRANNY_BUFF_DATA();

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2F_BUFF_REWARD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_TYRANNY_A2F_BUFF_BATTLE
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwCharID;

		TYRANNY_BUFF_DATA	sBattleBuff[TYRANNY_BATTLE_BUFF_NUM];

		SNETPC_TYRANNY_A2F_BUFF_BATTLE ()
			: dwCharID(GAEAID_NULL)
		{
			for( int i=0; i<TYRANNY_BATTLE_BUFF_NUM; ++i )
				sBattleBuff[i] = TYRANNY_BUFF_DATA();

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_TYRANNY_A2F_BUFF_BATTLE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};


	#pragma pack()
};


#pragma once

#include "GLContrlBaseMsg.h"
#include "GLContrlCaptureTheFlagMsgEnum.h"
#include "GLPVPCaptureTheFlagData.h"

namespace GLMSG
{
	#pragma pack(1)

	struct SNET_CAPTURE_THE_FLAG_A2FC_STATE_REGISTER	
	{
		NET_MSG_GENERIC			nmg;
		float		fTime;

		SNET_CAPTURE_THE_FLAG_A2FC_STATE_REGISTER () 
			: fTime(0.0f)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_STATE_REGISTER;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_CAPTURE_THE_FLAG_A2FC_STATE_BATTLE
	{
		NET_MSG_GENERIC			nmg;
		float		fTime;
		WORD		wPlayerNum[CAPTURE_THE_FLAG_TEAM_SIZE];

		SNET_CAPTURE_THE_FLAG_A2FC_STATE_BATTLE () 
			: fTime(0.0f)
		{
			for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
				wPlayerNum[i] = 0;

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_STATE_BATTLE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_CAPTURE_THE_FLAG_A2FC_STATE_REWARD
	{
		NET_MSG_GENERIC			nmg;
		float		fTime;
		WORD		wWinnerTeam;

		SNET_CAPTURE_THE_FLAG_A2FC_STATE_REWARD () 
			: fTime(0.0f)
			, wWinnerTeam(CAPTURE_THE_FLAG_TEAM_SIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_STATE_REWARD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_CAPTURE_THE_FLAG_A2FC_STATE_ENDED
	{
		NET_MSG_GENERIC			nmg;

		SNET_CAPTURE_THE_FLAG_A2FC_STATE_ENDED () 
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_STATE_ENDED;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_CAPTURE_THE_FLAG_A2C_TOBATTLE_TIME
	{
		NET_MSG_GENERIC			nmg;
		WORD					wTime;

		SNET_CAPTURE_THE_FLAG_A2C_TOBATTLE_TIME () 
			: wTime(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2C_TOBATTLE_TIME;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_CAPTURE_THE_FLAG_A2FC_NEXTSCHED
	{
		NET_MSG_GENERIC			nmg;
		CAPTURE_THE_FLAG_SCHED_NEXT		sScheduleNext;

		SNET_CAPTURE_THE_FLAG_A2FC_NEXTSCHED () 
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_NEXTSCHED;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_CAPTURE_THE_FLAG_F2A_BATTLEINFO_PC
	{
		NET_MSG_GENERIC			nmg;
		DWORD		dwCharID;
		BOOL		bEventMap;

		SNET_CAPTURE_THE_FLAG_F2A_BATTLEINFO_PC () 
			: dwCharID(GAEAID_NULL)
			, bEventMap(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2A_BATTLEINFO_PC;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_CAPTURE_THE_FLAG_A2C_BATTLEINFO_PC
	{
		NET_MSG_GENERIC			nmg;
		CAPTURE_THE_FLAG_SCHED_NEXT	sScheduleNext;
		CAPTURE_THE_FLAG_STATE		emState;
		float		fRemain;
		WORD		wLevelReq;
		LONGLONG	llContributionReq;
		WORD		wPlayerLimit;
		WORD		wMaxScore;
		WORD		wBattleTime;
		bool		bRegistered;
		WORD		wTeam;
		WORD		wPlayerNum[CAPTURE_THE_FLAG_TEAM_SIZE];

		SNET_CAPTURE_THE_FLAG_A2C_BATTLEINFO_PC () 
			: emState(CAPTURE_THE_FLAG_STATE_ENDED)
			, fRemain(0.0f)
			, wLevelReq(0)
			, llContributionReq(0)
			, wPlayerLimit(0)
			, wMaxScore(0)
			, wBattleTime(0)
			, bRegistered(false)
			, wTeam(CAPTURE_THE_FLAG_TEAM_SIZE)
		{
			for ( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
			{
				wPlayerNum[i] = 0;
			}

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2C_BATTLEINFO_PC;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_C2A_REGISTER_REQ
	{
		NET_MSG_GENERIC		nmg;
		DWORD	dwCharID;
		WORD	wLevel;
		LONGLONG	llContributionReq;
		BOOL	bRegister;
		WORD	wTeam;

		SNETPC_CAPTURE_THE_FLAG_C2A_REGISTER_REQ () 
			: dwCharID(CAPTURE_THE_FLAG_PLAYER_NULL)
			, wLevel(1)
			, llContributionReq(0)
			, bRegister(FALSE)
			, wTeam(CAPTURE_THE_FLAG_TEAM_SIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_C2A_REGISTER_REQ;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_A2C_REGISTER_FB
	{
		NET_MSG_GENERIC					nmg;
		CAPTURE_THE_FLAG_REGISTER_FB	emFB;
		WORD	wTeam;
		WORD	wPlayerNum[CAPTURE_THE_FLAG_TEAM_SIZE];

		SNETPC_CAPTURE_THE_FLAG_A2C_REGISTER_FB()
			: emFB(CAPTURE_THE_FLAG_REGISTER_FB_FAILED)
			, wTeam(CAPTURE_THE_FLAG_TEAM_SIZE)
		{
			for ( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
				wPlayerNum[i] = 0;

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2C_REGISTER_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNET_CAPTURE_THE_FLAG_A2C_PLAYER_NUM
	{
		NET_MSG_GENERIC			nmg;
		WORD	wPlayerNum[CAPTURE_THE_FLAG_TEAM_SIZE];

		SNET_CAPTURE_THE_FLAG_A2C_PLAYER_NUM () 
		{
			for ( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i )
				wPlayerNum[i] = 0;

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2C_PLAYER_NUM;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_A2F_PLAYER_TEAM
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwCharID;
		WORD				wTeam;

		SNETPC_CAPTURE_THE_FLAG_A2F_PLAYER_TEAM ()
			: dwCharID(CAPTURE_THE_FLAG_PLAYER_NULL)
			, wTeam(CAPTURE_THE_FLAG_TEAM_SIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2F_PLAYER_TEAM;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_F2C_PLAYER_TEAM_BRD : public SNETPC_BROAD
	{
		WORD	wTeam;

		SNETPC_CAPTURE_THE_FLAG_F2C_PLAYER_TEAM_BRD ()
			: wTeam(CAPTURE_THE_FLAG_TEAM_SIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2C_PLAYER_TEAM_BRD;
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_C2A_REJOIN_REQ
	{
		NET_MSG_GENERIC		nmg;
		DWORD	dwCharID;

		SNETPC_CAPTURE_THE_FLAG_C2A_REJOIN_REQ ()
			: dwCharID(CAPTURE_THE_FLAG_PLAYER_NULL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_C2A_REJOIN_REQ;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_A2C_REJOIN_FB
	{
		NET_MSG_GENERIC		nmg;
		CAPTURE_THE_FLAG_REJOIN_FB	emFB;

		SNETPC_CAPTURE_THE_FLAG_A2C_REJOIN_FB()
			: emFB(CAPTURE_THE_FLAG_REJOIN_FB_FAILED)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2C_REJOIN_FB;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_A2F_MAP_MOVE
	{
		NET_MSG_GENERIC		nmg;
		SNATIVEID	nidMAP;
		DWORD		dwCharID;
		D3DXVECTOR3	vPos;
		BOOL		bCurMapCheck;

		SNETPC_CAPTURE_THE_FLAG_A2F_MAP_MOVE () :
			nidMAP(false),
			dwCharID(GAEAID_NULL),
			vPos(0.0f,0.0f,0.0f),
			bCurMapCheck(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2F_MAP_MOVE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_C2A_MAPEXIT_REQ
	{
		NET_MSG_GENERIC		nmg;
		DWORD	dwCharID;

		SNETPC_CAPTURE_THE_FLAG_C2A_MAPEXIT_REQ ()
			: dwCharID(CAPTURE_THE_FLAG_PLAYER_NULL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_C2A_MAPEXIT_REQ;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_A2F_PLAYER_DATA
	{
		NET_MSG_GENERIC		nmg;
		WORD				wRankNum;
		CAPTURE_THE_FLAG_REGISTER_DATA	sPlayerData[CAPTURE_THE_FLAG_PACKET_PLAYER_DATA_NUM];

		SNETPC_CAPTURE_THE_FLAG_A2F_PLAYER_DATA () 
			: wRankNum(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2F_PLAYER_DATA;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}

		BOOL ADDRANK ( const CAPTURE_THE_FLAG_REGISTER_DATA& sRANK )
		{
			if ( wRankNum == CAPTURE_THE_FLAG_PACKET_PLAYER_DATA_NUM )	return FALSE;
			sPlayerData[wRankNum] = sRANK;
			wRankNum++;

			nmg.dwSize = sizeof(*this) - sizeof(CAPTURE_THE_FLAG_REGISTER_DATA) * ( CAPTURE_THE_FLAG_PACKET_PLAYER_DATA_NUM - wRankNum );
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
			return TRUE;
		}

		void Reset()
		{
			wRankNum = 0;
			for( int i=0; i<CAPTURE_THE_FLAG_PACKET_PLAYER_DATA_NUM; ++i ){
				sPlayerData[i] = CAPTURE_THE_FLAG_REGISTER_DATA();
			}
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_C2F_REVIVE_REQ
	{
		NET_MSG_GENERIC		nmg;
		DWORD	dwCharID;

		SNETPC_CAPTURE_THE_FLAG_C2F_REVIVE_REQ ()
			: dwCharID(CAPTURE_THE_FLAG_PLAYER_NULL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_C2F_REVIVE_REQ;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_C2A_SCORE_INFO_REQ
	{
		NET_MSG_GENERIC		nmg;
		DWORD	dwCharID;

		SNETPC_CAPTURE_THE_FLAG_C2A_SCORE_INFO_REQ ()
			: dwCharID(CAPTURE_THE_FLAG_PLAYER_NULL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_C2A_SCORE_INFO_REQ;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_A2C_SCORE_INFO
	{
		NET_MSG_GENERIC		nmg;
		WORD	wLastWinner;
		CAPTURE_THE_FLAG_SCORE_DATA	sScore[CAPTURE_THE_FLAG_TEAM_SIZE];
		BOOL		bFlagHold[CAPTURE_THE_FLAG_TEAM_SIZE];

		SNETPC_CAPTURE_THE_FLAG_A2C_SCORE_INFO ()
			: wLastWinner(CAPTURE_THE_FLAG_TEAM_SIZE)
		{
			for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++ i )
			{
				sScore[i].Reset();
				bFlagHold[i] = FALSE;
			}

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2C_SCORE_INFO;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_F2C_RANKING_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		WORD				wRankNum;
		CAPTURE_THE_FLAG_PLAYER_DATA	sPlayerData[CAPTURE_THE_FLAG_PACKET_RANK_NUM];

		SNETPC_CAPTURE_THE_FLAG_F2C_RANKING_UPDATE () 
			: wRankNum(0)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2C_RANKING_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}

		BOOL ADDRANK ( const CAPTURE_THE_FLAG_PLAYER_DATA& sRANK )
		{
			if ( wRankNum == CAPTURE_THE_FLAG_PACKET_RANK_NUM )	return FALSE;

			sPlayerData[wRankNum] = sRANK;
			wRankNum++;

			nmg.dwSize = sizeof(*this) - sizeof(CAPTURE_THE_FLAG_PLAYER_DATA) * ( CAPTURE_THE_FLAG_PACKET_RANK_NUM - wRankNum );
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
			return TRUE;
		}

		void Reset()
		{
			wRankNum = 0;
			for( int i=0; i<CAPTURE_THE_FLAG_PACKET_RANK_NUM; ++i ){
				sPlayerData[i] = CAPTURE_THE_FLAG_PLAYER_DATA();
			}
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_F2C_RANKING_END
	{
		NET_MSG_GENERIC		nmg;
		WORD		wPlayerNum[CAPTURE_THE_FLAG_TEAM_SIZE];

		SNETPC_CAPTURE_THE_FLAG_F2C_RANKING_END ()
		{
			for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i ){
				wPlayerNum[i] = 0;
			}

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2C_RANKING_END;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_F2A_RANKINFO_PC
	{
		NET_MSG_GENERIC		nmg;
		WORD				wRankNum;
		CAPTURE_THE_FLAG_PLAYER_DATA	sPlayerData[CAPTURE_THE_FLAG_MINI_RANKING_NUM];

		SNETPC_CAPTURE_THE_FLAG_F2A_RANKINFO_PC () 
			: wRankNum(0)
		{
			nmg.dwSize = sizeof(NET_MSG_GENERIC) + sizeof(WORD);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2A_RANKINFO_PC;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}

		bool ADDRANK ( const CAPTURE_THE_FLAG_PLAYER_DATA& sRANK )
		{
			if ( CAPTURE_THE_FLAG_MINI_RANKING_NUM==wRankNum )		return false;

			sPlayerData[wRankNum] = sRANK;

			++wRankNum;

			nmg.dwSize = sizeof(NET_MSG_GENERIC) + sizeof(WORD) + sizeof(CAPTURE_THE_FLAG_PLAYER_DATA)*wRankNum;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);

			return true;
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_A2C_RANKINFO_PC
	{
		NET_MSG_GENERIC		nmg;
		WORD				wLastWinner;
		CAPTURE_THE_FLAG_PLAYER_DATA	sPlayerData[CAPTURE_THE_FLAG_MINI_RANKING_NUM];

		SNETPC_CAPTURE_THE_FLAG_A2C_RANKINFO_PC () 
			: wLastWinner(CAPTURE_THE_FLAG_TEAM_SIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2C_RANKINFO_PC;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_F2A_KILL_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		WORD		wTeamKilled;
		WORD		wTeamKiller;

		SNETPC_CAPTURE_THE_FLAG_F2A_KILL_UPDATE ()
			: wTeamKilled(CAPTURE_THE_FLAG_TEAM_SIZE)
			, wTeamKiller(CAPTURE_THE_FLAG_TEAM_SIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2A_KILL_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_F2A_CAPTURE_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		WORD	wTeam;
	
		SNETPC_CAPTURE_THE_FLAG_F2A_CAPTURE_UPDATE ()
			: wTeam(CAPTURE_THE_FLAG_TEAM_SIZE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2A_CAPTURE_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_A2F_CAPTURE_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		CAPTURE_THE_FLAG_SCORE_DATA	sScore[CAPTURE_THE_FLAG_TEAM_SIZE];
		WORD	wTeam;
	
		SNETPC_CAPTURE_THE_FLAG_A2F_CAPTURE_UPDATE ()
			: wTeam(CAPTURE_THE_FLAG_TEAM_SIZE)
		{
			for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++ i )
			{
				sScore[i].Reset();
			}

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2F_CAPTURE_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_F2C_CAPTURE_UPDATE
	{
		NET_MSG_GENERIC		nmg;
		CAPTURE_THE_FLAG_SCORE_DATA	sScore[CAPTURE_THE_FLAG_TEAM_SIZE];
		WORD	wTeam;
		
		SNETPC_CAPTURE_THE_FLAG_F2C_CAPTURE_UPDATE ()
			: wTeam(CAPTURE_THE_FLAG_TEAM_SIZE)
		{
			for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++ i )
			{
				sScore[i].Reset();
			}

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2C_CAPTURE_UPDATE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_A2F_WINNER
	{
		NET_MSG_GENERIC		nmg;
		WORD	wLastWinner;
		BOOL	bFullCapture;

		SNETPC_CAPTURE_THE_FLAG_A2F_WINNER ()
			: wLastWinner(CAPTURE_THE_FLAG_TEAM_SIZE)
			, bFullCapture(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2F_WINNER;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_F2C_WINNER
	{
		NET_MSG_GENERIC		nmg;
		WORD	wLastWinner;
		BOOL	bFullCapture;

		SNETPC_CAPTURE_THE_FLAG_F2C_WINNER ()
			: wLastWinner(CAPTURE_THE_FLAG_TEAM_SIZE)
			, bFullCapture(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2C_WINNER;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_C2F_FLAG_ACTION_CAPTURE
	{
		NET_MSG_GENERIC		nmg;
		DWORD				dwGlobID;
		DWORD				dwCharID;

		SNETPC_CAPTURE_THE_FLAG_C2F_FLAG_ACTION_CAPTURE ()
			: dwGlobID(CAPTURE_THE_FLAG_PLAYER_NULL)
			, dwCharID(CAPTURE_THE_FLAG_PLAYER_NULL)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_C2F_FLAG_ACTION_CAPTURE;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_F2A_FLAG_HOLD
	{
		NET_MSG_GENERIC		nmg;
		WORD	wTeam;
		BOOL	bHold;
		
		SNETPC_CAPTURE_THE_FLAG_F2A_FLAG_HOLD ()
			: wTeam(CAPTURE_THE_FLAG_TEAM_SIZE)
			, bHold(FALSE)
		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2A_FLAG_HOLD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_F2C_FLAG_HOLD
	{
		NET_MSG_GENERIC		nmg;
		CAPTURE_THE_FLAG_FLAG_HOLD_STATUS	emStatus;
		WORD		wTeam;
		BOOL		bFlagHold[CAPTURE_THE_FLAG_TEAM_SIZE];
		char		szName[CAPTURE_THE_FLAG_CHARNAME_STRING_SIZE];

		SNETPC_CAPTURE_THE_FLAG_F2C_FLAG_HOLD ()
			: emStatus(CAPTURE_THE_FLAG_FLAG_HOLD_STATUS_NONE)
			, wTeam(CAPTURE_THE_FLAG_TEAM_SIZE) 
		{
			for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++ i )
				bFlagHold[i] = FALSE;

			memset (szName, 0, sizeof(char) * CAPTURE_THE_FLAG_CHARNAME_STRING_SIZE);

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2C_FLAG_HOLD;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_F2C_PLAYER_FLAG_HOLD : public SNETPC_BROAD
	{
		bool	bHold;

		SNETPC_CAPTURE_THE_FLAG_F2C_PLAYER_FLAG_HOLD ()
			: bHold(false)		{
			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2C_FLAG_HOLD_BRD;
		}
	};

	struct SNETPC_CAPTURE_THE_FLAG_F2C_FLAG_POSITION
	{
		NET_MSG_GENERIC		nmg;
		STARGETID		sFlagTarget[CAPTURE_THE_FLAG_TEAM_SIZE];

		SNETPC_CAPTURE_THE_FLAG_F2C_FLAG_POSITION ()
		{
			for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++ i )
				sFlagTarget[i].RESET();

			nmg.dwSize = sizeof(*this);
			nmg.nType = NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2C_FLAG_POSITION;
			GASSERT(nmg.dwSize<=NET_DATA_BUFSIZE);
		}
	};

	#pragma pack()
};


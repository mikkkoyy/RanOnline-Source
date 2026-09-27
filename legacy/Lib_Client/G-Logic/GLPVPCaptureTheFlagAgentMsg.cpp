#include "StdAfx.h"
#include "GLPVPCaptureTheFlagAgent.h"
#include "GLAgentServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HRESULT GLPVPCaptureTheFlagAgent::MsgProcess ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID )
{
	if ( !nmg ){
		return E_FAIL;
	}

	switch( nmg->nType )
	{
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2A_BATTLEINFO_PC:
		{
			GLMSG::SNET_CAPTURE_THE_FLAG_F2A_BATTLEINFO_PC* pnetmsg = ( GLMSG::SNET_CAPTURE_THE_FLAG_F2A_BATTLEINFO_PC* ) nmg;
			RequestInfoPC( pnetmsg->dwCharID, pnetmsg->bEventMap );
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_C2A_REGISTER_REQ:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_C2A_REGISTER_REQ* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_C2A_REGISTER_REQ* ) nmg;
			RequestPlayerRegister( pnetmsg->dwCharID, pnetmsg->wLevel, pnetmsg->llContributionReq, pnetmsg->bRegister, pnetmsg->wTeam );
		}break;
	
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_C2A_REJOIN_REQ:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_C2A_REJOIN_REQ* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_C2A_REJOIN_REQ* ) nmg;
			PlayerRejoinBattle( pnetmsg->dwCharID );
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_C2A_MAPEXIT_REQ:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_C2A_MAPEXIT_REQ* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_C2A_MAPEXIT_REQ* ) nmg;
			MapExit( pnetmsg->dwCharID );
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_C2A_SCORE_INFO_REQ:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_C2A_SCORE_INFO_REQ* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_C2A_SCORE_INFO_REQ* ) nmg;
			RequestScoreInfo( pnetmsg->dwCharID );
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2A_KILL_UPDATE:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_F2A_KILL_UPDATE* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_F2A_KILL_UPDATE* ) nmg;

			if ( pnetmsg->wTeamKilled < CAPTURE_THE_FLAG_TEAM_SIZE && pnetmsg->wTeamKiller < CAPTURE_THE_FLAG_TEAM_SIZE )
			{
				m_sScore[pnetmsg->wTeamKilled].wDeaths ++;
				m_sScore[pnetmsg->wTeamKiller].wKills ++;
			}
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2A_CAPTURE_UPDATE:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_F2A_CAPTURE_UPDATE* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_F2A_CAPTURE_UPDATE* ) nmg;

			if ( pnetmsg->wTeam < CAPTURE_THE_FLAG_TEAM_SIZE )
			{
				m_sScore[pnetmsg->wTeam].wCapture ++;
				ScoreCheck();
			
				GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_CAPTURE_UPDATE netmsgfield;
				netmsgfield.wTeam = pnetmsg->wTeam;
				for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i ){
					netmsgfield.sScore[i] = m_sScore[i];
				}
				GLAgentServer::GetInstance().SENDTOCHANNEL( &netmsgfield, 0 );
			}
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2A_RANKINFO_PC:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_F2A_RANKINFO_PC* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_F2A_RANKINFO_PC* ) nmg;

			for( int i=0; i<CAPTURE_THE_FLAG_MINI_RANKING_NUM; ++i )
				m_sLastTop[i] = CAPTURE_THE_FLAG_PLAYER_DATA();

			for( int i=0; i<pnetmsg->wRankNum; ++i )
			{
				if ( pnetmsg->sPlayerData[i].wRankAll > 0 && pnetmsg->sPlayerData[i].wRankAll <= CAPTURE_THE_FLAG_MINI_RANKING_NUM )
				{
					m_sLastTop[pnetmsg->sPlayerData[i].wRankAll-1] = pnetmsg->sPlayerData[i];
				}
			}

			GLMSG::SNETPC_CAPTURE_THE_FLAG_A2C_RANKINFO_PC netmsgclient;
			netmsgclient.wLastWinner = m_wLastWinner;
			for( int i=0; i<CAPTURE_THE_FLAG_MINI_RANKING_NUM; ++i )
				netmsgclient.sPlayerData[i] = m_sLastTop[i];

			GLAgentServer::GetInstance().SENDTOALLCLIENT( &netmsgclient );

		}break;


	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_F2A_FLAG_HOLD:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_F2A_FLAG_HOLD* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_F2A_FLAG_HOLD* ) nmg;

			if ( pnetmsg->wTeam < CAPTURE_THE_FLAG_TEAM_SIZE )
			{
				m_bFlagHold[pnetmsg->wTeam] = pnetmsg->bHold;
			}
		}break;

	};

	return S_OK;
}

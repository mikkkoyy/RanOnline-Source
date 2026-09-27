#include "StdAfx.h"
#include "GLPVPSchoolWarsAgent.h"
#include "GLAgentServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HRESULT GLPVPSchoolWarsAgent::MsgProcess ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID )
{
	if ( !nmg ){
		return E_FAIL;
	}

	switch( nmg->nType )
	{
	case NET_MSG_GCTRL_SCHOOLWARS_F2A_BATTLEINFO_PC:
		{
			GLMSG::SNET_SCHOOLWARS_F2A_BATTLEINFO_PC* pnetmsg = ( GLMSG::SNET_SCHOOLWARS_F2A_BATTLEINFO_PC* ) nmg;
			RequestInfoPC( pnetmsg->dwCharID );
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_C2A_REGISTER_REQ:
		{
			GLMSG::SNETPC_SCHOOLWARS_C2A_REGISTER_REQ* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_C2A_REGISTER_REQ* ) nmg;
			RequestPlayerRegister( pnetmsg->dwCharID, pnetmsg->wLevel, pnetmsg->llContri, pnetmsg->bRegister );
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_C2A_REJOIN_REQ:
		{
			GLMSG::SNETPC_SCHOOLWARS_C2A_REJOIN_REQ* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_C2A_REJOIN_REQ* ) nmg;
			PlayerRejoinBattle( pnetmsg->dwCharID );
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_C2A_MAPEXIT_REQ:
		{
			GLMSG::SNETPC_SCHOOLWARS_C2A_MAPEXIT_REQ* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_C2A_MAPEXIT_REQ* ) nmg;
			MapExit( pnetmsg->dwCharID );
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_C2A_SCORE_INFO_REQ:
		{
			GLMSG::SNETPC_SCHOOLWARS_C2A_SCORE_INFO_REQ* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_C2A_SCORE_INFO_REQ* ) nmg;
			RequestScoreInfo( pnetmsg->dwCharID );
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_F2A_SCORE_UPDATE:
		{
			GLMSG::SNETPC_SCHOOLWARS_F2A_SCORE_UPDATE* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_F2A_SCORE_UPDATE* ) nmg;

			if ( pnetmsg->wSchoolKilled < SCHOOLWARS_SCHOOL_SIZE && pnetmsg->wSchoolKiller < SCHOOLWARS_SCHOOL_SIZE )
			{
				WORD wPoint = 1;
				if ( m_bDoublePoint )	
					wPoint = 2;

				m_sScore[pnetmsg->wSchoolKilled].wDeaths += wPoint;
				m_sScore[pnetmsg->wSchoolKiller].wKills += wPoint;

				ScoreCheck();

				PGLCHARAG pchar = GLAgentServer::GetInstance().GetCharID( pnetmsg->dwCharID );
				if ( pchar )
				{
					GLMSG::SNETPC_SCHOOLWARS_A2C_SCORE_UPDATE netmsgclient;
					for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++i )
						netmsgclient.sScore[i] = m_sScore[i];
					GLAgentServer::GetInstance().SENDTOCLIENT( pchar->m_dwClientID, &netmsgclient);
				}
			}
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_F2A_RANKINFO_PC:
		{
			GLMSG::SNETPC_SCHOOLWARS_F2A_RANKINFO_PC* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_F2A_RANKINFO_PC* ) nmg;

			for( int i=0; i<SCHOOLWARS_MINI_RANKING_NUM; ++i )
				m_sLastTop[i] = SCHOOLWARS_PLAYER_DATA();

			for( int i=0; i<pnetmsg->wRankNum; ++i )
			{
				if ( pnetmsg->sPlayerData[i].wRankAll > 0 && pnetmsg->sPlayerData[i].wRankAll <= SCHOOLWARS_MINI_RANKING_NUM )
				{
					m_sLastTop[pnetmsg->sPlayerData[i].wRankAll-1] = pnetmsg->sPlayerData[i];
				}
			}

			GLMSG::SNETPC_SCHOOLWARS_A2C_RANKINFO_PC netmsgclient;
			netmsgclient.wLastWinner = m_wLastWinner;
			for( int i=0; i<SCHOOLWARS_MINI_RANKING_NUM; ++i )
				netmsgclient.sPlayerData[i] = m_sLastTop[i];

			GLAgentServer::GetInstance().SENDTOALLCLIENT( &netmsgclient );

		}break;
	};

	return S_OK;
}

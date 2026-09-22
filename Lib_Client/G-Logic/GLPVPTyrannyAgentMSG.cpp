#include "StdAfx.h"
#include "GLPVPTyrannyAgent.h"
#include "GLAgentServer.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HRESULT GLPVPTyrannyAgent::MsgProcess ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID )
{
	if ( !nmg ){
		return E_FAIL;
	}

	switch( nmg->nType )
	{
	case NET_MSG_GCTRL_TYRANNY_F2A_BATTLEINFO_PC:
		{
			/*global buffs map setting, Juver, 2018/01/23 */
			GLMSG::SNET_TYRANNY_F2A_BATTLEINFO_PC* pnetmsg = ( GLMSG::SNET_TYRANNY_F2A_BATTLEINFO_PC* ) nmg;
			RequestInfoPC( pnetmsg->dwCharID, pnetmsg->bTyrannyMap, pnetmsg->bDisableSystemBuffs );
		}break;

	case NET_MSG_GCTRL_TYRANNY_F2A_RANKINFO_PC:
		{
			GLMSG::SNETPC_TYRANNY_F2A_RANKINFO_PC* pnetmsg = ( GLMSG::SNETPC_TYRANNY_F2A_RANKINFO_PC* ) nmg;

			for( int i=0; i<TYRANNY_MINI_RANKING_NUM; ++i )
				m_sLastTop[i] = TYRANNY_PLAYER_DATA();

			for( int i=0; i<pnetmsg->wRankNum; ++i )
			{
				if ( pnetmsg->sPlayerData[i].wRankAll > 0 && pnetmsg->sPlayerData[i].wRankAll <= TYRANNY_MINI_RANKING_NUM )
				{
					m_sLastTop[pnetmsg->sPlayerData[i].wRankAll-1] = pnetmsg->sPlayerData[i];
				}
			}

			GLMSG::SNETPC_TYRANNY_A2C_RANKINFO_PC netmsgclient;
			netmsgclient.wLastWinner = m_wLastWinner;
			for( int i=0; i<TYRANNY_MINI_RANKING_NUM; ++i )
				netmsgclient.sPlayerData[i] = m_sLastTop[i];

			for( int i=0; i<TYRANNY_REWARD_BUFF_NUM; ++i )
				netmsgclient.sRewardBuff[i] = m_sRewardBuff[i].sidSkill;

			GLAgentServer::GetInstance().SENDTOALLCLIENT( &netmsgclient );

		}break;

	case NET_MSG_GCTRL_TYRANNY_C2A_REGISTER_REQ:
		{
			GLMSG::SNETPC_TYRANNY_C2A_REGISTER_REQ* pnetmsg = ( GLMSG::SNETPC_TYRANNY_C2A_REGISTER_REQ* ) nmg;
			RequestPlayerRegister( pnetmsg->dwCharID, pnetmsg->wLevel, pnetmsg->bRegister );
		}break;

	case NET_MSG_GCTRL_TYRANNY_C2A_REJOIN_REQ:
		{
			GLMSG::SNETPC_TYRANNY_C2A_REJOIN_REQ* pnetmsg = ( GLMSG::SNETPC_TYRANNY_C2A_REJOIN_REQ* ) nmg;
			PlayerRejoinBattle( pnetmsg->dwCharID );
		}break;

	case NET_MSG_GCTRL_TYRANNY_C2A_MAPEXIT_REQ:
		{
			GLMSG::SNETPC_TYRANNY_C2A_MAPEXIT_REQ* pnetmsg = ( GLMSG::SNETPC_TYRANNY_C2A_MAPEXIT_REQ* ) nmg;
			MapExit( pnetmsg->dwCharID );
		}break;

	case NET_MSG_GCTRL_TYRANNY_F2A_TOWER_CAPTURE:
		{	
			GLMSG::SNETPC_TYRANNY_F2A_TOWER_CAPTURE* pnetmsg = ( GLMSG::SNETPC_TYRANNY_F2A_TOWER_CAPTURE* ) nmg;
			TowerCapture( pnetmsg->sidMob, pnetmsg->wSchool );
		}break;

	case NET_MSG_GCTRL_TYRANNY_C2A_TOWER_INFO_REQ:
		{
			GLMSG::SNETPC_TYRANNY_C2A_TOWER_OWNER_INFO_REQ* pnetmsg = ( GLMSG::SNETPC_TYRANNY_C2A_TOWER_OWNER_INFO_REQ* ) nmg;
			RequestTowerInfo( pnetmsg->dwCharID );
		}break;
	
	case NET_MSG_GCTRL_TYRANNY_F2A_TOWER_SCORE:
		{
			GLMSG::SNETPC_TYRANNY_F2A_TOWER_SCORE* pnetmsg = ( GLMSG::SNETPC_TYRANNY_F2A_TOWER_SCORE* ) nmg;

			if ( pnetmsg->wSchoolKilled < TYRANNY_SCHOOL_SIZE && pnetmsg->wSchoolKiller < TYRANNY_SCHOOL_SIZE )
			{
				m_sTowerScore[pnetmsg->wSchoolKilled].wDeaths ++;
				m_sTowerScore[pnetmsg->wSchoolKiller].wKills ++;

				//CDebugSet::ToFileWithTime( "_pvptyrannyscore.txt", "school:[%u]add death", pnetmsg->wSchoolKilled );
				//CDebugSet::ToFileWithTime( "_pvptyrannyscore.txt", "school:[%u]add kill", pnetmsg->wSchoolKiller );
			}
		}break;
	};

	return S_OK;
}

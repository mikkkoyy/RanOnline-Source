#include "StdAfx.h"
#include "GLPVPSchoolWarsField.h"
#include "GLGaeaServer.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HRESULT GLPVPSchoolWarsField::MsgProcess ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID )
{
	if ( !nmg ){
		return E_FAIL;
	}

	switch ( nmg->nType )
	{
	case NET_MSG_GCTRL_SCHOOLWARS_A2FC_STATE_REGISTER:
		{
			GLMSG::SNET_SCHOOLWARS_A2FC_STATE_REGISTER* pnetmsg = ( GLMSG::SNET_SCHOOLWARS_A2FC_STATE_REGISTER* ) nmg;
			DoStateRegister();
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2FC_STATE_BATTLE:
		{
			GLMSG::SNET_SCHOOLWARS_A2FC_STATE_BATTLE* pnetmsg = ( GLMSG::SNET_SCHOOLWARS_A2FC_STATE_BATTLE* ) nmg;
			DoStateBattle();

			if( pnetmsg->wPlayerNum[SCHOOLWARS_SCHOOL_SG] != (WORD)m_mapPlayerDataSG.size() ||
				pnetmsg->wPlayerNum[SCHOOLWARS_SCHOOL_MP] != (WORD)m_mapPlayerDataMP.size() ||
				 pnetmsg->wPlayerNum[SCHOOLWARS_SCHOOL_PHX] != (WORD)m_mapPlayerDataPHX.size() )
			{
				CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "GLPVPSchoolWarsField Receive Player size error Agent:%d~%d~%d Field:%d~%d~%d", 
					 pnetmsg->wPlayerNum[SCHOOLWARS_SCHOOL_SG], pnetmsg->wPlayerNum[SCHOOLWARS_SCHOOL_MP], pnetmsg->wPlayerNum[SCHOOLWARS_SCHOOL_PHX],
					 (WORD)m_mapPlayerDataSG.size(), (WORD)m_mapPlayerDataMP.size(), (WORD)m_mapPlayerDataPHX.size() );
			}
				
			CDebugSet::ToFileWithTime( "_pvpschoolwars.txt", "GLPVPSchoolWarsField Receive Player Data Num:%d~%d~%d", (WORD)m_mapPlayerDataSG.size(), (WORD)m_mapPlayerDataMP.size(), (WORD)m_mapPlayerDataPHX.size() );

		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2FC_STATE_REWARD:
		{
			GLMSG::SNET_SCHOOLWARS_A2FC_STATE_REWARD* pnetmsg = ( GLMSG::SNET_SCHOOLWARS_A2FC_STATE_REWARD* ) nmg;
			DoStateReward( pnetmsg->wWinnerSchool );
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2FC_STATE_ENDED:
		{
			GLMSG::SNET_SCHOOLWARS_A2FC_STATE_ENDED* pnetmsg = ( GLMSG::SNET_SCHOOLWARS_A2FC_STATE_ENDED* ) nmg;
			DoStateEnd();
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2FC_NEXTSCHED:
		{
			GLMSG::SNET_SCHOOLWARS_A2FC_NEXTSCHED* pnetmsg = ( GLMSG::SNET_SCHOOLWARS_A2FC_NEXTSCHED* ) nmg;
			m_sScheduleNext = pnetmsg->sScheduleNext;
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2F_PLAYER_DATA:
		{
			GLMSG::SNETPC_SCHOOLWARS_A2F_PLAYER_DATA* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_A2F_PLAYER_DATA* ) nmg;

			for( int i=0; i<pnetmsg->wRankNum; ++i )
				InsertPlayerData( &pnetmsg->sPlayerData[i] );
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2F_SCORE_UPDATE:
		{
			GLMSG::SNETPC_SCHOOLWARS_A2F_SCORE_UPDATE* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_A2F_SCORE_UPDATE* ) nmg;

			GLMSG::SNETPC_SCHOOLWARS_F2C_SCORE_UPDATE netmsgclient;
			for( int i=0; i<SCHOOLWARS_SCHOOL_SIZE; ++i )
				netmsgclient.sScore[i] = pnetmsg->sScore[i];
			GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMap.dwID, &netmsgclient );
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_C2F_REVIVE_REQ:
		{
			GLMSG::SNETPC_SCHOOLWARS_C2F_REVIVE_REQ* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_C2F_REVIVE_REQ* ) nmg;
			PlayerRevive( dwGaeaID, pnetmsg->dwCharID );
		}break;

	case NET_MSG_GCTRL_SCHOOLWARS_A2F_SCORE_WINNER:
		{
			GLMSG::SNETPC_SCHOOLWARS_A2F_SCORE_WINNER* pnetmsg = ( GLMSG::SNETPC_SCHOOLWARS_A2F_SCORE_WINNER* ) nmg;

			GLMSG::SNETPC_SCHOOLWARS_F2C_SCORE_WINNER netmsgclient;
			netmsgclient.wLastWinner = pnetmsg->wLastWinner;
			netmsgclient.bFullScore = pnetmsg->bFullScore;
			GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMap.dwID, &netmsgclient );
		}break;

	};

	return S_OK;
}
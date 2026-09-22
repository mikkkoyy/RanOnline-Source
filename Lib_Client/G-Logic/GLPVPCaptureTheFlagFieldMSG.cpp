#include "StdAfx.h"
#include "GLPVPCaptureTheFlagField.h"
#include "GLGaeaServer.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HRESULT GLPVPCaptureTheFlagField::MsgProcess ( NET_MSG_GENERIC* nmg, DWORD dwClientID, DWORD dwGaeaID )
{
	if ( !nmg ){
		return E_FAIL;
	}

	switch ( nmg->nType )
	{
	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_STATE_REGISTER:
		{
			GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_REGISTER* pnetmsg = ( GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_REGISTER* ) nmg;
			DoStateRegister();
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_STATE_BATTLE:
		{
			GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_BATTLE* pnetmsg = ( GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_BATTLE* ) nmg;
			DoStateBattle();

			if( pnetmsg->wPlayerNum[CAPTURE_THE_FLAG_TEAM_A] != (WORD)m_mapPlayerDataA.size() ||
				pnetmsg->wPlayerNum[CAPTURE_THE_FLAG_TEAM_B] != (WORD)m_mapPlayerDataB.size()  )
			{
				CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagField Receive Player size error Agent:%d~%d Field:%d~%d", 
					pnetmsg->wPlayerNum[CAPTURE_THE_FLAG_TEAM_A], pnetmsg->wPlayerNum[CAPTURE_THE_FLAG_TEAM_B],
					(WORD)m_mapPlayerDataA.size(), (WORD)m_mapPlayerDataB.size() );
			}

			CDebugSet::ToFileWithTime( "_pvpcapturetheflag.txt", "GLPVPCaptureTheFlagField Receive Player Data Num:%d~%d", (WORD)m_mapPlayerDataA.size(), (WORD)m_mapPlayerDataB.size() );

		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_STATE_REWARD:
		{
			GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_REWARD* pnetmsg = ( GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_REWARD* ) nmg;
			DoStateReward( pnetmsg->wWinnerTeam );
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_STATE_ENDED:
		{
			GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_ENDED* pnetmsg = ( GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_STATE_ENDED* ) nmg;
			DoStateEnd();
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2FC_NEXTSCHED:
		{
			GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_NEXTSCHED* pnetmsg = ( GLMSG::SNET_CAPTURE_THE_FLAG_A2FC_NEXTSCHED* ) nmg;
			m_sScheduleNext = pnetmsg->sScheduleNext;
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2F_PLAYER_DATA:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_PLAYER_DATA* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_PLAYER_DATA* ) nmg;

			for( int i=0; i<pnetmsg->wRankNum; ++i )
				InsertPlayerData( &pnetmsg->sPlayerData[i] );
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2F_CAPTURE_UPDATE:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_CAPTURE_UPDATE* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_CAPTURE_UPDATE* ) nmg;

			GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_CAPTURE_UPDATE netmsgclient;
			netmsgclient.wTeam = pnetmsg->wTeam;
			for( int i=0; i<CAPTURE_THE_FLAG_TEAM_SIZE; ++i ){
				netmsgclient.sScore[i] = pnetmsg->sScore[i];
			}

			GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMap.dwID, &netmsgclient );
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_C2F_REVIVE_REQ:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_C2F_REVIVE_REQ* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_C2F_REVIVE_REQ* ) nmg;
			PlayerRevive( dwGaeaID, pnetmsg->dwCharID );
		}break;

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_A2F_WINNER:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_WINNER* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_A2F_WINNER* ) nmg;

			GLMSG::SNETPC_CAPTURE_THE_FLAG_F2C_WINNER netmsgclient;
			netmsgclient.wLastWinner = pnetmsg->wLastWinner;
			netmsgclient.bFullCapture = pnetmsg->bFullCapture;
			GLGaeaServer::GetInstance().SENDTOCLIENT_ONMAP( m_sMap.dwID, &netmsgclient );
		}break;
	

	case NET_MSG_GCTRL_CAPTURE_THE_FLAG_C2F_FLAG_ACTION_CAPTURE:
		{
			GLMSG::SNETPC_CAPTURE_THE_FLAG_C2F_FLAG_ACTION_CAPTURE* pnetmsg = ( GLMSG::SNETPC_CAPTURE_THE_FLAG_C2F_FLAG_ACTION_CAPTURE* ) nmg;
			FlagToHold( pnetmsg->dwCharID, pnetmsg->dwGlobID );
		}break;

	


	


	};

	return S_OK;
}